project(libsound_backends)

include_directories("../../../3rdparty")

file(GLOB SRCS_CXX "backend_impl.cpp" "file_backend.cpp" "service.cpp" "volume_control.cpp" "null_backend.cpp" "wav_backend.cpp" "text/backends.cpp")
file(GLOB SRCS_H "*.h" "text/backends.h")

if(WIN32)
set(SUPPORT_WAVEOUT 1)
set(SUPPORT_DIRECTSOUND 1)
set(SUPPORT_SDL 0)
else(WIN32)
set(SUPPORT_WAVEOUT 0)
set(SUPPORT_DIRECTSOUND 0)
set(SUPPORT_SDL 1)
endif(WIN32)
set(SUPPORT_ALSA 0)
set(SUPPORT_PULSEAUDIO 0)
set(SUPPORT_OSS 0)

set(SUPPORT_FLAC 1)
set(SUPPORT_MP3 1)
set(SUPPORT_OGG 1)

set(SUPPORT_AYLPT 0)

macro(declare_backend BACKEND BACKEND_SUPPORTED)
if(${BACKEND_SUPPORTED})
file(GLOB SRCS_BACKEND_CXX "${BACKEND}_backend.cpp")
list(APPEND SRCS_CXX "gates/${BACKEND}_api_dynamic.cpp")
list(APPEND SRCS_H "gates/${BACKEND}_api.h")
else()
file(GLOB SRCS_BACKEND_CXX "${BACKEND}_stub.cpp")
endif()
list(APPEND SRCS_CXX ${SRCS_BACKEND_CXX})
endmacro(declare_backend)

declare_backend(win32		SUPPORT_WAVEOUT)
declare_backend(dsound		SUPPORT_DIRECTSOUND)
declare_backend(alsa		SUPPORT_ALSA)
declare_backend(sdl			SUPPORT_SDL)
declare_backend(paudio		SUPPORT_PULSEAUDIO)
declare_backend(oss			SUPPORT_OSS)

declare_backend(flac		SUPPORT_FLAC)
declare_backend(mp3			SUPPORT_MP3)
declare_backend(ogg			SUPPORT_OGG)
declare_backend(vorbis		SUPPORT_OGG)
declare_backend(vorbisenc	SUPPORT_OGG)

declare_backend(aylpt		SUPPORT_AYLPT)

add_library(sound_backends ${SRCS_CXX} ${SRCS_H})
