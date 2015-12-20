project(libasync)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "src/*.h" "*.h")

add_library(async ${SRCS_CXX} ${SRCS_H})
