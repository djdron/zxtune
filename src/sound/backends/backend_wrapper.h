/*
Abstract:
  Safe backend wrapper. Used for safe playback stopping if object is destroyed

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

#pragma once
#ifndef __SOUND_BACKEND_WRAPPER_H_DEFINED__
#define __SOUND_BACKEND_WRAPPER_H_DEFINED__

//common includes
#include <error_tools.h>
//library includes
#include <sound/backend.h>
#include <sound/error_codes.h>
//boost includes
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>
//text includes
#include <sound/text/sound.h>

namespace ZXTune
{
  namespace Sound
  {
    template<class Impl>
    class SafeBackendWrapper : public Backend
    {
      SafeBackendWrapper(BackendInformation::Ptr info, BackendParameters::Ptr params, Module::Holder::Ptr module)
        : Information(info)
        , Delegate(new Impl(params, module))
      {
        //perform fast test to detect if parameters are correct
        Delegate->OnStartup();
        Delegate->OnShutdown();
      }
    public:
      static Error Create(BackendInformation::Ptr info, BackendParameters::Ptr params, Module::Holder::Ptr module,
        Backend::Ptr& result, Error::LocationRef loc)
      {
        try
        {
          result.reset(new SafeBackendWrapper<Impl>(info, params, module));
          return Error();
        }
        catch (const Error& e)
        {
          return MakeFormattedError(loc, BACKEND_FAILED_CREATE,
            Text::SOUND_ERROR_BACKEND_FAILED, info->Id()).AddSuberror(e);
        }
        catch (const std::bad_alloc&)
        {
          return Error(loc, BACKEND_NO_MEMORY, Text::SOUND_ERROR_BACKEND_NO_MEMORY);
        }
      }

      virtual ~SafeBackendWrapper()
      {
        Delegate->Stop();//TODO: warn if error
      }

      virtual BackendInformation::Ptr GetInformation() const
      {
        return Information;
      }

      virtual Module::Player::ConstPtr GetPlayer() const
      {
        return Delegate->GetPlayer();
      }

      virtual Error Play()
      {
        return Delegate->Play();
      }

      virtual Error Pause()
      {
        return Delegate->Pause();
      }

      virtual Error Stop()
      {
        return Delegate->Stop();
      }

      virtual Error SetPosition(uint_t frame)
      {
        return Delegate->SetPosition(frame);
      }

      virtual State GetCurrentState(Error* error) const
      {
        return Delegate->GetCurrentState(error);
      }

      virtual SignalsCollector::Ptr CreateSignalsCollector(uint_t signalsMask) const
      {
        return Delegate->CreateSignalsCollector(signalsMask);
      }

      virtual VolumeControl::Ptr GetVolumeControl() const
      {
        return Delegate->GetVolumeControl();
      }
    private:
      const BackendInformation::Ptr Information;
      boost::scoped_ptr<Impl> Delegate;
    };
  }
}

#endif //__SOUND_BACKEND_WRAPPER_H_DEFINED__
