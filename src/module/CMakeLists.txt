project(libmodule)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "src/*.h" "*.h")

add_library(module ${SRCS_CXX} ${SRCS_H})
set_property(TARGET module PROPERTY FOLDER "module")
