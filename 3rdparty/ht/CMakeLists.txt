project(libht)

add_definitions(-DEMU_COMPILE -DEMU_LITTLE_ENDIAN -DLSB_FIRST -DC68K_NO_JUMP_TABLE -DNO_DYNAREC)

set(SRCS_C "Core/arm.c" "Core/dcsound.c" "Core/satsound.c" "Core/sega.c" "Core/yam.c" "Core/c68k/c68k.c" "Core/c68k/c68kexec.c")

file(GLOB SRCS_H "Core/*.h" "Core/c68k/*.h" "Core/m68k/*.h")

add_library(ht ${SRCS_C} ${SRCS_H})
set_property(TARGET ht PROPERTY FOLDER "3rdparty")
