/**
* 
* @file
*
* @brief Player access implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "debug.h"
#include "global_options.h"
#include "module.h"
#include "player.h"
#include "properties.h"
#include "zxtune.h"
//common includes
#include <make_ptr.h>
//library includes
#include <parameters/merged_accessor.h>
#include <sound/mixer_factory.h>
//std includes
#include <deque>

namespace
{
  static_assert(Sound::Sample::CHANNELS == 2, "Incompatible sound channels count");
  static_assert(Sound::Sample::BITS == 16, "Incompatible sound sample bits count");
  static_assert(Sound::Sample::MID == 0, "Incompatible sound sample type");

  class BufferTarget : public Sound::Receiver
  {
  public:
    typedef std::shared_ptr<BufferTarget> Ptr;

    void ApplyData(Sound::Chunk::Ptr data) override
    {
      Buffers.emplace_back(std::move(data));
    }

    void Flush() override
    {
    }

    std::size_t GetSamples(std::size_t count, int16_t* target)
    {
      if (Buffers.empty())
      {
        return 0;
      }
      Buff& cur = Buffers.front();
      const std::size_t samples = count / Sound::Sample::CHANNELS;
      const std::size_t copied = cur.Get(samples, target);
      if (0 == cur.Avail)
      {
        Buffers.pop_front();
      }
      return copied * Sound::Sample::CHANNELS;
    }
  private:
    struct Buff
    {
      explicit Buff(Sound::Chunk::Ptr data)
        : Data(std::move(data))
        , Avail(Data->size())
      {
      }
      
      std::size_t Get(std::size_t count, void* target)
      {
        const std::size_t toCopy = std::min(count, Avail);
        std::memcpy(target, &Data->back() + 1 - Avail, toCopy * sizeof(Data->front()));
        Avail -= toCopy;
        return toCopy;
      }
    
      Sound::Chunk::Ptr Data;
      std::size_t Avail;
    };
    std::deque<Buff> Buffers;
  };

  class PlayerControl : public Player::Control
  {
  public:
    PlayerControl(Parameters::Container::Ptr params, Module::Renderer::Ptr render, BufferTarget::Ptr buffer)
      : Params(std::move(params))
      , Renderer(std::move(render))
      , Buffer(std::move(buffer))
      , TrackState(Renderer->GetTrackState())
      , Analyser(Renderer->GetAnalyzer())
    {
    }
    
    uint_t GetPosition() const override
    {
      return TrackState->Frame();
    }

    uint_t Analyze(uint_t maxEntries, uint32_t* bands, uint32_t* levels) const override
    {
      const auto& result = Analyser->GetState();
      uint_t doneEntries = 0;
      for (auto it = result.begin(), lim = result.end(); it != lim && doneEntries != maxEntries; ++it, ++doneEntries)
      {
        bands[doneEntries] = it->Band;
        levels[doneEntries] = it->Level;
      }
      return doneEntries;
    }

    Parameters::Container::Ptr GetParameters() const override
    {
      return Params;
    }
    
    bool Render(uint_t samples, int16_t* buffer) override
    {
      for (;;)
      {
        if (const std::size_t got = Buffer->GetSamples(samples, buffer))
        {
          buffer += got;
          samples -= got;
          if (!samples)
          {
            break;
          }
        }
        if (!Renderer->RenderFrame())
        {
          break;
        }
      }
      std::fill_n(buffer, samples, 0);
      return samples == 0;
    }

    void Seek(uint_t frame) override
    {
      Renderer->SetPosition(frame);
    }
  private:
    const Parameters::Container::Ptr Params;
    const Module::Renderer::Ptr Renderer;
    const BufferTarget::Ptr Buffer;
    const Module::TrackState::Ptr TrackState;
    const Module::Analyzer::Ptr Analyser;
  };

  Player::Control::Ptr CreateControl(Module::Holder::Ptr module)
  {
    auto globalParameters = Parameters::GlobalOptions();
    auto localParameters = Parameters::Container::Create();
    auto internalProperties = module->GetModuleProperties();
    auto properties = Parameters::CreateMergedAccessor(localParameters, std::move(internalProperties), std::move(globalParameters));
    auto buffer = MakePtr<BufferTarget>();
    auto renderer = module->CreateRenderer(properties, buffer);
    return MakePtr<PlayerControl>(std::move(localParameters), std::move(renderer), std::move(buffer));
  }

  template<class StorageType, class ResultType>
  class AutoArray
  {
  public:
    AutoArray(JNIEnv* env, StorageType storage)
      : Env(env)
      , Storage(storage)
      , Length(Env->GetArrayLength(Storage))
      , Content(static_cast<ResultType*>(Env->GetPrimitiveArrayCritical(Storage, 0)))
    {
    }

    ~AutoArray()
    {
      if (Content)
      {
        Env->ReleasePrimitiveArrayCritical(Storage, Content, 0);
      }
    }

    operator bool () const
    {
      return Length != 0 && Content != 0;
    }

    ResultType* Data() const
    {
      return Length ? Content : 0;
    }

    std::size_t Size() const
    {
      return Length;
    }
  private:
    JNIEnv* const Env;
    const StorageType Storage;
    const jsize Length;
    ResultType* const Content;
  };
}

namespace Player
{
  Player::Storage::HandleType Create(Module::Holder::Ptr module)
  {
    auto ctrl = CreateControl(module);
    Dbg("Player::Create(module=%p)=%p", module.get(), ctrl.get());
    return Player::Storage::Instance().Add(std::move(ctrl));
  }
}

JNIEXPORT jboolean JNICALL Java_app_zxtune_ZXTune_Player_1Render
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jshortArray buffer)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    typedef AutoArray<jshortArray, int16_t> ArrayType;
    if (ArrayType buf = ArrayType(env, buffer))
    {
      return player->Render(buf.Size(), buf.Data());
    }
  }
  return false;
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1Analyze
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jintArray bands, jintArray levels)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    typedef AutoArray<jintArray, uint32_t> ArrayType;
    ArrayType rawBands(env, bands);
    ArrayType rawLevels(env, levels);
    if (rawBands && rawLevels)
    {
      return player->Analyze(std::min(rawBands.Size(), rawLevels.Size()), rawBands.Data(), rawLevels.Data());
    }
  }
  return 0;
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1GetPosition
  (JNIEnv* /*env*/, jclass /*self*/, jint playerHandle)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    return player->GetPosition();
  }
  return -1;
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetPosition
  (JNIEnv* /*env*/, jclass /*self*/, jint playerHandle, jint position)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    player->Seek(position);
  }
}

JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong defVal)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    const auto& params = player->GetParameters();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  }
  return defVal;
}

JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring defVal)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    const auto& params = player->GetParameters();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  }
  return defVal;
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong value)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    const auto& params = player->GetParameters();
    Jni::PropertiesWriteHelper props(env, *params);
    props.Set(propName, value);
  }
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring value)
{
  if (const auto& player = Player::Storage::Instance().Get(playerHandle))
  {
    const auto& params = player->GetParameters();
    Jni::PropertiesWriteHelper props(env, *params);
    props.Set(propName, value);
  }
}
