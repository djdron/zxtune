/*
Abstract:
  VorbisEnc api gate interface

Last changed:
  $Id$

Author:
  (C) Vitamin/CAIG/2001
*/

#ifndef SOUND_BACKENDS_VORBISENC_API_H_DEFINED
#define SOUND_BACKENDS_VORBISENC_API_H_DEFINED

//platform-specific includes
#include <vorbis/vorbisenc.h>
//boost includes
#include <boost/shared_ptr.hpp>

namespace ZXTune
{
  namespace Sound
  {
    namespace VorbisEnc
    {
      class Api
      {
      public:
        typedef boost::shared_ptr<Api> Ptr;
        virtual ~Api() {}

        
        virtual int vorbis_encode_init(vorbis_info *vi, long channels, long rate, long max_bitrate, long nominal_bitrate, long min_bitrate) = 0;
        virtual int vorbis_encode_init_vbr(vorbis_info *vi, long channels, long rate, float base_quality) = 0;
      };

      //throw exception in case of error
      Api::Ptr LoadDynamicApi();

    }
  }
}

#endif //SOUND_BACKENDS_VORBISENC_API_H_DEFINED
