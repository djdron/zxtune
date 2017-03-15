project(libmodule_conversion)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(module_conversion ${SRCS_CXX} ${SRCS_H})
set_property(TARGET module_conversion PROPERTY FOLDER "module")
