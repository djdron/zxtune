dynamic_name := zxtune
dirs.root := ../../../../../..

source_dirs := .

libraries.common = analysis \
                   binary binary_compression binary_format \
                   core core_plugins_archives_lite core_plugins_players \
                   devices_aym devices_beeper devices_dac devices_fm devices_saa devices_z80 \
                   formats_chiptune formats_packed_archives formats_packed_decompilers formats_archived formats_archived_multitrack formats_multitrack \
                   module module_players \
                   parameters \
                   sound strings \
                   tools

libraries.3rdparty = asap ffmpeg FLAC gme he ht hvl lazyusf2 lhasa lzma mgba sidplayfp snesspc unrar v2m vgm vio2sf xmp z80ex zlib

ld_flags += -Wl,--version-script=libzxtune.version

include $(dirs.root)/makefile.mak

jni_plugin.h: ../java/app/zxtune/core/jni/Plugins.java
	javah -classpath ../java -o $@ app.zxtune.core.jni.Plugins

jni_global_options.h: ../java/app/zxtune/core/jni/GlobalOptions.java
	javah -classpath ../java -o $@ app.zxtune.core.jni.GlobalOptions

jni_module.h: ../java/app/zxtune/core/jni/JniModule.java
	javah -classpath ../java -o $@ app.zxtune.core.jni.JniModule

jni_player.h: ../java/app/zxtune/core/jni/JniPlayer.java
	javah -classpath ../java -o $@ app.zxtune.core.jni.JniPlayer

install_$(platform):
	$(call copyfile_cmd,$(target),$(DESTDIR))

install_debug:
	$(call copyfile_cmd,$(target).pdb,$(DESTDIR))
