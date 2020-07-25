/**
*
* @file
*
* @brief  OpenAL backend stub
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "sound/backends/openal.h"
#include "sound/backends/storage.h"
//library includes
#include <l10n/api.h>
#include <sound/backend_attrs.h>
//text includes
#include <sound/backends/text/backends.h>

namespace Sound
{
  void RegisterOpenAlBackend(BackendsStorage& storage)
  {
    storage.Register(Text::OPENAL_BACKEND_ID, L10n::translate("OpenAL backend"), CAP_TYPE_SYSTEM); 
  }

  namespace OpenAl
  {
    Strings::Array EnumerateDevices()
    {
      return Strings::Array();
    }
  }
}
