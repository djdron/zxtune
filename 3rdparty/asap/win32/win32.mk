# MinGW for most ports
WIN_CC = $(DO)$(if $(findstring x64,$(@D)),x86_64,i686)-w64-mingw32-gcc $(WIN_CARGS) $(filter-out %.h,$^)
WIN_CXX = $(DO)$(if $(findstring x64,$(@D)),x86_64,i686)-w64-mingw32-g++ $(WIN_CARGS) -std=c++20 $(filter-out %.h %.hpp,$^)
WIN_WINDRES = $(DO)$(if $(findstring x64,$(@D)),x86_64,i686)-w64-mingw32-windres -o $@ $<
VLC_INCLUDE = ../vlc/include
VLC_LIB32 = "C:/Program Files (x86)/VideoLAN/VLC"
VLC_LIB64 = "C:/Program Files/VideoLAN/VLC"

# Microsoft compiler for foobar2000
FOOBAR2000_SDK_DIR = ../foobar2000_SDK

# Windows Installer XML
WIX = $(DO)wix

# Code signing
DO_SIGN = $(DO)signtool sign -d "ASAP - Another Slight Atari Player $(VERSION)" -n "Open Source Developer, Piotr Fusik" -tr http://time.certum.pl -fd sha256 -td sha256 $^ && touch $@

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "win32.mk")
endif

comma = ,
WIN_CARGS = -O2 -Wall -Wl,--nxcompat -o $@ $(if $(filter %.dll,$@),-shared -Wl$(comma)-subsystem$(comma)windows) $(INCLUDEOPTS) -static -s
WIN_CL = $(DO)$(if $(filter-out %.obj,$@),mkdir -p win32/obj/$@ && )win32/foobar2000/msvc$(if $(findstring x64,$(@D)),64,32).bat \
	cl -std:c++20 -GR- -GS- -wd4996 -DNDEBUG -nologo -O2 -GL -W3 $(if $(filter %.obj,$@),-c -Fo$@,-Fe$@ -Fowin32/obj/$@/) $(if $(filter %.dll,$@),-LD) $(INCLUDEOPTS) $(filter-out %.h,$^)

mingw: $(addprefix win32/,asapconv.exe libasap.a asapscan.exe wasap.exe bass_asap.dll in_asap.dll xmp-asap.dll apokeysnd.dll ASAPShellEx.dll)
.PHONY: mingw

# asapconv

win32/asapconv.exe win32/x64/asapconv.exe: $(call src,asapconv.c asap-stdio.[ch] asap.[ch])
	$(WIN_CC) -DHAVE_LIBMP3LAME -DHAVE_LIBMP3LAME_DLL
CLEAN += win32/asapconv.exe win32/x64/asapconv.exe

win32/asapconv-static-lame.exe win32/x64/asapconv-static-lame.exe: $(call src,asapconv.c asap-stdio.[ch] asap.[ch])
	$(WIN_CC) -DHAVE_LIBMP3LAME -lmp3lame
CLEAN += win32/asapconv-static-lame.exe win32/x64/asapconv-static-lame.exe

win32/asapconv-no-lame.exe win32/x64/asapconv-no-lame.exe: $(call src,asapconv.c asap-stdio.[ch] asap.[ch])
	$(WIN_CC)
CLEAN += win32/asapconv-no-lame.exe win32/x64/asapconv-no-lame.exe

win32/msvc/asapconv.exe win32/msvc/x64/asapconv.exe: $(call src,asapconv.c asap-stdio.[ch] asap.[ch])
	$(WIN_CL) -DHAVE_LIBMP3LAME -DHAVE_LIBMP3LAME_DLL
CLEAN += win32/msvc/asapconv.exe win32/msvc/x64/asapconv.exe

# lib

win32/libasap.a: win32/asap.o
	$(DO_AR)
CLEAN += win32/libasap.a

win32/asap.o win32/x64/asap.o: $(call src,asap.[ch])
	$(WIN_CC) -c
CLEAN += win32/asap.o win32/x64/asap.o

# SDL

win32/asap-sdl.exe: $(call src,asap-sdl.c asap.[ch])
	$(WIN_CC) -lmingw32 -lSDLmain -lSDL
CLEAN += win32/asap-sdl.exe

# asapscan

win32/asapscan.exe win32/x64/asapscan.exe: $(call src,asapscan.c asap-stdio.[ch] asap-asapscan.h)
	$(WIN_CC)
CLEAN += win32/asapscan.exe win32/x64/asapscan.exe

# sap2txt

win32/sap2txt.exe win32/x64/sap2txt.exe: $(srcdir)sap2txt.c
	$(WIN_CC) -static -lz
CLEAN += win32/sap2txt.exe win32/x64/sap2txt.exe

# WASAP

win32/wasap.exe: $(call src,win32/wasap/wasap.[ch] asap.[ch] astil.[ch] win32/info_dlg.[ch]) win32/wasap/wasap-res.o
	$(WIN_CC) -Wl,-subsystem,windows -DWASAP -lcomctl32 -lcomdlg32 -lwinmm
