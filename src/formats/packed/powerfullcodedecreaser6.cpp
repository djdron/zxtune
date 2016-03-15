/**
* 
* @file
*
* @brief  PverfullCodeDecreaser v6.x packer support
*
* @author vitamin.caig@gmail.com
*
* @note   Based on XLook sources by HalfElf
*
**/

//local includes
#include "container.h"
#include "hrust1_bitstream.h"
#include "pack_utils.h"
//common includes
#include <byteorder.h>
#include <make_ptr.h>
#include <pointers.h>
//library includes
#include <binary/format_factories.h>
#include <formats/packed.h>
//std includes
#include <algorithm>
#include <iterator>
//boost includes
#include <boost/range/end.hpp>
//text includes
#include <formats/text/packed.h>

namespace Formats
{
namespace Packed
{
  namespace PowerfullCodeDecreaser6
  {
    const std::size_t MAX_DECODED_SIZE = 0xc000;

    struct Version61
    {
      static const String DESCRIPTION;
      static const std::string DEPACKER_PATTERN;

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
      PACK_PRE struct RawHeader
      {
        //+0
        uint8_t Padding1[0xe];
        //+0xe
        uint16_t LastOfSrcPacked;
        //+0x10
        uint8_t Padding2;
        //+0x11
        uint16_t LastOfDstPacked;
        //+0x13
        uint8_t Padding3;
        //+0x14
        uint16_t SizeOfPacked;
        //+0x16
        uint8_t Padding4[0xb3];
        //+0xc9
        uint8_t LastBytes[3];
        uint8_t Bitstream[2];
      } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

      BOOST_STATIC_ASSERT(sizeof(RawHeader) == 0xc9 + 3 + 2);

      static const std::size_t MIN_SIZE = sizeof(RawHeader);
    };

    struct Version61i
    {
      static const String DESCRIPTION;
      static const std::string DEPACKER_PATTERN;

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
      PACK_PRE struct RawHeader
      {
        //+0
        uint8_t Padding1[0xe];
        //+0xe
        uint16_t LastOfSrcPacked;
        //+0x10
        uint8_t Padding2;
        //+0x11
        uint16_t LastOfDstPacked;
        //+0x13
        uint8_t Padding3;
        //+0x14
        uint16_t SizeOfPacked;
        //+0x16
        uint8_t Padding4[0xab];
        //+0xc1
        uint8_t LastBytes[3];
        uint8_t Bitstream[2];
      } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

      BOOST_STATIC_ASSERT(sizeof(RawHeader) == 0xc1 + 3 + 2);

      static const std::size_t MIN_SIZE = sizeof(RawHeader);
    };
    
    struct Version62
    {
      static const String DESCRIPTION;
      static const std::string DEPACKER_PATTERN;

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
      PACK_PRE struct RawHeader
      {
        //+0
        uint8_t Padding1[0x14];
        //+0x14
        uint16_t LastOfSrcPacked;
        //+0x16
        uint8_t Padding2;
        //+0x17
        uint16_t LastOfDstPacked;
        //+0x19
        uint8_t Padding3;
        //+0x1a
        uint16_t SizeOfPacked;
        //+0x1c
        uint8_t Padding4[0xa8];
        //+0xc4
        uint8_t LastBytes[5];
        uint8_t Bitstream[2];
      } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

      BOOST_STATIC_ASSERT(sizeof(RawHeader) == 0xc4 + 5 + 2);

      static const std::size_t MIN_SIZE = sizeof(RawHeader);
    };

    const String Version61::DESCRIPTION = Text::PCD61_DECODER_DESCRIPTION;
    const std::string Version61::DEPACKER_PATTERN =
      "?"       // di/nop
      "21??"    // ld hl,xxxx 0xc017   depacker src
      "11??"    // ld de,xxxx 0xbf00   depacker target
      "01b500"  // ld bc,xxxx 0x00b5   depacker size
      "d5"      // push de
      "edb0"    // ldir
      "21??"    // ld hl,xxxx 0xd845   last of src packed (data = +0xe)
      "11??"    // ld de,xxxx 0xffff   last of dst packed (data = +0x11)
      "01??"    // ld bc,xxxx 0x177d   packed size (data = +0x14)
      "c9"      // ret
      "ed?"     // lddr/ldir                +0x18
      "21??"    // ld hl,xxxx 0xe883   rest bytes src (data = +0x1a)
      "11??"    // ld de,xxxx 0xbfb2   rest bytes dst (data = +0x1d)
      "01??"    // ld bc,xxxx 0x0003   rest bytes count
      "d5"      // push de
      "c5"      // push bc
      "edb0"    // ldir
      "ed73??"  // ld (xxxx),sp 0xbfa3
      "f9"      // ld sp,hl
      "11??"    // ld de,xxxx   0xc000 target of depack (data = +0x2c)
      "60"      // ld h,b
      "d9"      // exx
      "011001"  // ld bc,xxxx ; 0x110
      "3ed9"    // ld a,x ;0xd9
      "1002"    // djnz xxx, (2)
      "e1"      // pop hl
      "41"      // ld b,c
      "29"      // add hl,hl
      "3007"    // jr nc,xx
      "3b"      // dec sp
      "f1"      // pop af
      "d9"      // exx
      "12"      // ld (de),a
      "13"      // inc de
      "18f1"    // jr ...
    ;

