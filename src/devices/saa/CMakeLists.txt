project(libdevices_saa)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(devices_saa ${SRCS_CXX} ${SRCS_H})
set_property(TARGET devices_saa PROPERTY FOLDER "devices")