CLEAN += win32/wasap.exe

win32/x64/wasap.exe: $(call src,win32/wasap/wasap.[ch] asap.[ch] astil.[ch] win32/info_dlg.[ch]) win32/x64/wasap-res.o
	$(WIN_CC) -Wl,-subsystem,windows -DWASAP -lcomctl32 -lcomdlg32 -lwinmm
CLEAN += win32/x64/wasap.exe

win32/wasap/wasap-res.o win32/x64/wasap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/wasap/wasap.h win32/wasap/wasap.ico win32/wasap/play.ico win32/wasap/stop.ico)
	$(WIN_WINDRES) -DWASAP
CLEAN += win32/wasap/wasap-res.o win32/x64/wasap-res.o

# VLC

win32/libasap_plugin.dll: $(call src,vlc/libasap_plugin.c asap.[ch]) win32/libasap_plugin-res.o
	$(WIN_CC:-static=-static-libgcc) -I$(VLC_INCLUDE) -L$(VLC_LIB32) -lvlccore
CLEAN += win32/libasap_plugin.dll

win32/x64/libasap_plugin.dll: $(call src,vlc/libasap_plugin.c asap.[ch]) win32/x64/libasap_plugin-res.o
	$(WIN_CC:-static=-static-libgcc) -I$(VLC_INCLUDE) -L$(VLC_LIB64) -lvlccore
CLEAN += win32/x64/libasap_plugin.dll

win32/libasap_plugin-res.o win32/x64/libasap_plugin-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN_WINDRES) -DVLC
CLEAN += win32/libasap_plugin-res.o win32/x64/libasap_plugin-res.o

# BASS

win32/bass_asap.dll: $(call src,win32/bass/bass_asap.c asap.[ch] win32/bass/bass-addon.h win32/bass/bass.h win32/bass/bass.lib) win32/bass/bass_asap-res.o
	$(WIN_CC) -Wl,--kill-at -DBASS
CLEAN += win32/bass_asap.dll

win32/x64/bass_asap.dll: $(call src,win32/bass/bass_asap.c asap.[ch] win32/bass/bass-addon.h win32/bass/bass.h win32/bass/x64/bass.lib) win32/bass/x64/bass_asap-res.o
	$(WIN_CC) -DBASS
CLEAN += win32/x64/bass_asap.dll

win32/bass/bass_asap-res.o win32/bass/x64/bass_asap-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN_WINDRES) -DBASS
CLEAN += win32/bass/bass_asap-res.o win32/bass/x64/bass_asap-res.o

# foobar2000

win32/foo_asap.dll win32/x64/foo_asap.dll: $(call src,win32/foobar2000/foo_asap.cpp asap.[ch] astil.[ch] aatr-stdio.[ch] aatr.h win32/info_dlg.[ch] win32/settings_dlg.[ch]) win32/foobar2000/foo_asap.res \
	$(FOOBAR2000_SDK_DIR)/foobar2000/foobar2000_component_client/component_client.cpp \
	$(patsubst %, $(FOOBAR2000_SDK_DIR)/foobar2000/SDK/%.cpp, abort_callback album_art app_close_blocker audio_chunk audio_chunk_channel_config \
		cfg_var cfg_var_legacy commonObjects completion_notify configStore console file_info file_info_impl file_info_merge filesystem \
		filesystem_helper foosort fsItem guids input input_file_type main_thread_callback metadb_handle metadb_handle_list \
		playable_location playlist preferences_page replaygain_info service titleformat utility) \
	$(patsubst %, $(FOOBAR2000_SDK_DIR)/pfc/%.cpp, audio_math audio_sample bit_array bsearch cpuid filehandle guid other pathUtils sort \
		splitString2 string-compare string-lite string_base string_conv string-conv-lite threads timers unicode-normalize utf8 win-objects)
	$(WIN_CL) -DFOOBAR2000 -DWIN32 -DUNICODE -D_UNICODE -EHsc -I$(FOOBAR2000_SDK_DIR) -I$(FOOBAR2000_SDK_DIR)/foobar2000 \
		$(FOOBAR2000_SDK_DIR)/foobar2000/shared/shared-$(if $(findstring x64,$(@D)),x64,Win32).lib comctl32.lib comdlg32.lib ole32.lib shell32.lib shlwapi.lib user32.lib -link -release -noexp -noimplib
CLEAN += win32/foo_asap.dll win32/x64/foo_asap.dll

win32/foobar2000/foo_asap.res: $(call src,win32/gui.rc asap.h win32/settings_dlg.h)
	$(WIN_WINDRES) -DFOOBAR2000
CLEAN += win32/foobar2000/foo_asap.res

# Winamp

win32/in_asap.dll: $(call src,win32/winamp/in_asap.c asap.[ch] astil.[ch] aatr-stdio.[ch] aatr.h win32/info_dlg.[ch] win32/settings_dlg.[ch] win32/winamp/in2.h win32/winamp/out.h win32/winamp/ipc_pe.h win32/winamp/wa_ipc.h) win32/winamp/in_asap-res.o
	$(WIN_CC) -DWINAMP -lcomctl32 -lcomdlg32
