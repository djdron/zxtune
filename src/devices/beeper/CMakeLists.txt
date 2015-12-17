project(libdevices_beeper)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(devices_beeper ${SRCS_CXX} ${SRCS_H})
set_property(TARGET devices_beeper PROPERTY FOLDER "devices")
