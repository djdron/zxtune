project(libmodule_properties)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(module_properties ${SRCS_CXX} ${SRCS_H})
set_property(TARGET module_properties PROPERTY FOLDER "module")
