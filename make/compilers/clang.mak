#basic definitions for tools
tools.cxx ?= $($(platform).$(arch).execprefix)clang++
tools.cc ?= $($(platform).$(arch).execprefix)clang
tools.ld ?= $($(platform).$(arch).execprefix)clang++
tools.ar ?= $($(platform).$(arch).execprefix)ar
tools.objcopy ?= $($(platform).$(arch).execprefix)echo # STUB!
tools.strip ?= $($(platform).$(arch).execprefix)strip

LINKER_BEGIN_GROUP ?= -Wl,'-('
LINKER_END_GROUP ?= -Wl,'-)'

#set options according to mode
ifdef release
CXX_MODE_FLAGS = -O2 -DNDEBUG -funroll-loops
else
CXX_MODE_FLAGS = -O0
endif

#setup profiling
ifdef profile
CXX_MODE_FLAGS += -pg
LD_MODE_FLAGS += -pg
else
CXX_MODE_FLAGS += -fdata-sections -ffunction-sections
endif

#setup PIC code
ifdef pic
CXX_MODE_FLAGS += -fPIC
LD_MODE_FLAGS += -shared
endif

#setup code coverage
ifdef coverage
ifdef release
$(error Code coverage is not supported in release mode)
endif
CXX_MODE_FLAGS += --coverage
LD_MODE_FLAGS += --coverage
endif

DEFINES = $(defines) $(defines.$(platform)) $(defines.$(platform).$(arch))
INCLUDES_DIRS = $(sort $(includes.dirs) $(includes.dirs.$(platform)) $(includes.dirs.$(notdir $1)))
INCLUDES_FILES = $(includes.files) $(includes.files.$(platform))

linux.cxx.flags += -stdlib=libstdc++
linux.ld.flags += -stdlib=libstdc++

darwin.cxx.flags += -stdlib=libc++
darwin.ld.flags += -stdlib=libc++

#setup flags
CCFLAGS = -g $(CXX_MODE_FLAGS) $(cxx_flags) $($(platform).cxx.flags) $($(platform).$(arch).cxx.flags) \
	$(addprefix -D,$(DEFINES)) \
	-funsigned-char -fno-strict-aliasing -fvisibility=hidden \
	-W -Wall -Wextra -pipe \
	$(addprefix -I,$(INCLUDES_DIRS)) $(addprefix -include ,$(INCLUDES_FILES))

CXXFLAGS = $(CCFLAGS) -std=c++11 -fvisibility-inlines-hidden

ARFLAGS := crus
LDFLAGS = $(LD_MODE_FLAGS) $($(platform).ld.flags) $($(platform).$(arch).ld.flags) $(ld_flags)

#specify endpoint commands
build_obj_cmd_nodeps = $(tools.cxx) $(CXXFLAGS) -c $1 -o $2
build_obj_cmd = $(build_obj_cmd_nodeps) -MMD
build_obj_cmd_cc = $(tools.cc) $(CCFLAGS) -c $1 -o $2
build_lib_cmd = $(tools.ar) $(ARFLAGS) $2 $1
link_cmd = $(tools.ld) $(LDFLAGS) -o $@ $(OBJECTS) $(RESOURCES) \
        -L$(libraries.dir) $(LINKER_BEGIN_GROUP) $(addprefix -l,$(libraries)) $(LINKER_END_GROUP) \
        $(addprefix -L,$(libraries.dirs.$(platform)))\
        $(LINKER_BEGIN_GROUP) $(addprefix -l,$(sort $(libraries.$(platform)))) $(LINKER_END_GROUP)\
	$(if $(libraries.dynamic),-L$(output_dir) $(addprefix -l,$(libraries.dynamic)),)

postlink_cmd = $(tools.strip) $@ && touch $@.pdb

#include generated dependensies
include $(wildcard $(objects_dir)/*.d)
