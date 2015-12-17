project(libdevices_dac)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(devices_dac ${SRCS_CXX} ${SRCS_H})
set_property(TARGET devices_dac PROPERTY FOLDER "devices")
