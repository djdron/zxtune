/**
*
* @file
*
* @brief  Analyzer interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <types.h>
//boost includes
#include <boost/shared_ptr.hpp>

namespace Module
{
  //! @brief %Sound analyzer interface
  class Analyzer
  {
  public:
    //! Pointer type
    typedef boost::shared_ptr<const Analyzer> Ptr;

    virtual ~Analyzer() {}

    struct ChannelState
    {
      ChannelState()
        : Band()
        , Level()
      {
      }

      uint_t Band;
      uint_t Level;
    };

    virtual void GetState(std::vector<ChannelState>& channels) const = 0;
  };
}
