/*
Abstract:
  AY/YM chips interface implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001

  Based on sources of UnrealSpeccy by SMT
*/

//local includes
#include <devices/aym.h>
//common includes
#include <time_tools.h>
//std includes
#include <cassert>
#include <functional>
#include <limits>
#include <memory>
//boost includes
#include <boost/bind.hpp>

namespace
{
  using namespace Devices::AYM;

  enum
  {
    // set of registers which required input data masking (4 or 5 lsb)
    REGS_4BIT_SET = (1 << DataChunk::REG_TONEA_H) | (1 << DataChunk::REG_TONEB_H) |
                    (1 << DataChunk::REG_TONEC_H) | (1 << DataChunk::REG_ENV) | (1 << DataChunk::REG_BEEPER),
    REGS_5BIT_SET = (1 << DataChunk::REG_TONEN) | (1 << DataChunk::REG_VOLA) |
                    (1 << DataChunk::REG_VOLB) | (1 << DataChunk::REG_VOLC),
  };

  BOOST_STATIC_ASSERT(DataChunk::REG_LAST < 8 * sizeof(uint_t));

  typedef boost::array<Sample, 32> VolumeTable;

  // chip-specific volume tables- ym supports 32 volume steps, ay - only 16
  const VolumeTable AYVolumeTab =
  { {
    0x0000, 0x0000, 0x0340, 0x0340, 0x04C0, 0x04C0, 0x06F2, 0x06F2,
    0x0A44, 0x0A44, 0x0F13, 0x0F13, 0x1510, 0x1510, 0x227E, 0x227E,
    0x289F, 0x289F, 0x414E, 0x414E, 0x5B21, 0x5B21, 0x7258, 0x7258,
    0x905E, 0x905E, 0xB550, 0xB550, 0xD7A0, 0xD7A0, 0xFFFF, 0xFFFF
  } };
  const VolumeTable YMVolumeTab =
  { {
    0x0000, 0x0000, 0x00EF, 0x01D0, 0x0290, 0x032A, 0x03EE, 0x04D2,
    0x0611, 0x0782, 0x0912, 0x0A36, 0x0C31, 0x0EB6, 0x1130, 0x13A0,
    0x1751, 0x1BF5, 0x20E2, 0x2594, 0x2CA1, 0x357F, 0x3E45, 0x475E,
    0x5502, 0x6620, 0x7730, 0x8844, 0xA1D2, 0xC102, 0xE0A2, 0xFFFF
  } };

  typedef boost::array<uint_t, Devices::AYM::CHANNELS> LayoutData;

  const LayoutData LAYOUTS[] =
  {
    { {0, 1, 2} }, //ABC
    { {0, 2, 1} }, //ACB
    { {1, 0, 2} }, //BAC
    { {1, 2, 0} }, //BCA
    { {2, 1, 0} }, //CBA
    { {2, 0, 1} }, //CAB
  };

  const uint_t AYM_CLOCK_DIVISOR = 8;

  const uint_t MAX_DUTYCYCLE = 100;
  const uint_t NO_DUTYCYCLE = MAX_DUTYCYCLE / 2;

  //PSG-related functionality
  class Generator
  {
  public:
    Generator()
      : Counter()
      , Level()
      , Masked(true)
      , DutyCycle(NO_DUTYCYCLE)
      , HalfPeriod(), FullPeriod()
    {
    }

    void Reset()
    {
      Counter = 0;
      Level = false;
      Masked = true;
      DutyCycle = NO_DUTYCYCLE;
      SetPeriod(0);
    }

    void ResetCounter()
    {
      Counter = 0;
    }

    /*
    Perform square pulse width according to current state and duty parameter
    e.g. if duty == 25
      _     _
     | |   | |
    _| |___| |_

     |1| 3 |1| ...

    positive pulse width is 1/4 of period value
    */
    bool Tick()
    {
      ++Counter;
      if (Counter >= FullPeriod)
      {
        Counter = 0;
        Level = !Level;
        return true;
      }
      else if (Counter == HalfPeriod)
      {
        Level = !Level;
        return true;
      }
      return false;
    }

    bool GetLevel() const
    {
      return Masked || Level;
    }

    bool IsMasked() const
    {
      return Masked;
    }

