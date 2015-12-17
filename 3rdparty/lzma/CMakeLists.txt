project(liblzma)

add_definitions(-D_7ZIP_PPMD_SUPPPORT)

set(SRCS_C "C/7zArcIn.c"
	"C/7zBuf.c"
	"C/7zBuf2.c"
	"C/7zCrc.c"
	"C/7zCrcOpt.c"
	"C/7zDec.c"
	"C/7zStream.c"
	"C/Bcj2.c"
	"C/Bra.c"
	"C/Bra86.c"
	"C/BraIA64.c"
	"C/CpuArch.c"
	"C/Delta.c"
	"C/LzmaDec.c"
	"C/Lzma2Dec.c"
	"C/Ppmd7.c"
	"C/Ppmd7Dec.c")

file(GLOB SRCS_H "C/*.h")

add_library(lzma ${SRCS_C} ${SRCS_H})
set_property(TARGET lzma PROPERTY FOLDER "3rdparty")
