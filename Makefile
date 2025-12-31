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
JAG_SRC = $(JAG_DIR)/jaguar_main.c src/demo_bitmap.c src/jag_gpu.c src/libgcc.c src/jag_stubs.c
JAG_S = src/jag_startup.s

# Jaguar Libraries Location
JAG_LIB_DIR = $(JAG_DIR)

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

.PHONY: all jaguar pc clean

all: jaguar pc

# Jaguar Toolchain Definitions
CC = m68k-atari-mint-gcc
RMAC = rmac
RLN = rln
AR = m68k-atari-mint-ar

# Jaguar Compiler Flags
CFLAGS += -std=c99 -mshort -Wall -fno-builtin $(CORE_INC) -Isrc -I$(JAG_LIB_DIR)/rmvlib/include -I$(JAG_LIB_DIR)/jlibc/include -DJAGUAR
MACFLAGS = -fb -v
LINKFLAGS += -v -a 4000 x x

# Jaguar Objects
JAG_OBJS = $(JAG_S:.s=.o) $(JAG_SRC:.c=.o) $(CORE_SRC:.c=.o)

# Libraries
LIB_RMV = $(JAG_LIB_DIR)/rmvlib/rmvlib.a
LIB_JLIBC = $(JAG_LIB_DIR)/jlibc/jlibc.a
LIB_GCC = /opt/cross-mint/usr/lib64/gcc/m68k-atari-mint/7/libgcc.a

# Export variables
export CC AR RMAC RLN
export JAGPATH = $(CURDIR)/$(JAG_LIB_DIR)
export JLIBC = $(CURDIR)/$(JAG_LIB_DIR)/jlibc

# Common flags for libraries
LIB_CFLAGS_BASE = -m68000 -Wall -fomit-frame-pointer -O2 -msoft-float
PROJ_ROOT = $(CURDIR)

$(LIB_JLIBC):
	$(MAKE) -e -C $(JAG_LIB_DIR)/jlibc jlibc.a CFLAGS="$(LIB_CFLAGS_BASE) -I$(PROJ_ROOT)/$(JAG_LIB_DIR)/jlibc/include" OSUBDIRS=ctype MAKEFLAGS=--no-print-directory

$(LIB_RMV): $(LIB_JLIBC)
	$(MAKE) -e -C $(JAG_LIB_DIR)/rmvlib rmvlib.a JLIBC=$(PROJ_ROOT)/$(JAG_LIB_DIR)/jlibc CFLAGS="$(LIB_CFLAGS_BASE) -I$(PROJ_ROOT)/$(JAG_LIB_DIR)/rmvlib/include -I$(PROJ_ROOT)/$(JAG_LIB_DIR)/jlibc/include" OSUBDIRS= MAKEFLAGS="--no-print-directory -e"
	@echo "Manually updating rmvlib.a..."
	find $(JAG_LIB_DIR)/rmvlib -name "*.o" | xargs $(AR) rvs $(LIB_RMV)

# Jaguar Build Rule
jaguar: $(JAG_PROG)

$(JAG_PROG): $(JAG_OBJS) $(LIB_RMV)
	$(RLN) $(LINKFLAGS) -o $@ $(JAG_OBJS) $(LIB_RMV) $(LIB_JLIBC) $(LIB_GCC)

# PC Build Rule
pc: $(PC_PROG)$(EXT)

$(PC_PROG)$(EXT): $(PC_SRC) $(CORE_SRC)
	$(CC_PC) $(CFLAGS_PC) -o $@ $(PC_SRC) $(CORE_SRC)

# Pattern Rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(RMAC) $(MACFLAGS) $< -o $@

clean:
	$(RM_CMD) $(PC_PROG)$(EXT) *.cof *.sym *.map
	find src -name "*.o" -type f -delete
	$(MAKE) -C $(JAG_LIB_DIR)/rmvlib clean
	$(MAKE) -C $(JAG_LIB_DIR)/jlibc clean
