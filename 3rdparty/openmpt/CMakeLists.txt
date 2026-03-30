project(openmpt)

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DMPT_FRAMEWORK_ASSERT_IS_DEFINED -DMPT_ASSERT_USE_FRAMEWORK=1 -DMPT_FRAMEWORK_ASSERT_IS_ACTIVE=1 -D "ASSERT(p)=(void)(p)" -DMPT_LOG_IS_DISABLED -DMPT_DETERMINISTIC_RANDOM)
add_definitions(-DLIBOPENMPT_BUILD -DNO_LOGGING -DNO_MPG123 -DNO_CONTAINERS_SUPPORT -DOPENMPT_VERSION_REVISION=0)
add_definitions(-DMPT_WITH_MINIMP3 -DMINIMP3_IMPLEMENTATION -DMINIMP3_NONSTANDARD_BUT_LOGICAL -DMINIMP3_NO_SIMD)
add_definitions(-DMPT_WITH_VORBISFILE -DMPT_WITH_VORBIS)
add_definitions(-DMPT_WITH_OPUSFILE)

include_directories("common" "." "build/svn_version" "src")
include_directories("../../3rdparty/openmpt/include/opusfile/include" "../../3rdparty/opus/include" "../../3rdparty")

set(SRCS_C_CXX "libopenmpt/libopenmpt_cxx.cpp" "libopenmpt/libopenmpt_impl.cpp" "libopenmpt/libopenmpt_ext_impl.cpp"
"soundlib/modcommand.cpp" "soundlib/mod_specifications.cpp" "soundlib/modsmp_ctrl.cpp" "soundlib/patternContainer.cpp" "soundlib/pattern.cpp" "soundlib/tuning.cpp" "soundlib/tuningCollection.cpp"
"soundlib/ModChannel.cpp" "soundlib/ModInstrument.cpp" "soundlib/ModSample.cpp" "soundlib/ModSequence.cpp" "soundlib/PlayState.cpp"
"soundlib/Fastmix.cpp" "soundlib/UpgradeModule.cpp"
"soundlib/ITCompression.cpp" "soundlib/ITTools.cpp" "soundlib/S3MTools.cpp" "soundlib/XMTools.cpp" "soundlib/InstrumentExtensions.cpp" "soundlib/InstrumentSynth.cpp"
"soundlib/Message.cpp" "soundlib/MIDIMacros.cpp" "soundlib/MIDIMacroParser.cpp" "soundlib/MIDIEvents.cpp" "soundlib/MPEGFrame.cpp" "soundlib/MixerLoops.cpp" "soundlib/MixerSettings.cpp" "soundlib/MixFuncTable.cpp"
"soundlib/OggStream.cpp" "soundlib/OPL.cpp" "soundlib/Paula.cpp" "soundlib/RowVisitor.cpp"
"soundlib/SampleIO.cpp" "soundlib/SampleFormats.cpp"
"soundlib/SampleFormatBRR.cpp" "soundlib/SampleFormatFLAC.cpp" "soundlib/SampleFormatMediaFoundation.cpp" "soundlib/SampleFormatMP3.cpp" "soundlib/SampleFormatOpus.cpp" "soundlib/SampleFormatSFZ.cpp" "soundlib/SampleFormatVorbis.cpp"
"soundlib/SoundFilePlayConfig.cpp" "soundlib/Sndfile.cpp" "soundlib/Sndmix.cpp" "soundlib/Snd_flt.cpp" "soundlib/Snd_fx.cpp" "soundlib/Tables.cpp"
"soundlib/Load_667.cpp" "soundlib/Load_669.cpp" "soundlib/Load_amf.cpp" "soundlib/Load_ams.cpp" "soundlib/Load_c67.cpp" "soundlib/Load_cba.cpp" "soundlib/Load_dbm.cpp" "soundlib/Load_digi.cpp" "soundlib/Load_dmf.cpp" "soundlib/Load_dsm.cpp" "soundlib/Load_dsym.cpp" "soundlib/Load_dtm.cpp"
"soundlib/Load_etx.cpp" "soundlib/Load_far.cpp" "soundlib/Load_fc.cpp" "soundlib/Load_fmt.cpp" "soundlib/Load_ftm.cpp" "soundlib/Load_gdm.cpp" "soundlib/Load_gmc.cpp" "soundlib/Load_gt2.cpp" "soundlib/Load_ice.cpp" "soundlib/Load_imf.cpp" "soundlib/Load_ims.cpp" "soundlib/Load_it.cpp" "soundlib/Load_itp.cpp" "soundlib/Load_kris.cpp" "soundlib/Load_mdl.cpp" "soundlib/Load_med.cpp" "soundlib/Load_mo3.cpp" "soundlib/Load_mod.cpp"
"soundlib/Load_mt2.cpp" "soundlib/Load_mtm.cpp" "soundlib/Load_mus_km.cpp" "soundlib/Load_nru.cpp" "soundlib/Load_okt.cpp" "soundlib/Load_plm.cpp" "soundlib/Load_psm.cpp" "soundlib/Load_pt36.cpp" "soundlib/Load_ptm.cpp" "soundlib/Load_puma.cpp" "soundlib/Load_rtm.cpp" "soundlib/Load_s3m.cpp" "soundlib/Load_sfx.cpp" "soundlib/Load_ss.cpp" "soundlib/Load_stm.cpp"
"soundlib/Load_stk.cpp" "soundlib/Load_stp.cpp" "soundlib/Load_symmod.cpp" "soundlib/Load_tcb.cpp" "soundlib/Load_ult.cpp" "soundlib/Load_unic.cpp" "soundlib/Load_xm.cpp" "soundlib/Load_xmf.cpp"
"soundlib/load_j2b.cpp"
"soundlib/MODTools.cpp" "soundlib/TinyFFT.cpp" "soundlib/WindowedFIR.cpp" "soundlib/WAVTools.cpp" "soundlib/Tagging.cpp"

"soundlib/plugins/PlugInterface.cpp" "soundlib/plugins/PluginManager.cpp" "soundlib/plugins/DigiBoosterEcho.cpp" "soundlib/plugins/LFOPlugin.cpp" "soundlib/plugins/SymMODEcho.cpp"

"soundlib/plugins/dmo/Chorus.cpp" "soundlib/plugins/dmo/Compressor.cpp" "soundlib/plugins/dmo/Distortion.cpp" "soundlib/plugins/dmo/Echo.cpp" "soundlib/plugins/dmo/Flanger.cpp"
"soundlib/plugins/dmo/Gargle.cpp" "soundlib/plugins/dmo/I3DL2Reverb.cpp" "soundlib/plugins/dmo/ParamEq.cpp" "soundlib/plugins/dmo/WavesReverb.cpp" "soundlib/plugins/dmo/DMOUtils.cpp"

"sounddsp/Reverb.cpp"

"common/Logging.cpp" "common/mptPathString.cpp" "common/mptRandom.cpp" "common/mptStringBuffer.cpp" "common/mptTime.cpp"
"common/serialization_utils.cpp" "common/version.cpp"

"include/opusfile/src/opusfile.c" "include/opusfile/src/stream.c" "include/opusfile/src/info.c" "include/opusfile/src/internal.c"
)

file(GLOB SRCS_H "libopenmpt/*.h" "soundlib/*.h" "sounddsp/Reverb.h" "common/*.h" "include/opusfile/src/*.h" "include/opusfile/include/opusfile.h")

add_library(openmpt ${SRCS_C_CXX} ${SRCS_H})
set_property(TARGET openmpt PROPERTY FOLDER "3rdparty")
