/*
Abstract:
  Rar convertors support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "container_supp_common.h"
#include "core/plugins/registrator.h"
//library includes
#include <core/plugin_attrs.h>
#include <formats/archived_decoders.h>
//text includes
#include <core/text/plugins.h>

namespace
{
  using namespace ZXTune;

  const Char ID[] = {'R', 'A', 'R', '\0'};
  const Char* const INFO = Text::RAR_PLUGIN_INFO;
  const uint_t CAPS = CAP_STOR_MULTITRACK | CAP_STOR_DIRS;
}

namespace ZXTune
{
  void RegisterRarContainer(PluginsRegistrator& registrator)
  {
    const Formats::Archived::Decoder::Ptr decoder = Formats::Archived::CreateRarDecoder();
    const ArchivePlugin::Ptr plugin = CreateContainerPlugin(ID, INFO, CAPS, decoder);
    registrator.RegisterPlugin(plugin);
  }
}
