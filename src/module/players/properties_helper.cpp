/**
* 
* @file
*
* @brief  Module properties builder implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "properties_helper.h"
//library includes
#include <module/attributes.h>
#include <sound/sound_parameters.h>
#include <strings/optimize.h>
#include <time/stamp.h>
//boost includes
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace Module
{
  void PropertiesHelper::SetNonEmptyProperty(const String& name, const String& value)
  {
    const String& optimizedValue = Strings::Optimize(value);
    if (!optimizedValue.empty())
    {
      Delegate.SetValue(name, optimizedValue);
    }
  }

  void PropertiesHelper::SetType(const String& type)
  {
    Delegate.SetValue(ATTR_TYPE, type);
  }

  void PropertiesHelper::SetContainer(const String& container)
  {
    SetNonEmptyProperty(ATTR_CONTAINER, container);
  }

  void PropertiesHelper::SetSource(const Formats::Chiptune::Container& source)
  {
    Delegate.SetValue(ATTR_SIZE, source.Size());
    Delegate.SetValue(ATTR_CRC, source.Checksum());
    Delegate.SetValue(ATTR_FIXEDCRC, source.FixedChecksum());
  }

  void PropertiesHelper::SetAuthor(const String& author)
  {
    SetNonEmptyProperty(ATTR_AUTHOR, author);
  }

  void PropertiesHelper::SetTitle(const String& title)
  {
    SetNonEmptyProperty(ATTR_TITLE, title);
  }
  
  void PropertiesHelper::SetComment(const String& comment)
  {
    SetNonEmptyProperty(ATTR_COMMENT, comment);
  }

  void PropertiesHelper::SetProgram(const String& program)
  {
    SetNonEmptyProperty(ATTR_PROGRAM, program);
  }

  void PropertiesHelper::SetComputer(const String& computer)
  {
    SetNonEmptyProperty(ATTR_COMPUTER, computer);
  }
  
  void PropertiesHelper::SetStrings(const Strings::Array& strings)
  {
    String joined = boost::algorithm::join(boost::adaptors::transform(strings, Strings::Optimize), "\n");
    boost::algorithm::trim_all_if(joined, boost::algorithm::is_any_of("\n"));
    if (!joined.empty())
    {
      Delegate.SetValue(ATTR_STRINGS, joined);
    }
  }
  
  void PropertiesHelper::SetVersion(uint_t major, uint_t minor)
  {
    assert(minor < 10);
    const uint_t version = 10 * major + minor;
    Delegate.SetValue(ATTR_VERSION, version);
  }

  void PropertiesHelper::SetVersion(const String& version)
  {
    SetNonEmptyProperty(ATTR_VERSION, version);
  }

  void PropertiesHelper::SetDate(const String& date)
  {
    Delegate.SetValue(ATTR_DATE, date);
  }
  
  void PropertiesHelper::SetFramesFrequency(uint_t freq)
  {
    Delegate.SetValue(Parameters::ZXTune::Sound::FRAMEDURATION, Time::GetPeriodForFrequency<Time::Microseconds>(freq).Get());
  }
}
