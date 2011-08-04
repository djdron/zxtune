/*
Abstract:
  LZS support

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
  (C) Based on XLook sources by HalfElf
*/

//local includes
#include "pack_utils.h"
//common includes
#include <byteorder.h>
#include <detector.h>
#include <tools.h>
//library includes
#include <formats/packed.h>
//std includes
#include <algorithm>
#include <iterator>

namespace LZS
{
  const std::size_t MAX_DECODED_SIZE = 0xc000;

  const std::string DEPACKER_PATTERN(
    "cd??"    // call xxxx
    "?"       // di/nop
    "ed73??"  // ld (xxxx),sp
    "21??"    // ld hl,xxxx
    "11??"    // ld de,xxxx
    "01??"    // ld bc,xxxx
    "d5"      // push de
    "edb0"    // ldir
    "21??"    // ld hl,xxxx ;src of packed (data = +x15)
    "11??"    // ld de,xxxx ;dst of packed (data = +x18)
    "01??"    // ld bc,xxxx ;size of packed. (data = +x1b)
    "c9"      // ret
    //+1e
    "ed?"     // lddr/ldir  ;+x1f
    "21??"    // ld hl,xxxx ;src of packed (data = +x21)
    "11??"    // ld de,xxxx ;target to depack (data = +0x24)
    "06?"     // ld b,xx (0)
    "7e"      // ld a,(hl)
    "cb7f"    // bit 7,a
    "201d"    // jr nz,...
    "e6?"     // and xx (0xf)
    "47"      // ld b,a
    "ed6f"    // rld
    "c6?"     // add a,xx (3)
    "4f"      // ld c,a
    "23"      // inc hl
    "7b"      // ld a,e
    "96"      // sub (hl)
    "23"      // inc hl
    "f9"      // ld sp,hl
    "66"      // ld h,(hl)
    "6f"      // ld l,a
    "7a"      // ld a,d
    "98"      // sbc a,b
    "44"      // ld b,h
    "67"      // ld h,a
    "78"      // ld a,b
    "06?"     // ld b,xx (0)
    "edb0"    // ldir
    "60"      // ld h,b
    "69"      // ld l,c
    "39"      // add hl,sp
    "18df"    // jr ...
  /*
    "e6?"     // and xx (0x7f)
    "2819"    // jr z,...
    "23"      // inc hl
    "cb77"    // bit 6,a
    "2005"    // jr nz,...
    "4f"      // ld c,a
    "edb0"    // ldir
    "18d0"    // jr ...
    "e6?"     // and xx, (0x3f)
    "c6?"     // add a,xx (3)
    "47"      // ld b,a
    "7e"      // ld a,(hl)
    "23"      // inc hl
    "4e"      // ld c,(hl)
    "12"      // ld (de),a
    "13"      // inc de
    "10fc"    // djnz ...
    "79"      // ld a,c
    "18c2"    // jr ...
    "31??"    // ld sp,xxxx
    "06?"     // ld b,xx (3)
    "e1"      // pop hl
    "3b"      // dec sp
    "f1"      // pop af
    "77"      // ld (hl),a
    "10fa"    // djnz ...
    "31??"    // ld sp,xxxx
    "?"       // di/ei
    "c3??"    // jp xxxx (0x0052)
    */
  );

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
  PACK_PRE struct RawHeader
  {
    //+0
    uint8_t Padding1[0x15];
    //+0x15
    uint16_t PackedSource;
    //+0x17
    uint8_t Padding2;
    //+0x18
    uint16_t PackedTarget;
    //+0x1a
    uint8_t Padding3;
    //+0x1b
    uint16_t SizeOfPacked;
    //+0x1c
    uint8_t Padding4[2];
    //+0x1f
    uint8_t PackedDataCopyDirection;
    //+0x20
    uint8_t Padding5;
    //+0x21
    uint16_t FirstOfPacked;
    //+0x23
    uint8_t Padding6[0x5f];
    //+0x82
    uint8_t Data[1];
    //+0x83
  } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