CLEAN += win32/in_asap.dll

win32/winamp/in_asap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/settings_dlg.h)
	$(WIN_WINDRES) -DWINAMP
CLEAN += win32/winamp/in_asap-res.o

# XMPlay

win32/xmp-asap.dll: $(call src,win32/xmplay/xmp-asap.c asap.[ch] astil.[ch] win32/info_dlg.[ch] win32/settings_dlg.[ch] win32/xmplay/xmpin.h win32/xmplay/xmpfunc.h) win32/xmplay/xmp-asap-res.o
	$(WIN_CC) -Wl,--kill-at -DXMPLAY -lcomctl32 -lcomdlg32
CLEAN += win32/xmp-asap.dll

win32/xmplay/xmp-asap-res.o: $(call src,win32/gui.rc asap.h win32/info_dlg.h win32/settings_dlg.h)
	$(WIN_WINDRES) -DXMPLAY
CLEAN += win32/xmplay/xmp-asap-res.o

# Raster Music Tracker

win32/apokeysnd.dll: $(call src,win32/rmt/apokeysnd_dll.c) win32/rmt/pokey.c win32/rmt/pokey.h win32/rmt/apokeysnd-res.o
	$(WIN_CC) --std=c99 -DAPOKEYSND
CLEAN += win32/apokeysnd.dll

win32/rmt/pokey.h: $(srcdir)pokey.fu | win32/rmt/pokey.c

win32/rmt/pokey.c: $(srcdir)pokey.fu
	$(FUT) -D APOKEYSND
CLEAN += win32/rmt/pokey.c win32/rmt/pokey.h

win32/rmt/apokeysnd-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN_WINDRES) -DAPOKEYSND
CLEAN += win32/rmt/apokeysnd-res.o

# ASAPShellEx

win32/ASAPShellEx.dll: $(call src,win32/shellex/ASAPShellEx.cpp asap.h) win32/asap.o win32/shellex/ASAPShellEx-res.o
	$(WIN_CXX) -Wl,--kill-at -lole32 -loleaut32 -lshlwapi -luuid -lpropsys
CLEAN += win32/ASAPShellEx.dll

win32/x64/ASAPShellEx.dll: $(call src,win32/shellex/ASAPShellEx.cpp asap.h) win32/x64/asap.o win32/x64/ASAPShellEx-res.o
	$(WIN_CXX) -Wl,--kill-at -lole32 -loleaut32 -lshlwapi -luuid -lpropsys
CLEAN += win32/x64/ASAPShellEx.dll

win32/shellex/ASAPShellEx-res.o win32/x64/ASAPShellEx-res.o: $(call src,win32/gui.rc asap.h)
	$(WIN_WINDRES) -DSHELLEX
CLEAN += win32/shellex/ASAPShellEx-res.o win32/x64/ASAPShellEx-res.o

# setups

win32/setup: release/asap-$(VERSION)-win32.msi
.PHONY: win32/setup

release/asap-$(VERSION)-win32.msi: $(call src,win32/setup/asap.wxs release/release.mk win32/wasap/wasap.ico win32/setup/license.rtf win32/setup/asap-banner.jpg win32/setup/asap-dialog.jpg win32/diff-sap.js win32/shellex/ASAPShellEx.propdesc) \
	$(addprefix win32/,asapconv.exe sap2txt.exe wasap.exe in_asap.dll xmp-asap.dll bass_asap.dll apokeysnd.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll signed)
	$(WIX) build -o $@ -d VERSION=$(VERSION) -ext WixToolset.UI.wixext -b win32 -b release -b $(srcdir)win32/setup -b $(srcdir)win32 $<

release/asap-$(VERSION)-win64.msi: $(call src,win32/setup/asap.wxs release/release.mk win32/wasap/wasap.ico win32/setup/license.rtf win32/setup/asap-banner.jpg win32/setup/asap-dialog.jpg win32/shellex/ASAPShellEx.propdesc) \
	$(addprefix win32/,in_asap.dll xmp-asap.dll apokeysnd.dll signed) \
	$(addprefix win32/x64/,asapconv.exe sap2txt.exe wasap.exe bass_asap.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll)
	$(WIX) build -o $@ -d VERSION=$(VERSION) -ext WixToolset.UI.wixext -b win32 -arch x64 -b $(srcdir)/win32/setup -b $(srcdir)win32 $<

win32/signed: $(addprefix win32/,asapconv.exe asapscan.exe sap2txt.exe wasap.exe in_asap.dll xmp-asap.dll bass_asap.dll apokeysnd.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll) \
	$(addprefix win32/x64/,asapconv.exe asapscan.exe sap2txt.exe wasap.exe bass_asap.dll ASAPShellEx.dll foo_asap.dll libasap_plugin.dll)
	$(DO_SIGN)
CLEAN += win32/signed

release/signed-msi: release/asap-$(VERSION)-win32.msi release/asap-$(VERSION)-win64.msi
	$(DO_SIGN)
CLEAN += release/signed-msi
