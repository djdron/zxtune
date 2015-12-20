project(libcore_plugins_archives)

file(GLOB SRCS_CXX "*.cpp" "full/*.cpp")
file(GLOB SRCS_H "*.h")

add_library(core_plugins_archives ${SRCS_CXX} ${SRCS_H})
set_property(TARGET core_plugins_archives PROPERTY FOLDER "core")
