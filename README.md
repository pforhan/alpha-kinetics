# Jaguar Physics Engine

A simple, lightweight, fixed-point 2D physics engine written in C, designed for the Atari Jaguar.

## Features

- **Fixed-Point Arithmetic**: Uses 16.16 fixed-point math (`jag_fixed.h`) to avoid costly floating-point operations on the Jaguar's 68000.
- **Rigid Body Physics**: Supports position, velocity, acceleration, and mass.
- **Collision Detection**:
  - Circle-to-Circle
  - AABB-to-AABB
  - Circle-to-AABB
  - Circle-to-AABB
- **Collision Resolution**: Impulse-based resolution with restitution (bounciness) and positional correction.
- **Distance Constraints (Tethers)**: Supports massless, soft-constraint tethers between bodies (e.g., pendulums, bridges).
- **Portable**: Written in pure C99.
- **GPU-Ready**: Data structures aligned (16-byte) and logic isolated for easy offloading to the Jaguar's RISC processors.
- **Dual Target**:
  - **PC**: Compiles to a terminal-based simulation (ASCII output) for logic verification.
  - **Jaguar**: Compiles to a binary using standard Jaguar toolchains (e.g., `m68k-atari-jaguar-gcc`).

## File Structure

- `src/demo_main.c`: Entry point. Handles setup and main loop for the demo.
- `src/jag_physics.c` / `.h`: Core physics engine (`jp_*` prefix).
- `src/jag_fixed.h`: Math macros and functions.
- `src/jag_gpu.c` / `.h`: GPU interface and mock implementation.
- `src/jag_platform.h`: Platform-specific definitions.
- `src/demo_bitmap.c` / `.h`: Simple graphics routines for the framebuffer (used in demo).

## Building

### For PC (Simulation)
If you have GCC or a similar C compiler installed:
```bash
make
./jag_physics_pc
```
This will run the physics simulation and output ASCII frames to the terminal.

## Usage

Initialize the world:
```c
jp_world_t world;
jp_world_init(&world, (jp_vec2_t){0, INT_TO_FIXED(10)}); // Gravity
```

Add bodies:
```c
// Add a static ground box
jp_body_t* ground = jp_world_add_body(&world, 
    (jp_shape_t){.type = JP_SHAPE_AABB, .aabb = {INT_TO_FIXED(100), INT_TO_FIXED(10)}}, 
    INT_TO_FIXED(160), INT_TO_FIXED(220), 
    0); // Mass 0 = Static

// Add a dynamic circle
jp_body_t* ball = jp_world_add_body(&world, 
    (jp_shape_t){.type = JP_SHAPE_CIRCLE, .radius = INT_TO_FIXED(10)}, 
    INT_TO_FIXED(160), INT_TO_FIXED(50), 
    INT_TO_FIXED(1)); // Mass 1

// Add a tether connecting them (Pendulum)
jp_world_add_tether(&world, ground, ball, INT_TO_FIXED(50)); // Max length 50

```

Step the simulation:
```c
// Step the simulation
jp_world_step(&world, INT_TO_FIXED(1)/60);
```

## Typical Interactions

You can inspect the state of bodies after a step to react to changes.

### Handling Collisions
The engine automatically handles physical resolution (bouncing/sliding). For game logic reactions, you can iterate the bodies or check collision flags (future work).

### Handling Off-Screen Objects
```c
if (obj->position.x > INT_TO_FIXED(320) || obj->position.x < 0) {
    // Reset position
    obj->position.x = INT_TO_FIXED(160);
    obj->velocity.x = 0;
}
```

## Optimization for Jaguar
- The engine currently runs on the 68000.
- **GPU Offloading**: The `jp_world_step` function is designed to be moved to the GPU (RISC) processor. It uses isolated data arrays and aligned structs.
- `jag_fixed.h` macros are efficient, but inline assembly for `muls` and `divs` could speed up math on the 68k.
