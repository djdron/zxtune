project(libcore_plugins_archives_stub)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_plugins_archives_stub ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_plugins_archives_stub PROPERTY FOLDER "core")
