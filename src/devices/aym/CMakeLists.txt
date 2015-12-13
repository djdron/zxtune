project(libdevices_aym)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "*.h" "src/*.h")

add_library(devices_aym ${SRCS_CXX} ${SRCS_H})
