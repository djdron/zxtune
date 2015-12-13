project(libsound)

file(GLOB SRCS_CXX "impl/*.cpp")
file(GLOB SRCS_H "*.h" "impl/*.h")

add_library(sound ${SRCS_CXX} ${SRCS_H})
