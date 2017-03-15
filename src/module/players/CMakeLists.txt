project(libmodule_players)

file(GLOB SRCS_CXX "aym/*.cpp" "dac/*.cpp" "saa/*.cpp" "tfm/*.cpp" "*.cpp")
file(GLOB SRCS_H "aym/*.h" "dac/*.h" "saa/*.h" "tfm/*.h" "*.h")

add_library(module_players ${SRCS_CXX} ${SRCS_H})
set_property(TARGET module_players PROPERTY FOLDER "module")
