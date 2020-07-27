project(libmgba)

include_directories("./include")
include_directories("./src")

add_definitions(-DMINIMAL_CORE=3 -DDISABLE_THREADING -DDISABLE_LOGGING -DM_CORE_GBA -DHAVE_STRDUP -DHAVE_STRNDUP -DUSE_VFS_FILE)

set(SRCS_C
	"src/arm/arm.c" "src/arm/decoder-arm.c" "src/arm/decoder-thumb.c" "src/arm/isa-arm.c" "src/arm/isa-thumb.c"
	"src/core/cheats.c" "src/core/config.c" "src/core/core.c" "src/core/interface.c" "src/core/sync.c"
	"src/core/tile-cache.c" "src/core/timing.c"
	"src/gb/gb_audio.c"
	"src/gba/gba_audio.c" "src/gba/gba_core.c" "src/gba/gba_cheats.c" "src/gba/gba.c" "src/gba/gba_bios.c"
	"src/gba/gba_dma.c" "src/gba/gba_hardware.c" "src/gba/gba_io.c" "src/gba/gba_memory.c" "src/gba/gba_overrides.c"
	"src/gba/gba_savedata.c" "src/gba/gba_serialize.c" "src/gba/gba_sio.c" "src/gba/gba_timer.c" "src/gba/gba_vfame.c"
	"src/gba/gba_video.c" "src/gba/hle-bios.c"
	"src/gba/cheats/codebreaker.c" "src/gba/cheats/gameshark.c" "src/gba/cheats/parv3.c"
	"src/third-party/blip_buf/blip_buf.c" "src/third-party/inih/ini.c"
	"src/util/circle-buffer.c" "src/util/configuration.c" "src/util/crc32.c" "src/util/formatting.c"
	"src/util/patch.c" "src/util/patch-ips.c" "src/util/patch-ups.c" "src/util/hash.c" "src/util/string.c"
	"src/util/table.c" "src/util/vfs.c" "src/util/vfs/vfs-file.c" "src/util/vfs/vfs-mem.c"
	"src/platform/portable/memory.c"
)

file(GLOB SRCS_H "src/gba/*.h" "src/gba/cheats/*.h")

add_library(mgba ${SRCS_C} ${SRCS_H})
set_property(TARGET mgba PROPERTY FOLDER "3rdparty")
