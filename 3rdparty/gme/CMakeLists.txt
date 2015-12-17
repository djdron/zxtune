project(libgme)

add_definitions(-DNSF_EMU_APU_ONLY)

set(SRCS_C_CXX "gme/blargg_common.cpp"
	"gme/blargg_errors.cpp"
	"gme/Data_Reader.cpp"
	"gme/Gme_File.cpp"
	"gme/Gme_Loader.cpp"
	"gme/Music_Emu.cpp"
	"gme/Classic_Emu.cpp"
	"gme/Blip_Buffer.cpp"
	"gme/Multi_Buffer.cpp"
	"gme/Track_Filter.cpp"
	"gme/Rom_Data.cpp"

	"gme/Nsf_Emu.cpp"
	"gme/Nsf_Core.cpp"
	"gme/Nsf_Cpu.cpp"
	"gme/Nsf_Impl.cpp"
	"gme/Nsfe_Emu.cpp"

	"gme/Nes_Apu.cpp"
	"gme/Nes_Cpu.cpp"
	"gme/Nes_Oscs.cpp"

	"gme/Nes_Fds_Apu.cpp"
	"gme/Nes_Fme7_Apu.cpp"
	"gme/Nes_Namco_Apu.cpp"
	"gme/Nes_Vrc6_Apu.cpp"
	"gme/Nes_Vrc7_Apu.cpp"
	"gme/ym2413.c"

	"gme/Gbs_Emu.cpp"
	"gme/Gbs_Core.cpp"
	"gme/Gbs_Cpu.cpp"
	"gme/Gb_Apu.cpp"
	"gme/Gb_Cpu.cpp"
	"gme/Gb_Oscs.cpp"

	"gme/Sap_Emu.cpp"
	"gme/Sap_Core.cpp"
	"gme/Sap_Apu.cpp"
	"gme/Sap_Cpu.cpp"

	"gme/Hes_Emu.cpp"
	"gme/Hes_Core.cpp"
	"gme/Hes_Apu.cpp"
	"gme/Hes_Apu_Adpcm.cpp"
	"gme/Hes_Cpu.cpp"

	"gme/Vgm_Emu.cpp"
	"gme/Vgm_Core.cpp"
	"gme/Fir_Resampler.cpp"
	"gme/Dual_Resampler.cpp"
	"gme/Resampler.cpp"

	"gme/Ay_Apu.cpp"
	"gme/C140_Emu.cpp" "gme/c140.c"
	"gme/dac_control.c"
	"gme/K051649_Emu.cpp" "gme/k051649.c"
	"gme/K053260_Emu.cpp" "gme/k053260.c"
	"gme/K054539_Emu.cpp" "gme/k054539.c"
	"gme/Okim6258_Emu.cpp" "gme/okim6258.c"
	"gme/Okim6295_Emu.cpp" "gme/okim6295.c"
	"gme/Pwm_Emu.cpp" "gme/pwm.c"
	"gme/Qsound_Apu.cpp" "gme/qmix.c"
	"gme/Rf5C164_Emu.cpp" "gme/scd_pcm.c"
	"gme/Rf5C68_Emu.cpp" "gme/rf5c68.c"
	"gme/SegaPcm_Emu.cpp" "gme/segapcm.c"
	"gme/Sms_Apu.cpp"
	"gme/Ym2151_Emu.cpp" "gme/ym2151.c"
	"gme/Ym2203_Emu.cpp" "gme/fm.c" "gme/ymdeltat.cpp"
	"gme/Ym2413_Emu.cpp"
	"gme/Ym2608_Emu.cpp"
	"gme/Ym2610b_Emu.cpp"
	"gme/Ym2612_Emu.cpp" "gme/fm2612.c"
	"gme/Ym3812_Emu.cpp"
	"gme/Ymf262_Emu.cpp" "gme/dbopl.cpp"
	"gme/Ymz280b_Emu.cpp" "gme/ymz280b.c"

	"gme/Gym_Emu.cpp"
	"gme/Ym2612_Emu.cpp" "gme/fm2612.c"
	"gme/Sms_Apu.cpp")

file(GLOB SRCS_H "gme/*.h")

add_library(gme ${SRCS_C_CXX} ${SRCS_H})
set_property(TARGET gme PROPERTY FOLDER "3rdparty")
