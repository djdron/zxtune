cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(foo_input_zxtune)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DHAVE_BOOST)
add_definitions(-DNO_DEBUG_LOGS)
add_definitions(-DFMT_DEPRECATED_HEAVY_CORE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if("${FOOBAR_SDK_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find the foobar2000 SDK. Please set FOOBAR_SDK_ROOT to the root directory containing foobar2000 SDK. Get it here: http://www.foobar2000.org/SDK")
elseif(NOT EXISTS "${FOOBAR_SDK_ROOT}/foobar2000/SDK/foobar2000.h")
message(FATAL_ERROR "Unable to find the foobar2000 SDK in ${FOOBAR_SDK_ROOT}")
else()
include_directories("${FOOBAR_SDK_ROOT}")
include_directories("${FOOBAR_SDK_ROOT}/foobar2000")
endif()

if(WIN32)
if("${WTL_ROOT}" STREQUAL "")
message(FATAL_ERROR "Unable to find WTL. Please set WTL_ROOT to the root directory containing WTL. Get it here: http://wtl.sourceforge.net/")
elseif(NOT EXISTS "${WTL_ROOT}/Include/atlapp.h")
message(FATAL_ERROR "Unable to find WTL in ${WTL_ROOT}")
else()
include_directories("${WTL_ROOT}/Include")
endif()
endif(WIN32)

include_directories("../..")
include_directories("../../include")
include_directories("../../src")
include_directories("../../3rdparty/zlib")

cmake_path(SET SOURCES_ROOTX NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../..")
cmake_path(GET SOURCES_ROOTX PARENT_PATH SOURCES_ROOT)
add_definitions(-DSOURCES_ROOT="${SOURCES_ROOT}")

if(MSVC)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
add_compile_options(-Wno-deprecated-literal-operator -Wno-gnu-string-literal-operator-template -Wno-microsoft-include -Wno-deprecated-non-prototype)
add_compile_options(-Wno-bool-operation -Wno-unused-but-set-variable -Wno-unused-variable -Wno-misleading-indentation -Wno-unused-function)
add_compile_options(/showFilenames)
endif()

add_compile_options(/MP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4244 /wd4800 /wd4996 /wd4267 /wd4101")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4090 /wd4146")
endif(MSVC)

#3rdparty
add_subdirectory("../../3rdparty/asap" "asap")
add_subdirectory("../../3rdparty/gme" "gme")
add_subdirectory("../../3rdparty/he" "he")
add_subdirectory("../../3rdparty/ht" "ht")
add_subdirectory("../../3rdparty/hvl" "hvl")
add_subdirectory("../../3rdparty/lazyusf2" "lazyusf2")
add_subdirectory("../../3rdparty/lhasa" "lhasa")
add_subdirectory("../../3rdparty/mgba" "mgba")
add_subdirectory("../../3rdparty/ogg" "ogg")
add_subdirectory("../../3rdparty/openmpt" "openmpt")
add_subdirectory("../../3rdparty/opus" "opus")
add_subdirectory("../../3rdparty/sidplayfp" "sidplayfp")
add_subdirectory("../../3rdparty/snesspc" "snesspc")
add_subdirectory("../../3rdparty/sseqplayer" "sseqplayer")
add_subdirectory("../../3rdparty/v2m" "v2m")
add_subdirectory("../../3rdparty/vgm" "vgm")
#add_subdirectory("../../3rdparty/vgmstream" "vgmstream")
add_subdirectory("../../3rdparty/vio2sf" "vio2sf")
add_subdirectory("../../3rdparty/vorbis" "vorbis")
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
#add_subdirectory("../../src/module" "module")
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

set(LIBS asap gme he ht hvl lazyusf2 lhasa mgba ogg openmpt opus sidplayfp snesspc sseqplayer v2m vgm vio2sf vorbis xmp z80ex zlib analysis binary binary_compression binary_format core core_plugins_archives core_plugins_players dbg devices_aym devices_beeper devices_dac devices_fm devices_saa devices_z80 formats_archived formats_archived_multitrack formats_chiptune formats_packed formats_multitrack l10n_stub module_conversion module_players module_properties parameters platform_version sound strings tools)

if(WIN32)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
if (CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64EC")
set(ARCH "ARM64EC/")
set(ARCH2 "-ARM64EC")
else()
set(ARCH "x64/")
set(ARCH2 "-x64")
endif()
else()
set(ARCH "")
set(ARCH2 "-Win32")
endif()

set(FOOBAR_SDK_LIBRARIES
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/SDK/${ARCH}Release/foobar2000_SDK.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/${ARCH}Release/foobar2000_component_client.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/helpers/${ARCH}Release/foobar2000_sdk_helpers.lib"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared${ARCH2}.lib"
	optimized "${FOOBAR_SDK_ROOT}/pfc/${ARCH}Release/pfc.lib"
	optimized "${FOOBAR_SDK_ROOT}/libPPUI/${ARCH}Release/libPPUI.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/SDK/${ARCH}Debug/foobar2000_SDK.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/${ARCH}Debug/foobar2000_component_client.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/helpers/${ARCH}Debug/foobar2000_sdk_helpers.lib"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/shared/shared${ARCH2}.lib"
	debug "${FOOBAR_SDK_ROOT}/pfc/${ARCH}Debug/pfc.lib"
	debug "${FOOBAR_SDK_ROOT}/libPPUI/${ARCH}Debug/libPPUI.lib"
)

elseif(APPLE)

set(FOOBAR_SDK_LIBRARIES
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/SDK/build/Release/libfoobar2000_SDK.a"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/build/Release/libfoobar2000_component_client.a"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/helpers/build/Release/libfoobar2000_SDK_helpers.a"
	optimized "${FOOBAR_SDK_ROOT}/foobar2000/shared/build/Release/libshared.a"
	optimized "${FOOBAR_SDK_ROOT}/pfc/build/Release/libpfc-Mac.a"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/SDK/build/Debug/libfoobar2000_SDK.a"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/foobar2000_component_client/build/Debug/libfoobar2000_component_client.a"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/helpers/build/Debug/libfoobar2000_SDK_helpers.a"
	debug "${FOOBAR_SDK_ROOT}/foobar2000/shared/build/Debug/libshared.a"
	debug "${FOOBAR_SDK_ROOT}/pfc/build/Debug/libpfc-Mac.a"
)

endif(WIN32)

add_library(foo_input_zxtune MODULE ${SRCS_CXX} ${SRCS_H} ${RES_RC})
target_link_libraries(foo_input_zxtune ${LIBS} ${FOOBAR_SDK_LIBRARIES})
if(APPLE)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
target_link_libraries(foo_input_zxtune "-framework CoreFoundation" "-framework Cocoa")
set_target_properties(${PROJECT_NAME} PROPERTIES
	BUNDLE True
	BUNDLE_EXTENSION component
	MACOSX_BUNDLE_BUNDLE_NAME foo_input_zxtune
	MACOSX_BUNDLE_BUNDLE_VERSION 0.0.10
	MACOSX_BUNDLE_GUI_IDENTIFIER com.djdron.foo-input-zxtune
	MACOSX_BUNDLE_ICON_FILE AppIcon
	MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.10"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
	XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.djdron.foo-input-zxtune"
)
endif(APPLE)
set_property(TARGET foo_input_zxtune PROPERTY FOLDER "apps")
