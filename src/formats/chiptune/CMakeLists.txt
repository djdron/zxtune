project(libformats_chiptune)

file(GLOB SRCS_CXX "*.cpp" "aym/*.cpp" "digital/*.cpp" "emulation/*.cpp" "fm/*.cpp" "multidevice/*.cpp" "multitrack/*.cpp" "saa/*.cpp" "../text/chiptune.cpp")
file(GLOB SRCS_H "*.h" "aym/*.h" "digital/*.h" "emulation/*.h" "fm/*.h" "multidevice/*.h" "multitrack/*.h" "saa/*.h")

add_library(formats_chiptune ${SRCS_CXX} ${SRCS_H})
set_property(TARGET formats_chiptune PROPERTY FOLDER "formats")
