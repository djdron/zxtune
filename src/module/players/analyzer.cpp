/**
* 
* @file
*
* @brief  Analyzer implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "module/players/analyzer.h"
//common includes
#include <make_ptr.h>
//library includes
#include <sound/chunk.h>
//std includes
#include <algorithm>
#include <array>
#include <complex>
#include <utility>

namespace Module
{
  class StubAnalyzer : public Analyzer
  {
  public:
    SpectrumState GetState() const override
    {
      return SpectrumState();
    }
  };

  Analyzer::Ptr CreateStubAnalyzer()
  {
    return MakePtr<StubAnalyzer>();
  }

  class DevicesAnalyzer : public Analyzer
  {
  public:
    explicit DevicesAnalyzer(Devices::StateSource::Ptr delegate)
      : Delegate(std::move(delegate))
    {
    }

    SpectrumState GetState() const override
    {
      return Delegate->GetState();
    }
  private:
    const Devices::StateSource::Ptr Delegate;
  };

  Analyzer::Ptr CreateAnalyzer(Devices::StateSource::Ptr state)
  {
    return MakePtr<DevicesAnalyzer>(state);
  }

  class FFTAnalyzer : public SoundAnalyzer
  {
  private:
    static const std::size_t WindowSizeLog = 10;
    static const std::size_t WindowSize = 1 << WindowSizeLog;

  public:
    using Ptr = std::shared_ptr<FFTAnalyzer>;
    
    void AddSoundData(const Sound::Chunk& data) override
    {
      const uint_t MAX_PRODUCED_DELTA = 10;
      if (Produced < MAX_PRODUCED_DELTA)
      {
        for (const auto& smp : data)
        {
          static_assert(Sound::Sample::MID == 0, "Incompatible sample type");
          const auto level = (smp.Left() + smp.Right()) / 2;
          Input[Cursor] = level;
          if (++Cursor == WindowSize)
          {
            Cursor = 0;
            ++Produced;
          }
        }
      }
    }
    
    SpectrumState GetState() const override
    {
      Produced = 0;
      return FFT();
    }
  private:
    using Complex = std::complex<float>;

    class Lookup
    {
    private:
      Lookup()
      {
        for (std::size_t idx = 0; idx < BitRev.size(); ++idx)
        {
          BitRev[idx] = ReverseBits(idx);
        }
        for (std::size_t idx = 0; idx < Sinus.size(); ++idx)
        {
          const auto angle = GetAngle(idx);
          Sinus[idx] = std::sin(angle);
        }
        HammingWindow();
      }
      
      static float GetAngle(std::size_t idx)
      {
        return 2.0f * 3.14159265358f * idx / WindowSize;
      }
      
      //http://dspsystem.narod.ru/add/win/win.html
      void HammingWindow()
      {
        const auto a0 = 0.54f;
        const auto a1 = 0.46f;
        for (std::size_t idx = 0; idx < Window.size(); ++idx)
        {
          const auto angle = GetAngle(idx);
          Window[idx] = a0 - a1 * std::cos(angle);
        }
      }
      
      static const Lookup& Instance()
      {
        static const Lookup INSTANCE;
        return INSTANCE;
      }

      static uint_t ReverseBits(uint_t val)
      {
        uint_t reversed = 0;
        for (uint_t loop = 0; loop < WindowSizeLog; ++loop)
        {
          reversed <<= 1;
          reversed += (val & 1);
          val >>= 1;
        }
        return reversed;
      }
    public:
      static std::array<Complex, WindowSize> ToComplex(const std::array<int_t, WindowSize>& input, std::size_t offset)
      {
        const auto& self = Instance();
        std::array<Complex, WindowSize> result;
        for (std::size_t idx = 0; idx < WindowSize; ++idx)
        {
          result[idx] = self.Window[idx] * input[(offset + self.BitRev[idx]) % WindowSize];
        }
        return result;
      }
      
      static Complex Polar(uint_t val)
      {
        const auto& sinus = Instance().Sinus;
        return Complex(sinus[(val + WindowSize / 4) % WindowSize], sinus[val]);
      }
    private:
      std::array<uint_t, WindowSize> BitRev;
      std::array<float, WindowSize> Sinus;
      std::array<float, WindowSize> Window;
    };
  
    SpectrumState FFT() const
    {
      auto cplx = Lookup::ToComplex(Input, Cursor);
      uint_t exchanges = 1;
      uint_t factFact = WindowSize / 2;
      for (uint_t i = WindowSizeLog; i != 0; --i, exchanges *= 2, factFact /= 2)
      {
        for (uint_t j = 0; j != exchanges; ++j)
        {
          const auto fact = Lookup::Polar(j * factFact);
          for (uint_t k = j; k < WindowSize; k += exchanges * 2)
          {
            const auto k1 = k + exchanges;
            const auto tmp = fact * cplx[k1];
            cplx[k1] = cplx[k] - tmp;
            cplx[k] += tmp;
          }
        }
      }
      
      SpectrumState result;
      for (std::size_t i = 0; i < result.Data.size(); ++i)
      {
        const uint_t LIMIT = LevelType::PRECISION;
        const uint_t raw = std::abs(cplx[i + 1]) / (256 * 32);
        result.Data[i] = LevelType(std::min(raw, LIMIT), LIMIT);
      }
      return result;
    }
  private:
    mutable uint_t Produced = 0;
    std::array<int_t, WindowSize> Input;
    std::size_t Cursor = 0;
  };

  SoundAnalyzer::Ptr CreateSoundAnalyzer()
  {
    return MakePtr<FFTAnalyzer>();
  }
}
