/**
* 
* @file
*
* @brief  SoundTracker-based chiptune factory implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "soundtracker.h"
#include "aym_properties_helper.h"
#include "aym_base_track.h"
//common includes
#include <make_ptr.h>
#include <module/players/properties_meta.h>
#include <module/players/simple_orderlist.h>
#include <module/players/simple_ornament.h>

namespace Module
{
namespace SoundTracker
{
  enum CmdType
  {
    EMPTY,
    ENVELOPE,     //2p
    NOENVELOPE,   //0p
  };

  struct Sample : public Formats::Chiptune::SoundTracker::Sample
  {
    Sample()
      : Formats::Chiptune::SoundTracker::Sample()
    {
    }

    Sample(Formats::Chiptune::SoundTracker::Sample rh)
      : Formats::Chiptune::SoundTracker::Sample(std::move(rh))
    {
    }

    uint_t GetLoop() const
    {
      return Loop;
    }

    uint_t GetLoopLimit() const
    {
      return LoopLimit;
    }

    uint_t GetSize() const
    {
      return static_cast<uint_t>(Lines.size());
    }

    const Line& GetLine(uint_t idx) const
    {
      static const Line STUB;
      return Lines.size() > idx ? Lines[idx] : STUB;
    }
  };

  typedef SimpleOrderListWithTransposition<Formats::Chiptune::SoundTracker::PositionEntry> OrderListWithTransposition;
  typedef SimpleOrnament Ornament;

  class ModuleData : public TrackModel
  {
  public:
    typedef std::shared_ptr<const ModuleData> Ptr;
    typedef std::shared_ptr<ModuleData> RWPtr;

    ModuleData()
      : InitialTempo()
    {
    }

    uint_t GetInitialTempo() const override
    {
      return InitialTempo;
    }

    const OrderList& GetOrder() const override
    {
      return *Order;
    }

    const PatternsSet& GetPatterns() const override
    {
      return *Patterns;
    }

    uint_t InitialTempo;
    OrderListWithTransposition::Ptr Order;
    PatternsSet::Ptr Patterns;
    SparsedObjectsStorage<Sample> Samples;
    SparsedObjectsStorage<Ornament> Ornaments;
  };

  class DataBuilder : public Formats::Chiptune::SoundTracker::Builder
  {
  public:
    explicit DataBuilder(AYM::PropertiesHelper& props)
      : Data(MakeRWPtr<ModuleData>())
      , Properties(props)
      , Meta(props)
      , Patterns(PatternsBuilder::Create<AYM::TRACK_CHANNELS>())
    {
      Data->Patterns = Patterns.GetResult();
      Properties.SetFrequencyTable(TABLE_SOUNDTRACKER);
    }

    Formats::Chiptune::MetaBuilder& GetMetaBuilder() override
    {
      return Meta;
    }

    void SetInitialTempo(uint_t tempo) override
    {
      Data->InitialTempo = tempo;
    }

    void SetSample(uint_t index, const Formats::Chiptune::SoundTracker::Sample& sample) override
    {
      Data->Samples.Add(index, sample);
    }

    void SetOrnament(uint_t index, const Formats::Chiptune::SoundTracker::Ornament& ornament) override
    {
      Data->Ornaments.Add(index, Ornament(ornament.begin(), ornament.end()));
    }

    void SetPositions(const std::vector<Formats::Chiptune::SoundTracker::PositionEntry>& positions) override
    {
      Data->Order = MakePtr<OrderListWithTransposition>(positions.begin(), positions.end());
    }

    Formats::Chiptune::PatternBuilder& StartPattern(uint_t index) override
    {
      Patterns.SetPattern(index);
      return Patterns;
    }

    void StartChannel(uint_t index) override
    {
      Patterns.SetChannel(index);
    }

    void SetRest() override
    {
      Patterns.GetChannel().SetEnabled(false);
    }

    void SetNote(uint_t note) override
    {
      Patterns.GetChannel().SetEnabled(true);
      Patterns.GetChannel().SetNote(note);
    }

    void SetSample(uint_t sample) override
    {
      Patterns.GetChannel().SetSample(sample);
    }

    void SetOrnament(uint_t ornament) override
    {
      Patterns.GetChannel().SetOrnament(ornament);
    }

    void SetEnvelope(uint_t type, uint_t value) override
    {
      Patterns.GetChannel().AddCommand(SoundTracker::ENVELOPE, type, value);
    }

    void SetNoEnvelope() override
    {
      Patterns.GetChannel().AddCommand(SoundTracker::NOENVELOPE);
    }

    ModuleData::Ptr GetResult() const
    {
      return Data;
    }
  private:
    const ModuleData::RWPtr Data;
    AYM::PropertiesHelper& Properties;
    MetaProperties Meta;
    PatternsBuilder Patterns;
  };

  class ChannelBuilder
  {
  public:
    ChannelBuilder(int_t transposition, AYM::TrackBuilder& track, uint_t chanNum)
      : Transposition(transposition)
      , Track(track)
      , Channel(Track.GetChannel(chanNum))
    {
    }

    void SetLevel(uint_t level)
    {
      Channel.SetLevel(level);
    }

    void EnableEnvelope()
    {
      Channel.EnableEnvelope();
    }

    void SetTone(int_t halftones, int_t offset)
    {
      Channel.SetTone(halftones + Transposition, offset);
    }

    void DisableTone()
    {
      Channel.DisableTone();
    }

    void SetNoise(int_t level)
    {
      Track.SetNoise(level);
    }

    void DisableNoise()
    {
      Channel.DisableNoise();
    }
  private:
    const int_t Transposition;
    AYM::TrackBuilder& Track;
    AYM::ChannelBuilder Channel;
  };

  struct EnvelopeState
  {
    explicit EnvelopeState(uint_t& type, uint_t& tone)
      : Type(type)
      , Tone(tone)
      , Enabled(0)
    {
    }

    void Reset()
    {
      Enabled = 0;
    }

    void SetNewState(const Cell& src)
    {
      for (CommandsIterator it = src.GetCommands(); it; ++it)
      {
        ApplyCommand(*it);
      }
    }

    void Iterate()
    {
      if (2 == Enabled)
      {
        Type = 0;
      }
      else if (1 == Enabled)
      {
        Enabled = 2;
        Type = 0;
      }
    }

    void Synthesize(ChannelBuilder& channel) const
    {
      if (Enabled)
      {
        channel.EnableEnvelope();
      }
    } 
  private:
    void ApplyCommand(const Command& command)
    {
      if (command == ENVELOPE)
      {
        Type = command.Param1;
        Tone = command.Param2;
        Enabled = 1;
      }
      else if (command == NOENVELOPE)
      {
        Enabled = 0;
      }
    }
  private:
    uint_t& Type;
    uint_t& Tone;
    uint_t Enabled;
  };

  struct StateCursor
  {
    int_t CountDown;
    uint_t Position;

    StateCursor()
      : CountDown(-1)
      , Position(0)
    {
    }

    void Next(const Sample& sample)
    {
      if (!IsValid())
      {
        return;
      }
      --CountDown;
      Position = (Position + 1) & 0x1f;
      if (0 == CountDown)
      {
        if (const uint_t loop = sample.GetLoop())
        {
          Position = loop & 0x1f;
          CountDown = sample.GetLoopLimit() - loop;
        }
        else
        {
          CountDown = -1;
        }
      }
    }

    bool IsValid() const
    {
      return CountDown >= 0;
    }
  };

  struct ChannelState
  {
    explicit ChannelState(ModuleData::Ptr data, uint_t& envType, uint_t& envTone)
      : Data(std::move(data))
      , Note()
      , CurSample(GetStubSample())
      , CurOrnament(GetStubOrnament())
      , EnvState(envType, envTone)
    {
    }

    void Reset()
    {
      Note = 0;
      Cursor = StateCursor();
      CurSample = GetStubSample();
      CurOrnament = GetStubOrnament();
      EnvState.Reset();
    }

    void SetNewState(const Cell& src)
    {
      if (const bool* enabled = src.GetEnabled())
      {
        Cursor.CountDown = *enabled ? 32 : -1;
      }
      if (const uint_t* note = src.GetNote())
      {
        Note = *note;
        Cursor.Position = 0;
      }
      if (const uint_t* sample = src.GetSample())
      {
        CurSample = &Data->Samples.Get(*sample);
      }
      if (const uint_t* ornament = src.GetOrnament())
      {
        CurOrnament = &Data->Ornaments.Get(*ornament);
      }
      EnvState.SetNewState(src);
    }

    void Synthesize(ChannelBuilder& channel) const
    {
      StateCursor nextState(Cursor);
      nextState.Next(*CurSample);
      if (!nextState.IsValid())
      {
        channel.SetLevel(0);
        return;
      }

      const uint_t nextPosition = (nextState.Position - 1) & 0x1f;
      const Sample::Line& curSampleLine = CurSample->GetLine(nextPosition);
      //apply level
      channel.SetLevel(curSampleLine.Level);
      //apply tone
      const int_t halftones = int_t(Note) + CurOrnament->GetLine(nextPosition);
      channel.SetTone(halftones, curSampleLine.Effect);
      if (curSampleLine.EnvelopeMask)
      {
        channel.DisableTone();
      }
      //apply noise
      if (!curSampleLine.NoiseMask)
      {
        channel.SetNoise(curSampleLine.Noise);
      }
      else
      {
        channel.DisableNoise();
      }
      EnvState.Synthesize(channel);
    }

    void Iterate()
    {
      Cursor.Next(*CurSample);
      if (Cursor.IsValid())
      {
        EnvState.Iterate();
      }
    }
  private:
    static const Sample* GetStubSample()
    {
      static const Sample stubSample;
      return &stubSample;
    }

    static const Ornament* GetStubOrnament()
    {
      static const Ornament stubOrnament;
      return &stubOrnament;
    }
  private:
    const ModuleData::Ptr Data;
    uint_t Note;
    StateCursor Cursor;
    const Sample* CurSample;
    const Ornament* CurOrnament;
    EnvelopeState EnvState;
  };

  class DataRenderer : public AYM::DataRenderer
  {
  public:
    explicit DataRenderer(ModuleData::Ptr data)
      : Data(std::move(data))
      , StateA(Data, EnvType, EnvTone)
      , StateB(Data, EnvType, EnvTone)
      , StateC(Data, EnvType, EnvTone)
      , EnvType()
      , EnvTone()
    {
    }

    void Reset() override
    {
      StateA.Reset();
      StateB.Reset();
      StateC.Reset();
      EnvType = EnvTone = 0;
    }

    void SynthesizeData(const TrackModelState& state, AYM::TrackBuilder& track) override
    {
      if (0 == state.Quirk())
      {
        SwitchToNewLine(state);
      }
      SynthesizeChannelsData(state, track);
      IterateState();
    }
  private:
    void SwitchToNewLine(const TrackModelState& state)
    {
      assert(0 == state.Quirk());
      if (const Line::Ptr line = state.LineObject())
      {
        if (const Cell::Ptr chan = line->GetChannel(0))
        {
          StateA.SetNewState(*chan);
        }
        if (const Cell::Ptr chan = line->GetChannel(1))
        {
          StateB.SetNewState(*chan);
        }
        if (const Cell::Ptr chan = line->GetChannel(2))
        {
          StateC.SetNewState(*chan);
        }
      }
    }

    void SynthesizeChannelsData(const TrackState& state, AYM::TrackBuilder& track) const
    {
      const int_t transposition = Data->Order->GetTransposition(state.Position());
      {
        ChannelBuilder channel(transposition, track, 0);
        StateA.Synthesize(channel);
      }
      {
        ChannelBuilder channel(transposition, track, 1);
        StateB.Synthesize(channel);
      }
      {
        ChannelBuilder channel(transposition, track, 2);
        StateC.Synthesize(channel);
      }
      if (EnvType)
      {
        track.SetEnvelopeType(EnvType);
        track.SetEnvelopeTone(EnvTone);
      }
    }

    void IterateState()
    {
      StateA.Iterate();
      StateB.Iterate();
      StateC.Iterate();
    }
  private:
    const ModuleData::Ptr Data;
    ChannelState StateA;
    ChannelState StateB;
    ChannelState StateC;
    uint_t EnvType, EnvTone;
  };

  class Chiptune : public AYM::Chiptune
  {
  public:
    Chiptune(ModuleData::Ptr data, Parameters::Accessor::Ptr properties)
      : Data(std::move(data))
      , Properties(std::move(properties))
      , Info(CreateTrackInfo(Data, AYM::TRACK_CHANNELS))
    {
    }

    Information::Ptr GetInformation() const override
    {
      return Info;
    }

    Parameters::Accessor::Ptr GetProperties() const override
    {
      return Properties;
    }

    AYM::DataIterator::Ptr CreateDataIterator(AYM::TrackParameters::Ptr trackParams) const override
    {
      const TrackStateIterator::Ptr iter = CreateTrackStateIterator(Data);
      const DataRenderer::Ptr renderer = MakePtr<DataRenderer>(Data);
      return AYM::CreateDataIterator(trackParams, iter, renderer);
    }
  private:
    const ModuleData::Ptr Data;
    const Parameters::Accessor::Ptr Properties;
    const Information::Ptr Info;
  };

  class Factory : public AYM::Factory
  {
  public:
    explicit Factory(Formats::Chiptune::SoundTracker::Decoder::Ptr decoder)
      : Decoder(std::move(decoder))
    {
    }

    AYM::Chiptune::Ptr CreateChiptune(const Binary::Container& rawData, Parameters::Container::Ptr properties) const override
    {
      AYM::PropertiesHelper props(*properties);
      DataBuilder dataBuilder(props);
      if (const Formats::Chiptune::Container::Ptr container = Decoder->Parse(rawData, dataBuilder))
      {
        props.SetSource(*container);
        return MakePtr<Chiptune>(dataBuilder.GetResult(), properties);
      }
      else
      {
        return AYM::Chiptune::Ptr();
      }
    }
  private:
    const Formats::Chiptune::SoundTracker::Decoder::Ptr Decoder;
  };

  Factory::Ptr CreateFactory(Formats::Chiptune::SoundTracker::Decoder::Ptr decoder)
  {
    return MakePtr<Factory>(decoder);
  }
}
}
