/**
* 
* @file
*
* @brief Convert operation implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "operations_convert.h"
#include "operations_helpers.h"
#include "storage.h"
#include <apps/zxtune-qt/supp/playback_supp.h>
#include <apps/zxtune-qt/ui/utils.h>
//common includes
#include <contract.h>
#include <error_tools.h>
//library includes
#include <async/src/event.h>
#include <io/api.h>
#include <io/template.h>
#include <parameters/merged_accessor.h>
#include <parameters/template.h>
#include <sound/backends_parameters.h>
#include <sound/sound_parameters.h>
#include <sound/backend.h>
//std includes
#include <numeric>
//boost includes
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

namespace
{
  //TODO: rework
  class ConvertCallback : public Sound::BackendCallback
  {
    enum EventType
    {
      STOPPED = 1,
      CANCELED = 2
    };
  public:
    explicit ConvertCallback(Log::ProgressCallback& callback)
      : Callback(callback)
      , Event()
    {
    }

    virtual void OnStart()
    {
      Event.Reset();
    }

    virtual void OnFrame(const Module::TrackState& state)
    {
      try
      {
        Callback.OnProgress(state.Frame());
      }
      catch (const std::exception&)
      {
        Event.Set(CANCELED);
      }
    }

    virtual void OnStop()
    {
      Event.Set(STOPPED);
    }

    virtual void OnPause()
    {
    }

    virtual void OnResume()
    {
    }

    virtual void OnFinish()
    {
    }

    void WaitForFinish()
    {
      if (Event.WaitForAny(STOPPED, CANCELED) == CANCELED)
      {
        throw std::exception();
      }
    }
  private:
    Log::ProgressCallback& Callback;
    Async::Event<uint_t> Event;
  };

  //TODO: simplify
  class ConvertVisitor : public Playlist::Item::Visitor
  {
  public:
    ConvertVisitor(uint_t totalItems, const String& type, Sound::Service::Ptr service, Log::ProgressCallback& cb, Playlist::Item::ConversionResultNotification::Ptr result)
      : TotalItems(totalItems)
      , DoneItems(0)
      , Callback(cb)
      , Type(type)
      , Service(service)
      , Result(result)
    {
    }

    virtual void OnItem(Playlist::Model::IndexType /*index*/, Playlist::Item::Data::Ptr data)
    {
      const String path = data->GetFullPath();
      if (Module::Holder::Ptr holder = data->GetModule())
      {
        ConvertItem(path, holder);
      }
      else
      {
        Result->AddFailedToOpen(path);
      }
      ++DoneItems;
    }
  private:
    void ConvertItem(const String& path, Module::Holder::Ptr item)
    {
      try
      {
        const Log::ProgressCallback::Ptr curItemProgress = Log::CreateNestedPercentProgressCallback(TotalItems, DoneItems, Callback);
        const Module::Information::Ptr info = item->GetModuleInformation();
        const Log::ProgressCallback::Ptr framesProgress = Log::CreatePercentProgressCallback(info->FramesCount(), *curItemProgress);
        ConvertCallback cb(*framesProgress);
        const Sound::Backend::Ptr backend = Service->CreateBackend(Type, item, Sound::BackendCallback::Ptr(&cb, NullDeleter<Sound::BackendCallback>()));
        const Sound::PlaybackControl::Ptr control = backend->GetPlaybackControl();
        control->Play();
        cb.WaitForFinish();
        control->Stop();
        curItemProgress->OnProgress(100);
        Result->AddSucceed();
      }
      catch (const Error& err)
      {
        Result->AddFailedToConvert(path, err);
      }
    }
  private:
    const uint_t TotalItems;
    uint_t DoneItems;
    Log::ProgressCallback& Callback;
    const String Type;
    const Sound::Service::Ptr Service;
    const Playlist::Item::ConversionResultNotification::Ptr Result;
  };

  class SoundFormatConvertOperation : public Playlist::Item::TextResultOperation
  {
  public:
    SoundFormatConvertOperation(Playlist::Model::IndexSetPtr items,
      const String& type, Sound::Service::Ptr service, Playlist::Item::ConversionResultNotification::Ptr result)
      : SelectedItems(items)
      , Type(type)
      , Service(service)
      , Result(result)
    {
    }

    virtual void Execute(const Playlist::Item::Storage& stor, Log::ProgressCallback& cb)
    {
      const std::size_t totalItems = SelectedItems ? SelectedItems->size() : stor.CountItems();
      ConvertVisitor visitor(totalItems, Type, Service, cb, Result);
      if (SelectedItems)
      {
        stor.ForSpecifiedItems(*SelectedItems, visitor);
      }
      else
      {
        stor.ForAllItems(visitor);
      }
      emit ResultAcquired(Result);
    }
  private:
    const Playlist::Model::IndexSetPtr SelectedItems;
    const String Type;
    const Sound::Service::Ptr Service;
    const Playlist::Item::ConversionResultNotification::Ptr Result;
  };

  // Exporting
  class ExportOperation : public Playlist::Item::TextResultOperation
                        , private Playlist::Item::Visitor
  {
  public:
    ExportOperation(const String& nameTemplate, Parameters::Accessor::Ptr params, Playlist::Item::ConversionResultNotification::Ptr result)
      : SelectedItems()
      , NameTemplate(IO::CreateFilenameTemplate(nameTemplate))
      , Params(params)
      , Result(result)
    {
    }

    ExportOperation(Playlist::Model::IndexSetPtr items, const String& nameTemplate, Parameters::Accessor::Ptr params, Playlist::Item::ConversionResultNotification::Ptr result)
      : SelectedItems(items)
      , NameTemplate(IO::CreateFilenameTemplate(nameTemplate))
      , Params(params)
      , Result(result)
    {
    }

    virtual void Execute(const Playlist::Item::Storage& stor, Log::ProgressCallback& cb)
    {
      ExecuteOperation(stor, SelectedItems, *this, cb);
      emit ResultAcquired(Result);
    }
  private:
    virtual void OnItem(Playlist::Model::IndexType /*index*/, Playlist::Item::Data::Ptr data)
    {
      const String path = data->GetFullPath();
      if (const Module::Holder::Ptr holder = data->GetModule())
      {
        ExportItem(path, *holder, *data->GetModuleData());
      }
      else
      {
        Result->AddFailedToOpen(path);
      }
    }

    void ExportItem(const String& path, const Module::Holder& item, const Binary::Data& content)
    {
      try
      {
        const Parameters::Accessor::Ptr props = item.GetModuleProperties();
        const String filename = NameTemplate->Instantiate(Parameters::FieldsSourceAdapter<Strings::SkipFieldsSource>(*props));
        Save(content, filename);
        Result->AddSucceed();
      }
      catch (const Error& err)
      {
        Result->AddFailedToConvert(path, err);
      }
    }

    void Save(const Binary::Data& data, const String& filename) const
    {
      const Binary::OutputStream::Ptr stream = IO::CreateStream(ToLocal(filename), *Params, Log::ProgressCallback::Stub());
      stream->ApplyData(data);
    }
  private:
    const Playlist::Model::IndexSetPtr SelectedItems;
    const Strings::Template::Ptr NameTemplate;
    const Parameters::Accessor::Ptr Params;
    const Playlist::Item::ConversionResultNotification::Ptr Result;
  };
  
  Parameters::Accessor::Ptr CreateSoundParameters(const Playlist::Item::Conversion::Options& opts)
  {
    const Parameters::Container::Ptr overriden = Parameters::Container::Create();
    overriden->SetValue(Parameters::ZXTune::Sound::Backends::File::FILENAME, opts.FilenameTemplate);
    overriden->SetValue(Parameters::ZXTune::Sound::LOOPED, 0);
    return Parameters::CreateMergedAccessor(overriden, opts.Params);
  }
}

