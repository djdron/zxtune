/**
* 
* @file
*
* @brief  SPC support plugin
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "core/plugins/player_plugins_registrator.h"
#include "core/plugins/players/analyzer.h"
#include "core/plugins/players/duration.h"
#include "core/plugins/players/plugin.h"
#include <core/plugins/players/properties_helper.h>
#include "core/plugins/players/streaming.h"
//common includes
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <core/module_attrs.h>
#include <core/plugin_attrs.h>
#include <core/plugins_parameters.h>
#include <debug/log.h>
#include <devices/details/analysis_map.h>
#include <formats/chiptune/emulation/spc.h>
#include <math/numeric.h>
#include <parameters/tracking_helper.h>
#include <sound/chunk_builder.h>
#include <sound/render_params.h>
#include <sound/resampler.h>
#include <sound/sound_parameters.h>
//3rdparty
#include <3rdparty/snesspc/snes_spc/SNES_SPC.h>
#include <3rdparty/snesspc/snes_spc/SPC_Filter.h>

namespace Module
{
namespace SPC
{
  const Debug::Stream Dbg("Core::SIDSupp");

  class SPC : public Module::Analyzer
  {
    static const uint_t SPC_DIVIDER = 1 << 12;
    static const uint_t C_7_FREQ = 2093;
  public:
    typedef boost::shared_ptr<SPC> Ptr;
    
    explicit SPC(const Binary::Data& data)
      : Data(static_cast<const uint8_t*>(data.Start()), static_cast<const uint8_t*>(data.Start()) + data.Size())
    {
      CheckError(Spc.init());
      // #0040 is C-1 (32Hz) - min
      // #0080 is C-2
      // #0100 is C-3
      // #0200 is C-4
      // #0400 is C-5
      // #0800 is C-6
      // #1000 is C-7 (2093Hz) - normal 32kHz
      // #2000 is C-8 (4186Hz)
      // #3fff is B-8 (7902Hz) - max
      const uint_t DIVIDER = ::SNES_SPC::sample_rate * SPC_DIVIDER / C_7_FREQ;
      Analysis.SetClockAndDivisor(::SNES_SPC::sample_rate, DIVIDER);
      Reset();
    }
    
    void Reset()
    {
      Spc.reset();
      CheckError(Spc.load_spc(&Data.front(), Data.size()));
      Spc.clear_echo();
      Filter.clear();
    }
    
    void Render(uint_t samples, Sound::ChunkBuilder& target)
    {
      BOOST_STATIC_ASSERT(Sound::Sample::CHANNELS == 2);
      BOOST_STATIC_ASSERT(Sound::Sample::BITS == 16);
      ::SNES_SPC::sample_t* const buffer = safe_ptr_cast< ::SNES_SPC::sample_t*>(target.Allocate(samples));
      CheckError(Spc.play(samples * Sound::Sample::CHANNELS, buffer));
      Filter.run(buffer, samples * Sound::Sample::CHANNELS);
    }
    
    void Skip(uint_t samples)
    {
      CheckError(Spc.skip(samples * Sound::Sample::CHANNELS));
    }
    
    //http://wiki.superfamicom.org/snes/show/SPC700+Reference
    virtual void GetState(std::vector<ChannelState>& channels) const
    {
      const DspProperties dsp(Spc);
      const uint_t noise = dsp.GetNoiseChannels();
      const uint_t active = dsp.GetToneChannels() | noise;
      std::vector<ChannelState> result;
      const uint_t noisePitch = noise != 0
        ? dsp.GetNoisePitch()
        : 0;
      for (uint_t chan = 0; active != 0 && chan != 8; ++chan)
      {
        const uint_t mask = (1 << chan);
        if (0 == (active & mask))
        {
          continue;
        }
        const uint_t levelInt = dsp.GetLevel(chan);
        const bool isNoise = 0 != (noise & mask);
        const uint_t pitch = isNoise
          ? noisePitch 
          : dsp.GetPitch(chan);
        ChannelState state;
        state.Level = levelInt * 100 / 127;
        state.Band = Analysis.GetBandByScaledFrequency(pitch);
        result.push_back(state);
      }
      channels.swap(result);
    }
  private:
    inline static void CheckError(::blargg_err_t err)
    {
      Require(!err);//TODO: detalize
    }
    
    class DspProperties
    {
    public:
      explicit DspProperties(const ::SNES_SPC& spc)
        : Spc(spc)
      {
      }
      
      uint_t GetToneChannels() const
      {
        const uint_t keyOff = Spc.get_dsp_reg(0x5c);
        const uint_t endx = Spc.get_dsp_reg(0x7c);
        const uint_t echo = Spc.get_dsp_reg(0x4d);
        return ~(keyOff | endx) | echo;
      }
      
      uint_t GetNoiseChannels() const
      {
        return Spc.get_dsp_reg(0x3d);
      }
      
      uint_t GetNoisePitch() const
      {
        //absolute freqs based on 32kHz clocking
        static const uint_t NOISE_FREQS[32] =
        {
          0, 16, 21, 25, 31, 42, 50, 63,
          83, 100, 125, 167, 200, 250, 333, 400,
          500, 667, 800, 1000, 1300, 1600, 2000, 2700,
          3200, 4000, 5300, 6400, 8000, 10700, 16000, 32000
        };
        return NOISE_FREQS[Spc.get_dsp_reg(0x6c) & 0x1f] * SPC_DIVIDER / C_7_FREQ;
      }
      
      uint_t GetLevel(uint_t chan) const
      {
        const int_t volLeft = static_cast<int8_t>(Spc.get_dsp_reg(chan * 16 + 0));
        const int_t volRight = static_cast<int8_t>(Spc.get_dsp_reg(chan * 16 + 1));
        return std::max<uint_t>(Math::Absolute(volLeft), Math::Absolute(volRight));
      }
      
      uint_t GetPitch(uint_t chan) const
      {
        return (Spc.get_dsp_reg(chan * 16 + 3) & 0x3f) * 256 + Spc.get_dsp_reg(chan * 16 + 2);
      }
    private:
      const ::SNES_SPC& Spc;
    };
  private:
    const Dump Data;
    ::SNES_SPC Spc;
    ::SPC_Filter Filter;
    Devices::Details::AnalysisMap Analysis;
  };

  class Renderer : public Module::Renderer
  {
  public:
    Renderer(SPC::Ptr tune, StateIterator::Ptr iterator, Sound::Receiver::Ptr target, Parameters::Accessor::Ptr params)
      : Tune(tune)
      , Iterator(iterator)
      , State(Iterator->GetStateObserver())
      , SoundParams(Sound::RenderParameters::Create(params))
      , Target(target)
      , Looped()
      , SamplesPerFrame()
    {
      ApplyParameters();
    }

    virtual TrackState::Ptr GetTrackState() const
    {
      return State;
    }

    virtual Module::Analyzer::Ptr GetAnalyzer() const
    {
      return Tune;
    }

    virtual bool RenderFrame()
    {
      try
      {
        ApplyParameters();

        Sound::ChunkBuilder builder;
        builder.Reserve(SamplesPerFrame);
        Tune->Render(SamplesPerFrame, builder);
        Resampler->ApplyData(builder.GetResult());
        Iterator->NextFrame(Looped);
        return Iterator->IsValid();
      }
      catch (const std::exception&)
      {
        return false;
      }
    }

    virtual void Reset()
    {
      SoundParams.Reset();
      Tune->Reset();
      Iterator->Reset();
    }

    virtual void SetPosition(uint_t frame)
    {
      SeekTune(frame);
      Module::SeekIterator(*Iterator, frame);
    }
  private:
    void ApplyParameters()
    {
      if (SoundParams.IsChanged())
      {
        Looped = SoundParams->Looped();
        const Time::Microseconds frameDuration = SoundParams->FrameDuration();
        SamplesPerFrame = static_cast<uint_t>(frameDuration.Get() * ::SNES_SPC::sample_rate / frameDuration.PER_SECOND);
        Resampler = Sound::CreateResampler(::SNES_SPC::sample_rate, SoundParams->SoundFreq(), Target);
      }
    }

    void SeekTune(uint_t frame)
    {
      uint_t current = State->Frame();
      if (frame < current)
      {
        Tune->Reset();
        current = 0;
      }
      if (const uint_t delta = frame - current)
      {
        Tune->Skip(delta * SamplesPerFrame);
      }
    }
  private:
    const SPC::Ptr Tune;
    const StateIterator::Ptr Iterator;
    const TrackState::Ptr State;
    Parameters::TrackingHelper<Sound::RenderParameters> SoundParams;
    const Sound::Receiver::Ptr Target;
    Sound::Receiver::Ptr Resampler;
    bool Looped;
    std::size_t SamplesPerFrame;
  };
  
  class Holder : public Module::Holder
  {
  public:
    Holder(SPC::Ptr tune, Information::Ptr info, Parameters::Accessor::Ptr props)
      : Tune(tune)
      , Info(info)
      , Properties(props)
    {
    }

    virtual Module::Information::Ptr GetModuleInformation() const
    {
      return Info;
    }

    virtual Parameters::Accessor::Ptr GetModuleProperties() const
    {
      return Properties;
    }

    virtual Renderer::Ptr CreateRenderer(Parameters::Accessor::Ptr params, Sound::Receiver::Ptr target) const
    {
      return MakePtr<Renderer>(Tune, Module::CreateStreamStateIterator(Info), target, params);
    }
  private:
    const SPC::Ptr Tune;
    const Information::Ptr Info;
    const Parameters::Accessor::Ptr Properties;
  };

  class DataBuilder : public Formats::Chiptune::SPC::Builder
  {
  public:
    explicit DataBuilder(PropertiesHelper& props)
      : Properties(props)
    {
    }

    virtual void SetRegisters(uint16_t /*pc*/, uint8_t /*a*/, uint8_t /*x*/, uint8_t /*y*/, uint8_t /*psw*/, uint8_t /*sp*/)
    {
    }

    virtual void SetTitle(const String& title)
    {
      if (Title.empty())
      {
        Properties.SetTitle(Title = title);
      }
    }
    
    virtual void SetGame(const String& game)
    {
      if (Program.empty())
      {
        Properties.SetProgram(Program = game);
      }
    }
    
    virtual void SetDumper(const String& dumper)
    {
      if (Author.empty())
      {
        Properties.SetAuthor(Author = dumper);
      }
    }
    
    virtual void SetComment(const String& comment)
    {
      if (Comment.empty())
      {
        Properties.SetComment(Comment = comment);
      }
    }
    
    virtual void SetDumpDate(const String& date)
    {
      Properties.SetDate(date);
    }
    
    virtual void SetIntro(Time::Milliseconds duration)
    {
      Intro = std::max(Intro, duration);
    }
    
    virtual void SetLoop(Time::Milliseconds duration)
    {
      Loop = duration;
    }
    
    virtual void SetFade(Time::Milliseconds duration)
    {
      Fade = duration;
    }
    
    virtual void SetArtist(const String& artist)
    {
      Properties.SetAuthor(Author = artist);
    }
    
    virtual void SetRAM(const void* /*data*/, std::size_t /*size*/)
    {
    }
    
    virtual void SetDSPRegisters(const void* /*data*/, std::size_t /*size*/)
    {
    }
    
    virtual void SetExtraRAM(const void* /*data*/, std::size_t /*size*/)
    {
    }
    
    Time::Milliseconds GetDuration(const Parameters::Accessor& params) const
    {
      Time::Milliseconds total = Intro;
      total += Loop;
      total += Fade;
      return total.Get() ? total : Time::Milliseconds(Module::GetDuration(params));
    }
  private:
    PropertiesHelper& Properties;
    String Title;
    String Program;
    String Author;
    String Comment;
    Time::Milliseconds Intro;
    Time::Milliseconds Loop;
    Time::Milliseconds Fade;
  };
  
  class Factory : public Module::Factory
  {
  public:
    virtual Module::Holder::Ptr CreateModule(const Parameters::Accessor& params, const Binary::Container& rawData, Parameters::Container::Ptr properties) const
    {
      try
      {
        PropertiesHelper props(*properties);
        DataBuilder dataBuilder(props);
        if (const Formats::Chiptune::Container::Ptr container = Formats::Chiptune::SPC::Parse(rawData, dataBuilder))
        {
          const SPC::Ptr tune = MakePtr<SPC>(rawData);
          props.SetSource(*container);
          props.SetFramesFrequency(50);
          const Time::Milliseconds duration = dataBuilder.GetDuration(params);
          const Time::Milliseconds period = Time::Milliseconds(20);
          const uint_t frames = duration.Get() / period.Get();
          const Information::Ptr info = CreateStreamInfo(frames);
          return MakePtr<Holder>(tune, info, properties);
        }
      }
      catch (const std::exception& e)
      {
        Dbg("Failed to create SPC: %s", e.what());
      }
      return Module::Holder::Ptr();
    }
  };
}
}

namespace ZXTune
{
  void RegisterSPCSupport(PlayerPluginsRegistrator& registrator)
  {
    const Char ID[] = {'S', 'P', 'C', 0};
    const uint_t CAPS = Capabilities::Module::Type::MEMORYDUMP | Capabilities::Module::Device::SPC700;

    const Formats::Chiptune::Decoder::Ptr decoder = Formats::Chiptune::CreateSPCDecoder();
    const Module::SPC::Factory::Ptr factory = MakePtr<Module::SPC::Factory>();
    const PlayerPlugin::Ptr plugin = CreatePlayerPlugin(ID, CAPS, decoder, factory);
    registrator.RegisterPlugin(plugin);
  }
}
