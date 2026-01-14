# Alpha Kinetics TODO

This file tracks potential future work, architectural enhancements, and known issues for the Alpha Kinetics physics engine.

## Physics Engine Enhancements

### Selective Collisions (Collision Layers)
- **Goal**: Allow objects to selectively collide with other objects or passes through them.
- **Implementation**: Add `bitmask` fields to `ak_body_t` (e.g., `uint32_t categories`, `uint32_t mask`).
- **Potential Pitfalls**:
    - **Performance**: Bitwise operations are cheap, but increasing `ak_body_t` size affects Cache and DMA on Jaguar.
    - **Memory**: Keep the mask size small (e.g., 8-16 bits) to minimize RAM impact on Arduboy.

### Rotational Physics (Angled Objects)
- **Goal**: Support rotation, angular velocity, and moment of inertia.
- **Implementation**:
    - Add `angle`, `angular_velocity`, `torque`, and `inertia` to `ak_body_t`.
    - Implement OBB (Oriented Bounding Box) collision detection (Separating Axis Theorem).
- **Potential Pitfalls**:
    - **Complexity**: SAT is significantly more CPU intensive than AABB checks.
    - **Fixed-Point Precision**: Sin/Cos lookups or approximations are needed. Rotating vectors in fixed-point can lead to drift if not careful.
    - **Arduboy**: Might be too heavy for many objects on an 8-bit AVR.

### Advanced Collision Resolution
- **Friction**: Implement static and dynamic friction. Current implementation only handles restitution.
- **Improved Restitution**: Refine the impulse calculation to better handle stacked objects or high-speed impacts.
- **Continuous Collision Detection (CCD)**: Prevent "tunneling" for fast-moving small objects.
- **Potential Pitfalls**:
    - CCD is very expensive. A simpler sweep-test or multi-stepping approach might be better for this platform.

### Spatial Partitioning (Broadphase)
- **Goal**: Optimize collision detection for scenes with many objects.
- **Implementation**: Spatial hashing or a simple grid.
- **Potential Pitfalls**:
    - **Memory**: Grids take up precious RAM. Dynamic spatial hashing might be better but harder to implement without `malloc`.

---

## Demos & Rendering

### Curved Tether Display
- **Goal**: In the Playdate and Jaguar demos, draw tethers as curved lines (catenary or parabola approximation) when they are slack.
- **Implementation**: If `distance < max_length`, calculate 3-5 segments with a downward sag factor.
- **Potential Pitfalls**:
    - **Jaguar**: Line drawing is handled by the Blitter or CPU. Drawing many segments for many tethers might impact framerate.

### Interactive Sandbox Features
- **Goal**: Better interaction in demos.
- **Ideas**:
    - **Playdate**: Use the crank to rotate the gravity vector or a "spinner" object.
    - **Arduboy/Jaguar**: Select and drag objects with a cursor.

---

## Maintenance & Refactoring

### Bug: Restitution "Snap"
- **Issue**: In `ResolveCircleAABB`, there's a suspicious `m.depth = r;` overwrite which might cause jitter when a circle is deeply embedded in a box.
- **Fix**: Ensure depth 64-bit math is consistent across all solvers.

### Optimization
- **Jaguar DMA**: Further optimize `ak_body_t` layout. Ensure the solver can process bodies in chunks that fit in Scratchpad RAM.
- **Arduboy**: Evaluate if `int16_t` for some properties (like radius or half-extents) would save enough RAM and cycles without sacrificing world scale.
