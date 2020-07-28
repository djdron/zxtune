project(libsnesspc)

file(GLOB SRCS_CXX "snes_spc/SNES_SPC.cpp" "snes_spc/SNES_SPC_misc.cpp" "snes_spc/SPC_DSP.cpp" "snes_spc/SPC_Filter.cpp")
file(GLOB SRCS_H "snes_spc/*.h")

add_library(snesspc ${SRCS_CXX} ${SRCS_H})
set_property(TARGET snesspc PROPERTY FOLDER "3rdparty")
