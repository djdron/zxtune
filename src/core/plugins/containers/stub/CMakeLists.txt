project(libcore_plugins_containers_stub)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_plugins_containers_stub ${SRCS_CXX} ${SRCS_H})
