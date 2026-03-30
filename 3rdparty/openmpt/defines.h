#pragma once

// environment
#define LIBOPENMPT_BUILD
#define NO_LOGGING 
#define NO_MPG123 
#define NO_CONTAINERS_SUPPORT 
#define OPENMPT_VERSION_REVISION 0
//declare ASSERT macro as Require to avoid AssertHandler
#define MPT_FRAMEWORK_ASSERT_IS_DEFINED 
#define MPT_ASSERT_USE_FRAMEWORK 1
#define MPT_FRAMEWORK_ASSERT_IS_ACTIVE 1
#define ASSERT(p) (void)(p)
#define MPT_LOG_IS_DISABLED
#define MPT_DETERMINISTIC_RANDOM
#define UNICODE
#define _UNICODE

// features
#define MPT_WITH_MINIMP3 
#define MPT_WITH_VORBIS
#define MPT_WITH_VORBISFILE 
#define MPT_WITH_OPUSFILE