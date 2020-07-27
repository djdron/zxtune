project(libbinary_compression)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "*.h")

add_library(binary_compression ${SRCS_CXX} ${SRCS_H})
