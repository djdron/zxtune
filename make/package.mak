#package generating
pkg_revision := $(subst :,_,$(shell svnversion $(path_step)))
pkg_subversion := $(if $(release),,_dbg)

pkg_name ?= $(binary_name)
pkg_desc ?= The $(binary_name) application is used to play chiptunes from ZX Spectrum
pkg_tag := $(platform)$(if $(arch),_$(arch),)$(if $(distro),_$(distro),)
pkg_dir := $(path_step)/Builds/Revision$(pkg_revision)_$(pkg_tag)
pkg_log := $(pkg_dir)/packaging_$(pkg_name).log
pkg_build_log := $(pkg_dir)/$(pkg_name).log
pkg_debug := $(pkg_dir)/$(pkg_name)_debug.zip

pkg_root = $(pkg_dir)/root

package: $(pkg_root)
	@$(MAKE) $(if $(distro),package_$(distro),package_any)
	@$(call rmdir_cmd,$(pkg_root))

$(pkg_debug): $(pkg_build_log)
	@$(call showtime_cmd)
	$(info Packaging debug information and build log)
	@zip -9Dj $@ $(target).pdb
	@zip -9Djm $@ $(pkg_build_log)

$(pkg_build_log):
	@$(call showtime_cmd)
	$(info Building $(pkg_name))
	$(MAKE) defines="ZXTUNE_VERSION=rev$(pkg_revision)" > $(pkg_build_log) 2>&1

pkg_dependency: $(if $(no_debuginfo),$(pkg_build_log),$(pkg_debug))

$(pkg_root):
	@$(call makedir_cmd,$@)

ifdef distro
include $(path_step)/make/package_$(distro).mak
else
include $(path_step)/make/package_any.mak
endif
