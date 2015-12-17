project(libdevices_fm)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(devices_fm ${SRCS_CXX} ${SRCS_H})
set_property(TARGET devices_fm PROPERTY FOLDER "devices")