    const String Version61i::DESCRIPTION = Text::PCD61i_DECODER_DESCRIPTION;
    const std::string Version61i::DEPACKER_PATTERN =
      "?"       // di/nop
      "21??"    // ld hl,xxxx 0x7017   depacker src
      "11??"    // ld de,xxxx 0xc000   depacker target
      "01ad00"  // ld bc,xxxx 0x00ad   depacker size
      "d5"      // push de
      "edb0"    // ldir
      "21??"    // ld hl,xxxx 0x9ef5   last of src packed (data = +0xe)
      "11??"    // ld de,xxxx 0xb732   last of dst packed (data = +0x11)
      "01??"    // ld bc,xxxx 0x2e32   packed size (data = +0x14)
      "c9"      // ret
      "ed?"     // lddr/ldir                +0x18
      "ed73??"  // ld (xxxx),sp 0xc098
      "31??"    // ld sp,xxx 0x8901
      "11??"    // ld de,xxxx   0x7000 target of depack (data = +0x21)
      "60"      // ld h,b
      "d9"      // exx
      "011001"  // ld bc,xxxx ; 0x110
      "3ed9"    // ld a,x ;0xd9
      "1002"    // djnz xxx, (2)
      "e1"      // pop hl
      "41"      // ld b,c
      "29"      // add hl,hl
      "3007"    // jr nc,xx
      "3b"      // dec sp
      "f1"      // pop af
      "d9"      // exx
      "12"      // ld (de),a
      "13"      // inc de
      "18f1"    // jr ...
    ;
    
    const String Version62::DESCRIPTION = Text::PCD62_DECODER_DESCRIPTION;
    const std::string Version62::DEPACKER_PATTERN =
      "?"       // di/nop
      "21??"    // ld hl,xxxx 0x6026   depacker src
      "11??"    // ld de,xxxx 0x5b00   depacker target
      "01a300"  // ld bc,xxxx 0x00a3   depacker size
      "edb0"    // ldir
      "011001"  // ld bc,xxxx ; 0x110
      "d9"      // exx
      "22??"    // ld (xxxx),hl 0x5b97
      "21??"    // ld hl,xxxx 0xb244   last of src packed (data = +0x14)
      "11??"    // ld de,xxxx 0xfaa4   last of dst packed (data = +0x17)
      "01??"    // ld bc,xxxx 0x517c   packed size (data = +0x1a)
      "ed73??"  // ld (xxxx),sp 0x0000
      "31??"    // ld sp,xxxx   0xa929
      "c3??"    // jp xxxx 0x5b00
      "ed?"     // lddr/lddr           +0x26
      "11??"    // ld de,xxxx 0x6000   target of depack (data = +0x29)
      "60"      // ld h,b
      "d9"      // exx
      "1002"    // djnz xxx, (2)
      "e1"      // pop hl
      "41"      // ld b,c
      "29"      // add hl,hl
      "3007"    // jr nc,xx
      "3b"      // dec sp
      "f1"      // pop af
      "d9"      // exx
      "12"      // ld (de),a
      "13"      // inc de
      "18f1"    // jr ...
    ;

    template<class Version>
    class Container
    {
    public:
      Container(const void* data, std::size_t size)
        : Data(static_cast<const uint8_t*>(data))
        , Size(size)
      {
      }

      bool FastCheck() const
      {
        if (Size <= sizeof(typename Version::RawHeader))
        {
          return false;
        }
        const typename Version::RawHeader& header = GetHeader();
        if (fromLE(header.SizeOfPacked) <= sizeof(header.LastBytes))
        {
          return false;
        }
        return true;
      }

      const typename Version::RawHeader& GetHeader() const
      {
        assert(Size >= sizeof(typename Version::RawHeader));
        return *safe_ptr_cast<const typename Version::RawHeader*>(Data);
      }
      
      std::size_t GetSize() const
      {
        return Size;
      }
    private:
      const uint8_t* const Data;
      const std::size_t Size;
    };

    class Bitstream : public ByteStream
    {
    public:
      Bitstream(const uint8_t* data, std::size_t size)
        : ByteStream(data, size)
        , Bits(), Mask(0)
      {
      }

      uint_t GetBit()
      {
        if (!(Mask >>= 1))
        {
          Bits = GetLEWord();
          Mask = 0x8000;
        }
        return (Bits & Mask) != 0 ? 1 : 0;
      }

      uint_t GetBits(uint_t count)
      {
        uint_t result = 0;
        while (count--)
        {
          result = 2 * result | GetBit();
        }
        return result;
      }
    private:
      uint_t Bits;
      uint_t Mask;
    };

    class BitstreamDecoder
    {
    public:
      BitstreamDecoder(const uint8_t* data, std::size_t size)
        : Stream(data, size)
        , Result(new Dump())
        , Decoded(*Result)
      {
        assert(!Stream.Eof());
      }

