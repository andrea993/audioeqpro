MOD_OBJ_OUT=m-test.o
MOD_OBJ_OUT_DBG=m-test-dbg.o
MOD_SO_OUT=m-test.so
MOD_SO_OUT_DBG=m-test-dbg.so
CC=gcc
DEF=-DHAVE_CONFIG_H -DPA_SINK_SHARE_VOLUME_WITH_MASTER=0
CFLAGS_LINKSTEP=-shared -lm -lpulse -o $(MOD_SO_OUT)
CFLAGS=-Wall -Wextra -fPIC -O2 -g -c -o $(DEF) $(MOD_OBJ_OUT) 
SRC=m-test.c

dbg:CFLAGS_LINKSTEP=-shared -lm -lpulse -o $(MOD_SO_OUT_DBG)
dbg:CFLAGS=-Wall -Wextra -fPIC -O0 -g -c -DEQPRO_DEBUG $(DEF) -o $(MOD_OBJ_OUT_DBG) 
all:build

build:
	@echo " >>> (build) Building object from: $(SRC)..."
	$(CC) $(CFLAGS) $(SRC)  || exit 1
	@echo " >>> (build) Linking..."
	$(CC) $(CFLAGS_LINKSTEP) $(MOD_OBJ_OUT) || exit 2
	@echo " >>> (build) Output: $(MOD_SO_OUT)"
	@echo " >>> [DONE] Finished"
dbg:
	@echo " >>> (build)(debug) Building object from: $(SRC)..."
	$(CC) $(CFLAGS) $(SRC) || exit 1
	@echo " >>> (build)(debug) Linking..."
	$(CC) $(CFLAGS_LINKSTEP) $(MOD_OBJ_OUT_DBG) || exit 2
	@echo " >>> (build)(debug) Output: $(MOD_SO_OUT_DBG)"
	@echo " >>> [DONE] Finished"
install:
ifeq ($(INSTALL_DBG),1)
MOD_SO_OUT=$(MOD_SO_OUT_DBG)
endif

install: PULSEVER:=$(shell sh pa_ver.sh )
install: MODULES_DIRECTORY=/usr/lib/pulse-$(PULSEVER)/modules/
install:
	@echo " >>> (install) Checking requirements..."
	if [[ $(PULSEVER) == "null-pulseaudio-version" ]]; then \
		exit 2; \
	fi
	@echo " >>> (install) Pulseaudio version ... passed"
	if [ ! -f $(MOD_SO_OUT) ]; then \
		exit 1; \
	fi
	
	@echo " >>> (install) Module exists and found"
	if [ ! -d $(MODULES_DIRECTORY) ]; then \
		exit 3; \
	fi
	@echo " >>> (install) Modules directory exists and found"
	@echo " --- "
	@echo " >>> (install) Installing $(MOD_SO_OUT)"
	@echo " >>> (install) PulseAudio version detected: $(PULSEVER)"
	@echo " >>> (install) Modules directory found: $(MODULES_DIRECTORY)"
	cp $(MOD_SO_OUT) $(MODULES_DIRECTORY) && echo " >>> Done installing module, now use pactl to load this module" || echo "Mmh... check if you are root :P" 

clean:
	@echo " >>> (clean) Removing $(MOD_SO_OUT) and $(MOD_SO_OUT_DBG)"
	rm $(MOD_SO_OUT) $(MOD_SO_OUT_DBG) || echo "One or both files don't exist"
	@echo " >>> (clean) Removing object files"
	rm *.o 

.PHONY:
	build, install, all, clean

ifndef .VERBOSE
.SILENT:
endif
