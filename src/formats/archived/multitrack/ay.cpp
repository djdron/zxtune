/**
* 
* @file
*
* @brief  AY/EMUL containers support
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <formats/archived/decoders.h>
#include <formats/chiptune/emulation/ay.h>
//std includes
#include <sstream>
//text includes
#include <formats/text/archived.h>

namespace Formats
{
namespace Archived
{
  namespace MultiAY
  {
    class File : public Archived::File
    {
    public:
      File(const String& name, Binary::Container::Ptr data)
        : Name(name)
        , Data(data)
      {
      }

      virtual String GetName() const
      {
        return Name;
      }

      virtual std::size_t GetSize() const
      {
        return Data->Size();
      }

      virtual Binary::Container::Ptr GetData() const
      {
        return Data;
      }
    private:
      const String Name;
      const Binary::Container::Ptr Data;
    };

    class Filename
    {
    public:
      Filename(const String& prefix, const String& value)
        : Str(value)
        , Valid(false)
        , Index()
      {
        if (0 == value.compare(0, prefix.size(), prefix))
        {
          std::basic_istringstream<Char> str(value.substr(prefix.size()));
          Valid = !!(str >> Index);
        }
      }

      Filename(const String& prefix, uint_t index)
        : Valid(true)
        , Index(index)
      {
        std::basic_ostringstream<Char> str;
        str << prefix << index;
        Str = str.str();
      }

      bool IsValid() const
      {
        return Valid;
      }

      uint_t GetIndex() const
      {
        return Index;
      }

      String ToString() const
      {
        return Str;
      }
    private:
      String Str;
      bool Valid;
      uint_t Index;
    };

    class Container : public Archived::Container
    {
    public:
      explicit Container(Binary::Container::Ptr data)
        : Delegate(data)
      {
      }

      //Binary::Container
      virtual const void* Start() const
      {
        return Delegate->Start();
      }

      virtual std::size_t Size() const
      {
        return Delegate->Size();
      }

      virtual Binary::Container::Ptr GetSubcontainer(std::size_t offset, std::size_t size) const
      {
        return Delegate->GetSubcontainer(offset, size);
      }

      //Container
      virtual void ExploreFiles(const Container::Walker& walker) const
      {
        for (uint_t idx = 0, total = CountFiles(); idx < total; ++idx)
        {
          const Formats::Chiptune::AY::BlobBuilder::Ptr builder = Formats::Chiptune::AY::CreateFileBuilder();
          if (const Formats::Chiptune::Container::Ptr parsed = Formats::Chiptune::AY::Parse(*Delegate, idx, *builder))
          {
            const String subPath = Filename(Text::MULTITRACK_FILENAME_PREFIX, idx).ToString();
            const Binary::Container::Ptr subData = builder->Result();
            const File file(subPath, subData);
            walker.OnFile(file);
          }
        }
      }

      virtual File::Ptr FindFile(const String& name) const
      {
        const Filename rawName(Text::AY_RAW_FILENAME_PREFIX, name);
        const Filename ayName(Text::MULTITRACK_FILENAME_PREFIX, name);
        if (!rawName.IsValid() && !ayName.IsValid())
        {
          return File::Ptr();
        }
        const uint_t index = rawName.IsValid() ? rawName.GetIndex() : ayName.GetIndex();
        const uint_t subModules = Formats::Chiptune::AY::GetModulesCount(*Delegate);
        if (subModules < index)
        {
          return File::Ptr();
        }
        const Formats::Chiptune::AY::BlobBuilder::Ptr builder = rawName.IsValid()
          ? Formats::Chiptune::AY::CreateMemoryDumpBuilder()
          : Formats::Chiptune::AY::CreateFileBuilder();
        if (!Formats::Chiptune::AY::Parse(*Delegate, index, *builder))
        {
          return File::Ptr();
        }
        const Binary::Container::Ptr data = builder->Result();
        return MakePtr<File>(name, data);
      }

      virtual uint_t CountFiles() const
      {
        return Formats::Chiptune::AY::GetModulesCount(*Delegate);
      }
    private:
      const Binary::Container::Ptr Delegate;
    };

    const std::string HEADER_FORMAT(
      "'Z'X'A'Y" // uint8_t Signature[4];
      "'E'M'U'L" // only one type is supported now
    );
  }//namespace MultiAY

  class MultiAYDecoder : public Decoder
  {
  public:
    MultiAYDecoder()
      : Format(Binary::CreateFormat(MultiAY::HEADER_FORMAT))
    {
    }

    virtual String GetDescription() const
    {
      return Text::AY_ARCHIVE_DECODER_DESCRIPTION;
    }

    virtual Binary::Format::Ptr GetFormat() const
    {
      return Format;
    }

    virtual Container::Ptr Decode(const Binary::Container& rawData) const
    {
      const uint_t subModules = Formats::Chiptune::AY::GetModulesCount(rawData);
      if (subModules < 2)
      {
        return Container::Ptr();
      }
      Formats::Chiptune::AY::Builder& stub = Formats::Chiptune::AY::GetStubBuilder();
      std::size_t maxSize = 0;
      for (uint_t idx = subModules; idx; --idx)
      {
        if (Formats::Chiptune::Container::Ptr ayData = Formats::Chiptune::AY::Parse(rawData, idx - 1, stub))
        {
          maxSize = std::max(maxSize, ayData->Size());
        }
      }
      if (maxSize)
      {
        const Binary::Container::Ptr ayData = rawData.GetSubcontainer(0, maxSize);
        return MakePtr<MultiAY::Container>(ayData);
      }
      else
      {
        return Container::Ptr();
      }
    }
  private:
    const Binary::Format::Ptr Format;
  };

  Decoder::Ptr CreateAYDecoder()
  {
    return MakePtr<MultiAYDecoder>();
  }
}//namespace Archived
}//namespace Formats
