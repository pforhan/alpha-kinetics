# Jaguar Physics Engine

A simple, lightweight, fixed-point 2D physics engine written in C, designed for the Atari Jaguar.

## Features

- **Fixed-Point Arithmetic**: Uses 16.16 fixed-point math (`fixed_point.h`) to avoid costly floating-point operations on the Jaguar's 68000.
- **Rigid Body Physics**: Supports position, velocity, acceleration, and mass.
- **Collision Detection**:
  - Circle-to-Circle
  - AABB-to-AABB
- **Collision Resolution**: Impulse-based resolution with restitution (bounciness) and positional correction.
- **Portable**: Written in pure C99.
- **GPU-Ready**: Data structures aligned (16-byte) and logic isolated for easy offloading to the Jaguar's RISC processors.
- **Dual Target**:
  - **PC**: Compiles to a terminal-based simulation (ASCII output) for logic verification.
  - **Jaguar**: Compiles to a binary using standard Jaguar toolchains (e.g., `m68k-atari-jaguar-gcc`).

## File Structure

- `src/main.c`: Entry point. Handles setup and main loop.
- `src/physics.c` / `.h`: Core physics engine.
- `src/fixed_point.h`: Math macros and functions.
- `src/jaguar.h`: Hardware register definitions for the Atari Jaguar.
- `src/bitmap.c` / `.h`: Simple graphics routines for the framebuffer.

## Building

### For PC (Simulation)
If you have GCC installed:
```bash
gcc -o jag_physics src/main.c src/physics.c src/bitmap.c -Isrc
./jag_physics
```
This will run the physics simulation and output ASCII frames to the terminal.

### For Atari Jaguar
You need a Jaguar cross-compiler (like `m68k-atari-jaguar-gcc` from the Jaguar SDK).
Uncomment the Jaguar lines in the `Makefile` or run:
```bash
m68k-atari-jaguar-gcc -o jag_physics.cof src/main.c src/physics.c src/bitmap.c -D JAGUAR -Isrc -O2
```
Then convert the `.cof` to a `.jag` or `.rom` file using `rln` or `objcopy` as appropriate for your toolchain.

## Usage

Initialize the world:
```c
World world;
World_Init(&world, (Vec2){0, INT_TO_FP(10)}); // Gravity
```

Add bodies:
```c
// Add a static ground box
World_AddBody(&world, 
    (Shape){SHAPE_AABB, .bounds.aabb = {INT_TO_FP(100), INT_TO_FP(10)}}, 
    INT_TO_FP(160), INT_TO_FP(220), 
    0); // Mass 0 = Static

// Add a dynamic circle
World_AddBody(&world, 
    (Shape){SHAPE_CIRCLE, .bounds.circle = {INT_TO_FP(10)}}, 
    INT_TO_FP(160), INT_TO_FP(50), 
    INT_TO_FP(1)); // Mass 1
```

Step the simulation:
```c
// Step the simulation with event recording
Contact contacts[16];
int contact_count = 0;
World_Step(world.bodies, world.body_count, world.gravity, INT_TO_FP(1)/60, contacts, &contact_count, 16);
```

## Typical Interactions

You can inspect the `Contact` list returned by `World_Step` to react to collisions.

### Handling Collisions (Event System)
```c
for (int i = 0; i < contact_count; i++) {
    Contact* c = &contacts[i];
    // Check if specific bodies collided (by ID)
    if (c->body_a_id == player_id || c->body_b_id == player_id) {
        printf("Player hit something!\n");
        // TODO: Play sound or decrease health
    }
}
```

### Handling Off-Screen Objects
```c
if (obj->position.x > INT_TO_FP(320) || obj->position.x < 0) {
    // TODO: Remove body or reset position
    obj->position.x = INT_TO_FP(160);
    obj->velocity.x = 0;
}
```

## Optimization for Jaguar
- The engine currently runs on the 68000.
- **GPU Offloading**: The `World_Step` function is designed to be moved to the GPU (RISC) processor. It uses isolated data arrays and aligned structs.
- `fixed_point.h` macros are efficient, but inline assembly for `muls` and `divs` could speed up math on the 68k.
