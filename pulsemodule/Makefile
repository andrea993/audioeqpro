MOD_SO_OUT=module-test.so
MOD_SO_OUT_DBG=module-test-dbg.so
CC=gcc
CFLAGS=-fPIC -shared -lm -O2 -o $(MOD_SO_OUT)
SRC=module-test.c

dbg:CFLAGS=-lm -fPIC -shared -g -DEQPRO_DEBUG -o $(MOD_SO_OUT_DBG)
all:build

build:
	@echo " >>> (build) Compiling $(SRC)..."
	$(CC) $(CFLAGS) $(SRC) && echo " >>> Done compiling $(SRC)" || echo " >>> Something went wrong while compiling $(SRC)"

dbg:
	@echo " >>> (build)(debug) Compiling $(SRC)..."
	$(CC) $(CFLAGS) $(SRC) && echo " >>> Done compiling $(SRC)" || echo " >>> Something went wrong while compiling $(SRC)"

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

.PHONY:
	build, install, all, clean

ifndef .VERBOSE
.SILENT:
endif
