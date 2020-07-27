project(libvio2sf)

add_definitions(-DNO_ASM)

file(GLOB SRCS_C "desmume/*.c")
file(GLOB SRCS_C_CXX "desmume/*.cpp")
file(GLOB SRCS_H "desmume/*.h")

add_library(vio2sf ${SRCS_H} ${SRCS_C} ${SRCS_C_CXX})
set_property(TARGET vio2sf PROPERTY FOLDER "3rdparty")
