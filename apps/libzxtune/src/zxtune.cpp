/*
Abstract:
  ZXTune simple library implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#define ZXTUNE_API ZXTUNE_API_EXPORT
#include "../zxtune.h"
#include <apps/version/api.h>
//common includes
#include <contract.h>
#include <cycle_buffer.h>
#include <error_tools.h>
#include <progress_callback.h>
#include <tools.h>
//library includes
#include <binary/container.h>
#include <binary/container_factories.h>
#include <core/core_parameters.h>
#include <core/module_detect.h>
#include <core/module_holder.h>
#include <core/module_player.h>
#include <io/api.h>
#include <sound/mixer_factory.h>
#include <sound/sound_parameters.h>
//std includes
#include <map>
//boost includes
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

namespace Text
{
  extern const Char PROGRAM_NAME[] = {'l', 'i', 'b', 'z', 'x', 't', 'u', 'n', 'e', 0};
}

namespace
{
  template<class ObjPtrType>
  struct ObjectTraits
  {
    static ZXTuneHandle GetHandle(ObjPtrType obj)
    {
      return reinterpret_cast<ZXTuneHandle>(obj.get());
    }
  };

  template<class PtrType>
  class HandlesCache
  {
  public:
    ZXTuneHandle Add(PtrType val)
    {
      const ZXTuneHandle result = ObjectTraits<PtrType>::GetHandle(val);
      Require(result != ZXTuneHandle());
      Require(Handles.insert(typename Handle2Object::value_type(result, val)).second);
      return result;
    }

    void Delete(ZXTuneHandle handle)
    {
      Handles.erase(handle);
    }

    PtrType Get(ZXTuneHandle handle) const
    {
      const typename Handle2Object::const_iterator it = Handles.find(handle);
      Require(it != Handles.end());
      return it->second;
    }

    static HandlesCache<PtrType>& Instance()
    {
      static HandlesCache<PtrType> self;
      return self;
    }
  private:
    typedef std::map<ZXTuneHandle, PtrType> Handle2Object;
    Handle2Object Handles;
  };

  typedef HandlesCache<Binary::Container::Ptr> ContainersCache;
  typedef HandlesCache<ZXTune::Module::Holder::Ptr> ModulesCache;

  class BufferRender : public ZXTune::Sound::Receiver
  {
  public:
    typedef boost::shared_ptr<BufferRender> Ptr;

    BufferRender()
      : Buffer(32768)
      , DoneSamples()
    {
    }

    virtual void ApplyData(const ZXTune::Sound::MultiSample& data)
    {
      Buffer.Put(&data, 1);
    }

    virtual void Flush()
    {
    }

    std::size_t GetCurrentSample() const
    {
      return DoneSamples;
    }

    std::size_t GetSamples(std::size_t count, ZXTune::Sound::MultiSample* target)
    {
      const ZXTune::Sound::MultiSample* part1 = 0;
      std::size_t part1Size = 0;
      const ZXTune::Sound::MultiSample* part2 = 0;
      std::size_t part2Size = 0;
      if (const std::size_t toGet = Buffer.Peek(count, part1, part1Size, part2, part2Size))
      {
        ZXTune::Sound::ChangeSignCopy(part1, part1 + part1Size, target);
        if (part2)
        {
          ZXTune::Sound::ChangeSignCopy(part2, part2 + part2Size, target + part1Size);
        }
        Buffer.Consume(toGet);
        DoneSamples += toGet;
        return toGet;
      }
      return 0;
    }

    std::size_t DropSamples(std::size_t count)
    {
      const std::size_t toDrop = Buffer.Consume(count);
      DoneSamples += toDrop;
      return toDrop;
    }

    void Reset()
    {
      Buffer.Reset();
      DoneSamples = 0;
    }
  private:
    CycleBuffer<ZXTune::Sound::MultiSample> Buffer;
    std::size_t DoneSamples;
  };

  class PlayerWrapper
  {
  public:
    typedef boost::shared_ptr<PlayerWrapper> Ptr;

    PlayerWrapper(Parameters::Container::Ptr params, ZXTune::Module::Renderer::Ptr renderer, BufferRender::Ptr buffer)
      : Params(params)
      , Renderer(renderer)
      , Buffer(buffer)
    {
    }

    std::size_t RenderSound(ZXTune::Sound::MultiSample* target, std::size_t samples)
    {
      std::size_t result = 0;
      while (samples)
      {
        const std::size_t got = Buffer->GetSamples(samples, target);
        target += got;
        samples -= got;
        result += got;
        if (0 == samples || !Renderer->RenderFrame())
        {
          break;
        }
      }
      return result;
    }

    std::size_t Seek(std::size_t samples)
    {
      if (samples < Buffer->GetCurrentSample())
      {
        Reset();
      }
      while (samples != Buffer->GetCurrentSample())
      {
        const std::size_t toDrop = samples - Buffer->GetCurrentSample();
        if (const std::size_t dropped = Buffer->DropSamples(toDrop))
        {
          continue;
        }
        if (!Renderer->RenderFrame())
        {
          break;
        }
      }
      return Buffer->GetCurrentSample();
    }

    void Reset()
    {
      Renderer->Reset();
      Buffer->Reset();
    }

    Parameters::Container::Ptr GetParameters() const
    {
      return Params;
    }

    static Ptr Create(ZXTune::Module::Holder::Ptr holder)
    {
      const ZXTune::Module::Information::Ptr info = holder->GetModuleInformation();
      const uint_t channels = info->PhysicalChannels();
      const Parameters::Container::Ptr params = Parameters::Container::Create();
      //copy initial properties
      holder->GetModuleProperties()->Process(*params);
      const ZXTune::Sound::Mixer::Ptr mixer = ZXTune::Sound::CreateMixer(channels, params);
      const ZXTune::Module::Renderer::Ptr renderer = holder->CreateRenderer(params, mixer);
      const BufferRender::Ptr buffer = boost::make_shared<BufferRender>();
      mixer->SetTarget(buffer);
      return boost::make_shared<PlayerWrapper>(params, renderer, buffer);
    }
  private:
    const Parameters::Container::Ptr Params;
    const ZXTune::Module::Renderer::Ptr Renderer;
    const BufferRender::Ptr Buffer;
  };

  typedef HandlesCache<PlayerWrapper::Ptr> PlayersCache;

  bool FindDefaultValue(const Parameters::NameType& name, Parameters::IntType& value)
  {
    typedef std::pair<Parameters::NameType, Parameters::IntType> Name2Val;
    static const Name2Val DEFAULTS[] =
    {
      Name2Val(Parameters::ZXTune::Sound::FREQUENCY, Parameters::ZXTune::Sound::FREQUENCY_DEFAULT),
      Name2Val(Parameters::ZXTune::Core::AYM::CLOCKRATE, Parameters::ZXTune::Core::AYM::CLOCKRATE_DEFAULT),
      Name2Val(Parameters::ZXTune::Sound::FRAMEDURATION, Parameters::ZXTune::Sound::FRAMEDURATION_DEFAULT),
    };
    const Name2Val* const defVal = std::find_if(DEFAULTS, ArrayEnd(DEFAULTS), boost::bind(&Name2Val::first, _1) == name);
    if (ArrayEnd(DEFAULTS) == defVal)
    {
      return false;
    }
    value = defVal->second;
    return true;
  }
}

const char* ZXTune_GetVersion()
{
  static const String VERSION = ::GetProgramVersionString();
  return VERSION.c_str();
}

ZXTuneHandle ZXTune_OpenData(const char* filename, const char** subname)
{
  try
  {
    const String uri(filename);
    const IO::Identifier::Ptr id = IO::ResolveUri(uri);
    const Parameters::Accessor::Ptr params = Parameters::Container::Create();
    const Binary::Container::Ptr result = IO::OpenData(id->Path(), *params, Log::ProgressCallback::Stub());
    Require(result->Size() != 0);
    if (subname)
    {
      const String subpath = id->Subpath();
      const String::size_type subpathOffset = uri.size() - subpath.size();
      *subname = filename + subpathOffset;
    }
    return ContainersCache::Instance().Add(result);
  }
  catch (const Error&)
  {
    return ZXTuneHandle();
  }
  catch (const std::exception&)
  {
    return ZXTuneHandle();
  }
}

ZXTuneHandle ZXTune_CreateData(const void* data, size_t size)
{
  try
  {
    const Binary::Container::Ptr result = Binary::CreateContainer(data, size);
    return ContainersCache::Instance().Add(result);
  }
  catch (const std::exception&)
  {
    return ZXTuneHandle();
  }
}

ZXTuneHandle ZXTune_ReadData(ZXTuneReadFunc reader, void* userData)
{
  try
  {
    Require(reader != 0);
    std::size_t BLOCK_TO_READ = 1048576;
    std::auto_ptr<Dump> data(new Dump());
    for (std::size_t done = 0; ;)
    {
      data->resize(done + BLOCK_TO_READ);
      const std::size_t read = reader(&data->at(done), BLOCK_TO_READ, userData);
      done += read;
      if (read != BLOCK_TO_READ)
      {
        data->resize(done);
        break;
      }
    }
    Require(!data->empty());
    const Binary::Container::Ptr result = Binary::CreateContainer(data);
    return ContainersCache::Instance().Add(result);
  }
  catch (const std::exception&)
  {
    return ZXTuneHandle();
  }
}

void ZXTune_CloseData(ZXTuneHandle data)
{
  ContainersCache::Instance().Delete(data);
}

ZXTuneHandle ZXTune_OpenModule(ZXTuneHandle data, const char* subname)
{
  try
  {
    const Binary::Container::Ptr src = ContainersCache::Instance().Get(data);
    const Parameters::Accessor::Ptr params = Parameters::Container::Create();
    const String subpath = subname ? String(subname) : String();
    const ZXTune::Module::Holder::Ptr result = ZXTune::OpenModule(params, src, subpath);
    return ModulesCache::Instance().Add(result);
  }
  catch (const Error&)
  {
    return ZXTuneHandle();
  }
  catch (const std::exception&)
  {
    return ZXTuneHandle();
  }
}

void ZXTune_CloseModule(ZXTuneHandle module)
{
  ModulesCache::Instance().Delete(module);
}

bool ZXTune_GetModuleInfo(ZXTuneHandle module, ZXTuneModuleInfo* info)
{
  try
  {
    Require(info != 0);
    const ZXTune::Module::Holder::Ptr holder = ModulesCache::Instance().Get(module);
    const ZXTune::Module::Information::Ptr modinfo = holder->GetModuleInformation();
    info->Positions = modinfo->PositionsCount();
    info->LoopPosition = modinfo->LoopPosition();
    info->Patterns = modinfo->PatternsCount();
    info->Frames = modinfo->FramesCount();
    info->LoopFrame = modinfo->LoopFrame();
    info->LogicalChannels = modinfo->LogicalChannels();
    info->PhysicalChannels = modinfo->PhysicalChannels();
    info->InitialTempo = modinfo->Tempo();
    return true;
  }
  catch (const Error&)
  {
    return false;
  }
  catch (const std::exception&)
  {
    return false;
  }
}


ZXTuneHandle ZXTune_CreatePlayer(ZXTuneHandle module)
{
  try
  {
    const ZXTune::Module::Holder::Ptr holder = ModulesCache::Instance().Get(module);
    const PlayerWrapper::Ptr result = PlayerWrapper::Create(holder);
    return PlayersCache::Instance().Add(result);
  }
  catch (const Error&)
  {
    return ZXTuneHandle();
  }
  catch (const std::exception&)
  {
    return ZXTuneHandle();
  }
}

void ZXTune_DestroyPlayer(ZXTuneHandle player)
{
  PlayersCache::Instance().Delete(player);
}

// returns actually rendered bytes
int ZXTune_RenderSound(ZXTuneHandle player, void* buffer, size_t samples)
{
  try
  {
    const PlayerWrapper::Ptr wrapper = PlayersCache::Instance().Get(player);
    return wrapper->RenderSound(static_cast<ZXTune::Sound::MultiSample*>(buffer), samples);
  }
  catch (const Error&)
  {
    return -1;
  }
  catch (const std::exception&)
  {
    return -1;
  }
}

int ZXTune_SeekSound(ZXTuneHandle player, size_t sample)
{
  try
  {
    const PlayerWrapper::Ptr wrapper = PlayersCache::Instance().Get(player);
    return wrapper->Seek(sample);
  }
  catch (const Error&)
  {
    return -1;
  }
  catch (const std::exception&)
  {
    return -1;
  }
}

bool ZXTune_ResetSound(ZXTuneHandle player)
{
  try
  {
    const PlayerWrapper::Ptr wrapper = PlayersCache::Instance().Get(player);
    wrapper->Reset();
    return true;
  }
  catch (const Error&)
  {
    return false;
  }
  catch (const std::exception&)
  {
    return false;
  }
}

bool ZXTune_GetPlayerParameterInt(ZXTuneHandle player, const char* paramName, int* paramValue)
{
  try
  {
    Require(paramValue != 0);
    const PlayerWrapper::Ptr wrapper = PlayersCache::Instance().Get(player);
    const Parameters::Accessor::Ptr props = wrapper->GetParameters();
    const Parameters::NameType name(paramName);
    Parameters::IntType value;
    if (!props->FindValue(name, value) && !FindDefaultValue(name, value))
    {
      return false;
    }
    *paramValue = static_cast<int>(value);
    return true;
  }
  catch (const Error&)
  {
    return false;
  }
  catch (const std::exception&)
  {
    return false;
  }
}

bool ZXTune_SetPlayerParameterInt(ZXTuneHandle player, const char* paramName, int paramValue)
{
  try
  {
    const PlayerWrapper::Ptr wrapper = PlayersCache::Instance().Get(player);
    const Parameters::Modifier::Ptr props = wrapper->GetParameters();
    const Parameters::NameType name(paramName);
    props->SetValue(name, paramValue);
    return true;
  }
  catch (const Error&)
  {
    return false;
  }
  catch (const std::exception&)
  {
    return false;
  }
}
