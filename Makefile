#############################################################################
# Makefile for Jaguar Physics Demo
#############################################################################

# Standalone Build Configuration

#############################################################################
# Build Configuration
#############################################################################

PROG = jag_physics.cof

# Source Files
SRC_C_COMMON = src/demo_main.c src/jag_physics.c src/demo_bitmap.c src/jag_gpu.c src/libgcc.c
SRC_C_JAGUAR = src/jag_stubs.c src/jag_main.c
SRC_C_PC = src/string.c src/stdlib.c src/stdio.c
SRC_C = $(SRC_C_COMMON) $(SRC_C_JAGUAR)
SRC_S = src/jag_startup.s

# PC Build Configuration
CC_PC = gcc
CFLAGS_PC = -Wall -O2 -Isrc

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

.PHONY: all jaguar ascii clean

# Default Target (Jaguar)
# Default Target (Jaguar)
all: jaguar

# Jaguar Toolchain Definitions
CC = m68k-atari-mint-gcc
RMAC = rmac
RLN = rln
AR = m68k-atari-mint-ar

# Jaguar Compiler Flags
# Include rmvlib, jlibc, and GCC standard headers
CFLAGS += -std=c99 -mshort -Wall -fno-builtin -Isrc -Irmvlib/include -Ijlibc/include -DJAGUAR
MACFLAGS = -fb -v
LINKFLAGS += -v -a 4000 x x

# Objects
OBJS = $(SRC_S:.s=.o) $(SRC_C:.c=.o)

# Libraries
LIB_RMV = rmvlib/rmvlib.a
LIB_JLIBC = jlibc/jlibc.a
LIB_GCC = /opt/cross-mint/usr/lib64/gcc/m68k-atari-mint/7/libgcc.a

# Export variables to sub-makes to ensure consistency and propagation
export CC AR RMAC RLN
export JAGPATH = $(CURDIR)
export JLIBC = $(CURDIR)/jlibc

# Common flags for libraries
LIB_CFLAGS_BASE = -m68000 -Wall -fomit-frame-pointer -O2 -msoft-float
PROJ_ROOT = $(CURDIR)

# Build jlibc
# We target jlibc.a directly. We use -e to force environment overrides of CC/AR/CFLAGS.
$(LIB_JLIBC):
	$(MAKE) -e -C jlibc jlibc.a CFLAGS="$(LIB_CFLAGS_BASE) -I$(PROJ_ROOT)/jlibc/include" OSUBDIRS=ctype MAKEFLAGS=--no-print-directory

# Build rmvlib
# We target rmvlib.a directly.
# After building, we manually update the archive to include all subdirectories,
# as the default rmvlib.a rule misses many (like 'display' and 'interrupt').
$(LIB_RMV): $(LIB_JLIBC)
	$(MAKE) -e -C rmvlib rmvlib.a JLIBC=$(PROJ_ROOT)/jlibc CFLAGS="$(LIB_CFLAGS_BASE) -I$(PROJ_ROOT)/rmvlib/include -I$(PROJ_ROOT)/jlibc/include" OSUBDIRS= MAKEFLAGS="--no-print-directory -e"
	@echo "Manually updating rmvlib.a with all object files..."
	find rmvlib -name "*.o" | xargs $(AR) rvs $(LIB_RMV)

# Linker Rule
# usage of rmvlib requires linking against it.
# Include jlibc and libgcc.a for __divsi3 etc.
OBJS_LINK = $(OBJS) $(LIB_RMV) $(LIB_JLIBC) $(LIB_GCC)

# Jaguar Build Rule
jaguar: $(PROG)

$(PROG): $(OBJS) $(LIB_RMV)
	$(RLN) $(LINKFLAGS) -o $@ $(OBJS_LINK)

# Pattern Rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(RMAC) $(MACFLAGS) $< -o $@

# PC/ASCII Build Rule
ascii: jag_physics_pc$(EXT)

jag_physics_pc$(EXT): $(SRC_C_COMMON) $(SRC_C_PC)
	$(CC_PC) $(CFLAGS_PC) -o $@ $(SRC_C_COMMON) $(SRC_C_PC)

clean:
	$(RM_CMD) jag_physics_pc$(EXT) src/*.o *.o *.cof *.sym *.map
	$(MAKE) -C rmvlib clean
	$(MAKE) -C jlibc clean
