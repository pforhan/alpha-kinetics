CC = gcc
CFLAGS = -Wall -O2 -Isrc

# Jaguar Toolchain
JAG_CC = m68k-atari-jaguar-gcc
JAG_CFLAGS = -O2 -fomit-frame-pointer -m68000 -D JAGUAR -Isrc

SRC = src/demo_main.c src/jag_physics.c src/demo_bitmap.c src/jag_gpu.c
OBJ = $(SRC:.c=.o)

# Detect OS for clean command
ifeq ($(OS),Windows_NT)
	RM = del /Q /F
	EXT = .exe
else
	RM = rm -f
	EXT =
endif

.PHONY: all clean jaguar check_tool

all: jag_physics_pc$(EXT)

jag_physics_pc$(EXT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Jaguar Build Rule
jaguar: check_tool $(SRC)
	$(JAG_CC) $(JAG_CFLAGS) -o jag_physics.cof $(SRC)
	@echo "Jaguar build successful: jag_physics.cof"

check_tool:
	@which $(JAG_CC) > /dev/null 2>&1 || (echo "Error: $(JAG_CC) not found in PATH. Please install the Jaguar SDK." && exit 1)

clean:
	$(RM) jag_physics_pc$(EXT) *.o jag_physics.cof
