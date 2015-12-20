project(libcore_plugins_containers)

file(GLOB SRCS_CXX "*.cpp" "full/*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_plugins_containers ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_plugins_containers PROPERTY FOLDER "core")
