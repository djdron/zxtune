project(libasap)

set(SRCS_C "asap.c")

add_library(asap ${SRCS_C})
set_property(TARGET asap PROPERTY FOLDER "3rdparty")
