# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "javascript.mk")
endif

javascript: javascript/asap.js
.PHONY: javascript

javascript/asap.js: $(call src,asap.fu asap6502.fu asapinfo.fu cpu6502.fu pokey.fu) $(ASM6502_PLAYERS_OBX)
	$(FUT)
CLEAN += javascript/asap.js

javascript/asap-with-asapwriter.js: $(call src,asap.fu asap6502.fu asapinfo.fu asapwriter.fu cpu6502.fu flashpack.fu pokey.fu) $(ASM6502_OBX)
	$(FUT)
CLEAN += javascript/asap-with-asapwriter.js
