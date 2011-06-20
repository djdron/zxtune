/*
Abstract:
  CompressorCode v3 convertors support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "extraction_result.h"
#include <core/plugins/registrator.h>
//library includes
#include <core/plugin_attrs.h>
#include <formats/packed_decoders.h>
#include <io/container.h>
//boost includes
#include <boost/enable_shared_from_this.hpp>
//text includes
#include <core/text/plugins.h>

namespace
{
  using namespace ZXTune;

  const Char CC4_PLUGIN_ID[] = {'C', 'C', '4', '\0'};
  const Char CC4PLUS_PLUGIN_ID[] = {'C', 'C', '4', 'P', 'L', 'U', 'S', '\0'};
  const String CC4_PLUGIN_VERSION(FromStdString("$Rev$"));

  class CC4Plugin : public ArchivePlugin
                  , public boost::enable_shared_from_this<CC4Plugin>
  {
  public:
    CC4Plugin()
      : Decoder(Formats::Packed::CreateCompressorCode4Decoder())
    {
    }

    virtual String Id() const
    {
      return CC4_PLUGIN_ID;
    }

    virtual String Description() const
    {
      return Text::CC4_PLUGIN_INFO;
    }

    virtual String Version() const
    {
      return CC4_PLUGIN_VERSION;
    }

    virtual uint_t Capabilities() const
    {
      return CAP_STOR_CONTAINER;
    }

    virtual DetectionResult::Ptr Detect(DataLocation::Ptr inputData, const Module::DetectCallback& callback) const
    {
      return DetectModulesInArchive(shared_from_this(), *Decoder, inputData, callback);
    }

    virtual DataLocation::Ptr Open(const Parameters::Accessor& /*parameters*/,
                                   DataLocation::Ptr inputData,
                                   const DataPath& pathToOpen) const
    {
      return OpenDataFromArchive(shared_from_this(), *Decoder, inputData, pathToOpen);
    }
  private:
    const Formats::Packed::Decoder::Ptr Decoder;
  };

  class CC4PlusPlugin : public ArchivePlugin
                      , public boost::enable_shared_from_this<CC4PlusPlugin>
  {
  public:
    CC4PlusPlugin()
      : Decoder(Formats::Packed::CreateCompressorCode4PlusDecoder())
    {
    }

    virtual String Id() const
    {
      return CC4PLUS_PLUGIN_ID;
    }

    virtual String Description() const
    {
      return Text::CC4PLUS_PLUGIN_INFO;
    }

    virtual String Version() const
    {
      return CC4_PLUGIN_VERSION;
    }

    virtual uint_t Capabilities() const
    {
      return CAP_STOR_CONTAINER;
    }

    virtual DetectionResult::Ptr Detect(DataLocation::Ptr inputData, const Module::DetectCallback& callback) const
    {
      return DetectModulesInArchive(shared_from_this(), *Decoder, inputData, callback);
    }

    virtual DataLocation::Ptr Open(const Parameters::Accessor& /*parameters*/,
                                   DataLocation::Ptr inputData,
                                   const DataPath& pathToOpen) const
    {
      return OpenDataFromArchive(shared_from_this(), *Decoder, inputData, pathToOpen);
    }
  private:
    const Formats::Packed::Decoder::Ptr Decoder;
  };
}

namespace ZXTune
{
  void RegisterCC4Convertor(PluginsRegistrator& registrator)
  {
    const ArchivePlugin::Ptr plugin(new CC4Plugin());
    registrator.RegisterPlugin(plugin);
    const ArchivePlugin::Ptr pluginPlus(new CC4PlusPlugin());
    registrator.RegisterPlugin(pluginPlus);
  }
}