    void SetDutyCycle(uint_t dutyCycle)
    {
      assert(dutyCycle > 0 && dutyCycle < MAX_DUTYCYCLE);
      DutyCycle = dutyCycle;
      UpdateHalfPeriod();
    }

    void SetPeriod(uint_t period)
    {
      FullPeriod = period * 2;
      UpdateHalfPeriod();
    }
    
    void SetMask(bool masked)
    {
      Masked = masked;
    }
  private:
    void UpdateHalfPeriod()
    {
      HalfPeriod = DutyCycle * FullPeriod / MAX_DUTYCYCLE;
    }
  private:
    uint_t Counter;
    bool Level;
    bool Masked;
    uint_t DutyCycle;
    uint_t HalfPeriod;
    uint_t FullPeriod;
  };

  class NoiseGenerator : public Generator
  {
  public:
    NoiseGenerator()
      : Seed()
    {
    }

    void Reset()
    {
      Seed = 0;
      Generator::Reset();
    }

    bool Tick()
    {
      if (Generator::Tick())
      {
        Seed = (Seed * 2 + 1) ^ (((Seed >> 16) ^ (Seed >> 13)) & 1);
        return true;
      }
      return false;
    }

    bool GetLevel() const
    {
      return 0 != (Seed & 0x10000);
    }
  private:
    uint_t Seed;
  };

  class EnvelopeGenerator : public Generator
  {
  public:
    EnvelopeGenerator()
      : Type()
      , Level()
      , Decay()
    {
    }

    void Reset()
    {
      Type = 0;
      Level = 0;
      Decay = 0;
      Generator::Reset();
    }

