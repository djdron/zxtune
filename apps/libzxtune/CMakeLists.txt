cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(libzxtune)

include_directories("../../../lib/boost_1_59_0")
add_definitions(-DHAVE_BOOST)
add_definitions(-DNO_DEBUG_LOGS)

include_directories("../..")
include_directories("../../include")
include_directories("../../src")

set(CMAKE_CXX_FLAGS "-Wno-unused-local-typedef")

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})


#3rdparty
add_subdirectory("../../3rdparty/gme" "gme")
add_subdirectory("../../3rdparty/hvl" "hvl")
add_subdirectory("../../3rdparty/lhasa" "lhasa")
add_subdirectory("../../3rdparty/lzma" "lzma")
add_subdirectory("../../3rdparty/sidplayfp" "sidplayfp")
add_subdirectory("../../3rdparty/snesspc" "snesspc")
add_subdirectory("../../3rdparty/xmp" "xmp")
add_subdirectory("../../3rdparty/z80ex" "z80ex")

#core
add_subdirectory("../../src/analysis" "analysis")
add_subdirectory("../../src/binary" "binary")
add_subdirectory("../../src/binary/format" "binary_format")
add_subdirectory("../../src/core" "core")
add_subdirectory("../../src/core/plugins/archives/stub" "core_plugins_archives_stub")
add_subdirectory("../../src/core/plugins/containers/stub" "core_plugins_containers_stub")
add_subdirectory("../../src/core/plugins/players" "core_plugins_players")
add_subdirectory("../../src/debug" "debug")
add_subdirectory("../../src/devices/aym" "devices_aym")
add_subdirectory("../../src/devices/beeper" "devices_beeper")
add_subdirectory("../../src/devices/dac" "devices_dac")
add_subdirectory("../../src/devices/fm" "devices_fm")
add_subdirectory("../../src/devices/saa" "devices_saa")
add_subdirectory("../../src/devices/z80" "devices_z80")
add_subdirectory("../../src/formats/chiptune" "formats_chiptune")
add_subdirectory("../../src/formats/packed" "formats_packed")
add_subdirectory("../../src/formats/multitrack" "formats_multitrack")
add_subdirectory("../../src/l10n/stub" "l10n_stub")
add_subdirectory("../../src/parameters" "parameters")
add_subdirectory("../../src/platform/version" "platform_version")
add_subdirectory("../../src/sound" "sound")
add_subdirectory("../../src/strings" "strings")
add_subdirectory("../../src/tools" "tools")

file(GLOB SRCS_CXX "src/*.cpp")
set(SRCS_H "zxtune.h")

add_library(zxtune MODULE ${SRCS_CXX} ${SRCS_H})
add_dependencies(zxtune gme hvl lhasa lzma sidplayfp snesspc xmp z80ex analysis binary binary_format core core_plugins_archives_stub core_plugins_containers_stub core_plugins_players debug devices_aym devices_beeper devices_dac devices_fm devices_saa devices_z80 formats_chiptune formats_packed formats_multitrack l10n_stub parameters platform_version sound strings tools)
target_link_libraries(zxtune gme hvl lhasa lzma sidplayfp snesspc xmp z80ex analysis binary binary_format core core_plugins_archives_stub core_plugins_containers_stub core_plugins_players debug devices_aym devices_beeper devices_dac devices_fm devices_saa devices_z80 formats_chiptune formats_packed formats_multitrack l10n_stub parameters platform_version sound strings tools ${ZLIB_LIBRARIES})
