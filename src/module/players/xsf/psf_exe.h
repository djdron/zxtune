/**
* 
* @file
*
* @brief  PSF related stuff. Exe
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "memory_region.h"
//std includes
#include <memory>

namespace Binary
{
  class Container;
}

namespace Module
{
  namespace PSF
  {
    struct PsxExe
    {
      using Ptr = std::unique_ptr<const PsxExe>;
      using RWPtr = std::unique_ptr<PsxExe>;
      
      PsxExe() = default;
      PsxExe(const PsxExe&) = delete;
      PsxExe& operator = (const PsxExe&) = delete;
      
      uint_t RefreshRate = 0;
      uint32_t PC = 0;
      uint32_t SP = 0;
      MemoryRegion RAM;
      
      static void Parse(const Binary::Container& data, PsxExe& exe);
    };
  }
}