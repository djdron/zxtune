/**
*
* @file
*
* @brief  Format implementation details access
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <binary/format.h>

namespace Binary
{
  class FormatDetails : public Format
  {
  public:
    virtual std::size_t GetMinSize() const = 0;
  };
}
