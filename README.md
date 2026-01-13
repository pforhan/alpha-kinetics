# Alpha Kinetics

A portable, lightweight, fixed-point 2D physics engine written in C99. Designed for retro consoles and embedded systems (Atari Jaguar, Arduboy FX, Playdate).

## Features

- **Fixed-Point Arithmetic**: Uses 16.16 fixed-point math (`ak_fixed.h`) to ensure consistent behavior across platforms without an FPU.
- **Rigid Body Physics**: Supports linear physics (position, velocity, acceleration, mass).
- **Collision Detection**: 
  - Circle-to-Circle
  - AABB-to-AABB
  - Circle-to-AABB
- **Collision Resolution**: Impulse-based resolution with restitution (bounciness) and positional correction.
- **Distance Constraints (Tethers)**: Supports massless, soft-constraint tethers (pendulums, chains).
- **Platform Agnostic Core**: Logic isolated in `src/core`, platform specific code in `src/platforms`.

## Project Structure

- `src/core/`: Platform-independent library.
  - `ak_physics.c/.h`: Core solver and API.
  - `ak_fixed.h`: Fixed-point math macros.
  - `ak_demo_setup.c/.h`: Shared scene configurations for demos.
- `src/platforms/`: Platform-specific entry points and rendering.
  - `jaguar/`: Atari Jaguar demo.
    - `rmvlib/`: Removers Video Library (Atari Jaguar).
    - `jlibc/`: Removers C Library (Atari Jaguar).
  - `pc/`: Terminal-based ASCII simulation.
  - `arduboy/`: Arduboy FX demo boilerplate.
  - `playdate/`: Playdate C SDK demo boilerplate.

## Getting Started

### For PC (ASCII Simulation)
Quickly test logic in your terminal:
```bash
make pc
./alpha_kinetics_pc
```

### For Atari Jaguar
Builds for the console using `m68k-atari-mint-gcc`:
```bash
make jaguar
```
Produces `alpha_kinetics_jag.cof`.

### For Arduboy FX
Integration via Arduino IDE or PlatformIO:
1. Include `src/core/ak_physics.h` and `.c`.
2. Define `-DAK_MAX_BODIES=16` to save RAM.
3. Link with `Arduboy2` and `ArduboyFX` libraries.

**Build using Make:**
Requires `arduino-cli` installed and configured.
```bash
make arduboy
```
Output located in `build/arduboy/AlphaKinetics/`.

### For Playdate
1. Add `src/core/ak_physics.c` to your project's source list.
2. include `ak_physics.h` in your `main.c`.

**Build using Make:**
Requires Playdate SDK and `cmake`.
```bash
make playdate
```
Output located in `build/playdate/AlphaKinetics.pdx`.

## Usage API

### 1. Initialize World
```c
ak_world_t world;
ak_world_init(&world, (ak_vec2_t){0, AK_INT_TO_FIXED(50)}); // Gravity
```

### 2. Add Bodies
```c
// Static ground
ak_world_add_body(&world, 
    (ak_shape_t){.type = AK_SHAPE_AABB, .bounds.aabb = {AK_INT_TO_FIXED(50), AK_INT_TO_FIXED(10)}}, 
    AK_INT_TO_FIXED(80), AK_INT_TO_FIXED(120), 0);

// Dynamic Circle
ak_body_t* ball = ak_world_add_body(&world, 
    (ak_shape_t){.type = AK_SHAPE_CIRCLE, .bounds.circle = {AK_INT_TO_FIXED(8)}}, 
    AK_INT_TO_FIXED(80), AK_INT_TO_FIXED(20), AK_INT_TO_FIXED(1));
```

### 3. Simulation Step
```c
ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60;
ak_world_step(&world, dt);
```

## Optimization and Portability
- **DMA Friendly**: `ak_body_t` padding is optimized for Jaguar DMA when `-DJAGUAR` is defined.
- **Memory Constraints**: Adjust `AK_MAX_BODIES` and `AK_MAX_TETHERS` at compile time for tight RAM targets.
- **Fixed-Point Intermediates**: Math routines use `int64_t` intermediates where necessary to prevent overflow during calculations involving screen-width distances.
