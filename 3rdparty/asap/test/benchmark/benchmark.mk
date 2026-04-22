GME_PATH = ../game-music-emu
SAP_PATH = ../sapLib
WIN64_CC = $(DO)x86_64-w64-mingw32-gcc $(WIN_CARGS) $(filter-out %.h,$^)
WIN64_CXX = $(DO)x86_64-w64-mingw32-g++ $(WIN_CARGS) $(filter-out %.h,$^)

benchmark: test/benchmark/BENCHMARK.csv
.PHONY: benchmark

test/benchmark/BENCHMARK.csv: $(srcdir)test/benchmark/benchmark.pl win32/asapconv.exe win32/x64/asapconv.exe win32/msvc/asapconv.exe win32/msvc/x64/asapconv.exe \
	d/asap2wav.exe csharp/asap2wav.exe java/asap2wav.jar swift/asap2wav.exe javascript/asap.js python/asap.py test/benchmark/gme_benchmark.exe test/benchmark/sap_benchmark.exe
	perl $< > $@

test/benchmark/gme_benchmark.exe: $(call src,test/benchmark/gme_benchmark.c asap.h) win32/x64/asap.o $(GME_PATH)/gme/gme.h $(GME_PATH)/build/gme/libgme.a
	$(WIN64_CXX) -lz
CLEAN += test/benchmark/gme_benchmark.exe

test/benchmark/sap_benchmark.exe: $(call src,test/benchmark/sap_benchmark.cpp asap.h) win32/x64/asap.o $(SAP_PATH)/pokey0.cpp $(SAP_PATH)/pokey1.cpp $(SAP_PATH)/sapCpu.cpp $(SAP_PATH)/sapEngine.cpp $(SAP_PATH)/sapLib.h $(SAP_PATH)/sapPokey.cpp
	$(WIN64_CXX)
CLEAN += test/benchmark/sap_benchmark.exe

profile: gmon.out
	gprof -bpQ test/benchmark/asapconv-profile.exe

gmon.out: test/benchmark/asapconv-profile.exe
	$(DO)./test/benchmark/asapconv-profile.exe -b -o .wav test/benchmark/Drunk_Chessboard.sap
CLEAN += gmon.out

test/benchmark/asapconv-profile.exe: $(call src,asapconv.c asap-stdio.[ch] asap.[ch])
	$(WIN64_CC:-s=) -no-pie -pg
CLEAN += test/benchmark/asapconv-profile.exe
