# Jaguar Physics Engine Walkthrough

I have implemented a lightweight 2D physics engine in C, tailored for the Atari Jaguar's architecture.

## Design Decisions

1.  **Fixed-Point Arithmetic (`fixed_point.h`)**:
    *   The Atari Jaguar's 68000 CPU does not have a Floating Point Unit (FPU).
    *   I implemented a 16.16 fixed-point math library to ensure performance.
    *   All physics calculations (position, velocity, collision) use `fixed_t` instead of `float`.

2.  **Core Physics (`physics.c`)**:
    *   **Integration**: Uses Semi-Implicit Euler (Symplectic Euler) for stability.
    *   **Collision Detection**:
        *   **Circle-Circle**: Fast distance check.
        *   **AABB-AABB**: Fast axis overlap check.
    *   **Resolution**: Impulse-based resolution handles bouncing and mass ratios correctly.

3.  **GPU Readiness (Refactored)**:
    *   **Data Alignment**: `Body` structs are padded to 64 bytes (16-byte aligned) for efficient DMA transfers to GPU RAM.
    *   **Logic Isolation**: `World_Step` operates on a raw array of bodies, removing dependencies on the `World` struct layout. This allows the function to be easily moved to the GPU.

4.  **GPU Interface (`gpu_interface.h`)**:
    *   Provides `GPU_Run` and `GPU_Wait` helpers to abstract the offloading process.
    *   Allows the main CPU (68k) to dispatch physics tasks to the GPU (RISC) and synchronize results.

5.  **Collision Events**:
    *   `World_Step` now returns a list of `Contact` events.
    *   Allows client code to react to specific collisions (e.g., deleting objects) without modifying the core engine.

6.  **Hardware Abstraction (`jaguar.h`)**:
    *   Defined memory-mapped I/O addresses for the Jaguar's TOM and JERRY chips.
    *   This allows the code to compile for the target hardware while being readable.

4.  **Demo Application (`main.c`)**:
    *   **Dual-Mode**:
        *   `#ifdef JAGUAR`: Writes directly to the Jaguar's framebuffer in DRAM.
        *   `#else` (PC): Runs a simulation loop and prints an ASCII representation of the world to the terminal.
    *   This allows you to verify logic on a PC before deploying to the console.

## Files

- `src/fixed_point.h`: Math library.
- `src/physics.h` / `src/physics.c`: The engine.
- `src/bitmap.h` / `src/bitmap.c`: Graphics drawing.
- `src/jaguar.h`: Hardware definitions.
- `src/main.c`: The demo app.
- `Makefile`: Build script.

## Next Steps

- **Compilation**: Use your Jaguar GCC toolchain (e.g., `m68k-atari-jaguar-gcc`) to compile the project.
- **Optimization**: Move the `World_Step` function to the GPU (RISC) for parallel processing.
- **Graphics**: Replace the software `Bitmap_Draw` functions with Blitter commands (`BLIT_CMD`) for hardware acceleration.
