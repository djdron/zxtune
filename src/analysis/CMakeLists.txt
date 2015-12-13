project(libanalysis)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "src/*.h")

add_library(analysis ${SRCS_CXX} ${SRCS_H})
