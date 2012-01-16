/*
Abstract:
  AY modules format implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

//local includes
#include "ay.h"
//common includes
#include <byteorder.h>
#include <contract.h>
#include <crc.h>
#include <range_checker.h>
#include <tools.h>
//library includes
#include <binary/typed_container.h>
#include <formats/chiptune.h>
//std includes
#include <cstring>
#include <list>
//boost includes
#include <boost/make_shared.hpp>

namespace Formats
{
namespace Chiptune
{
namespace AY
{
  const uint8_t SIGNATURE[] = {'Z', 'X', 'A', 'Y'};
#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
  PACK_PRE struct Header
  {
    uint8_t Signature[4];//ZXAY
    uint8_t Type[4];
    uint8_t FileVersion;
    uint8_t PlayerVersion;
    int16_t SpecialPlayerOffset;
    int16_t AuthorOffset;
    int16_t MiscOffset;
    uint8_t LastModuleIndex;
    uint8_t FirstModuleIndex;
    int16_t DescriptionsOffset;
  } PACK_POST;

  PACK_PRE struct ModuleDescription
  {
    int16_t TitleOffset;
    int16_t DataOffset;
  } PACK_POST;

  namespace EMUL
  {
    const uint8_t SIGNATURE[] = {'E', 'M', 'U', 'L'};

    PACK_PRE struct ModuleData
    {
      uint8_t AmigaChannelsMapping[4];
      uint16_t TotalLength;
      uint16_t FadeLength;
      uint16_t RegValue;
      int16_t PointersOffset;
      int16_t BlocksOffset;
    } PACK_POST;

    PACK_PRE struct ModulePointers
    {
      uint16_t SP;
      uint16_t InitAddr;
      uint16_t PlayAddr;
    } PACK_POST;

    PACK_PRE struct ModuleBlock
    {
      uint16_t Address;
      uint16_t Size;
      int16_t Offset;
    } PACK_POST;
  }
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

  BOOST_STATIC_ASSERT(sizeof(Header) == 0x14);

  class Parser
  {
  public:
    Parser(const Binary::Container& rawData)
      : RawData(rawData)
      , Delegate(rawData)
      , Ranges(RangeChecker::CreateShared(RawData.Size()))
      , Start(Delegate.GetField<uint8_t>(0))
      , Finish(Start + rawData.Size())
    {
    }

    template<class T>
    const T& GetField(std::size_t offset) const
    {
      const T* const res = Delegate.GetField<T>(offset);
      Require(res != 0);
      Require(Ranges->AddRange(offset, sizeof(T)));
      return *res;
    }

    template<class T>
    const T& PeekField(const int16_t* beField, std::size_t idx = 0) const
    {
      const uint8_t* const result = GetPointer(beField) + sizeof(T) * idx;
      return *safe_ptr_cast<const T*>(result);
    }

    template<class T>
    const T& GetField(const int16_t* beField, std::size_t idx = 0) const
    {
      const uint8_t* const result = GetPointer(beField) + sizeof(T) * idx;
      const std::size_t offset = result - Start;
      Require(Ranges->AddRange(offset, sizeof(T)));
      return *safe_ptr_cast<const T*>(result);
    }

    String GetString(const int16_t* beOffset) const
    {
      const uint8_t* const strStart = GetPointer(beOffset);
      const uint8_t* const strEnd = std::find(strStart, Finish, 0);
      Require(Ranges->AddRange(strStart - Start, strEnd - strStart + 1));
      return String(strStart, strEnd);
    }

    Binary::Container::Ptr GetBlob(const int16_t* beOffset, std::size_t size) const
    {
      const uint8_t* const ptr = GetPointerNocheck(beOffset);
      if (ptr < Start || ptr >= Finish)
      {
        return Binary::Container::Ptr();
      }
      const std::size_t offset = ptr - Start;
      const std::size_t maxSize = Finish - ptr;
      const std::size_t effectiveSize = std::min(size, maxSize);
      Require(Ranges->AddRange(offset, effectiveSize));
      return RawData.GetSubcontainer(offset, effectiveSize);
    }

    std::size_t GetSize() const
    {
      return Ranges->GetAffectedRange().second;
    }
  private:
    const uint8_t* GetPointerNocheck(const int16_t* beField) const
    {
      const int16_t relOffset = fromBE(*beField);
      return safe_ptr_cast<const uint8_t*>(beField) + relOffset;
    }

    const uint8_t* GetPointer(const int16_t* beField) const
    {
      const uint8_t* const result = GetPointerNocheck(beField);
      Require(result >= Start);
      return result;
    }
  private:
    const Binary::Container& RawData;
    const Binary::TypedContainer Delegate;
    const RangeChecker::Ptr Ranges;
    const uint8_t* const Start;
    const uint8_t* const Finish;
  };

  class Container : public Formats::Chiptune::Container
  {
  public:
    Container(Binary::Container::Ptr delegate, uint_t crc)
      : Delegate(delegate)
      , Checksum(crc)
    {
    }

    virtual std::size_t Size() const
    {
      return Delegate->Size();
    }

    virtual const void* Data() const
    {
      return Delegate->Data();
    }

    virtual Binary::Container::Ptr GetSubcontainer(std::size_t offset, std::size_t size) const
    {
      return Delegate->GetSubcontainer(offset, size);
    }

    virtual uint_t FixedChecksum() const
    {
      return Checksum;
    }
  private:
    const Binary::Container::Ptr Delegate;
    const uint_t Checksum;
  };

  class StubBuilder : public Builder
  {
  public:
    virtual void SetTitle(const String& /*title*/) {}
    virtual void SetAuthor(const String& /*author*/) {}
    virtual void SetComment(const String& /*comment*/) {}
    virtual void SetDuration(uint_t /*duration*/) {}
    virtual void SetRegisters(uint16_t /*reg*/, uint16_t /*sp*/) {}
    virtual void SetRoutines(uint16_t /*init*/, uint16_t /*play*/) {}
    virtual void AddBlock(uint16_t /*addr*/, const void* /*data*/, std::size_t /*size*/) {}
  };

  class MemoryDumpBuilder : public BlobBuilder
  {
    class Im1Player
    {
    public:
      Im1Player(uint16_t init, uint16_t introutine)
      {
        static const uint8_t PLAYER_TEMPLATE[] = 
        {
          0xf3, //di
          0xcd, 0, 0, //call init (+2)
          0xed, 0x56, //loop: im 1
          0xfb, //ei
          0x76, //halt
          0xcd, 0, 0, //call routine (+9)
          0x18, 0xf7 //jr loop
        };
        BOOST_STATIC_ASSERT(sizeof(Im1Player) == sizeof(PLAYER_TEMPLATE));
        std::copy(PLAYER_TEMPLATE, ArrayEnd(PLAYER_TEMPLATE), Data.begin());
        Data[0x2] = init & 0xff;
        Data[0x3] = init >> 8;
        Data[0x9] = introutine & 0xff; 
        Data[0xa] = introutine >> 8; //call routine
      }
    private:
      boost::array<uint8_t, 13> Data;
    };

    class Im2Player
    {
    public:
      explicit Im2Player(uint16_t init)
      {
        static const uint8_t PLAYER_TEMPLATE[] = 
        {
          0xf3, //di
          0xcd, 0, 0, //call init (+2)
          0xed, 0x5e, //loop: im 2
          0xfb, //ei
          0x76, //halt
          0x18, 0xfa //jr loop
        };
        BOOST_STATIC_ASSERT(sizeof(Im2Player) == sizeof(PLAYER_TEMPLATE));
        std::copy(PLAYER_TEMPLATE, ArrayEnd(PLAYER_TEMPLATE), Data.begin());
        Data[0x2] = init & 0xff;
        Data[0x3] = init >> 8;
      }
    private:
      boost::array<uint8_t, 10> Data;
    };
  public:
    virtual void SetTitle(const String& /*title*/)
    {
    }

    virtual void SetAuthor(const String& /*author*/)
    {
    }

    virtual void SetComment(const String& /*comment*/)
    {
    }

    virtual void SetDuration(uint_t /*duration*/)
    {
    }

    virtual void SetRegisters(uint16_t /*reg*/, uint16_t /*sp*/)
    {
    }

    virtual void SetRoutines(uint16_t init, uint16_t play)
    {
      assert(init);
      if (play)
      {
        Im1Player player(init, play);
        AddBlock(0, &player, sizeof(player));
      }
      else
      {
        Im2Player player(init);
        AddBlock(0, &player, sizeof(player));
      }
    }

    virtual void AddBlock(uint16_t addr, const void* src, std::size_t size)
    {
      Dump& data = AllocateData();
      const std::size_t toCopy = std::min(size, data.size() - addr);
      std::memcpy(&data[addr], src, toCopy);
    }

    virtual Binary::Container::Ptr Result() const
    {
      return Data
        ? Binary::CreateContainer(Data, 0, Data->size())
        : Binary::Container::Ptr();
    }
  private:
    Dump& AllocateData()
    {
      if (!Data)
      {
        Data.reset(new Dump(65536));
        InitializeBlock(0xc9, 0, 0x100);
        InitializeBlock(0xff, 0x100, 0x3f00);
        InitializeBlock(uint8_t(0x00), 0x4000, 0xc000);
        Data->at(0x38) = 0xfb;
      }
      return *Data;
    }

    void InitializeBlock(uint8_t src, std::size_t offset, std::size_t size)
    {
      Dump& data = *Data;
      const std::size_t toFill = std::min(size, data.size() - offset);
      std::memset(&data[offset], src, toFill);
    }
  private:
    boost::shared_ptr<Dump> Data;
  };

  class FileBuilder : public BlobBuilder
  {
    //as a container
    class VariableDump : public Dump
    {
    public:
      VariableDump()
      {
        reserve(1000000);
      }

      template<class T>
      T* Add(const T& obj)
      {
        return static_cast<T*>(Add(&obj, sizeof(obj)));
      }

      char* Add(const String& str)
      {
        return static_cast<char*>(Add(str.c_str(), str.size() + 1));
      }

      void* Add(const void* src, std::size_t srcSize)
      {
        const std::size_t prevSize = size();
        resize(prevSize + srcSize);
        void* const dst = &front() + prevSize;
        std::memcpy(dst, src, srcSize);
        return dst;
      }
    };

    template<class T>
    static void SetPointer(int16_t* ptr, const T obj)
    {
      BOOST_STATIC_ASSERT(boost::is_pointer<T>::value);
      const std::ptrdiff_t offset = safe_ptr_cast<const uint8_t*>(obj) - safe_ptr_cast<const uint8_t*>(ptr);
      assert(offset > 0);//layout data sequentally
      *ptr = fromBE<int16_t>(static_cast<uint16_t>(offset));
    }
  public:
    FileBuilder()
      : Duration()
      , Register(), StackPointer()
      , InitRoutine(), PlayRoutine()
    {
    }

    virtual void SetTitle(const String& title)
    {
      Title = title;
    }

    virtual void SetAuthor(const String& author)
    {
      Author = author;
    }

    virtual void SetComment(const String& comment)
    {
      Comment = comment;
    }

    virtual void SetDuration(uint_t duration)
    {
      Duration = static_cast<uint16_t>(duration);
    }

    virtual void SetRegisters(uint16_t reg, uint16_t sp)
    {
      Register = reg;
      StackPointer = sp;
    }

    virtual void SetRoutines(uint16_t init, uint16_t play)
    {
      InitRoutine = init;
      PlayRoutine = play;
    }

    virtual void AddBlock(uint16_t addr, const void* data, std::size_t size)
    {
      const uint8_t* const fromCopy = static_cast<const uint8_t*>(data);
      const std::size_t toCopy = std::min(size, std::size_t(0x10000 - addr));
      Blocks.push_back(BlocksList::value_type(addr, Dump(fromCopy, fromCopy + toCopy)));
    }

    virtual Binary::Container::Ptr Result() const
    {
      std::auto_ptr<VariableDump> result(new VariableDump());
      //init header
      Header* const header = result->Add(Header());
      std::copy(SIGNATURE, ArrayEnd(SIGNATURE), header->Signature);
      std::copy(EMUL::SIGNATURE, ArrayEnd(EMUL::SIGNATURE), header->Type);
      SetPointer(&header->AuthorOffset, result->Add(Author));
      SetPointer(&header->MiscOffset, result->Add(Comment));
      //init descr
      ModuleDescription* const descr = result->Add(ModuleDescription());
      SetPointer(&header->DescriptionsOffset, descr);
      SetPointer(&descr->TitleOffset, result->Add(Title));
      //init data
      EMUL::ModuleData* const data = result->Add(EMUL::ModuleData());
      SetPointer(&descr->DataOffset, data);
      data->TotalLength = fromBE(Duration);
      data->RegValue = fromBE(Register);
      //init pointers
      EMUL::ModulePointers* const pointers = result->Add(EMUL::ModulePointers());
      SetPointer(&data->PointersOffset, pointers);
      pointers->SP = fromBE(StackPointer);
      pointers->InitAddr = fromBE(InitRoutine);
      pointers->PlayAddr = fromBE(PlayRoutine);
      //init blocks
      std::list<EMUL::ModuleBlock*> blockPtrs;
      //all blocks + limiter
      for (uint_t block = 0; block != Blocks.size() + 1; ++block)
      {
        blockPtrs.push_back(result->Add(EMUL::ModuleBlock()));
      }
      SetPointer(&data->BlocksOffset, blockPtrs.front());
      //fill blocks
      for (BlocksList::const_iterator it = Blocks.begin(), lim = Blocks.end(); it != lim; ++it, blockPtrs.pop_front())
      {
        EMUL::ModuleBlock* const dst = blockPtrs.front();
        dst->Address = fromBE<uint16_t>(it->first);
        dst->Size = fromBE<uint16_t>(static_cast<uint16_t>(it->second.size()));
        SetPointer(&dst->Offset, result->Add(&it->second[0], it->second.size()));
      }
      return Binary::CreateContainer(std::auto_ptr<Dump>(result));
    }
  private:
    String Title;
    String Author;
    String Comment;
    uint16_t Duration;
    uint16_t Register;
    uint16_t StackPointer;
    uint16_t InitRoutine;
    uint16_t PlayRoutine;
    typedef std::list<std::pair<uint16_t, Dump> > BlocksList;
    BlocksList Blocks;
  };

  std::size_t GetModulesCount(const Binary::Container& rawData)
  {
    const Binary::TypedContainer data(rawData);
    if (const Header* header = data.GetField<Header>(0))
    {
      if (0 != std::memcmp(header->Signature, SIGNATURE, sizeof(SIGNATURE)))
      {
        return 0;
      }
      if (0 != std::memcmp(header->Type, EMUL::SIGNATURE, sizeof(EMUL::SIGNATURE)))
      {
        return 0;
      }
      return header->FirstModuleIndex <= header->LastModuleIndex
        ? header->LastModuleIndex + 1
        : 0;    
    }
    return 0;
  }

  Formats::Chiptune::Container::Ptr Parse(const Binary::Container& rawData, std::size_t idx, Builder& target)
  {
    if (idx >= GetModulesCount(rawData))
    {
      return Formats::Chiptune::Container::Ptr();
    }
    try
    {
      const Parser data(rawData);
      const Header& header = data.GetField<Header>(std::size_t(0));
      target.SetAuthor(data.GetString(&header.AuthorOffset));
      target.SetComment(data.GetString(&header.MiscOffset));
      const ModuleDescription& description = data.GetField<ModuleDescription>(&header.DescriptionsOffset, idx);
      target.SetTitle(data.GetString(&description.TitleOffset));

      const EMUL::ModuleData& moddata = data.GetField<EMUL::ModuleData>(&description.DataOffset);
      if (const uint_t duration = fromBE(moddata.TotalLength))
      {
        target.SetDuration(duration);
      }
      const EMUL::ModulePointers& modpointers = data.GetField<EMUL::ModulePointers>(&moddata.PointersOffset);
      target.SetRegisters(fromBE(moddata.RegValue), fromBE(modpointers.SP));
      const EMUL::ModuleBlock& firstBlock = data.GetField<EMUL::ModuleBlock>(&moddata.BlocksOffset);
      target.SetRoutines(fromBE(modpointers.InitAddr ? modpointers.InitAddr : firstBlock.Address), fromBE(modpointers.PlayAddr));
      uint32_t crc = 0;
      for (std::size_t blockIdx = 0; ; ++blockIdx)
      {
        if (!data.PeekField<uint16_t>(&moddata.BlocksOffset, 3 * blockIdx))
        {
          break;
        }
        const EMUL::ModuleBlock& block = data.GetField<EMUL::ModuleBlock>(&moddata.BlocksOffset, blockIdx);
        const uint16_t blockAddr = fromBE(block.Address);
        const std::size_t blockSize = fromBE(block.Size);
        if (Binary::Container::Ptr blockData = data.GetBlob(&block.Offset, blockSize))
        {
          target.AddBlock(blockAddr, blockData->Data(), blockData->Size());
          crc = Crc32(static_cast<const uint8_t*>(blockData->Data()), blockData->Size(), crc);
        }
      }
      const Binary::Container::Ptr containerData = rawData.GetSubcontainer(0, data.GetSize());
      return boost::make_shared<Container>(containerData, crc);
    }
    catch (const std::exception&)
    {
      return Formats::Chiptune::Container::Ptr();
    }
  }

  Builder& GetStubBuilder()
  {
    static StubBuilder stub;
    return stub;
  }

  BlobBuilder::Ptr CreateMemoryDumpBuilder()
  {
    return boost::make_shared<MemoryDumpBuilder>();
  }

  BlobBuilder::Ptr CreateFileBuilder()
  {
    return boost::make_shared<FileBuilder>();
  }
} //namespace AY
} //namespace Chiptune
} //namespace Formats
