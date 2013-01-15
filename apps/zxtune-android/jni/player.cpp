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
  inline int16_t ToNativeSample(ZXTune::Sound::Sample in)
  {
    return static_cast<int16_t>(ZXTune::Sound::ToSignedSample(in));
  }

  class BufferTarget : public ZXTune::Sound::Receiver
  {
  public:
    typedef boost::shared_ptr<BufferTarget> Ptr;

    virtual void ApplyData(const ZXTune::Sound::MultiSample& data)
    {
      Buffer.push_back(data);
    }

    virtual void Flush()
    {
    }

    std::size_t GetSamples(std::size_t count, int16_t* target)
    {
      const std::size_t toGet = std::min(count / ZXTune::Sound::OUTPUT_CHANNELS, Buffer.size());
      const BufferType::iterator endToCopy = Buffer.begin() + toGet;
      std::transform(&Buffer.begin()->front(), &endToCopy->front(), target, &ToNativeSample);
      Buffer.erase(Buffer.begin(), endToCopy);
      return toGet * ZXTune::Sound::OUTPUT_CHANNELS;
    }

    std::size_t AvailSamples() const
    {
      return Buffer.size() * ZXTune::Sound::OUTPUT_CHANNELS;
    }
  private:
    typedef std::vector<ZXTune::Sound::MultiSample> BufferType;
    BufferType Buffer;
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
      while (samples > Buffer->AvailSamples() && Renderer->RenderFrame())
      {
      }
      const std::size_t done = Buffer->GetSamples(samples, buffer);
      std::fill(buffer + done, buffer + samples, 0);
      return done == samples;
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

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1Create(JNIEnv* /*env*/, jclass /*self*/, jint moduleHandle)
{
  if (const ZXTune::Module::Holder::Ptr module = Module::Storage::Instance().Get(moduleHandle))
  {
    const Player::Control::Ptr ctrl = CreateControl(module);
    return Player::Storage::Instance().Add(ctrl);
  }
  return 0;
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1Destroy(JNIEnv* /*env*/, jclass /*self*/, jint playerHandle)
{
  Player::Storage::Instance().Fetch(playerHandle);
}

JNIEXPORT jboolean JNICALL Java_app_zxtune_ZXTune_Player_1Render(JNIEnv* env, jclass /*self*/, jint playerHandle, jobject buffer)
{
  if (const Player::Control::Ptr player = Player::Storage::Instance().Get(playerHandle))
  {
    int16_t* buf = static_cast<int16_t*>(env->GetDirectBufferAddress(buffer));
    const std::size_t size = env->GetDirectBufferCapacity(buffer);
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