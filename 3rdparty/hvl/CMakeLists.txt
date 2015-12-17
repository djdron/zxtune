project(libhvl)

file(GLOB SRCS_C "*.c")
file(GLOB SRCS_H "*.h")

add_library(hvl ${SRCS_C} ${SRCS_H})
set_property(TARGET hvl PROPERTY FOLDER "3rdparty")
