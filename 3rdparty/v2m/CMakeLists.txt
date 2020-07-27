project(libv2m)

add_definitions(-DRONAN)

set(SRCS_C_CXX "src/ronan.cpp" "src/sounddef.cpp" "src/synth_core.cpp" "src/v2mconv.cpp" "src/v2mplayer.cpp")

file(GLOB SRCS_H "src/*.h")

add_library(v2m ${SRCS_C_CXX} ${SRCS_H})
set_property(TARGET v2m PROPERTY FOLDER "3rdparty")
