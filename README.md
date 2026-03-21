# Alpha Kinetics

A portable, lightweight, fixed-point 2D physics engine written in C89/C99. Designed for retro consoles and embedded systems (Atari Jaguar, Atari Lynx, Arduboy FX, Playdate).

## Features

- **Flexible Fixed-Point Arithmetic**: Uses 16.16 fixed-point math (`ak_fixed.h`) by default. Automatically switches to 8.8 for 8-bit platforms (Atari Lynx, Arduboy) for performance.
- **Rigid Body Physics**: Supports linear physics (position, velocity, acceleration, mass).
- **Collision Detection**:
  - Circle-to-Circle
  - AABB-to-AABB
  - Circle-to-AABB
- **Collision Resolution**: Impulse-based resolution with restitution (bounciness) and positional correction.
- **Distance Constraints (Tethers)**: Supports massless, soft-constraint tethers (pendulums, chains).
- **8-bit Optimized**: Refactored to avoid returning structs by value and to use pointers, ensuring compatibility with compilers like `cc65`.
- **Platform Agnostic Core**: Logic isolated in `src/core`, platform specific code in `src/platforms`.

## Project Structure

- `src/core/`: Platform-independent library.
  - `ak_physics.c/.h`: Core solver and API.
  - `ak_fixed.h`: Fixed-point math macros.
  - `ak_demo_setup.c/.h`: Shared scene configurations for demos.
- `src/platforms/`: Platform-specific entry points and rendering.
  - `lynx/`: Atari Lynx demo (using `cc65` and `tgi`).
  - `jaguar/`: Atari Jaguar demo.
    - `rmvlib/`: Removers Video Library (Atari Jaguar).
    - `jlibc/`: Removers C Library (Atari Jaguar).
  - `pc/`: Terminal-based ASCII simulation.
  - `arduboy/`: Arduboy FX demo boilerplate.
  - `playdate/`: Playdate C SDK demo boilerplate.

## Building

### For PC (ASCII Simulation)
Quickly test logic in your terminal:
```bash
make pc
./alpha_kinetics_pc
```

### For Atari Lynx
Builds for the handheld using `cc65`:
```bash
make lynx
```
Produces `build/lynx/alpha_kinetics.lnx`.

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

**Build using Make:**
Requires `arduino-cli` installed and configured.

```bash
make arduboy
```
Output .ino project located in `build/arduboy/AlphaKinetics/`.
Binary output files located in `build/arduboy/bin/`.

### For Playdate
See a [sample](https://youtu.be/LW0G3OG3wR8?si=wGSbuTkPkvYbQIEq) running on real hardware.

**Build using Make:**
Requires Playdate SDK and `cmake`.

```bash
make playdate
```
Output located in `build/playdate_sim` or `build/playdate_device`.

## Using the API

### 1. Initialize World
```c
ak_world_t world;
ak_vec2_t gravity;
ak_vec2_set(&gravity, 0, AK_INT_TO_FIXED(50));
ak_world_init(&world, AK_INT_TO_FIXED(320), AK_INT_TO_FIXED(240), &gravity);
```

### 2. Add Bodies
```c
ak_shape_t shape;

// Static ground
ak_shape_aabb_set(&shape, AK_INT_TO_FIXED(50), AK_INT_TO_FIXED(10));
ak_world_add_body(&world, &shape, AK_INT_TO_FIXED(80), AK_INT_TO_FIXED(120), 0);

// Dynamic Circle
ak_shape_circle_set(&shape, AK_INT_TO_FIXED(8));
ak_body_t* ball = ak_world_add_body(&world, &shape, AK_INT_TO_FIXED(80), AK_INT_TO_FIXED(20), AK_INT_TO_FIXED(1));
```

### 3. Simulation Step
```c
ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60;
ak_world_step(&world, dt);
```

## Optimization and Portability
- **8-bit Platforms**: Switches to 8.8 fixed-point automatically for `__CC65__` or `ARDUBOY`.
- **Pass-by-Pointer**: All struct-based API calls use pointers to avoid return-value limitations on 6502/AVR.
- **Memory Constraints**: Adjust `AK_MAX_BODIES` and `AK_MAX_TETHERS` at compile time for tight RAM targets.
