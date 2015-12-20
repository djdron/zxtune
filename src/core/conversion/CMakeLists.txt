project(libcore_conversion)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_conversion ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_conversion PROPERTY FOLDER "core")