      bool DecodeMainData()
      {
        while (GetSingleBytes() &&
               Decoded.size() < MAX_DECODED_SIZE)
        {
          const uint_t index = GetIndex();
          uint_t offset = 0;
          uint_t len = index + 1;
          if (0 == index)
          {
            offset = Stream.GetBits(4) + 1;
          }
          else if (1 == index)
          {
            offset = 1 + Stream.GetByte();
            if (0x100 == offset)
            {
              return true;
            }
          }
          else
          {
            len = GetLongLen(index);
            offset = GetLongOffset();
          }
          if (!CopyFromBack(offset, Decoded, len))
          {
            break;
          }
        }
        return false;
      }
      
      std::size_t GetUsedSize() const
      {
        return Stream.GetProcessedBytes();
      }
    private:
      bool GetSingleBytes()
      {
        while (Stream.GetBit())
        {
          Decoded.push_back(Stream.GetByte());
        }
        return !Stream.Eof();
      }

      uint_t GetIndex()
      {
        uint16_t idx = 0;
        for (uint_t bits = 3; bits == 3 && idx != 0xf;)
        {
          bits = Stream.GetBits(2);
          idx += bits;
        }
        return idx;
      }

      uint_t GetLongLen(uint_t idx)
      {
        uint_t len = idx;
        if (3 == len)
        {
          len = Stream.GetByte();
          if (len)
          {
            len += 0xf;
          }
          else
          {
            len = Stream.GetLEWord();
          }
        }
        if (2 == len)
        {
          len = 3;
        }
        return len;
      }

      uint_t GetLongOffset()
      {
        const uint_t loOffset = Stream.GetByte();
        uint8_t hiOffset = 0;
        if (Stream.GetBit())
        {
          do
          {
            hiOffset = 4 * hiOffset + Stream.GetBits(2);
          }
          while (Stream.GetBit());
          ++hiOffset;
        }
        return 256 * hiOffset + loOffset;
      }
    protected:
      Bitstream Stream;
      std::auto_ptr<Dump> Result;
      Dump& Decoded;
    };

    template<class Version>
    class DataDecoder : private BitstreamDecoder
    {
    public:
      explicit DataDecoder(const Container<Version>& container)
        : BitstreamDecoder(container.GetHeader().Bitstream, container.GetSize() - offsetof(typename Version::RawHeader, Bitstream))
        , IsValid(container.FastCheck())
        , Header(container.GetHeader())
      {
        if (IsValid && !Stream.Eof())
        {
          IsValid = DecodeData();
        }
      }

      std::auto_ptr<Dump> GetResult()
      {
        return IsValid
          ? Result
          : std::auto_ptr<Dump>();
      }
      
      std::size_t GetUsedSize() const
      {
        return offsetof(typename Version::RawHeader, Bitstream) + BitstreamDecoder::GetUsedSize();
      }
    private:
      bool DecodeData()
      {
        Decoded.reserve(2 * fromLE(Header.SizeOfPacked));

        if (!DecodeMainData())
        {
          return false;
        }
        std::copy(Header.LastBytes, boost::end(Header.LastBytes), std::back_inserter(Decoded));
        return true;
      }
    private:
      bool IsValid;
      const typename Version::RawHeader& Header;
    };
  }//namespace PowerfullCodeDecreaser6

  template<class Version>
  class PowerfullCodeDecreaser6Decoder : public Decoder
  {
  public:
    PowerfullCodeDecreaser6Decoder()
      : Depacker(Binary::CreateFormat(Version::DEPACKER_PATTERN, Version::MIN_SIZE))
    {
    }

    virtual String GetDescription() const
    {
      return Version::DESCRIPTION;
    }

    virtual Binary::Format::Ptr GetFormat() const
    {
      return Depacker;
    }

    virtual Container::Ptr Decode(const Binary::Container& rawData) const
    {
      if (!Depacker->Match(rawData))
      {
        return Container::Ptr();
      }
      const PowerfullCodeDecreaser6::Container<Version> container(rawData.Start(), rawData.Size());
      if (!container.FastCheck())
      {
        return Container::Ptr();
      }
      PowerfullCodeDecreaser6::DataDecoder<Version> decoder(container);
      return CreateContainer(decoder.GetResult(), decoder.GetUsedSize());
    }
  private:
    const Binary::Format::Ptr Depacker;
  };

  Decoder::Ptr CreatePowerfullCodeDecreaser61Decoder()
  {
    return MakePtr<PowerfullCodeDecreaser6Decoder<PowerfullCodeDecreaser6::Version61> >();
  }

  Decoder::Ptr CreatePowerfullCodeDecreaser61iDecoder()
  {
    return MakePtr<PowerfullCodeDecreaser6Decoder<PowerfullCodeDecreaser6::Version61i> >();
  }
  
  Decoder::Ptr CreatePowerfullCodeDecreaser62Decoder()
  {
    return MakePtr<PowerfullCodeDecreaser6Decoder<PowerfullCodeDecreaser6::Version62> >();
  }
}//namespace Packed
}//namespace Formats
