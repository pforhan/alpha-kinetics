CC = gcc
CFLAGS = -Wall -O2 -Isrc

# Jaguar Toolchain (Example)
# JAG_CC = m68k-atari-jaguar-gcc
# JAG_CFLAGS = -O2 -fomit-frame-pointer -m68000 -D JAGUAR -Isrc

SRC = src/demo_main.c src/jag_physics.c src/demo_bitmap.c src/jag_gpu.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean jaguar

all: jag_physics_pc

jag_physics_pc: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Jaguar Build Rule (Example)
# jaguar: $(SRC)
# 	$(JAG_CC) $(JAG_CFLAGS) -o jag_physics.cof $^
# 	# Convert COF to JAG or ROM if needed

clean:
	rm -f jag_physics_pc *.o
