#############################################################################
# Makefile for Alpha Kinetics (Multi-Platform)
#############################################################################

# Core Library
CORE_DIR = src/core
CORE_SRC = $(CORE_DIR)/ak_physics.c $(CORE_DIR)/ak_demo_setup.c
CORE_INC = -I$(CORE_DIR)

# Jaguar Build Configuration
JAG_DIR = src/platforms/jaguar
JAG_PROG = alpha_kinetics_jag.cof
JAG_SRC = $(JAG_DIR)/jaguar_main.c src/demo_bitmap.c src/jag_gpu.c src/jag_stubs.c
JAG_S = src/jag_startup.s

# Jaguar Libraries Location
JAG_LIB_DIR = $(JAG_DIR)/jaguar-sdk

# PC Build Configuration
PC_DIR = src/platforms/pc
PC_PROG = alpha_kinetics_pc
PC_SRC = $(PC_DIR)/pc_main.c
CC_PC = gcc
CFLAGS_PC = -Wall -O2 $(CORE_INC)

# OS Detection for Clean
ifeq ($(OS),Windows_NT)
	RM_CMD = del /Q /F
	EXT = .exe
else
	RM_CMD = rm -f
	EXT =
endif

#############################################################################
# Targets
#############################################################################

.PHONY: all jaguar pc clean docker-jaguar

all: jaguar pc arduboy playdate

# Toolchain for Jaguar
CC_JAG = m68k-atari-mint-gcc
AR_JAG = m68k-atari-mint-ar
RMAC = rmac
RLN = rln

# Dynamically locate libgcc based on the current compiler
LIB_GCC = $(shell command -v $(CC_JAG) >/dev/null 2>&1 && $(CC_JAG) -m68000 -mshort -print-libgcc-file-name)

# Jaguar Compiler Flags
CFLAGS += -std=c99 -mshort -Wall -fno-builtin $(CORE_INC) -Isrc -I$(JAG_LIB_DIR)/jaguar/include -DJAGUAR
MACFLAGS = -fb -v
LINKFLAGS += -v -a 4000 x x

# Jaguar Objects
JAG_OBJS = $(JAG_S:.s=.o) $(JAG_SRC:.c=.o) $(CORE_SRC:.c=.o)

# Export variables
export CC=$(CC_JAG)
export AR=$(AR_JAG)
export RMAC RLN
export JAGPATH = $(CURDIR)/$(JAG_LIB_DIR)

# Common flags for libraries
LIB_CFLAGS_BASE = -m68000 -Wall -fomit-frame-pointer -O2 -msoft-float
PROJ_ROOT = $(CURDIR)

# Jaguar Build Rule
jaguar: $(JAG_PROG)

$(JAG_PROG): $(JAG_OBJS)
	$(RLN) $(LINKFLAGS) -o $@ $(JAG_OBJS) $(LIB_GCC)

# Docker Jaguar Build Rule
docker-jaguar:
	@echo "Building Jaguar ROM via Docker..."
	docker build -t alpha-kinetics-dev -f .devcontainer/Dockerfile .
	docker run --rm -v "$(CURDIR)":/workspaces/jag-kinetics -w /workspaces/jag-kinetics alpha-kinetics-dev make jaguar

# PC Build Rule
pc: $(PC_PROG)$(EXT)

$(PC_PROG)$(EXT): $(PC_SRC) $(CORE_SRC)
	$(CC_PC) $(CFLAGS_PC) -o $@ $(PC_SRC) $(CORE_SRC)

# Arduboy Build Rule
arduboy:
	@echo "Building for Arduboy..."
	@mkdir -p build/arduboy/AlphaKinetics build/arduboy/bin
	@cp src/platforms/arduboy/arduboy_demo.cpp build/arduboy/AlphaKinetics/AlphaKinetics.ino
	@cp src/core/* build/arduboy/AlphaKinetics/
	arduino-cli compile --fqbn "arduboy-homemade:avr:arduboy-fx" --output-dir build/arduboy/bin build/arduboy/AlphaKinetics --build-property "compiler.c.extra_flags=-DAK_MAX_BODIES=16" --build-property "compiler.cpp.extra_flags=-DAK_MAX_BODIES=16"

arduboy_flash: arduboy
	@echo "Flashing to Arduboy..."
	ardugotools sketch write any --infile build/arduboy/bin/AlphaKinetics.ino.hex --runnow

# Playdate Build Rules
playdate: playdate_simulator playdate_device

playdate_simulator:
	@echo "Building for Playdate Simulator..."
	@mkdir -p build/playdate_sim
	unset CC CFLAGS MACFLAGS LINKFLAGS AR; cmake -S src/platforms/playdate -B build/playdate_sim
	unset CC CFLAGS MACFLAGS LINKFLAGS AR; $(MAKE) -C build/playdate_sim

playdate_device:
	@echo "Building for Playdate Device..."
	@mkdir -p build/playdate_device
	unset CC CFLAGS MACFLAGS LINKFLAGS AR; cmake -S src/platforms/playdate -B build/playdate_device -DDEVICE_BUILD=ON
	unset CC CFLAGS MACFLAGS LINKFLAGS AR; $(MAKE) -C build/playdate_device

# Pattern Rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(RMAC) $(MACFLAGS) $< -o $@

clean:
	$(RM_CMD) $(PC_PROG)$(EXT) *.cof *.sym *.map
	find src -name "*.o" -type f -delete
	rm -rf build
	rm -rf src/platforms/playdate/AlphaKinetics.pdx
	find src/platforms/playdate/Source -maxdepth 1 -type f -not -name 'README.md' -delete
