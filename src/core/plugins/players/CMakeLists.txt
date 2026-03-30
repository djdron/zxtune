project(libcore_plugins_players)

file(GLOB SRCS_CXX "*.cpp" "asap/*.cpp" "ay/*.cpp" "dac/*.cpp" "gme/*.cpp" "mpt/*.cpp" "multi/*.cpp" "saa/*.cpp" "sid/*.cpp" "tfm/*.cpp" "vgm/*.cpp" "vgmstream/*.cpp" "xmp/*.cpp" "xsf/*.cpp")
file(GLOB SRCS_H "*.h" "asap/*.h" "ay/*.h" "dac/*.h" "gme/*.h" "mpt/*.h" "multi/*.h" "saa/*.h" "sid/*.h" "tfm/*.h" "vgm/*.h" "vgmstream/*.h" "xmp/*.h" "xsf/*.h")

add_definitions(-DBUILDING_STATIC -DHAVE_STDINT_H)

include_directories("../../../../3rdparty/sidplayfp/src")

add_library(core_plugins_players ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_plugins_players PROPERTY FOLDER "core")
