project(libcore_plugins_players)

file(GLOB SRCS_CXX "*.cpp" "ay/*.cpp" "dac/*.cpp" "gme/*.cpp" "multi/*.cpp" "saa/*.cpp" "sid/*.cpp" "tfm/*.cpp" "xmp/*.cpp")
file(GLOB SRCS_H "*.h" "ay/*.h" "dac/*.h" "gme/*.h" "multi/*.h" "saa/*.h" "sid/*.h" "tfm/*.h" "xmp/*.h")

add_definitions(-DHAVE_CONFIG_H)
include_directories("../../../../3rdparty/sidplayfp")

add_library(core_plugins_players ${SRCS_CXX} ${SRCS_H})
