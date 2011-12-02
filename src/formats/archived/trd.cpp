/*
Abstract:
  TRD containers support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "trdos_catalogue.h"
#include "trdos_utils.h"
//common includes
#include <byteorder.h>
#include <logging.h>
#include <range_checker.h>
#include <tools.h>
//std includes
#include <cstring>
#include <numeric>
//boost includes
#include <boost/make_shared.hpp>
//text include
#include <formats/text/archived.h>

namespace TRD
{
  using namespace Formats;

  const std::string FORMAT(
    "+2048+"//skip to service sector
    "+227+"//skip zeroes in service sector
    "16"            //type DS_DD
    "?"             //files
    "?%00000xxx"    //free sectors
    "10"            //ID
  );

  const std::string THIS_MODULE("Formats::Archived::TRD");

  //hints
  const std::size_t MODULE_SIZE = 655360;
  const uint_t BYTES_PER_SECTOR = 256;
  const uint_t SECTORS_IN_TRACK = 16;
  const uint_t MAX_FILES_COUNT = 128;
  const uint_t SERVICE_SECTOR_NUM = 8;

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif

  enum
  {
    NOENTRY = 0,
    DELETED = 1
  };
  PACK_PRE struct CatEntry
  {
    char Name[8];
    char Type[3];
    uint16_t Length;
    uint8_t SizeInSectors;
    uint8_t Sector;
    uint8_t Track;
  } PACK_POST;

  enum
  {
    TRDOS_ID = 0x10,

    DS_DD = 0x16,
    DS_SD = 0x17,
    SS_DD = 0x18,
    SS_SD = 0x19
  };

  PACK_PRE struct ServiceSector
  {
    uint8_t Zero;
    uint8_t Reserved1[224];
    uint8_t FreeSpaceSect;
    uint8_t FreeSpaceTrack;
    uint8_t Type;
    uint8_t Files;
    uint16_t FreeSectors;
    uint8_t ID;//0x10
    uint8_t Reserved2[12];
    uint8_t DeletedFiles;
    uint8_t Title[8];
    uint8_t Reserved3[3];
  } PACK_POST;

#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

  BOOST_STATIC_ASSERT(sizeof(CatEntry) == 16);
  BOOST_STATIC_ASSERT(sizeof(ServiceSector) == 256);

  const Char TRACK0_FILENAME[] = {'$', 'T', 'r', 'a', 'c', 'k', '0', 0};
  const Char HIDDEN_FILENAME[] = {'$', 'H', 'i', 'd', 'd', 'e', 'n', 0};
  const Char UNALLOCATED_FILENAME[] = {'$', 'U', 'n', 'a', 'l', 'l', 'o', 'c', 'a', 't', 'e', 'd', 0};

  class Visitor
  {
  public:
    virtual ~Visitor() {}

    virtual void OnFile(const String& name, std::size_t offset, std::size_t size) = 0;
  };

  bool Parse(const Binary::Container& data, Visitor& visitor)
  {
    if (data.Size() < MODULE_SIZE)
    {
      return false;
    }
    const ServiceSector* const sector = safe_ptr_cast<const ServiceSector*>(data.Data()) + SERVICE_SECTOR_NUM;
    if (sector->ID != TRDOS_ID || sector->Type != DS_DD || 0 != sector->Zero)
    {
      return false;
    }
    const CatEntry* catEntry = safe_ptr_cast<const CatEntry*>(data.Data());

    const std::size_t totalSectors = MODULE_SIZE / BYTES_PER_SECTOR;
    std::vector<bool> usedSectors(totalSectors);
    std::fill_n(usedSectors.begin(), SECTORS_IN_TRACK, true);
    uint_t idx = 0;
    for (; idx != MAX_FILES_COUNT && NOENTRY != catEntry->Name[0]; ++idx, ++catEntry)
    {
      if (!catEntry->SizeInSectors)
      {
        continue;
      }
      const uint_t offset = SECTORS_IN_TRACK * catEntry->Track + catEntry->Sector;
      const uint_t size = catEntry->SizeInSectors;
      if (offset + size > totalSectors)
      {
        return false;//out of bounds
      }
      const std::vector<bool>::iterator begin = usedSectors.begin() + offset;
      const std::vector<bool>::iterator end = begin + size;
      if (end != std::find(begin, end, true))
      {
        return false;//overlap
      }
      std::fill(begin, end, true);
      String entryName = TRDos::GetEntryName(catEntry->Name, catEntry->Type);
      if (DELETED == catEntry->Name[0])
      {
        entryName.insert(0, 1, '~');
      }
      visitor.OnFile(entryName, offset * BYTES_PER_SECTOR, size * BYTES_PER_SECTOR);
    }
    if (!idx)
    {
      return false;
    }
    visitor.OnFile(TRACK0_FILENAME, 0, SECTORS_IN_TRACK * BYTES_PER_SECTOR);

    const std::vector<bool>::iterator begin = usedSectors.begin();
    const std::vector<bool>::iterator limit = usedSectors.end();
    const std::size_t freeArea = (SECTORS_IN_TRACK * sector->FreeSpaceTrack + sector->FreeSpaceSect);
    if (freeArea > SECTORS_IN_TRACK && freeArea < totalSectors)
    {
      const std::vector<bool>::iterator freeBegin = usedSectors.begin() + freeArea;
      if (usedSectors.end() == std::find(freeBegin, limit, true))
      {
        std::fill(freeBegin, limit, true);
        visitor.OnFile(UNALLOCATED_FILENAME, freeArea * BYTES_PER_SECTOR, (totalSectors - freeArea) * BYTES_PER_SECTOR);
      }
    }
    for (std::vector<bool>::iterator empty = std::find(begin, limit, false); 
         empty != limit;
         )
    {
      const std::vector<bool>::iterator emptyEnd = std::find(empty, limit, true);
      const std::size_t offset = BYTES_PER_SECTOR * (empty - begin);
      const std::size_t size = BYTES_PER_SECTOR * (emptyEnd - empty);
      visitor.OnFile(HIDDEN_FILENAME, offset, size);
      empty = std::find(emptyEnd, limit, false);
    }
    return true;
  }

  class StubVisitor : public Visitor
  {
  public:
    virtual void OnFile(const String& /*filename*/, std::size_t /*offset*/, std::size_t /*size*/) {}
  };

  class BuildVisitorAdapter : public Visitor
  {
  public:
    explicit BuildVisitorAdapter(TRDos::CatalogueBuilder& builder)
      : Builder(builder)
    {
    }

    virtual void OnFile(const String& filename, std::size_t offset, std::size_t size)
    {
      const TRDos::File::Ptr file = TRDos::File::CreateReference(filename, offset, size);
      Builder.AddFile(file);
    }
  private:
    TRDos::CatalogueBuilder& Builder;
  };
}

namespace Formats
{
  namespace Archived
  {
    class TRDDecoder : public Decoder
    {
    public:
      TRDDecoder()
        : Format(Binary::Format::Create(TRD::FORMAT))
      {
      }

      virtual String GetDescription() const
      {
        return Text::TRD_DECODER_DESCRIPTION;
      }

      virtual Binary::Format::Ptr GetFormat() const
      {
        return Format;
      }

      virtual bool Check(const Binary::Container& data) const
      {
        static TRD::StubVisitor STUB;
        return TRD::Parse(data, STUB);
      }

      virtual Container::Ptr Decode(const Binary::Container& data) const
      {
        const TRDos::CatalogueBuilder::Ptr builder = TRDos::CatalogueBuilder::CreateFlat();
        TRD::BuildVisitorAdapter visitor(*builder);
        if (TRD::Parse(data, visitor))
        {
          builder->SetRawData(data.GetSubcontainer(0, TRD::MODULE_SIZE));
          return builder->GetResult();
        }
        return Container::Ptr();
      }
    private:
      const Binary::Format::Ptr Format;
    };

    Decoder::Ptr CreateTRDDecoder()
    {
      return boost::make_shared<TRDDecoder>();
    }
  }
}

