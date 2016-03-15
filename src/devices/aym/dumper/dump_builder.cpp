/**
* 
* @file
*
* @brief  AY/YM dump builder implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "dump_builder.h"
//common includes
#include <make_ptr.h>

namespace Devices
{
namespace AYM
{
  class RenderState
  {
  public:
    typedef boost::shared_ptr<RenderState> Ptr;
    virtual ~RenderState() {}

    virtual void Reset() = 0;

    virtual void Add(const Registers& delta) = 0;
    virtual Registers GetBase() const = 0;
    virtual Registers GetDelta() const = 0;
    virtual void CommitDelta() = 0;
  };

  uint8_t GetValueMask(Registers::Index idx)
  {
    const uint_t REGS_4BIT_SET = (1 << Registers::TONEA_H) | (1 << Registers::TONEB_H) |
      (1 << Registers::TONEC_H) | (1 << Registers::ENV);
    const uint_t REGS_5BIT_SET = (1 << Registers::TONEN) | (1 << Registers::VOLA) |
      (1 << Registers::VOLB) | (1 << Registers::VOLC);

    const uint_t mask = 1 << idx;
    if (mask & REGS_4BIT_SET)
    {
      return 0x0f;
    }
    else if (mask & REGS_5BIT_SET)
    {
      return 0x1f;
    }
    else
    {
      return 0xff;
    }
  }

  void ApplyMerge(Registers& dst, const Registers& src)
  {
    for (Registers::IndicesIterator it(src); it; ++it)
    {
      const Registers::Index idx = *it;
      dst[idx] = src[idx] & GetValueMask(idx);
    }
  }

  class NotOptimizedRenderState : public RenderState
  {
  public:
    virtual void Reset()
    {
      Base = Registers();
      Delta = Registers();
    }

    virtual void Add(const Registers& delta)
    {
      ApplyMerge(Delta, delta);
    }

    virtual Registers GetBase() const
    {
      return Base;
    }

    virtual Registers GetDelta() const
    {
      return Delta;
    }

    void CommitDelta()
    {
      ApplyMerge(Base, Delta);
      Delta = Registers();
    }
  protected:
    Registers Base;
    Registers Delta;
  };

  class OptimizedRenderState : public NotOptimizedRenderState
  {
  public:
    virtual void Add(const Registers& delta)
    {
      for (Registers::IndicesIterator it(delta); it; ++it)
      {
        const Registers::Index reg = *it;
        const uint8_t newVal = delta[reg] & GetValueMask(reg);
        if (Registers::ENV != reg && Base.Has(reg))
        {
          uint8_t& base = Base[reg];
          if (newVal == base)
          {
            Delta.Reset(reg);
            continue;
          }
        }
        Delta[reg] = newVal;
      }
    }
  };

  class FrameDumper : public Dumper
  {
  public:
    FrameDumper(const Time::Microseconds& frameDuration, FramedDumpBuilder::Ptr builder, RenderState::Ptr state)
      : FrameDuration(frameDuration)
      , Builder(builder)
      , State(state)
      , FramesToSkip(0)
      , NextFrame()
    {
      Reset();
    }

    virtual void RenderData(const DataChunk& src)
    {
      if (!(src.TimeStamp < NextFrame))
      {
        FinishFrame();
      }
      State->Add(src.Data);
    }

    virtual void RenderData(const std::vector<DataChunk>& src)
    {
      for (std::vector<DataChunk>::const_iterator it = src.begin(), lim = src.end(); it != lim; ++it)
      {
        RenderData(*it);
      }
    }

    virtual void Reset()
    {
      Builder->Initialize();
      State->Reset();
      FramesToSkip = 0;
      NextFrame = FrameDuration;
    }

    virtual void GetDump(Dump& result) const
    {
      if (FramesToSkip)
      {
        const Registers delta = State->GetDelta();
        State->CommitDelta();
        Builder->WriteFrame(FramesToSkip, State->GetBase(), delta);
        FramesToSkip = 0;
      }
      Builder->GetResult(result);
    }
  private:
    void FinishFrame()
    {
      ++FramesToSkip;
      const Registers delta = State->GetDelta();
      if (!delta.Empty())
      {
        State->CommitDelta();
        const Registers& current = State->GetBase();
        Builder->WriteFrame(FramesToSkip, current, delta);
        FramesToSkip = 0;
      }
      NextFrame += FrameDuration;
    }
  private:
    const Stamp FrameDuration;
    const FramedDumpBuilder::Ptr Builder;
    const RenderState::Ptr State;
    mutable uint_t FramesToSkip;
    Stamp NextFrame;
  };

  Dumper::Ptr CreateDumper(DumperParameters::Ptr params, FramedDumpBuilder::Ptr builder)
  {
    RenderState::Ptr state;
    switch (params->OptimizationLevel())
    {
    case DumperParameters::NONE:
      state = MakePtr<NotOptimizedRenderState>();
      break;
    default:
      state = MakePtr<OptimizedRenderState>();
    }
    return MakePtr<FrameDumper>(params->FrameDuration(), builder, state);
  }
}
}
