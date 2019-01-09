/**
* 
* @file
*
* @brief  TurboFM Dump chiptune factory implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "tfd.h"
#include "tfm_base_stream.h"
//common includes
#include <make_ptr.h>
#include <module/players/properties_helper.h>
#include <module/players/streaming.h>
//library includes
#include <formats/chiptune/fm/tfd.h>
//text includes
#include <module/text/platforms.h>

namespace Module
{
namespace TFD
{
  class ModuleData : public TFM::StreamModel
  {
  public:
    typedef std::shared_ptr<ModuleData> RWPtr;

    ModuleData()
      : LoopPos()
    {
    }

    uint_t Size() const override
    {
      return static_cast<uint_t>(Offsets.size() - 1);
    }

    uint_t Loop() const override
    {
      return LoopPos;
    }

    void Get(uint_t frameNum, Devices::TFM::Registers& res) const override
    {
      const std::size_t start = Offsets[frameNum];
      const std::size_t end = Offsets[frameNum + 1];
      res.assign(Data.begin() + start, Data.begin() + end);
    }
    
    void Append(std::size_t count)
    {
      Offsets.resize(Offsets.size() + count, Data.size());
    }
    
    void AddRegister(const Devices::TFM::Register& reg)
    {
     if (!Offsets.empty())
     {
       Data.push_back(reg);
     }
    }
    
    void SetLoop()
    {
      if (!Offsets.empty())
      {
        LoopPos = static_cast<uint_t>(Offsets.size() - 1);
      }
    }
  private:
    uint_t LoopPos;
    Devices::TFM::Registers Data;
    std::vector<std::size_t> Offsets;
  };

  class DataBuilder : public Formats::Chiptune::TFD::Builder
  {
  public:
   explicit DataBuilder(PropertiesHelper& props)
    : Properties(props)
    , Data(MakeRWPtr<ModuleData>())
    , Chip(0)
   {
   }

   void SetTitle(const String& title) override
   {
     Properties.SetTitle(title);
   }

   void SetAuthor(const String& author) override
   {
     Properties.SetAuthor(author);
   }

   void SetComment(const String& comment) override
   {
     Properties.SetComment(comment);
   }

   void BeginFrames(uint_t count) override
   {
     Chip = 0;
     Data->Append(count);
   }

   void SelectChip(uint_t idx) override
   {
     Chip = idx;
   }

   void SetLoop() override
   {
     Data->SetLoop();
   }

   void SetRegister(uint_t idx, uint_t val) override
   {
     Data->AddRegister(Devices::TFM::Register(Chip, idx, val));
   }

   TFM::StreamModel::Ptr GetResult() const
   {
     return Data;
   }
  private:
    PropertiesHelper& Properties;
    const ModuleData::RWPtr Data;
    uint_t Chip;
  };

  class Factory : public TFM::Factory
  {
  public:
    TFM::Chiptune::Ptr CreateChiptune(const Binary::Container& rawData, Parameters::Container::Ptr properties) const override
    {
      PropertiesHelper props(*properties);
      DataBuilder dataBuilder(props);
      if (const auto container = Formats::Chiptune::TFD::Parse(rawData, dataBuilder))
      {
        auto data = dataBuilder.GetResult();
        if (data->Size())
        {
          props.SetSource(*container);
          props.SetPlatform(Platforms::ZX_SPECTRUM);
          return TFM::CreateStreamedChiptune(std::move(data), std::move(properties));
        }
      }
      return TFM::Chiptune::Ptr();
    }
  };
  
  Factory::Ptr CreateFactory()
  {
    return MakePtr<Factory>();
  }
}
}