namespace Playlist
{
  namespace Item
  {
    TextResultOperation::Ptr CreateSoundFormatConvertOperation(Playlist::Model::IndexSetPtr items,
      const String& type, Sound::Service::Ptr service, ConversionResultNotification::Ptr result)
    {
      return boost::make_shared<SoundFormatConvertOperation>(items, type, service, result);
    }

    TextResultOperation::Ptr CreateExportOperation(const String& nameTemplate, Parameters::Accessor::Ptr params, ConversionResultNotification::Ptr result)
    {
      return boost::make_shared<ExportOperation>(nameTemplate, params, result);
    }

    TextResultOperation::Ptr CreateExportOperation(Playlist::Model::IndexSetPtr items, const String& nameTemplate, Parameters::Accessor::Ptr params, ConversionResultNotification::Ptr result)
    {
      return boost::make_shared<ExportOperation>(items, nameTemplate, params, result);
    }

    TextResultOperation::Ptr CreateConvertOperation(Playlist::Model::IndexSetPtr items, const Conversion::Options& opts, ConversionResultNotification::Ptr result)
    {
      if (opts.Type.empty())
      {
        return CreateExportOperation(items, opts.FilenameTemplate, opts.Params, result);
      }
      else
      {
        const Parameters::Accessor::Ptr soundParams = CreateSoundParameters(opts);
        const Sound::Service::Ptr service = Sound::CreateFileService(soundParams);
        return CreateSoundFormatConvertOperation(items, opts.Type, service, result);
      }
    }

    TextResultOperation::Ptr CreateConvertOperation(const Conversion::Options& opts, ConversionResultNotification::Ptr result)
    {
      return CreateConvertOperation(Playlist::Model::IndexSetPtr(), opts, result);
    }
  }
}
