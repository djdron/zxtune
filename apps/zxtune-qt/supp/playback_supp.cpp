/*
Abstract:
  Playback support implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001

  This file is a part of zxtune-qt application based on zxtune library
*/

//local includes
#include "playback_supp.h"
#include "playlist/supp/data.h"
#include "ui/utils.h"
//common includes
#include <contract.h>
#include <error.h>
#include <tools.h>
//library includes
#include <parameters/merged_accessor.h>
#include <sound/backend.h>
//boost inlcudes
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
//qt includes
#include <QtCore/QTimer>

namespace
{
  //TODO: simplify this shitcode
  class BackendParams : public Sound::CreateBackendParameters
  {
  public:
    BackendParams(Parameters::Accessor::Ptr params, Module::Holder::Ptr module, Sound::BackendCallback::Ptr callback)
      : Params(params)
      , Module(module)
      , Properties(Module->GetModuleProperties())
      , Callback(callback)
    {
    }

    virtual Parameters::Accessor::Ptr GetParameters() const
    {
      return Parameters::CreateMergedAccessor(Properties, Params);
    }

    virtual Module::Holder::Ptr GetModule() const
    {
      return Module;
    }

    virtual Sound::BackendCallback::Ptr GetCallback() const
    {
      return Callback;
    }
  private:
    const Parameters::Accessor::Ptr Params;
    const Module::Holder::Ptr Module;
    const Parameters::Accessor::Ptr Properties;
    const Sound::BackendCallback::Ptr Callback;
  };

  class StubControl : public Sound::PlaybackControl
  {
  public:
    virtual void Play()
    {
    }

    virtual void Pause()
    {
    }

    virtual void Stop()
    {
    }

    virtual void SetPosition(uint_t /*frame*/)
    {
    }
    
    virtual State GetCurrentState() const
    {
      return STOPPED;
    }

    static Ptr Instance()
    {
      static StubControl instance;
      return Ptr(&instance, NullDeleter<Sound::PlaybackControl>());
    }
  };

  class PlaybackSupportImpl : public PlaybackSupport
                            , private Sound::BackendCallback
  {
  public:
    PlaybackSupportImpl(QObject& parent, Parameters::Accessor::Ptr sndOptions)
      : PlaybackSupport(parent)
      , Params(sndOptions)
      , Control(StubControl::Instance())
    {
      const unsigned UI_UPDATE_FPS = 5;
      Timer.setInterval(1000 / UI_UPDATE_FPS);
      Require(Timer.connect(this, SIGNAL(OnStartModule(Sound::Backend::Ptr, Playlist::Item::Data::Ptr)), SLOT(start())));
      Require(Timer.connect(this, SIGNAL(OnStopModule()), SLOT(stop())));
      Require(connect(&Timer, SIGNAL(timeout()), SIGNAL(OnUpdateState())));
    }

    virtual void SetItem(Playlist::Item::Data::Ptr item)
    {
      const Module::Holder::Ptr module = item->GetModule();
      if (!module)
      {
        return;
      }
      try
      {
        Stop();
        Control = StubControl::Instance();
        Backend.reset();
        Backend = CreateBackend(Params, module);
        if (Backend)
        {
          Control = Backend->GetPlaybackControl();
          Item = item;
          Control->Play();
        }
      }
      catch (const Error& e)
      {
        emit ErrorOccurred(e);
      }
    }

    virtual void Play()
    {
      try
      {
        Control->Play();
      }
      catch (const Error& e)
      {
        emit ErrorOccurred(e);
      }
    }

    virtual void Stop()
    {
      try
      {
        Control->Stop();
      }
      catch (const Error& e)
      {
        emit ErrorOccurred(e);
      }
    }

    virtual void Pause()
    {
      try
      {
        const Sound::PlaybackControl::State curState = Control->GetCurrentState();
        if (Sound::PlaybackControl::STARTED == curState)
        {
          Control->Pause();
        }
        else if (Sound::PlaybackControl::PAUSED == curState)
        {
          Control->Play();
        }
      }
      catch (const Error& e)
      {
        emit ErrorOccurred(e);
      }
    }

    virtual void Seek(int frame)
    {
      try
      {
        Control->SetPosition(frame);
      }
      catch (const Error& e)
      {
        emit ErrorOccurred(e);
      }
    }

    //BackendCallback
    virtual void OnStart(Module::Holder::Ptr /*module*/)
    {
      emit OnStartModule(Backend, Item);
    }

    virtual void OnFrame(const Module::TrackState& /*state*/)
    {
    }

    virtual void OnStop()
    {
      emit OnStopModule();
    }

    virtual void OnPause()
    {
      emit OnPauseModule();
    }

    virtual void OnResume()
    {
      emit OnResumeModule();
    }

    virtual void OnFinish()
    {
      emit OnFinishModule();
    }
  private:
    Sound::Backend::Ptr CreateBackend(Parameters::Accessor::Ptr params, Module::Holder::Ptr module)
    {
      //create backend
      const Sound::BackendCallback::Ptr cb(static_cast<Sound::BackendCallback*>(this), NullDeleter<Sound::BackendCallback>());
      const Sound::CreateBackendParameters::Ptr createParams = MakeBackendParameters(params, module, cb);
      std::list<Error> errors;
      const Sound::BackendsScope::Ptr systemBackends = Sound::BackendsScope::CreateSystemScope(params);
      for (Sound::BackendCreator::Iterator::Ptr backends = systemBackends->Enumerate();
        backends->IsValid(); backends->Next())
      {
        const Sound::BackendCreator::Ptr creator = backends->Get();
        Sound::Backend::Ptr result;
        try
        {
          return creator->CreateBackend(createParams);
        }
        catch (const Error& err)
        {
          errors.push_back(err);
        }
      }
      ReportErrors(errors);
      return Sound::Backend::Ptr();
    }

    void ReportErrors(const std::list<Error>& errors)
    {
      for (std::list<Error>::const_iterator it = errors.begin(), lim = errors.end(); it != lim; ++it)
      {
        emit ErrorOccurred(*it);
      }
    }
  private:
    const Parameters::Accessor::Ptr Params;
    QTimer Timer;
    Playlist::Item::Data::Ptr Item;
    Sound::Backend::Ptr Backend;
    Sound::PlaybackControl::Ptr Control;
  };
}

PlaybackSupport::PlaybackSupport(QObject& parent) : QObject(&parent)
{
}

PlaybackSupport* PlaybackSupport::Create(QObject& parent, Parameters::Accessor::Ptr sndOptions)
{
  REGISTER_METATYPE(Sound::Backend::Ptr);
  REGISTER_METATYPE(Error);
  return new PlaybackSupportImpl(parent, sndOptions);
}

Sound::CreateBackendParameters::Ptr MakeBackendParameters(Parameters::Accessor::Ptr params, Module::Holder::Ptr module, Sound::BackendCallback::Ptr callback)
{
  return boost::make_shared<BackendParams>(params, module, callback);
}
