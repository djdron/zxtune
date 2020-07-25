/**
* 
* @file
*
* @brief Version API implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
// Used information from http://sourceforge.net/p/predef/wiki/Home/
#include "platform/version/src/os.h"
#include "platform/version/src/arch.h"
#include "platform/version/src/toolset.h"
//library includes
#include <platform/version/api.h>
#include <strings/format.h>
//text includes
#include <platform/version/text/text.h>

namespace Text
{
  extern const Char PROGRAM_NAME[];
}

namespace Platform
{
  namespace Version
  {
    String GetProgramTitle()
    {
      return Text::PROGRAM_NAME;
    }

    String GetProgramVersion()
    {
      #define TOSTRING(a) #a
      #define STR(a) TOSTRING(a)
      static const char VERSION[] = STR(BUILD_VERSION);
      return FromStdString(VERSION);
    }

    String GetBuildDate()
    {
      static const char DATE[] = __DATE__;
      return FromStdString(DATE);
    }

    String GetBuildPlatform()
    {
      const String os = FromStdString(Details::OS);
      const String toolset = FromStdString(Details::TOOLSET);
      //some business-logic
      if (os == "windows" && toolset == "mingw")
      {
        return toolset;
      }
      else
      {
        return os;
      }
    }
    
    String GetBuildArchitecture()
    {
      return FromStdString(Details::ARCH);
    }
    
    String GetBuildArchitectureVersion()
    {
      return FromStdString(Details::ARCH_VERSION);
    }

    String GetProgramVersionString()
    {
      return Strings::Format(Text::PROGRAM_VERSION_STRING,
        GetProgramTitle(),
        GetProgramVersion(),
        GetBuildDate(),
        GetBuildPlatform(),
        GetBuildArchitecture(),
        GetBuildArchitectureVersion());
    }
  }
}
