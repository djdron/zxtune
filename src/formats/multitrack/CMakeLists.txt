project(libformats_multitrack)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h")

add_library(formats_multitrack ${SRCS_CXX} ${SRCS_H})
set_property(TARGET formats_multitrack PROPERTY FOLDER "formats")
