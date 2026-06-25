# Performance Analysis - Alpha Kinetics

This document outlines potential performance bottlenecks and areas for optimization within the Alpha Kinetics physics engine, specifically targeting low-powered retro and embedded platforms (Atari Jaguar, Atari Lynx, Arduboy FX, Playdate).

## 1. Collision Detection Complexity ($O(n^2)$)
The current implementation of `ak_world_step` uses a nested loop to check every pair of bodies for collisions:
```c
// src/core/ak_physics.c:355
for (int i = 0; i < world->body_count; i++) {
  for (int j = i + 1; j < world->body_count; j++) {
    // ... collision checks ...
  }
}
```
As the number of bodies ($n$) increases, the number of checks grows quadratically. On systems with very limited CPU cycles, this will rapidly become the primary bottleneck.

**Recommendation:** Implement a spatial partitioning system (e.g., a uniform grid or a basic quadtree) to reduce the number of collision pairs checked per frame from $O(n^2)$ to approximately $O(n)$.

## 2. Expensive Square Root Operations
The `AK_FIXED_SQRT` implementation in `src/core/ak_fixed.h` uses an iterative bit-by-bit approach. While necessary for precision, frequent calls to this function (during circle-to-circle, circle-to-AABB collision resolution, and tether updates) are computationally expensive on processors without dedicated hardware support.

**Recommendation:** 
- Continue using squared distance checks (`ak_vec2_len_sqr`) where possible to avoid `sqrt` entirely (as already done in some parts of the engine).
- For very tight loops, consider a faster, lower-precision approximation of the square root if the accuracy trade-off is acceptable for the specific use case.

## 3. Fixed-Point Division and Multiplication
The engine relies heavily on `AK_FIXED_DIV` (which involves 64-bit left shifts and division) and `AK_FIXED_MUL`. High-frequency calls to these macros, especially within nested loops, add significant overhead.

**Recommendation:**
- Minimize the use of divisions in hot loops. Many operations can be refactored to use multiplications by precomputed reciprocals (e.g., storing `inv_mass` as already done).
- In `ResolveTethers`, the engine calculates `AK_FIXED_DIV(AK_FIXED_ONE, dist)` inside the loop. If multiple tethers share a body, some of this work could potentially be cached or optimized.

## 4. Memory and Cache Locality
While the use of arrays (`ak_body_t bodies[AK_MAX_BODIES]`) is good for cache locality on many platforms, the `ak_world_t` structure contains large arrays directly within it. On systems with extremely limited RAM (like Arduboy), the stack/static memory usage might become a concern if `AK_MAX_B

**Recommendation:** Keep monitoring the memory footprint of `ak_world_t` as `AK_MAX_BODIES` and `AK_MAX_TETHERS` are increased.
