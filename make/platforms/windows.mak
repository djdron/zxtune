makebin_name = $(1).exe
makelib_name = $(1).lib
makedyn_name = $(1).dll
makeobj_name = $(1).obj
makeres_name = $(1).res
makeres_cmd = rc $(addprefix /d, $(DEFINITIONS)) /r /fo$(2) $(1)
makedir_cmd = if NOT EXIST $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
rmdir_cmd = rmdir /Q /S $(subst /,\,$(1))
rmfiles_cmd = del /Q $(subst /,\,$(1))
showtime_cmd = echo %TIME%

compiler := msvs

# built-in features
support_waveout = 1
support_aylpt_dlportio = 1
#support_sdl = 1

ifdef boost_libraries
# installable boost names convention used
# [prefix]boost_[lib]-[msvs]-mt[-gd]-[version].lib
# prefix - 'lib' for static libraries
# lib - library name
# msvs - version of MSVS used to compile and link
# -gd - used for debug libraries
# version - boost version major_minor
windows_libraries += $(foreach lib,$(boost_libraries),$(if $(boost_dynamic),,lib)boost_$(lib)-$(MSVS_VERSION)-mt$(if $(release),,-gd)-$(BOOST_VERSION))
endif

ifdef qt_libraries
# buildable qt names convention used
# Qt[lib][d][4].lib
# lib - library name
# d - used for debug libraries
# 4 - used for dynamic linkage
windows_libraries += $(foreach lib,$(if $(qt_libraries),$(qt_libraries) main,),Qt$(lib)$(if $(release),,d)$(if $(qt_dynamic),4,))
endif
