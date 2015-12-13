project(libtools)

file(GLOB SRCS_CXX "src/*.cpp" "text/tools.cpp")
file(GLOB SRCS_H "src/*.h" "text/tools.h")

add_library(tools ${SRCS_CXX} ${SRCS_H})
