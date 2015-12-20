project(libdevices_aym_dumper)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(devices_aym_dumper ${SRCS_CXX} ${SRCS_H})
set_property(TARGET devices_aym_dumper PROPERTY FOLDER "devices")
