project(libformats_packed)

file(GLOB SRCS_CXX "*.cpp" "../text/packed.cpp")
file(GLOB SRCS_H "*.h")

add_library(formats_packed ${SRCS_CXX} ${SRCS_H})
