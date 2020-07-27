project(libgme)

add_definitions(-DNSF_EMU_APU_ONLY)

set(SRCS_C_CXX
	"gme/blargg_common.cpp"
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
	"gme/ym2413.cpp"

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

	"gme/Gym_Emu.cpp"
	"gme/Ym2612_Emu.cpp"
	"gme/fm2612.cpp"
	"gme/Sms_Apu.cpp"

	"gme/Kss_Emu.cpp"
	"gme/Kss_Core.cpp"
	"gme/Kss_Cpu.cpp"
	"gme/Kss_Scc_Apu.cpp"
	"gme/Z80_Cpu.cpp"
	"gme/Opl_Apu.cpp"
	"gme/fmopl.cpp"
	"gme/Ay_Apu.cpp"

	"gme/Resampler.cpp"
	"gme/Fir_Resampler.cpp"
	"gme/Dual_Resampler.cpp"
)

file(GLOB SRCS_H "gme/*.h")

add_library(gme ${SRCS_C_CXX} ${SRCS_H})
set_property(TARGET gme PROPERTY FOLDER "3rdparty")
