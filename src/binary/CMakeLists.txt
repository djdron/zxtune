project(libbinary)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "*.h" "src/*.h")

add_library(binary ${SRCS_CXX} ${SRCS_H})
