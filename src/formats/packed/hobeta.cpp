/**
* 
* @file
*
* @brief  Hobeta image support
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "formats/packed/container.h"
//common includes
#include <byteorder.h>
#include <make_ptr.h>
#include <pointers.h>
//library includes
#include <binary/format_factories.h>
#include <formats/packed.h>
#include <math/numeric.h>
//std includes
#include <array>
#include <functional>
#include <numeric>
//text includes
#include <formats/text/packed.h>

namespace Formats
{
namespace Packed
{
  namespace Hobeta
  {
#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct Header
    {
      std::array<uint8_t, 9> Filename;
      uint16_t Start;
      uint16_t Length;
      uint16_t FullLength;
      uint16_t CRC;
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(Header) == 17, "Invalid layout");
    const std::size_t MIN_SIZE = 0x100;
    const std::size_t MAX_SIZE = 0xff00;

    bool Check(Binary::View rawData)
    {
      const auto* header = rawData.As<Header>();
      if (!header)
      {
        return false;
      }
      const auto* data = rawData.As<uint8_t>();
      const std::size_t dataSize = fromLE(header->Length);
      const std::size_t fullSize = fromLE(header->FullLength);
      if (!Math::InRange(dataSize, MIN_SIZE, MAX_SIZE) ||
          dataSize + sizeof(*header) > rawData.Size() ||
          fullSize != Math::Align<std::size_t>(dataSize, 256) ||
          //check for valid name
          header->Filename.end() != std::find_if(header->Filename.begin(), header->Filename.end(),
            std::bind2nd(std::less<uint8_t>(), uint8_t(' ')))
          )
      {
        return false;
      }
      //check for crc
      if (fromLE(header->CRC) == ((105 + 257 * std::accumulate(data, data + 15, 0u)) & 0xffff))
      {
        return true;
      }
      return false;
    }

    const std::string FORMAT(
      //Filename
      "20-7a 20-7a 20-7a 20-7a 20-7a 20-7a 20-7a 20-7a 20-7a"
      //Start
      "??"
      //Length
      "?01-ff"
      //FullLength
      "0001-ff"
    );
  }//namespace Hobeta

  class HobetaDecoder : public Decoder
  {
  public:
    HobetaDecoder()
      : Format(Binary::CreateFormat(Hobeta::FORMAT, Hobeta::MIN_SIZE))
    {
    }

    String GetDescription() const override
    {
      return Text::HOBETA_DECODER_DESCRIPTION;
    }

    Binary::Format::Ptr GetFormat() const override
    {
      return Format;
    }

    Formats::Packed::Container::Ptr Decode(const Binary::Container& rawData) const override
    {
      const Binary::View data(rawData);
      if (!Format->Match(data))
      {
        return Formats::Packed::Container::Ptr();
      }
      if (!Hobeta::Check(data))
      {
        return Formats::Packed::Container::Ptr();
      }
      const auto* header = data.As<Hobeta::Header>();
      const std::size_t dataSize = fromLE(header->Length);
      const std::size_t fullSize = fromLE(header->FullLength);
      auto subdata = rawData.GetSubcontainer(sizeof(*header), dataSize);
      return CreateContainer(std::move(subdata), fullSize + sizeof(*header));
    }
  private:
    const Binary::Format::Ptr Format;
  };

  Decoder::Ptr CreateHobetaDecoder()
  {
    return MakePtr<HobetaDecoder>();
  }
}//namespace Packed
}//namespace Formats
