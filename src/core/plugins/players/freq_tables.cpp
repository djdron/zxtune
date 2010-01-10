/*
Abstract:
  Frequency tables for AY-trackers implementation

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

#include "freq_tables.h"

#include <error_tools.h>
#include <tools.h>
#include <core/error_codes.h>

#include <boost/bind.hpp>

#include <text/core.h>

#define FILE_TAG E8071E22

namespace
{
  using namespace ZXTune::Module;
  
  const Char REVERT_TABLE_MARK = '~';
  
  struct FreqTableEntry
  {
    const String Name;
    const FrequencyTable Table;
  };
  
  static const FreqTableEntry TABLES[] = {
    //SoundTracker
    { 
      TABLE_SOUNDTRACKER,
      { {
        0xef8, 0xe10, 0xd60, 0xc80, 0xbd8, 0xb28, 0xa88, 0x9f0, 0x960, 0x8e0, 0x858, 0x7e0,
        0x77c, 0x708, 0x6b0, 0x640, 0x5ec, 0x594, 0x544, 0x4f8, 0x4b0, 0x470, 0x42c, 0x3f0,
        0x3be, 0x384, 0x358, 0x320, 0x2f6, 0x2ca, 0x2a2, 0x27c, 0x258, 0x238, 0x216, 0x1f8,
        0x1df, 0x1c2, 0x1ac, 0x190, 0x17b, 0x165, 0x151, 0x13e, 0x12c, 0x11c, 0x10b, 0x0fc,
        0x0ef, 0x0e1, 0x0d6, 0x0c8, 0x0bd, 0x0b2, 0x0a8, 0x09f, 0x096, 0x08e, 0x085, 0x07e,
        0x077, 0x070, 0x06b, 0x064, 0x05e, 0x059, 0x054, 0x04f, 0x04b, 0x047, 0x042, 0x03f,
        0x03b, 0x038, 0x035, 0x032, 0x02f, 0x02c, 0x02a, 0x027, 0x025, 0x023, 0x021, 0x01f,
        0x01d, 0x01c, 0x01a, 0x019, 0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00f
      } }
    }
  };
}

namespace ZXTune
{
  namespace Module
  {
    Error GetFreqTable(const String& id, FrequencyTable& result)
    {
      const bool doRevert(!id.empty() && *id.begin() == REVERT_TABLE_MARK);
      const String idNormal(doRevert ? id.substr(1) : id);
      const FreqTableEntry* const entry = std::find_if(TABLES, ArrayEnd(TABLES),
        boost::bind(&FreqTableEntry::Name, _1) == idNormal);
      if (entry == ArrayEnd(TABLES))
      {
        return MakeFormattedError(THIS_LINE, ERROR_INVALID_PARAMETERS, TEXT_MODULE_ERROR_INVALID_FREQ_TABLE_NAME, id);
      }
      if (doRevert)
      {
        std::copy(entry->Table.rbegin(), entry->Table.rend(), result.begin());
      }
      else
      {
        std::copy(entry->Table.begin(), entry->Table.end(), result.begin());
      }
      return Error();
    }
  }
}