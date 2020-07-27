project(liblazyusf2)

include_directories(".")

set(SRCS_C
	"ai/ai_controller.c" "api/callbacks.c" "main/main.c" "main/rom.c" "main/savestates.c"
	"main/util.c" "memory/memory_impl.c" "pi/cart_rom.c" "pi/pi_controller.c"
	"r4300/cached_interp.c" "r4300/cp0.c" "r4300/cp1.c" "r4300/exception.c" "r4300/interupt.c"
	"r4300/mi_controller.c" "r4300/r4300.c" "r4300/r4300_core.c" "r4300/recomp.c" "r4300/reset.c" "r4300/tlb.c"
	"rdp/rdp_core.c"
	"ri/rdram.c" "ri/rdram_detection_hack.c" "ri/ri_controller.c"
	"rsp/rsp_core.c"
	"rsp_hle/alist.c" "rsp_hle/alist_audio.c" "rsp_hle/alist_naudio.c" "rsp_hle/alist_nead.c" "rsp_hle/audio.c"
	"rsp_hle/cicx105.c" "rsp_hle/hle.c" "rsp_hle/jpeg.c" "rsp_hle/memory.c" "rsp_hle/mp3.c" "rsp_hle/musyx.c" "rsp_hle/plugin.c"
	"rsp_lle/rsp.c"
	"si/cic.c" "si/game_controller.c" "si/n64_cic_nus_6105.c" "si/pif.c" "si/si_controller.c"
	"usf/usf.c" "usf/resampler.c"
	"vi/vi_controller.c"
)

file(GLOB SRCS_H
	"ai/*.h" "api/*.h" "main/*.h" "memory/*.h" "r4300/*.h" "rdp/*.h" "ri/*.h"
	"rsp/*.h" "rsp_hle/*.h" "rsp_lle/*.h" "si/*.h" "usf/*.h" "vi/*.h"
)

add_library(lazyusf2 ${SRCS_H} ${SRCS_C})
set_property(TARGET lazyusf2 PROPERTY FOLDER "3rdparty")
