library_name := openmpt
dirs.root := ../..
include $(dirs.root)/make/default.mak

includes.dirs += common . build/svn_version src

source_files += $(addprefix libopenmpt/,\
                  libopenmpt_cxx libopenmpt_impl libopenmpt_ext_impl)

source_files += $(addprefix soundlib/,\
                  modcommand mod_specifications modsmp_ctrl patternContainer pattern tuning tuningCollection \
                  ModChannel ModInstrument ModSample ModSequence PlayState \
                  Fastmix UpgradeModule \
                  ITCompression ITTools S3MTools XMTools InstrumentExtensions InstrumentSynth \
                  Message MIDIMacros MIDIMacroParser MIDIEvents MPEGFrame MixerLoops MixerSettings MixFuncTable \
                  OggStream OPL Paula RowVisitor \
                  SampleIO SampleFormats $(addprefix SampleFormat,BRR FLAC MediaFoundation MP3 Opus SFZ Vorbis) \
                  SoundFilePlayConfig Sndfile Sndmix Snd_flt Snd_fx Tables \
                  $(addprefix Load_,\
                    667 669 amf ams c67 cba dbm digi dmf dsm dsym dtm etx far fc fmt ftm gdm gmc gt2 ice imf ims it itp kris \
                    mdl med mo3 mod mt2 mtm mus_km nru okt plm psm pt36 ptm puma rtm s3m sfx ss stm stk stp symmod tcb ult unic xm xmf \
                  ) load_j2b \
                  MODTools TinyFFT WindowedFIR WAVTools Tagging \
                  )

source_files += $(addprefix soundlib/plugins/,\
                  PlugInterface PluginManager \
                  DigiBoosterEcho LFOPlugin SymMODEcho \
                  )
source_files += $(addprefix soundlib/plugins/dmo/,\
                  Chorus Compressor Distortion Echo Flanger Gargle I3DL2Reverb ParamEq WavesReverb \
                  DMOUtils \
                  )

source_files += $(addprefix sounddsp/,Reverb)

source_files += $(addprefix common/,\
                  Logging mptPathString mptRandom mptStringBuffer mptTime \
                  serialization_utils version)

source_files := $(addsuffix .cpp,$(source_files))

opusfile_includes = $(dirs.root)/3rdparty/openmpt/include/opusfile/include $(dirs.root)/3rdparty/opus/include $(dirs.root)/3rdparty

# force using 3rdparty externals
includes.dirs.SampleFormatMP3.cpp = $(dirs.root)/3rdparty
includes.dirs.SampleFormatVorbis.cpp = $(dirs.root)/3rdparty
includes.dirs.SampleFormatOpus.cpp = $(opusfile_includes)
includes.dirs.Load_mo3.cpp = $(dirs.root)/3rdparty
includes.dirs.Load_xm.cpp = $(dirs.root)/3rdparty

# TODO: remove if using external opusfilelibrary
includes.dirs.opusfile.c = $(opusfile_includes)
includes.dirs.stream.c = $(opusfile_includes)
includes.dirs.info.c = $(opusfile_includes)
includes.dirs.internal.c = $(opusfile_includes)
source_files += $(addprefix include/opusfile/src/,opusfile.c stream.c info.c internal.c)

includes.files = defines.h

include $(dirs.root)/makefile.mak
