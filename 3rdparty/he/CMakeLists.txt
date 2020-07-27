project(libhe)

add_definitions(-DEMU_COMPILE -DEMU_LITTLE_ENDIAN)

set(SRCS_C "Core/bios.c" "Core/iop.c" "Core/ioptimer.c" "Core/psx.c" "Core/r3000.c" "Core/spu.c" "Core/spucore.c" "Core/vfs.c")
file(GLOB SRCS_H "Core/*.h")

add_library(he ${SRCS_C} ${SRCS_H})
set_property(TARGET he PROPERTY FOLDER "3rdparty")
