makebin_name = $(1)
makelib_name = lib$(1).a
makedyn_name = lib$(1).dylib
makeobj_name = $(1).o
makedir_cmd = mkdir -p $(1)
rmdir_cmd = rm -Rf $(1)
rmfiles_cmd = rm -f $(1)
showtime_cmd = date +"%x %X"

compiler=clang
host=macos
LINKER_BEGIN_GROUP=
LINKER_END_GROUP=

$(platform)_include_dirs += /opt/local/include
$(platform)_libraries_dirs += /opt/local/lib

$(platform)_libraries += $(foreach lib,$(libraries.boost),boost_$(lib)-mt)

$(platform).cxx.flags += $(addprefix -FQt,$(libraries.qt))
$(platform).ld.flags += $(addprefix -framework Qt,$(libraries.qt))
