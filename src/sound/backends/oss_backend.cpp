/*
Abstract:
  Oss backend implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "backend_impl.h"
#include "enumerator.h"
//common includes
#include <byteorder.h>
#include <debug_log.h>
#include <error_tools.h>
#include <tools.h>
//library includes
#include <io/fs_tools.h>
#include <l10n/api.h>
#include <sound/backend_attrs.h>
#include <sound/backends_parameters.h>
#include <sound/error_codes.h>
#include <sound/render_params.h>
#include <sound/sound_parameters.h>
//platform-specific includes
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
//std includes
#include <algorithm>
//boost includes
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
//text includes
#include <sound/text/backends.h>

#define FILE_TAG 69200152

namespace
{
  using namespace ZXTune;
  using namespace ZXTune::Sound;

  const Debug::Stream Dbg("Sound::Backend::Oss");
  const L10n::TranslateFunctor translate = L10n::TranslateFunctor("sound");

  const uint_t MAX_OSS_VOLUME = 100;

  const uint_t CAPABILITIES = CAP_TYPE_SYSTEM | CAP_FEAT_HWVOLUME;
  
  class AutoDescriptor : public boost::noncopyable
  {
  public:
    AutoDescriptor()
      : Handle(-1)
    {
    }
    AutoDescriptor(const String& name, int mode)
      : Name(name)
      , Handle(::open(IO::ConvertToFilename(name).c_str(), mode, 0))
    {
      CheckResult(-1 != Handle, THIS_LINE);
      Dbg("Opened device '%1%'", Name);
    }

    ~AutoDescriptor()
    {
      try
      {
        Close();
      }
      catch (const Error&)
      {
      }
    }

    void CheckResult(bool res, Error::LocationRef loc) const
    {
      if (!res)
      {
        throw MakeFormattedError(loc, BACKEND_PLATFORM_ERROR,
          translate("Error in OSS backend while working with device '%1%': %2%."), Name, ::strerror(errno));
      }
    }

    void Swap(AutoDescriptor& rh)
    {
      std::swap(rh.Handle, Handle);
      std::swap(rh.Name, Name);
    }

    void Close()
    {
      if (-1 != Handle)
      {
        Dbg("Close device '%1%'", Name);
        int tmpHandle = -1;
        std::swap(Handle, tmpHandle);
        Name.clear();
        CheckResult(0 == ::close(tmpHandle), THIS_LINE);
      }
    }

    int Get() const
    {
      return Handle;
    }
  private:
    String Name;
    //leave handle as int
    int Handle;
  };

  class SoundFormat
  {
  public:
    explicit SoundFormat(int supportedFormats)
      : Native(GetSoundFormat(SAMPLE_SIGNED))
      , Negated(GetSoundFormat(!SAMPLE_SIGNED))
      , NativeSupported(0 != (supportedFormats & Native))
      , NegatedSupported(0 != (supportedFormats & Negated))
    {
    }
    
    bool IsSupported() const
    {
      return NativeSupported || NegatedSupported;
    }
    
    bool ChangeSign() const
    {
      return !NativeSupported && NegatedSupported;
    }
    
    int Get() const
    {
      return NativeSupported
        ? Native
        : NegatedSupported ? Negated : -1;
    }
  private:
    static int GetSoundFormat(bool isSigned)
    {
      switch (sizeof(Sample))
      {
      case 1:
        return isSigned ? AFMT_S8 : AFMT_U8;
      case 2:
        return isSigned
          ? (isLE() ? AFMT_S16_LE : AFMT_S16_BE)
          : (isLE() ? AFMT_U16_LE : AFMT_U16_BE);
      default:
        assert(!"Invalid format");
        return -1;
      };
    }
  private:
    const int Native;
    const int Negated;
    const bool NativeSupported;
    const bool NegatedSupported;
  };


  class OssVolumeControl : public VolumeControl
  {
  public:
    OssVolumeControl(boost::mutex& stateMutex, AutoDescriptor& mixer)
      : StateMutex(stateMutex), MixHandle(mixer)
    {
    }

    virtual Error GetVolume(MultiGain& volume) const
    {
      try
      {
        Dbg("GetVolume");
        boost::mutex::scoped_lock lock(StateMutex);
        if (-1 != MixHandle.Get())
        {
          boost::array<uint8_t, sizeof(int)> buf;
          MixHandle.CheckResult(-1 != ::ioctl(MixHandle.Get(), SOUND_MIXER_READ_VOLUME,
            safe_ptr_cast<int*>(&buf[0])), THIS_LINE);
          std::transform(buf.begin(), buf.begin() + OUTPUT_CHANNELS, volume.begin(),
            std::bind2nd(std::divides<Gain>(), MAX_OSS_VOLUME));
        }
        else
        {
          volume = MultiGain();
        }
        return Error();
      }
      catch (const Error& e)
      {
        return e;
      }
    }

    virtual Error SetVolume(const MultiGain& volume)
    {
      if (volume.end() != std::find_if(volume.begin(), volume.end(), std::bind2nd(std::greater<Gain>(), Gain(1.0))))
      {
        return Error(THIS_LINE, BACKEND_INVALID_PARAMETER, translate("Failed to set volume: gain is out of range."));
      }
      try
      {
        Dbg("SetVolume");
        boost::mutex::scoped_lock lock(StateMutex);
        if (-1 != MixHandle.Get())
        {
          boost::array<uint8_t, sizeof(int)> buf = { {0} };
          std::transform(volume.begin(), volume.end(), buf.begin(),
            std::bind2nd(std::multiplies<Gain>(), MAX_OSS_VOLUME));
          MixHandle.CheckResult(-1 != ::ioctl(MixHandle.Get(), SOUND_MIXER_WRITE_VOLUME,
            safe_ptr_cast<int*>(&buf[0])), THIS_LINE);
        }
        return Error();
      }
      catch (const Error& e)
      {
        return e;
      }
    }
  private:
    boost::mutex& StateMutex;
    AutoDescriptor& MixHandle;
  };

  class OssBackendParameters
  {
  public:
    explicit OssBackendParameters(const Parameters::Accessor& accessor)
      : Accessor(accessor)
    {
    }

    String GetDeviceName() const
    {
      Parameters::StringType strVal = Parameters::ZXTune::Sound::Backends::Oss::DEVICE_DEFAULT;
      Accessor.FindValue(Parameters::ZXTune::Sound::Backends::Oss::DEVICE, strVal);
      return strVal;
    }

    String GetMixerName() const
    {
      Parameters::StringType strVal = Parameters::ZXTune::Sound::Backends::Oss::MIXER_DEFAULT;
      Accessor.FindValue(Parameters::ZXTune::Sound::Backends::Oss::MIXER, strVal);
      return strVal;
    }
  private:
    const Parameters::Accessor& Accessor;
  };

  class OssBackendWorker : public BackendWorker
                         , private boost::noncopyable
  {
  public:
    explicit OssBackendWorker(Parameters::Accessor::Ptr params)
      : BackendParams(params)
      , RenderingParameters(RenderParameters::Create(BackendParams))
      , ChangeSign(false)
      , CurrentBuffer(Buffers.begin(), Buffers.end())
      , VolumeController(new OssVolumeControl(StateMutex, MixHandle))
    {
    }

    virtual ~OssBackendWorker()
    {
      assert(-1 == DevHandle.Get() || !"OssBackend should be stopped before destruction.");
    }

    virtual void Test()
    {
      AutoDescriptor tmpMixer;
      AutoDescriptor tmpDevice;
      bool tmpSign = false;
      SetupDevices(tmpDevice, tmpMixer, tmpSign);
      Dbg("Tested!");
    }

    virtual VolumeControl::Ptr GetVolumeControl() const
    {
      return VolumeController;
    }

    virtual void OnStartup(const Module::Holder& /*module*/)
    {
      assert(-1 == MixHandle.Get() && -1 == DevHandle.Get());
      SetupDevices(DevHandle, MixHandle, ChangeSign);
      Dbg("Successfully opened");
    }

    virtual void OnShutdown()
    {
      DevHandle.Close();
      MixHandle.Close();
      Dbg("Successfully closed");
    }

    virtual void OnPause()
    {
    }

    virtual void OnResume()
    {
    }

    virtual void OnFrame(const Module::TrackState& /*state*/)
    {
    }

    virtual void OnBufferReady(Chunk& buffer)
    {
      if (ChangeSign)
      {
        std::transform(buffer.front().begin(), buffer.back().end(), buffer.front().begin(), &ToSignedSample);
      }
      Chunk& buf(*CurrentBuffer);
      buf.swap(buffer);
      assert(-1 != DevHandle.Get());
      std::size_t toWrite(buf.size() * sizeof(buf.front()));
      const uint8_t* data(safe_ptr_cast<const uint8_t*>(&buf[0]));
      while (toWrite)
      {
        const int res = ::write(DevHandle.Get(), data, toWrite * sizeof(*data));
        DevHandle.CheckResult(res >= 0, THIS_LINE);
        toWrite -= res;
        data += res;
      }
      ++CurrentBuffer;
    }
  private:
    void SetupDevices(AutoDescriptor& device, AutoDescriptor& mixer, bool& changeSign) const
    {
      const OssBackendParameters params(*BackendParams);

      AutoDescriptor tmpMixer(params.GetMixerName(), O_RDWR);
      AutoDescriptor tmpDevice(params.GetDeviceName(), O_WRONLY);
      BOOST_STATIC_ASSERT(1 == sizeof(Sample) || 2 == sizeof(Sample));
      int tmp = 0;
      tmpDevice.CheckResult(-1 != ::ioctl(tmpDevice.Get(), SNDCTL_DSP_GETFMTS, &tmp), THIS_LINE);
      Dbg("Supported formats %1%", tmp);
      const SoundFormat format(tmp);
      if (!format.IsSupported())
      {
        throw Error(THIS_LINE, BACKEND_SETUP_ERROR, translate("No suitable formats supported by OSS."));
      }
      tmp = format.Get();
      Dbg("Setting format to %1%", tmp);
      tmpDevice.CheckResult(-1 != ::ioctl(tmpDevice.Get(), SNDCTL_DSP_SETFMT, &tmp), THIS_LINE);

      tmp = OUTPUT_CHANNELS;
      Dbg("Setting channels to %1%", tmp);
      tmpDevice.CheckResult(-1 != ::ioctl(tmpDevice.Get(), SNDCTL_DSP_CHANNELS, &tmp), THIS_LINE);

      tmp = RenderingParameters->SoundFreq();
      Dbg("Setting frequency to %1%", tmp);
      tmpDevice.CheckResult(-1 != ::ioctl(tmpDevice.Get(), SNDCTL_DSP_SPEED, &tmp), THIS_LINE);

      device.Swap(tmpDevice);
      mixer.Swap(tmpMixer);
      changeSign = format.ChangeSign();
    }
  private:
    const Parameters::Accessor::Ptr BackendParams;
    const RenderParameters::Ptr RenderingParameters;
    boost::mutex StateMutex;
    AutoDescriptor MixHandle;
    AutoDescriptor DevHandle;
    bool ChangeSign;
    boost::array<Chunk, 2> Buffers;
    CycledIterator<Chunk*> CurrentBuffer;
    const VolumeControl::Ptr VolumeController;
  };

  const String ID = Text::OSS_BACKEND_ID;
  const char* const DESCRIPTION = L10n::translate("OSS sound system backend");

  class OssBackendCreator : public BackendCreator
  {
  public:
    virtual String Id() const
    {
      return ID;
    }

    virtual String Description() const
    {
      return translate(DESCRIPTION);
    }

    virtual uint_t Capabilities() const
    {
      return CAPABILITIES;
    }

    virtual Error Status() const
    {
      return Error();
    }

    virtual Error CreateBackend(CreateBackendParameters::Ptr params, Backend::Ptr& result) const
    {
      try
      {
        const Parameters::Accessor::Ptr allParams = params->GetParameters();
        const BackendWorker::Ptr worker(new OssBackendWorker(allParams));
        result = Sound::CreateBackend(params, worker);
        return Error();
      }
      catch (const Error& e)
      {
        return MakeFormattedError(THIS_LINE, BACKEND_FAILED_CREATE,
          translate("Failed to create backend '%1%'."), Id()).AddSuberror(e);
      }
    }
  };
}

namespace ZXTune
{
  namespace Sound
  {
    void RegisterOssBackend(BackendsEnumerator& enumerator)
    {
      const BackendCreator::Ptr creator(new OssBackendCreator());
      enumerator.RegisterCreator(creator);
    }
  }
}
