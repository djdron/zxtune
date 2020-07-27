project(libmodule_players)

include_directories("../../../3rdparty/mgba/include")

file(GLOB SRCS_CXX "aym/*.cpp" "dac/*.cpp" "saa/*.cpp" "tfm/*.cpp" "xsf/*.cpp" "*.cpp")
file(GLOB SRCS_H "aym/*.h" "dac/*.h" "saa/*.h" "tfm/*.h" "xsf/*.h" "*.h")

add_library(module_players ${SRCS_CXX} ${SRCS_H})
set_property(TARGET module_players PROPERTY FOLDER "module")
