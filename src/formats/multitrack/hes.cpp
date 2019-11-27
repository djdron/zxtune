/**
* 
* @file
*
* @brief  HES support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <byteorder.h>
#include <contract.h>
#include <make_ptr.h>
#include <pointers.h>
//library includes
#include <binary/container_base.h>
#include <binary/container_factories.h>
#include <binary/crc.h>
#include <binary/format_factories.h>
#include <formats/multitrack.h>
#include <math/numeric.h>
//std includes
#include <array>
#include <cstring>
#include <utility>

namespace Formats
{
namespace Multitrack
{
  namespace HES
  {
    typedef std::array<uint8_t, 4> SignatureType;
    
    const SignatureType SIGNATURE = {{'H', 'E', 'S', 'M'}};

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct RawHeader
    {
      SignatureType Signature;
      uint8_t Version;
      uint8_t StartSong;
      uint16_t RequestAddress;
      uint8_t InitialMPR[8];
      SignatureType DataSignature;
      uint32_t DataSize;
      uint32_t DataAddress;
      uint32_t Unused;
      //uint8_t Unused2[0x20];
      //uint8_t Fields[0x90];
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(RawHeader) == 0x20, "Invalid layout");

    const std::string FORMAT =
        "'H'E'S'M" //signature
        "?"        //version
        "?"        //start song
        "??"       //requested address
        "?{8}"     //MPR
        "'D'A'T'A" //data signature
        "? ? 0x 00"//1MB size limit
        "? ? 0x 00"//1MB size limit
     ;

    const std::size_t MIN_SIZE = 256;
    
    const uint_t TOTAL_TRACKS_COUNT = 32;
     
    const RawHeader* GetHeader(Binary::DataView rawData)
    {
      if (rawData.Size() < MIN_SIZE)
      {
        return nullptr;
      }
      const RawHeader* hdr = safe_ptr_cast<const RawHeader*>(rawData.Start());
      if (hdr->Signature != SIGNATURE)
      {
        return nullptr;
      }
      return hdr;
    }
    
    class Container : public Binary::BaseContainer<Multitrack::Container>
    {
    public:
      Container(const RawHeader* hdr, Binary::Container::Ptr data)
        : BaseContainer(std::move(data))
        , Hdr(hdr)
      {
      }
      
      uint_t FixedChecksum() const override
      {
        return Binary::Crc32(*Delegate);
      }

      uint_t TracksCount() const override
      {
        return TOTAL_TRACKS_COUNT;
      }

      uint_t StartTrackIndex() const override
      {
        return Hdr->StartSong;
      }
      
      Container::Ptr WithStartTrackIndex(uint_t idx) const override
      {
        std::unique_ptr<Dump> content(new Dump(Delegate->Size()));
        std::memcpy(content->data(), Delegate->Start(), content->size());
        RawHeader* const hdr = safe_ptr_cast<RawHeader*>(content->data());
        Require(idx < TOTAL_TRACKS_COUNT);
        hdr->StartSong = idx;
        return MakePtr<Container>(hdr, Binary::CreateContainer(std::move(content)));
      }
    private:
      const RawHeader* const Hdr;
    };

    class Decoder : public Formats::Multitrack::Decoder
    {
    public:
      //Use match only due to lack of end detection
      Decoder()
        : Format(Binary::CreateMatchOnlyFormat(FORMAT, MIN_SIZE))
      {
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Multitrack::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        if (const RawHeader* hdr = GetHeader(rawData))
        {
          const std::size_t totalSize = sizeof(*hdr) + fromLE(hdr->DataSize);
          //GME support truncated files
          const std::size_t realSize = std::min(rawData.Size(), totalSize);
          const Binary::Container::Ptr used = rawData.GetSubcontainer(0, realSize);
          return MakePtr<Container>(hdr, used);
        }
        else
        {
          return Formats::Multitrack::Container::Ptr();
        }
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }//namespace HES

  Decoder::Ptr CreateHESDecoder()
  {
    return MakePtr<HES::Decoder>();
  }
}//namespace Multitrack
}//namespace Formats
