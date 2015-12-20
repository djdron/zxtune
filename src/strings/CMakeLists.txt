project(libstrings)

file(GLOB SRCS_CXX "src/*.cpp" "text/time_format.cpp")
file(GLOB SRCS_H "*.h" "src/*.h" "text/time_format.h")

add_library(strings ${SRCS_CXX} ${SRCS_H})
