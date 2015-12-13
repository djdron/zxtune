cmake_minimum_required (VERSION 2.8)
project (z80ex)

set (API_REVISION 1)
set (VERSION_MAJOR 1)
set (VERSION_MINOR 19)
set (RELEASE_TYPE "pre1")
set (VERSION_STR "${API_REVISION}.${VERSION_MAJOR}.${VERSION_MINOR}${RELEASE_TYPE}")

option (OPSTEP_FAST_AND_ROUGH "Fast and rough opcode step emulation mode" ON)

include_directories(BEFORE . include)

include (TestBigEndian)
test_big_endian(BIG_ENDIAN)
#endianness (one of: WORDS_LITTLE_ENDIAN, WORDS_BIG_ENDIAN)
if (BIG_ENDIAN)
    set (ENDIANNESS WORDS_BIG_ENDIAN)
else ()
    set (ENDIANNESS WORDS_LITTLE_ENDIAN)
endif ()

add_definitions (-D${ENDIANNESS} -DZ80EX_VERSION_STR=${VERSION_STR} -DZ80EX_API_REVISION=${API_REVISION} -DZ80EX_VERSION_MAJOR=${VERSION_MAJOR} -DZ80EX_VERSION_MINOR=${VERSION_MINOR} -DZ80EX_RELEASE_TYPE=${RELEASE_TYPE})

if (OPSTEP_FAST_AND_ROUGH)
    add_definitions (-DZ80EX_OPSTEP_FAST_AND_ROUGH)
endif ()

set(z80ex_sources z80ex.c)
add_library(z80ex STATIC ${z80ex_sources})
