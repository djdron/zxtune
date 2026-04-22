ACIDSAP = ../Acid800/out/Release/AcidSAP/standalone

# no user-configurable paths below this line

TESTS = antic_nmires antic_nmist antic_vcount_ntsc antic_vcount_pal antic_wsync cpu_anx cpu_decimal cpu_las cpu_shx gtia_consol pokey_irqst pokey_pot pokey_potst pokey_random pokey_timerirq
TESTS_SAP = $(TESTS:%=test/%.sap)
TESTS_ACIDSAP = $(wildcard $(ACIDSAP)/*.sap)
INC_PASSED = ((passed++))

check test: test/conv test/acid
.PHONY: check test

test/conv: asapconv
	mkdir -p test/actual test/actual/relocated
	./asapconv -o test/actual/.sap $(filter-out %.d15 %.d8, $(wildcard examples/*))
	./asapconv -o test/actual/.%e examples/{Cropky,Near,Pneumatic_Driller,Sill_Of_Destiny}.sap
	./asapconv -o test/actual/.xex $(filter-out %.d15 %.d8 %/Ucieczka_w_Nicosc.sap, $(wildcard examples/*))
	./asapconv -o test/actual/relocated/.%e --address=6789 $(filter-out %.d15 %.d8 %/Sweet_Illusions_Techno.sap %/Ucieczka_w_Nicosc.sap %/X_Ray_2.sap, $(wildcard examples/*))
	md5sum test/actual/* test/actual/relocated/* | diff -u test/expected.txt -
.PHONY: test/conv

test/acid: asapscan $(TESTS_SAP)
	@$(TESTS_SAP:%=./asapscan -a % && $(INC_PASSED);) \
		echo PASSED $$passed of $(words $(TESTS_SAP)) ASAP tests, 9 expected
	@test -d $(ACIDSAP) && $(TESTS_ACIDSAP:%=./asapscan -a % && $(INC_PASSED);) \
		echo PASSED $$passed of $(words $(TESTS_ACIDSAP)) AcidSAP tests, 11 expected || true
.PHONY: test/acid

test/%.sap: $(srcdir)test/%.asx
	$(XASM) -d SAP=1

test/%_ntsc.sap: $(srcdir)test/%.asx
	$(XASM) -d SAP=1 -d NTSC=1

test/%_pal.sap: $(srcdir)test/%.asx
	$(XASM) -d SAP=1 -d NTSC=0

test/%.xex: $(srcdir)test/%.asx
	$(XASM) -d SAP=0

CLEAN += test/*.sap test/*.xex

test/timevsnative: $(call src,test/timevsnative.c asap.[ch])
	$(DO_CC)
CLEAN += test/timevsnative

test/loadsap.exe: $(call src,test/loadsap.cs csharp/asap.cs)
	$(CSC)
CLEAN += test/loadsap.exe

test/ultrasap.exe: $(call src,test/ultrasap.cs csharp/asap.cs)
	$(CSC)
CLEAN += test/ultrasap.exe

test/crashsap.exe: $(call src,test/crashsap.cs csharp/asap.cs)
	$(CSC)
CLEAN += test/crashsap.exe

include $(srcdir)test/benchmark/benchmark.mk
