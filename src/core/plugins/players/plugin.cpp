/**
* 
* @file
*
* @brief  Player plugin implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "plugin.h"
#include "properties_helper.h"
#include "core/src/callback.h"
#include <core/plugins/plugins_types.h>
#include <core/plugin_attrs.h>
//common includes
#include <make_ptr.h>
//library includes
#include <core/module_attrs.h>

namespace ZXTune
{
  class CommonPlayerPlugin : public PlayerPlugin
  {
  public:
    CommonPlayerPlugin(Plugin::Ptr descr, Formats::Chiptune::Decoder::Ptr decoder, Module::Factory::Ptr factory)
      : Description(descr)
      , Decoder(decoder)
      , Factory(factory)
    {
    }

    virtual Plugin::Ptr GetDescription() const
    {
      return Description;
    }

    virtual Binary::Format::Ptr GetFormat() const
    {
      return Decoder->GetFormat();
    }

    virtual Analysis::Result::Ptr Detect(const Parameters::Accessor& params, DataLocation::Ptr inputData, const Module::DetectCallback& callback) const
    {
      const Binary::Container::Ptr data = inputData->GetData();
      if (Decoder->Check(*data))
      {
        const Parameters::Container::Ptr properties = Parameters::Container::Create();
        Module::PropertiesHelper props(*properties);
        props.SetType(Description->Id());
        props.SetContainer(inputData->GetPluginsChain()->AsString());
        if (const Module::Holder::Ptr holder = Factory->CreateModule(params, *data, properties))
        {
          callback.ProcessModule(inputData, Description, holder);
          Parameters::IntType usedSize = 0;
          properties->FindValue(Module::ATTR_SIZE, usedSize);
          return Analysis::CreateMatchedResult(static_cast<std::size_t>(usedSize));
        }
      }
      return Analysis::CreateUnmatchedResult(Decoder->GetFormat(), data);
    }

    virtual Module::Holder::Ptr Open(const Parameters::Accessor& params, const Binary::Container& data) const
    {
      if (Decoder->Check(data))
      {
        const Parameters::Container::Ptr properties = Parameters::Container::Create();
        Module::PropertiesHelper(*properties)
          .SetType(Description->Id());
        return Factory->CreateModule(params, data, properties);
      }
      return Module::Holder::Ptr();
    }
  private:
    const Plugin::Ptr Description;
    const Formats::Chiptune::Decoder::Ptr Decoder;
    const Module::Factory::Ptr Factory;
  };

  PlayerPlugin::Ptr CreatePlayerPlugin(const String& id, uint_t caps,
    Formats::Chiptune::Decoder::Ptr decoder, Module::Factory::Ptr factory)
  {
    const Plugin::Ptr description = CreatePluginDescription(id, decoder->GetDescription(), caps | Capabilities::Category::MODULE);
    return MakePtr<CommonPlayerPlugin>(description, decoder, factory);
  }
}