    bool Tick()
    {
      if (!Generator::Tick())
      {
        return false;
      }
      Level += Decay;
      if (0 == (Level & ~31u))
      {
        return true;
      }
      const uint_t envTypeMask = 1 << Type;
      if (envTypeMask & ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) |
                         (1 << 5) | (1 << 6) | (1 << 7) | (1 << 9) | (1 << 15)))
      {
        Level = Decay = 0;
      }
      else
      {
        if (envTypeMask & ((1 << 8) | (1 << 12)))
        {
          Level &= 31;
        }
        else
        {
          if (envTypeMask & ((1 << 10) | (1 << 14)))
          {
            Decay = -Decay;
            Level += Decay;
          }
          else
          {
            Level = 31;
            Decay = 0; //11, 13
          }
        }
      }
      return true;
    }

    void SetType(uint_t type)
    {
      Generator::ResetCounter();
      Type = type;
      if (Type & 4) //down-up envelopes
      {
        Level = 0;
        Decay = 1;
      }
      else //up-down envelopes
      {
        Level = 31;
        Decay = -1;
      }
    }

    uint_t GetValue() const
    {
      return Level;
    }
  private:
    uint_t Type;
    uint_t Level;
    int_t Decay;
  };

  class Renderer
  {
  public:
    virtual ~Renderer() {}

    virtual void Reset() = 0;
    virtual bool Tick() = 0;
    virtual void GetLevels(MultiSample& result) const = 0;
  };

  class AYMRenderer : public Renderer
  {
  public:
    AYMRenderer()
      : VolA(State.Data[DataChunk::REG_VOLA]), VolB(State.Data[DataChunk::REG_VOLB]), VolC(State.Data[DataChunk::REG_VOLC])
      , Mixer(State.Data[DataChunk::REG_MIXER]), Beeper(State.Data[DataChunk::REG_BEEPER])
      , ChangedFromLastTick()
      , IsYM()
    {
      State.Data[DataChunk::REG_MIXER] = 0xff;
    }

    void SetType(bool isYM)
    {
      IsYM = isYM;
    }

    void SetDutyCycle(uint_t value, uint_t mask)
    {
      GenA.SetDutyCycle(0 != (mask & DataChunk::CHANNEL_MASK_A) ? value : NO_DUTYCYCLE);
      GenB.SetDutyCycle(0 != (mask & DataChunk::CHANNEL_MASK_B) ? value : NO_DUTYCYCLE);
      GenC.SetDutyCycle(0 != (mask & DataChunk::CHANNEL_MASK_C) ? value : NO_DUTYCYCLE);
      GenN.SetDutyCycle(0 != (mask & DataChunk::CHANNEL_MASK_N) ? value : NO_DUTYCYCLE);
      GenE.SetDutyCycle(0 != (mask & DataChunk::CHANNEL_MASK_E) ? value : NO_DUTYCYCLE);
    }

    virtual void Reset()
    {
      State = DataChunk();
      State.Data[DataChunk::REG_MIXER] = 0xff;
      GenA.Reset();
      GenB.Reset();
      GenC.Reset();
      GenN.Reset();
      GenE.Reset();
      ChangedFromLastTick = false;
    }

    void ApplyData(const DataChunk& data)
    {
      for (uint_t idx = 0, mask = 1; idx != data.Data.size(); ++idx, mask <<= 1)
      {
        if (0 == (data.Mask & mask))
        {
          //no new data
          continue;
        }
        //copy registers
        uint8_t reg = data.Data[idx];
        //limit values
        if (mask & REGS_4BIT_SET)
        {
          reg &= 0x0f;
        }
        else if (mask & REGS_5BIT_SET)
        {
          reg &= 0x1f;
        }
        State.Data[idx] = reg;
      }
      if (data.Mask & (1 << DataChunk::REG_MIXER))
      {
        const bool newMaskA = 0 != (Mixer & DataChunk::REG_MASK_TONEA);
        const bool newMaskB = 0 != (Mixer & DataChunk::REG_MASK_TONEB);
        const bool newMaskC = 0 != (Mixer & DataChunk::REG_MASK_TONEC);
        const bool newMaskN = 0 == (~Mixer & (DataChunk::REG_MASK_NOISEA | DataChunk::REG_MASK_NOISEB | DataChunk::REG_MASK_NOISEC));
        ChangedFromLastTick = newMaskA != GenA.IsMasked() ||
          newMaskB != GenB.IsMasked() || newMaskC != GenC.IsMasked() || newMaskN != GenN.IsMasked();
        GenA.SetMask(newMaskA);
        GenB.SetMask(newMaskB);
        GenC.SetMask(newMaskC);
        GenN.SetMask(newMaskN);
      }
      if (data.Mask & ((1 << DataChunk::REG_TONEA_L) | (1 << DataChunk::REG_TONEA_H)))
      {
        GenA.SetPeriod(GetToneA());
      }
      if (data.Mask & ((1 << DataChunk::REG_TONEB_L) | (1 << DataChunk::REG_TONEB_H)))
      {
        GenB.SetPeriod(GetToneB());
      }
      if (data.Mask & ((1 << DataChunk::REG_TONEC_L) | (1 << DataChunk::REG_TONEC_H)))
      {
        GenC.SetPeriod(GetToneC());
      }
      if (data.Mask & (1 << DataChunk::REG_TONEN))
      {
        GenN.SetPeriod(GetToneN());
      }
      if (data.Mask & ((1 << DataChunk::REG_TONEE_L) | (1 << DataChunk::REG_TONEE_H)))
      {
        GenE.SetPeriod(GetToneE());
      }
      if (data.Mask & (1 << DataChunk::REG_ENV))
      {
        //update r13
        GenE.SetType(GetEnvType());
        ChangedFromLastTick = true;
      }
      if (data.Mask & REGS_5BIT_SET)
      {
        GenE.SetMask(0 == ((VolA | VolB | VolC) & DataChunk::REG_MASK_ENV));
        ChangedFromLastTick = true;
      }
      if (data.Mask & (1 << DataChunk::REG_BEEPER))
      {
        ChangedFromLastTick = true;
      }
    }

    virtual bool Tick()
    {
      bool res = ChangedFromLastTick;
      res |= GenA.Tick() && !GenA.IsMasked();
      res |= GenB.Tick() && !GenB.IsMasked();
      res |= GenC.Tick() && !GenC.IsMasked();
      res |= GenN.Tick() && !GenN.IsMasked();
      res |= GenE.Tick() && !GenE.IsMasked();
      ChangedFromLastTick = false;
      return res;
    }

    virtual void GetLevels(MultiSample& result) const
    {
      const bool triggeredNoise = GenN.GetLevel();
      const uint_t envelope = GenE.GetValue();
      //references to mixered bits. updated automatically
      const uint_t levelA = (((VolA & DataChunk::REG_MASK_VOL) << 1) + 1);
      const uint_t levelB = (((VolB & DataChunk::REG_MASK_VOL) << 1) + 1);
      const uint_t levelC = (((VolC & DataChunk::REG_MASK_VOL) << 1) + 1);
      const bool maskedNoiseA = triggeredNoise || 0 != (Mixer & DataChunk::REG_MASK_NOISEA);
      const uint_t outA = (VolA & DataChunk::REG_MASK_ENV) ? envelope : levelA;
      const bool maskedNoiseB = triggeredNoise || 0 != (Mixer & DataChunk::REG_MASK_NOISEB);
      const uint_t outB = (VolB & DataChunk::REG_MASK_ENV) ? envelope : levelB;
      const bool maskedNoiseC = triggeredNoise || 0 != (Mixer & DataChunk::REG_MASK_NOISEC);
      const uint_t outC = (VolC & DataChunk::REG_MASK_ENV) ? envelope : levelC;

      const VolumeTable& table = IsYM ? YMVolumeTab : AYVolumeTab;
      assert(outA < 32 && outB < 32 && outC < 32);
      result[0] = maskedNoiseA && GenA.GetLevel() ? table[outA] : 0;
      result[1] = maskedNoiseB && GenB.GetLevel() ? table[outB] : 0;
      result[2] = maskedNoiseC && GenC.GetLevel() ? table[outC] : 0;
      if (Beeper)
      {
        assert(Beeper < 16);
        const uint_t levelBeeper = table[(Beeper << 1) + 1] / 2;
        result[0] = result[0] / 2 + levelBeeper;
        result[1] = result[1] / 2 + levelBeeper;
        result[2] = result[2] / 2 + levelBeeper;
        return;
      }
    }

    virtual void GetState(uint64_t ticksPerSec, ChannelsState& state) const
    {
      const uint_t MAX_LEVEL = 100;
      //one channel is noise
      ChanState& noiseChan = state[CHANNELS];
      noiseChan = ChanState('N');
      noiseChan.Band = GetBandByPeriod(ticksPerSec, GetToneN());
      //one channel is envelope    
      ChanState& envChan = state[CHANNELS + 1];
      envChan = ChanState('E');
      envChan.Band = GetBandByPeriod(ticksPerSec, 16 * GetToneE());
      const uint_t mixer = ~Mixer;
      for (uint_t chan = 0; chan != CHANNELS; ++chan) 
      {
        const uint_t volReg = State.Data[DataChunk::REG_VOLA + chan];
        const bool hasNoise = 0 != (mixer & (uint_t(DataChunk::REG_MASK_NOISEA) << chan));
        const bool hasTone = 0 != (mixer & (uint_t(DataChunk::REG_MASK_TONEA) << chan));
        const bool hasEnv = 0 != (volReg & DataChunk::REG_MASK_ENV);
        //accumulate level in noise channel
        if (hasNoise)
        {
          noiseChan.Enabled = true;
          noiseChan.LevelInPercents += MAX_LEVEL / CHANNELS;
        }
        //accumulate level in envelope channel      
        if (hasEnv)
        {        
          envChan.Enabled = true;
          envChan.LevelInPercents += MAX_LEVEL / CHANNELS;
        }
        //calculate tone channel
        ChanState& channel = state[chan];
        channel.Name = 'A' + chan;
        if (hasTone)
        {
          channel.Enabled = true;
          channel.LevelInPercents = (volReg & DataChunk::REG_MASK_VOL) * MAX_LEVEL / 15;
          const uint_t chanTone = 256 * State.Data[DataChunk::REG_TONEA_H + chan * 2] +
            State.Data[DataChunk::REG_TONEA_L + chan * 2];
          channel.Band = GetBandByPeriod(ticksPerSec, chanTone);
        }
      } 
    }

  private:
    static uint_t GetBandByPeriod(uint64_t ticksPerSec, uint_t period)
    {
      const uint_t FREQ_MULTIPLIER = 100;
      //table in Hz * FREQ_MULTIPLIER
      static const uint_t FREQ_TABLE[] =
      {
        //octave1
        3270,   3465,   3671,   3889,   4120,   4365,   4625,   4900,   5191,   5500,   5827,   6173,
        //octave2
        6541,   6929,   7342,   7778,   8241,   8730,   9250,   9800,  10382,  11000,  11654,  12346,
        //octave3
        13082,  13858,  14684,  15556,  16482,  17460,  18500,  19600,  20764,  22000,  23308,  24692,
        //octave4
        26164,  27716,  29368,  31112,  32964,  34920,  37000,  39200,  41528,  44000,  46616,  49384,
        //octave5
        52328,  55432,  58736,  62224,  65928,  69840,  74000,  78400,  83056,  88000,  93232,  98768,
        //octave6
        104650, 110860, 117470, 124450, 131860, 139680, 148000, 156800, 166110, 176000, 186460, 197540,
        //octave7
        209310, 221720, 234940, 248890, 263710, 279360, 296000, 313600, 332220, 352000, 372930, 395070,
        //octave8
        418620, 443460, 469890, 497790, 527420, 558720, 592000, 627200, 664450, 704000, 745860, 790140,
        //octave9
        837200, 886980, 939730, 995610,1054800,1117500,1184000,1254400,1329000,1408000,1491700,1580400
      };
      const uint_t freq = static_cast<uint_t>(ticksPerSec * FREQ_MULTIPLIER / (2 * AYM_CLOCK_DIVISOR * (period ? period : 1)));
      const uint_t maxBand = static_cast<uint_t>(ArraySize(FREQ_TABLE) - 1);
      const uint_t currentBand = static_cast<uint_t>(std::lower_bound(FREQ_TABLE, ArrayEnd(FREQ_TABLE), freq) - FREQ_TABLE);
      return std::min(currentBand, maxBand);
    }
  private:
    uint_t GetToneA() const
    {
      return 256 * State.Data[DataChunk::REG_TONEA_H] + State.Data[DataChunk::REG_TONEA_L];
    }

    uint_t GetToneB() const
    {
      return 256 * State.Data[DataChunk::REG_TONEB_H] + State.Data[DataChunk::REG_TONEB_L];
    }

    uint_t GetToneC() const
    {
      return 256 * State.Data[DataChunk::REG_TONEC_H] + State.Data[DataChunk::REG_TONEC_L];
    }

    uint_t GetToneN() const
    {
      return 2 * State.Data[DataChunk::REG_TONEN];//for optimization
    }

    uint_t GetToneE() const
    {
      return 256 * State.Data[DataChunk::REG_TONEE_H] + State.Data[DataChunk::REG_TONEE_L];
    }

    uint_t GetEnvType() const
    {
      return State.Data[DataChunk::REG_ENV];
    }
  private:
    //registers state
    DataChunk State;
    //aliases for registers
    uint8_t& VolA;
    uint8_t& VolB;
    uint8_t& VolC;
    uint8_t& Mixer;
    uint8_t& Beeper;
    //generators
    Generator GenA;
    Generator GenB;
    Generator GenC;
    NoiseGenerator GenN;
    EnvelopeGenerator GenE;
    //state
    bool ChangedFromLastTick;
    //parameters
    bool IsYM;
  };

  class InterpolatedRenderer : public Renderer
  {
  public:
    explicit InterpolatedRenderer(Renderer& delegate)
      : Delegate(delegate)
      , Levels()
      , AccumulatedSamples()
    {
    }

    virtual void Reset()
    {
      std::fill(Levels.begin(), Levels.end(), 0);
      AccumulatedSamples = 0;
      Delegate.Reset();
    }

    virtual bool Tick()
    {
      if (Delegate.Tick())
      {
        Delegate.GetLevels(Levels);
      }
      std::transform(Accumulators.begin(), Accumulators.end(), Levels.begin(), Accumulators.begin(), std::plus<uint_t>());
      ++AccumulatedSamples;
      return true;
    }

    virtual void GetLevels(MultiSample& result) const
    {
      std::transform(Accumulators.begin(), Accumulators.end(), result.begin(), std::bind2nd(std::divides<uint_t>(), AccumulatedSamples));
      std::fill(Accumulators.begin(), Accumulators.end(), 0);
      AccumulatedSamples = 0;
    }
  private:
    Renderer& Delegate;
    MultiSample Levels;
    mutable boost::array<uint_t, CHANNELS> Accumulators;
    mutable uint_t AccumulatedSamples;
  };

  class RelayoutRenderer : public Renderer
  {
  public:
    RelayoutRenderer(Renderer& delegate, LayoutType layout)
      : Delegate(delegate)
      , Layout(LAYOUTS[layout])
    {
    }

    virtual void Reset()
    {
      return Delegate.Reset();
    }

    virtual bool Tick()
    {
      return Delegate.Tick();
    }

    virtual void GetLevels(MultiSample& result) const
    {
      MultiSample tmp;
      Delegate.GetLevels(tmp);
      for (uint_t idx = 0; idx < Devices::AYM::CHANNELS; ++idx)
      {
        const uint_t chipChannel = Layout[idx];
        result[idx] = tmp[chipChannel];
      }
    }
  private:
    Renderer& Delegate;
    const LayoutData Layout;
  };

  class ClockSource
  {
  public:
    ClockSource()
      : LastTick()
    {
    }

    void SetFrequency(uint64_t clockFreq, uint_t soundFreq)
    {
      PsgScaler.SetFrequency(clockFreq);
      SndScaler.SetFrequency(soundFreq);
    }

    void Reset()
    {
      LastTick = 0;
      PsgScaler.Reset();
      SndScaler.Reset();
    }

    void ApplyData(const DataChunk& data)
    {
      LastTick = data.Tick;
    }

    bool Tick()
    {
      PsgScaler.AdviceTick(AYM_CLOCK_DIVISOR);
      if (PsgScaler.GetTime() > SndScaler.GetTime())
      {
        SndScaler.AdviceTick(1);
        return true;
      }
      return false;
    }

    bool InFrame() const
    {
      return PsgScaler.GetTick() < LastTick;
    }
  private:
    uint64_t LastTick;
    Time::NanosecFreqScaler PsgScaler;
    Time::NanosecFreqScaler SndScaler;
  };

  class ChipImpl : public Chip
  {
  public:
    ChipImpl(ChipParameters::Ptr params, Receiver::Ptr target)
      : Params(params)
      , Target(target)
      , Render(PSG)
      , Clock()
    {
      Reset();
    }

    virtual void RenderData(const DataChunk& src)
    {
      BufferedData.push_back(src);
    }

    virtual void Flush()
    {
      ApplyParameters();
      Renderer& targetRender = Params->Interpolate()
        ? static_cast<Renderer&>(Render)
        : static_cast<Renderer&>(PSG);
      const LayoutType layout = Params->Layout();
      if (LAYOUT_ABC == layout)
      {
        RenderChunks(targetRender);
      }
      else
      {
        RelayoutRenderer relayoutRender(targetRender, layout);
        RenderChunks(relayoutRender);
      }
      BufferedData.clear();
    }

    virtual void GetState(ChannelsState& state) const
    {
      return PSG.GetState(Params->ClockFreq(), state);
    }

    virtual void Reset()
    {
      Render.Reset();
      Clock.Reset();
      BufferedData.clear();
    }
  private:
    void ApplyParameters()
    {
      PSG.SetType(Params->IsYM());
      PSG.SetDutyCycle(Params->DutyCycleValue(), Params->DutyCycleMask());
      Clock.SetFrequency(Params->ClockFreq(), Params->SoundFreq());
    }

    void RenderChunks(Renderer& render)
    {
      std::for_each(BufferedData.begin(), BufferedData.end(), boost::bind(&ChipImpl::RenderSingleChunk, this, _1, boost::ref(render)));
    }

    void RenderSingleChunk(const DataChunk& src, Renderer& render)
    {
      PSG.ApplyData(src);
      Clock.ApplyData(src);
      MultiSample result;
      while (Clock.InFrame())
      {
        render.Tick();
        if (Clock.Tick())
        {
          render.GetLevels(result);
          Target->ApplyData(result);
        }
      }
    }
  private:
    const ChipParameters::Ptr Params;
    const Receiver::Ptr Target;
    AYMRenderer PSG;
    InterpolatedRenderer Render;
    ClockSource Clock;
    std::vector<DataChunk> BufferedData;
  };
}

namespace Devices
{
  namespace AYM
  {
    Chip::Ptr CreateChip(ChipParameters::Ptr params, Receiver::Ptr target)
    {
      return Chip::Ptr(new ChipImpl(params, target));
    }
  }
}
