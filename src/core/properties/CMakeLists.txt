project(libcore_properties)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_properties ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_properties PROPERTY FOLDER "core")
