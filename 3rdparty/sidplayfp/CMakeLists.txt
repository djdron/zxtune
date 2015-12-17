project(libsidplayfp)

add_definitions(-DFAST_AND_ROUGH -DHAVE_CONFIG_H)
include_directories(".")
include_directories("sidplayfp")

file(GLOB SRCS_CXX "sidplayfp/*.cpp" "sidplayfp/c64/*.cpp" "sidplayfp/c64/CIA/*.cpp" "sidplayfp/c64/CPU/*.cpp" "sidplayfp/c64/VIC_II/*.cpp" "sidplayfp/sidtune/*.cpp"
	"utils/MD5/*.cpp"
	"builders/resid-builder/*.cpp"
	"builders/resid-builder/resid/*.cc")

file(GLOB SRCS_H "sidplayfp/*.h" "sidplayfp/c64/*.h" "sidplayfp/c64/CIA/*.h" "sidplayfp/c64/CPU/*.h" "sidplayfp/c64/VIC_II/*.h" "sidplayfp/sidtune/*.h"
	"utils/MD5/*.h"
	"builders/resid-builder/*.h"
	"builders/resid-builder/resid/*.h")

add_library(sidplayfp ${SRCS_CXX} ${SRCS_H})
set_property(TARGET sidplayfp PROPERTY FOLDER "3rdparty")
