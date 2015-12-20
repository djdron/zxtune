project(libunrar)

add_definitions("-DSILENT -DRAR_NOCRYPT -DGUI -DSHELL_EXT -DNOVOLUME")

set(SRCS_CXX "getbits.cpp" "unpack.cpp" "rarvm.cpp" "crc.cpp" "smallfn.cpp" "system.cpp" "global.cpp" "rdwrfn.cpp" "stubs/errhnd_stub.cpp" "stubs/file_stub.cpp")
file(GLOB SRCS_H "*.h")

add_library(unrar ${SRCS_CXX} ${SRCS_H})
set_property(TARGET unrar PROPERTY FOLDER "3rdparty")
