project(libbinary_format)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(binary_format ${SRCS_CXX} ${SRCS_H})
