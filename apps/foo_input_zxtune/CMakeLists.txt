cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(foo_input_zxtune)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DHAVE_BOOST)
add_definitions(-DNO_DEBUG_LOGS)

if("${FOOBAR_SDK_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find the foobar2000 SDK. Please set FOOBAR_SDK_ROOT to the root directory containing foobar2000 SDK. Get it here: http://www.foobar2000.org/SDK")
elseif(NOT EXISTS "${FOOBAR_SDK_ROOT}/foobar2000/SDK/foobar2000.h")
message(FATAL_ERROR "Unable to find the foobar2000 SDK in ${FOOBAR_SDK_ROOT}")
else()
include_directories("${FOOBAR_SDK_ROOT}")
include_directories("${FOOBAR_SDK_ROOT}/foobar2000")
endif()

if("${WTL_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find WTL. Please set WTL_ROOT to the root directory containing WTL. Get it here: http://wtl.sourceforge.net/")
elseif(NOT EXISTS "${WTL_ROOT}/Include/atlapp.h")
message(FATAL_ERROR "Unable to find WTL in ${WTL_ROOT}")
else()
include_directories("${WTL_ROOT}/Include")
endif()

include_directories("../..")
include_directories("../../include")
include_directories("../../src")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4244 /wd4800 /wd4996 /wd4267 /wd4101")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4090 /wd4146")

#3rdparty
add_subdirectory("../../3rdparty/asap" "asap")
add_subdirectory("../../3rdparty/gme" "gme")
add_subdirectory("../../3rdparty/he" "he")
add_subdirectory("../../3rdparty/ht" "ht")
add_subdirectory("../../3rdparty/hvl" "hvl")
add_subdirectory("../../3rdparty/lazyusf2" "lazyusf2")
add_subdirectory("../../3rdparty/lhasa" "lhasa")
add_subdirectory("../../3rdparty/mgba" "mgba")
add_subdirectory("../../3rdparty/sidplayfp" "sidplayfp")
add_subdirectory("../../3rdparty/snesspc" "snesspc")
add_subdirectory("../../3rdparty/v2m" "v2m")
add_subdirectory("../../3rdparty/vgm" "vgm")
add_subdirectory("../../3rdparty/vio2sf" "vio2sf")
add_subdirectory("../../3rdparty/xmp" "xmp")
add_subdirectory("../../3rdparty/z80ex" "z80ex")
add_subdirectory("../../3rdparty/zlib" "zlib")

#core
add_subdirectory("../../src/analysis" "analysis")
add_subdirectory("../../src/binary" "binary")
add_subdirectory("../../src/binary/compression" "binary_compression")
add_subdirectory("../../src/binary/format" "binary_format")
add_subdirectory("../../src/core" "core")
add_subdirectory("../../src/core/plugins/archives" "core_plugins_archives")
add_subdirectory("../../src/core/plugins/players" "core_plugins_players")
add_subdirectory("../../src/debug" "dbg")
add_subdirectory("../../src/devices/aym" "devices_aym")
add_subdirectory("../../src/devices/beeper" "devices_beeper")
add_subdirectory("../../src/devices/dac" "devices_dac")
add_subdirectory("../../src/devices/fm" "devices_fm")
add_subdirectory("../../src/devices/saa" "devices_saa")
add_subdirectory("../../src/devices/z80" "devices_z80")
add_subdirectory("../../src/formats/archived" "formats_archived")
add_subdirectory("../../src/formats/archived/multitrack" "formats_archived_multitrack")
add_subdirectory("../../src/formats/chiptune" "formats_chiptune")
add_subdirectory("../../src/formats/packed" "formats_packed")
add_subdirectory("../../src/formats/multitrack" "formats_multitrack")
add_subdirectory("../../src/l10n/stub" "l10n_stub")
add_subdirectory("../../src/module" "module")
add_subdirectory("../../src/module/conversion" "module_conversion")
add_subdirectory("../../src/module/players" "module_players")
add_subdirectory("../../src/module/properties" "module_properties")
add_subdirectory("../../src/parameters" "parameters")
add_subdirectory("../../src/platform/version" "platform_version")
add_subdirectory("../../src/sound" "sound")
add_subdirectory("../../src/strings" "strings")
add_subdirectory("../../src/tools" "tools")

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "src/*.h")
file(GLOB RES_RC "src/*.rc")

set(LIBS asap gme he ht hvl lazyusf2 lhasa mgba sidplayfp snesspc v2m vgm vio2sf xmp z80ex zlib analysis binary binary_compression binary_format core core_plugins_archives core_plugins_players dbg devices_aym devices_beeper devices_dac devices_fm devices_saa devices_z80 formats_archived formats_archived_multitrack formats_chiptune formats_packed formats_multitrack l10n_stub module module_conversion module_players module_properties parameters platform_version sound strings tools)

set(FOOBAR_SDK_LIBRARIES
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/SDK/Release/foobar2000_SDK.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/Release/foobar2000_component_client.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/helpers/Release/foobar2000_sdk_helpers.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared.lib"
	optimized "${FOOBAR_SDK_ROOT}/pfc/Release/pfc.lib"
	optimized "${FOOBAR_SDK_ROOT}/libPPUI/Release/libPPUI.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/SDK/Debug/foobar2000_SDK.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/Debug/foobar2000_component_client.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/helpers/Debug/foobar2000_sdk_helpers.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared.lib"
	debug "${FOOBAR_SDK_ROOT}/pfc/Debug/pfc.lib"
	debug "${FOOBAR_SDK_ROOT}/libPPUI/Debug/libPPUI.lib"
)

add_library(foo_input_zxtune MODULE ${SRCS_CXX} ${SRCS_H} ${RES_RC})
add_dependencies(foo_input_zxtune ${LIBS})
target_link_libraries(foo_input_zxtune ${LIBS} ${FOOBAR_SDK_LIBRARIES})
set_property(TARGET foo_input_zxtune PROPERTY FOLDER "apps")
