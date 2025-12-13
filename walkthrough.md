# Refactoring Verification Walkthrough

The codebase has been refactored to use a clear naming convention:
*   `jp_` for Core Physics (`jag_physics`, `jag_fixed`)
*   `jag_` for Platform/Hardware (`jag_platform`, `jag_gpu`)
*   `demo_` for the Demo Application (`demo_main`, `demo_bitmap`)

## Verification Steps (WSL)

Since you have WSL (Windows Subsystem for Linux) installed, verifying the build is straightforward.

### 1. Build the Project
Run the following command in your terminal (PowerShell or Command Prompt) to compile the code using WSL's `gcc`:

```powershell
wsl make clean; wsl make
```

You should see output similar to:
```text
rm -f jag_physics_pc *.o
gcc -Wall -O2 -Isrc -o jag_physics_pc src/demo_main.c src/jag_physics.c src/demo_bitmap.c src/jag_gpu.c
```

### 2. Run the Demo
Run the compiled executable:

```powershell
wsl ./jag_physics_pc
```

**Note**: The demo now runs in an infinite loop (~60 FPS). Press `Ctrl+C` to stop it.

### 3. Expected Output
The demo runs a physics simulation (falling circle and box).
*   **Characters**:
    *   `O` = Dynamic Body (Circle)
    *   `[` = Dynamic Body (AABB / Box)
    *   `#` = Static Body (Ground / AABB)
*   **Behavior**:
    1.  The Circle (`O`) falls and hits the Ground (`#`).
    2.  Collision is detected (Circle vs AABB).
    3.  A message "Collision Event: Circle hit Ground! Deleting Circle..." appears.
    4.  The Circle disappears.
    5.  The Box (`[`) continues to bounce on the Ground.

Example ASCII Frame:
```text
........................................
........................................
...................O....................
........................................
........................................
..................[[[...................
..................[[[...................
........................................
........................................
........................................
#######.................................
```

## Changes made
*   Renamed `src/physics.c` -> `src/jag_physics.c`
*   Renamed `src/gpu_interface.c` -> `src/jag_gpu.c`
*   Renamed `src/main.c` -> `src/demo_main.c`
*   Refactored all code to use `jp_` and `jag_` prefixes.
*   Updated `demo_main.c` to run indefinitely on PC with a frame delay for better visibility.
*   **Fixed Physics**: Added `SolveCircleAABB` to correctly handle Circle-Ground collisions.
*   **Fixed Visuals**: Updated ASCII renderer to fill shapes (rectangles and rough circles) instead of just single points.
*   **Fixed Stability**: Resolved Floating Point Exception (crash) caused by integer overflow in distance squared calculation. Initially added a safety check with `LIMIT=10M`, but later reduced to `LIMIT=5M` to prevent `x*x + y*y` from overflowing signed 32-bit integer max value (which caused invalid negative results and subsequent division by zero).
*   **Fixed Collision Logic**: Corrected inverted collision normal in `SolveCircleAABB` which caused objects to pass through each other in certain valid collision scenarios (like top-down stacking).
*   **Added Tethers**: Implemented a distance constraint (tether) system.
    - Added `jp_tether_t` struct and `jp_world_add_tether`.
    - Added `ResolveTethers` step to the physics solver.
    - Added a pendulum demo to `demo_main.c` to visualize the tether.
    - **Stability Fix**: Implemented a "soft constraint" approach (0.5 stiffness) and clamped maximum position correction per frame to prevent energy injection (explosions) when the tether snaps taut.
    - **Overflow Fix**: Implemented `jp_vec2_len` using 64-bit intermediate calculations to correctly handle distances > 181 pixels (which previously overflowed 32-bit fixed point), removing the artificial `LIMIT` that caused physics explosions on screen-width movements.
    - **Velocity Logic Fix**: Corrected a math error in `ResolveTethers` where the correction impulse was being divided by `total_imass` twice (once in calculation, once in application), causing a 5x positive feedback loop in velocity that led to "snapping" and "disappearing" bodies at the apex of the swing.

### Demo Expansion
The `jag_physics_pc` demo now features a complex scene demonstrating multiple interaction types:
1.  **Anchored Pendulum**: Validates static-dynamic tether constraints.
2.  **Tri-Bolas**: A chain of 3 dynamic bodies connected by tethers, validating dynamic-dynamic constraints and momentum transfer (spinning/tumbling).
3.  **Free Falling Objects**: A Box and a Circle to demonstrate independent physics integration alongside tethers.
4.  **Ground Plane**: A static AABB at the bottom to catch objects.
