project(libvgm)

add_definitions(-DNO_EXTERNAL_ROM -DSNDDEV_SELECT)
add_definitions(-DSNDDEV_SN76496 -DEC_SN76496_MAME -DSNDDEV_YM2413 -DEC_YM2413_MAME)
add_definitions(-DSNDDEV_YM2612 -DEC_YM2612_GPGX -DSNDDEV_YM2151 -DSNDDEV_SEGAPCM)
add_definitions(-DSNDDEV_RF5C68 -DEC_RF5C68_MAME -DSNDEMU_RF5C68_GENS -DSNDDEV_YM2203 -DSNDDEV_YM2608 -DSNDDEV_YM2610)
add_definitions(-DSNDDEV_YM3812 -DEC_YM3812_ADLIBEMU -DSNDDEV_YM3526 -DSNDDEV_Y8950)
add_definitions(-DSNDDEV_YMF262 -DEC_YMF262_NUKED -DEC_YMF262_ADLIBEMU -DSNDDEV_YMF278B -DSNDDEV_YMZ280B -DSNDDEV_YMF271)
add_definitions(-DSNDDEV_AY8910 -DEC_AY8910_EMU2149 -DSNDDEV_32X_PWM -DSNDDEV_GAMEBOY)
add_definitions(-DSNDDEV_NES_APU -DEC_NES_NSFPLAY -DEC_NES_NSFP_FDS -DSNDDEV_YMW258 -DSNDDEV_UPD7759 -DSNDDEV_OKIM6258 -DSNDDEV_OKIM6295 -DSNDDEV_K051649 -DSNDDEV_K054539)
add_definitions(-DSNDDEV_C6280 -DEC_C6280_OOTAKE -DSNDDEV_C140 -DSNDDEV_C219 -DSNDDEV_K053260 -DSNDDEV_POKEY)
add_definitions(-DSNDDEV_QSOUND -DEC_QSOUND_CTR -DSNDDEV_SCSP -DSNDDEV_WSWAN -DSNDDEV_VBOY_VSU)
add_definitions(-DSNDDEV_SAA1099 -DEC_SAA1099_VB -DSNDDEV_ES5503 -DSNDDEV_ES5506 -DSNDDEV_X1_010 -DSNDDEV_C352 -DSNDDEV_GA20)

set(SRCS_C
	"emu/SoundEmu.c" "emu/Resampler.c" "emu/panning.c" "emu/dac_control.c"
	"utils/DataLoader.c" "utils/StrUtils-CPConv_Stub.c"
	"player/helper.c" "player/dblk_compr.c"
#mame default
	"emu/cores/sn764intf.c" "emu/cores/sn76496.c" "emu/cores/2413intf.c" "emu/cores/ym2413.c"
#gpgx default
	"emu/cores/2612intf.c" "emu/cores/fmopn2612.c" "emu/cores/ym2151.c" "emu/cores/segapcm.c"
#both cores used
	"emu/cores/rf5cintf.c" "emu/cores/rf5c68.c" "emu/cores/scd_pcm.c" "emu/cores/opnintf.c"
	"emu/cores/fmopn.c" "emu/cores/opnintf.c" "emu/cores/fmopn.c" "emu/cores/ymdeltat.c"
#adlibemu default
	"emu/cores/oplintf.c" "emu/cores/adlibemu_opl2.c" "emu/cores/fmopl.c" "emu/cores/oplintf.c"
	"emu/cores/fmopl.c" "emu/cores/fmopl.c" "emu/cores/ymdeltat.c"
#nuke default, adlib for 278b
	"emu/cores/262intf.c" "emu/cores/nukedopl.c" "emu/cores/adlibemu_opl3.c" "emu/cores/ymf278b.c"
	"emu/cores/ymz280b.c" "emu/cores/ymf271.c"
#fcc_emu default
	"emu/cores/ayintf.c" "emu/cores/emu2149.c" "emu/cores/pwm.c" "emu/cores/gb.c"
#nsfp default with fds
	"emu/cores/nesintf.c" "emu/cores/np_nes_apu.c" "emu/cores/np_nes_dmc.c" "emu/cores/np_nes_fds.c"
	"emu/cores/multipcm.c" "emu/cores/upd7759.c" "emu/cores/okim6258.c" "emu/cores/okim6295.c"
	"emu/cores/okiadpcm.c" "emu/cores/k051649.c" "emu/cores/k054539.c"
#ootk default
	"emu/cores/c6280intf.c" "emu/cores/Ootake_PSG.c" "emu/cores/c140.c" "emu/cores/c219.c"
	"emu/cores/k053260.c" "emu/cores/pokey.c"
#ctr default
	"emu/cores/qsoundintf.c" "emu/cores/qsound_ctr.c" "emu/cores/scsp.c" "emu/cores/scspdsp.c"
	"emu/cores/ws_audio.c" "emu/cores/vsu.c"
#vbel default
	"emu/cores/saaintf.c" "emu/cores/saa1099_vb.c" "emu/cores/es5503.c" "emu/cores/es5506.c"
	"emu/cores/x1_010.c" "emu/cores/c352.c" "emu/cores/iremga20.c"
)

set(SRCS_C_CXX "player/playerbase.cpp" "player/vgmplayer.cpp" "player/vgmplayer_cmdhandler.cpp")

file(GLOB SRCS_H "emu/*.h" "utils/*.h" "player/*.h" "emu/cores/*.h")

add_library(vgm ${SRCS_H} ${SRCS_C} ${SRCS_C_CXX})
set_property(TARGET vgm PROPERTY FOLDER "3rdparty")
