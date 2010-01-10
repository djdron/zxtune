/*
Abstract:
  Plugins parameters names

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/
#ifndef __CORE_PLUGINS_PARAMETERS_H_DEFINED__
#define __CORE_PLUGINS_PARAMETERS_H_DEFINED__

#include <parameters.h>

namespace Parameters
{
  namespace ZXTune
  {
    namespace Core
    {
      namespace Plugins
      {
        //raw scanner's attributes
        namespace Raw
        {
          //! Scanning step
          const IntType SCAN_STEP_DEFAULT = 1;
          const Char SCAN_STEP[] =
          {
            'z','x','t','u','n','e','.','c','o','r','e','.','p','l','u','g','i','n','s','.','r','a','w','.','s','c','a','n','_','s','t','e','p','\0'
          };
        }
      }
    }
  }
}
#endif //__CORE_PLUGINS_PARAMETERS_H_DEFINED__
