/*
Abstract:
  Player implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "debug.h"
#include "module.h"
#include "player.h"
#include "properties.h"
#include "zxtune.h"
//library includes
#include <sound/mixer_factory.h>
//boost includes
#include <boost/make_shared.hpp>

namespace
{
  class BufferTarget : public ZXTune::Sound::Receiver
  {
  public:
    typedef boost::shared_ptr<BufferTarget> Ptr;
    
    BufferTarget()
      : Taken()
    {
    }

    virtual void ApplyData(const ZXTune::Sound::MultiSample& data)
    {
      Buffer.push_back(data);
    }

    virtual void Flush()
    {
    }

    std::size_t GetSamples(std::size_t count, int16_t* target)
    {
      const std::size_t toGet = std::min(count / ZXTune::Sound::OUTPUT_CHANNELS, Buffer.size() - Taken);
      const BufferType::const_iterator beginToCopy = Buffer.begin() + Taken;
      const BufferType::const_iterator endToCopy = beginToCopy + toGet;
      ZXTune::Sound::ChangeSignCopy(&*beginToCopy, &*beginToCopy + toGet, safe_ptr_cast<ZXTune::Sound::MultiSample*>(target));
      if (endToCopy == Buffer.end())
      {
        Buffer.clear();
        Taken = 0;
      }
      else
      {
        Taken += toGet;
      }
      return toGet * ZXTune::Sound::OUTPUT_CHANNELS;
    }

    std::size_t AvailSamples() const
    {
      return Buffer.size() * ZXTune::Sound::OUTPUT_CHANNELS;
    }
  private:
    typedef std::vector<ZXTune::Sound::MultiSample> BufferType;
    BufferType Buffer;
    std::size_t Taken;
  };

  class PlayerControl : public Player::Control
  {
  public:
    PlayerControl(Parameters::Container::Ptr params, ZXTune::Module::Renderer::Ptr render, BufferTarget::Ptr buffer)
      : Params(params)
      , Renderer(render)
      , Buffer(buffer)
    {
    }
    
    virtual uint_t GetPosition() const
    {
      return Renderer->GetTrackState()->Frame();
    }

    virtual Parameters::Container::Ptr GetParameters() const
    {
      return Params;
    }
    
    virtual bool Render(std::size_t samples, int16_t* buffer)
    {
      for (;;)
      {
        const std::size_t got = Buffer->GetSamples(samples, buffer);
        buffer += got;
        if (0 == (samples -= got) || !Renderer->RenderFrame())
        {
          break;
        }
      }
      std::fill_n(buffer, samples, 0);
      return samples == 0;
    }

    virtual void Seek(uint_t frame)
    {
      Renderer->SetPosition(frame);
    }
  private:
    const Parameters::Container::Ptr Params;
    const ZXTune::Module::Renderer::Ptr Renderer;
    const BufferTarget::Ptr Buffer;
  };

  Player::Control::Ptr CreateControl(const ZXTune::Module::Holder::Ptr module)
  {
    const Parameters::Container::Ptr params = Parameters::Container::Create();
    const ZXTune::Module::Information::Ptr info = module->GetModuleInformation();
    const ZXTune::Sound::Mixer::Ptr mixer = ZXTune::Sound::CreateMixer(info->PhysicalChannels(), params);
    const Parameters::Accessor::Ptr props = module->GetModuleProperties();
    const Parameters::Accessor::Ptr allProps = Parameters::CreateMergedAccessor(params, props);
    const ZXTune::Module::Renderer::Ptr renderer = module->CreateRenderer(allProps, mixer);
    const BufferTarget::Ptr buffer = boost::make_shared<BufferTarget>();
    mixer->SetTarget(buffer);
    return boost::make_shared<PlayerControl>(params, renderer, buffer);
  }
}

namespace Player
{
  int Create(ZXTune::Module::Holder::Ptr module)
  {
    const Player::Control::Ptr ctrl = CreateControl(module);
    Dbg("Player::Create(module=%p)=%p", module.get(), ctrl.get());
    return Player::Storage::Instance().Add(ctrl);
  }
}

JNIEXPORT jboolean JNICALL Java_app_zxtune_ZXTune_Player_1Render
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jint size, jobject buffer)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    int16_t* buf = static_cast<int16_t*>(env->GetDirectBufferAddress(buffer));
    return player->Render(size / sizeof(*buf), buf);
  }
  return false;
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1GetPosition(JNIEnv* /*env*/, jclass /*self*/, jint playerHandle)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    return player->GetPosition();
  }
  return -1;
}

JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong defVal)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    const Parameters::Container::Ptr params = player->GetParameters();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  }
  return defVal;
}

JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring defVal)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    const Parameters::Container::Ptr params = player->GetParameters();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  }
  return defVal;
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong value)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    const Parameters::Container::Ptr params = player->GetParameters();
    Jni::PropertiesWriteHelper props(env, *params);
    props.Set(propName, value);
  }
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring value)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    const Parameters::Container::Ptr params = player->GetParameters();
    Jni::PropertiesWriteHelper props(env, *params);
    props.Set(propName, value);
  }
}
