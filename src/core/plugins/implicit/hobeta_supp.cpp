/*
Abstract:
  Hobeta implicit convertors support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

#include "../enumerator.h"

#include <byteorder.h>

#include <core/plugin.h>
#include <core/plugin_attrs.h>

#include <io/container.h>

#include <numeric>

#include <text/plugins.h>

#define FILE_TAG 1CF1A62A

namespace
{
  using namespace ZXTune;

  const Char HOBETA_PLUGIN_ID[] = {'H', 'o', 'b', 'e', 't', 'a', '\0'};
  const String TEXT_HOBETA_VERSION(FromChar("Revision: $Rev$"));

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
  PACK_PRE struct Header
  {
    uint8_t Filename[8];
    uint8_t Filetype[3];
    uint16_t Length;
    uint8_t Zero;
    uint8_t SizeInSectors;
    uint16_t CRC;
  } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

  BOOST_STATIC_ASSERT(sizeof(Header) == 17);
  const std::size_t HOBETA_MAX_SIZE = 0xff00;

  //////////////////////////////////////////////////////////////////////////
  void DescribeHobetaPlugin(PluginInformation& info)
  {
    info.Id = HOBETA_PLUGIN_ID;
    info.Description = TEXT_HOBETA_INFO;
    info.Version = TEXT_HOBETA_VERSION;
    info.Capabilities = 0;//TODO
  }

  bool ProcessHobeta(const IO::DataContainer& input, IO::DataContainer::Ptr& output, ModuleRegion& region)
  {
    const std::size_t limit(input.Size());
    const uint8_t* const data(static_cast<const uint8_t*>(input.Data()));
    const Header* const header(safe_ptr_cast<const Header*>(data));
    const unsigned dataSize(fromLE(header->Length));
    if (dataSize > HOBETA_MAX_SIZE ||
        dataSize + sizeof(*header) > limit)
    {
      return false;
    }
    if (fromLE(header->CRC) == ((105 + 257 * std::accumulate(data, data + 15, 0u)) & 0xffff))
    {
      region.Offset = 0;
      region.Size = dataSize + sizeof(*header);
      output = input.GetSubcontainer(sizeof(*header), dataSize);
      return true;
    }
    return false;
  }
}

namespace ZXTune
{
  void RegisterHobetaConvertor(PluginsEnumerator& enumerator)
  {
    PluginInformation info;
    DescribeHobetaPlugin(info);
    enumerator.RegisterImplicitPlugin(info, ProcessHobeta);
  }
}
