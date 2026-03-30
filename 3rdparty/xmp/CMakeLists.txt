project(libxmp)

include_directories("include" "src")
add_definitions(-DBUILDING_STATIC -DNO_VERSIONED_SYMBOLS -DDECRUNCH_MAX=0 -DNO_PROWIZARD -DNO_EXTERNALFILES -DNO_COMPOSITE_LOADER -DPATH_MAX=1024)

set(SRCS_C
	"src/control.c" "src/dataio.c" "src/effects.c" "src/envelope.c" "src/extras.c" "src/filter.c" "src/fnmatch.c" "src/format.c" "src/hio.c" "src/hmn_extras.c" "src/lfo.c"
	"src/load.c" "src/load_helpers.c" "src/md5.c" "src/med_extras.c" "src/misc.c" "src/mixer.c" "src/mix_all.c" "src/period.c" "src/player.c" "src/read_event.c" "src/scan.c" "src/smix.c" "src/virtual.c"

	"src/loaders/common.c" "src/loaders/iff.c" "src/loaders/sample.c" "src/loaders/mmd_common.c"
	"src/loaders/dtt_load.c" "src/loaders/emod_load.c" "src/loaders/fnk_load.c" "src/loaders/liq_load.c"
	"src/loaders/med2_load.c" "src/loaders/med3_load.c" "src/loaders/med4_load.c"
	"src/loaders/no_load.c" "src/loaders/stim_load.c" "src/loaders/stx_load.c"
)

file(GLOB SRCS_H "src/*.h" "src/depackers/*.h" "src/loaders/*.h" "src/loaders/prowizard/*.h")

if(WIN32)
add_definitions(-Dinline=__inline -D_USE_MATH_DEFINES)
endif(WIN32)

add_library(xmp ${SRCS_C} ${SRCS_H})
set_property(TARGET xmp PROPERTY FOLDER "3rdparty")
