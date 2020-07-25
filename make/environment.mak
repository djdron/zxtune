#apply default values
ifdef arch
ifndef distro
ifndef prebuilt.dir
$(error No prebuilt.dir defined)
endif
endif
endif

#generic defines
defines += HAVE_STDINT_H

#android
android.cxx.flags = -no-canonical-prefixes -funwind-tables -fstack-protector-strong -fomit-frame-pointer -Wa,--noexecstack
android.ld.flags = -no-canonical-prefixes -Wl,-soname,$(notdir $@) -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -static-libstdc++
#assume that all the platforms are little-endian
#this required to use boost which doesn't know anything about __armel__ or __mipsel__
defines.android += ANDROID __ANDROID__ __LITTLE_ENDIAN__ NO_DEBUG_LOGS NO_L10N LITTLE_ENDIAN
# x86
android.x86.toolchain = $(toolchains.root)/i686-linux-android
android.x86.execprefix = $(android.x86.toolchain)/bin/i686-linux-android-
android.x86.cxx.flags = -m32
# x86_64
android.x86_64.toolchain = $(toolchains.root)/x86_64-linux-android
android.x86_64.execprefix = $(android.x86_64.toolchain)/bin/x86_64-linux-android-
android.x86_64.cxx.flags = -m64
# armeabi-v7a
android.armeabi-v7a.toolchain = $(toolchains.root)/arm-linux-androideabi
android.armeabi-v7a.execprefix = $(android.armeabi-v7a.toolchain)/bin/arm-linux-androideabi-
android.armeabi-v7a.cxx.flags = -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb
android.armeabi-v7a.ld.flags = -march=armv7-a -Wl,--fix-cortex-a8
# arm64-v8a
android.arm64-v8a.toolchain = $(toolchains.root)/aarch64-linux-android
android.arm64-v8a.execprefix = $(android.arm64-v8a.toolchain)/bin/aarch64-linux-android-

#linux.i686
linux.i686.cxx.flags = -march=i686 -m32 -mmmx
linux.i686.ld.flags = -m32

#linux.x86_64
linux.x86_64.cxx.flags = -m64 -mmmx
linux.x86_64.ld.flags = -m64

#linux armhf
linux.armhf.toolchain = $(toolchains.root)/armhf-linux
linux.armhf.execprefix = $(linux.armhf.toolchain)/bin/arm-linux-gnueabihf-
linux.armhf.crossroot = $(prebuilt.dir)/root-linux-armhf
linux.armhf.qt.libs = $(linux.armhf.crossroot)/usr/lib/arm-linux-gnueabihf
linux.armhf.cxx.flags = -march=armv6 -mfpu=vfp -mfloat-abi=hard -Wa,--no-warn

#mingw
mingw.toolchain = $(toolchains.root)/MinGW
mingw.execprefix ?= $(mingw.toolchain)/bin/
mingw.cxx.flags = -mthreads -mwin32 -mno-ms-bitfields -mmmx -msse -msse2
mingw.ld.flags = -mthreads -static -Wl,--allow-multiple-definition
# x86
mingw.x86.execprefix = $(mingw.execprefix)
mingw.x86.cxx.flags = -m32
mingw.x86.ld.flags = -m32
# x86_64
mingw.x86_64.execprefix = $(mingw.execprefix)
mingw.x86_64.cxx.flags = -m64
mingw.x86_64.ld.flags = -m64

#windows
# x86
windows.x86.cxx.flags = /arch:IA32
# x86_64

#darwin
# x86_64
#darwin.x86_64.cxx.flags = -m64 -mmmx
#darwin.x86_64.ld.flags = -m64