  BOOST_STATIC_ASSERT(sizeof(RawHeader) == 0x83);

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
      if (Size < sizeof(RawHeader))
      {
        return false;
      }
      const RawHeader& header = GetHeader();
      const DataMovementChecker checker(fromLE(header.PackedSource), fromLE(header.PackedTarget), fromLE(header.SizeOfPacked), header.PackedDataCopyDirection);
      if (!checker.IsValid())
      {
        return false;
      }
      if (checker.FirstOfMovedData() != fromLE(header.FirstOfPacked))
      {
        return false;
      }
      const uint_t usedSize = GetUsedSize();
      if (usedSize > Size)
      {
        return false;
      }
      return true;
    }

    uint_t GetUsedSize() const
    {
      const RawHeader& header = GetHeader();
      return sizeof(header) + fromLE(header.SizeOfPacked) - sizeof(header.Data);
    }

    const RawHeader& GetHeader() const
    {
      assert(Size >= sizeof(RawHeader));
      return *safe_ptr_cast<const RawHeader*>(Data);
    }
  private:
    const uint8_t* const Data;
    const std::size_t Size;
  };

  class DataDecoder
  {
  public:
    explicit DataDecoder(const Container& container)
      : IsValid(container.FastCheck())
      , Header(container.GetHeader())
      , Stream(Header.Data, fromLE(Header.SizeOfPacked))
    {
      assert(IsValid && !Stream.Eof());
    }

    Dump* GetDecodedData()
    {
      if (IsValid && !Stream.Eof())
      {
        IsValid = DecodeData();
      }
      return IsValid ? &Decoded : 0;
    }
  private:
    bool DecodeData()
    {
      // The main concern is to decode data as much as possible, skipping defenitely invalid structure
      Decoded.reserve(2 * fromLE(Header.SizeOfPacked));
      //assume that first byte always exists due to header format
      while (!Stream.Eof() && Decoded.size() < MAX_DECODED_SIZE)
      {
        const uint_t data = Stream.GetByte();
        if (0x80 == data)
        {
          //exit
          break;
        }
        //at least one more byte required
        if (Stream.Eof())
        {
          return false;
        }
        const uint_t code = data & 0xc0;
        if (0x80 == code)
        {
          uint_t len = data & 0x3f;
          assert(len);
          for (; len && !Stream.Eof(); --len)
          {
            Decoded.push_back(Stream.GetByte());
          }
          if (len)
          {
            return false;
          }
        }
        else if (0xc0 == code)
        {
          const std::size_t len = (data & 0x3f) + 3;
          const uint8_t data = Stream.GetByte();
          std::fill_n(std::back_inserter(Decoded), len, data);
        }
        else
        {
          const std::size_t len = ((data & 0xf0) >> 4) + 3;
          const uint_t offset = 256 * (data & 0x0f) + Stream.GetByte();
          if (!CopyFromBack(offset, Decoded, len))
          {
            return false;
          }
        }
      }
      while (!Stream.Eof())
      {
        Decoded.push_back(Stream.GetByte());
      }
      return true;
    }
  private:
    bool IsValid;
    const RawHeader& Header;
    ByteStream Stream;
    Dump Decoded;
  };
}

namespace Formats
{
  namespace Packed
  {
    class LZSDecoder : public Decoder
    {
    public:
      LZSDecoder()
        : Depacker(DataFormat::Create(LZS::DEPACKER_PATTERN))
      {
      }

      virtual DataFormat::Ptr GetFormat() const
      {
        return Depacker;
      }

      virtual bool Check(const void* data, std::size_t availSize) const
      {
        const LZS::Container container(data, availSize);
        return container.FastCheck() && Depacker->Match(data, availSize);
      }

      virtual std::auto_ptr<Dump> Decode(const void* data, std::size_t availSize, std::size_t& usedSize) const
      {
        const LZS::Container container(data, availSize);
        if (!container.FastCheck() || !Depacker->Match(data, availSize))
        {
          return std::auto_ptr<Dump>();
        }
        LZS::DataDecoder decoder(container);
        if (Dump* decoded = decoder.GetDecodedData())
        {
          usedSize = container.GetUsedSize();
          std::auto_ptr<Dump> res(new Dump());
          res->swap(*decoded);
          return res;
        }
        return std::auto_ptr<Dump>();
      }
    private:
      const DataFormat::Ptr Depacker;
    };

    Decoder::Ptr CreateLZSDecoder()
    {
      return Decoder::Ptr(new LZSDecoder());
    }
  }
}
