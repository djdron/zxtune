project(libdbg)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "*.h" "src/*.h")

add_library(dbg ${SRCS_CXX} ${SRCS_H})
