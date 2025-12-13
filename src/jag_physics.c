#include "jag_physics.h"
#include <stddef.h>

// --- Vector Math ---

jp_vec2_t jp_vec2_add(jp_vec2_t a, jp_vec2_t b) {
  return (jp_vec2_t){FIXED_ADD(a.x, b.x), FIXED_ADD(a.y, b.y)};
}

jp_vec2_t jp_vec2_sub(jp_vec2_t a, jp_vec2_t b) {
  return (jp_vec2_t){FIXED_SUB(a.x, b.x), FIXED_SUB(a.y, b.y)};
}

jp_vec2_t jp_vec2_mul(jp_vec2_t v, jag_fixed_t s) {
  return (jp_vec2_t){FIXED_MUL(v.x, s), FIXED_MUL(v.y, s)};
}

jag_fixed_t jp_vec2_dot(jp_vec2_t a, jp_vec2_t b) {
  return FIXED_ADD(FIXED_MUL(a.x, b.x), FIXED_MUL(a.y, b.y));
}

// Safe length squared to prevent overflow
jag_fixed_t jp_vec2_len_sqr(jp_vec2_t v) {
  const jag_fixed_t LIMIT = 5000000;
  if (v.x > LIMIT || v.x < -LIMIT || v.y > LIMIT || v.y < -LIMIT) {
    return 2147483647; // INT32_MAX
  }
  return jp_vec2_dot(v, v);
}

// Safe length using 64-bit intermediates to support screen-width distances
// dist_sqr for >181px overflows 32-bit fixed point.
jag_fixed_t jp_vec2_len(jp_vec2_t v) {
  int64_t x = v.x;
  int64_t y = v.y;
  int64_t sqr = (x * x) + (y * y); // 32.32 format essentially

  // Sqrt logic for 64-bit input to 16.16 output
  // dist = sqrt(x^2 + y^2)
  // We want result in 16.16.
  // Input 'sqr' is effectively (value * 65536)^2.
  // Sqrt(sqr) = value * 65536.
  // So standard integer sqrt of the 64-bit sum gives the correct fixed-point
  // representation directly.

  if (sqr <= 0)
    return 0;

  uint64_t root = 0;
  uint64_t rem = (uint64_t)sqr;
  uint64_t place = 1ULL << 62;

  while (place > rem)
    place >>= 2;

  while (place) {
    if (rem >= root + place) {
      rem -= root + place;
      root += place * 2;
    }
    root >>= 1;
    place >>= 2;
  }

  return (jag_fixed_t)root;
}

// -- World --

void jp_world_init(jp_world_t *world, jp_vec2_t gravity) {
  world->gravity = gravity;
  world->body_count = 0;
  world->tether_count = 0;
}

jp_body_t *jp_world_add_body(jp_world_t *world, jp_shape_t shape, jag_fixed_t x,
                             jag_fixed_t y, jag_fixed_t mass) {
  if (world->body_count >= JP_MAX_BODIES) {
    return 0;
  }
  jp_body_t *b = &world->bodies[world->body_count++];
  b->position = (jp_vec2_t){x, y};
  b->velocity = (jp_vec2_t){0, 0};
  b->force = (jp_vec2_t){0, 0};
  b->shape = shape;
  b->inv_mass = (mass > 0) ? FIXED_DIV(JAG_FIXED_ONE, mass) : 0;
  b->restitution = FIXED_DIV(INT_TO_FIXED(7), INT_TO_FIXED(10)); // 0.7
  b->is_static = (mass == 0);
  return b;
}

void jp_world_add_tether(jp_world_t *world, jp_body_t *a, jp_body_t *b,
                         jag_fixed_t max_length) {
  if (world->tether_count >= JP_MAX_TETHERS)
    return;
  jp_tether_t *t = &world->tethers[world->tether_count++];
  t->a = a;
  t->b = b;
  t->max_length_sqr = FIXED_MUL(max_length, max_length);
}

static void ResolveTethers(jp_world_t *world) {
  for (int i = 0; i < world->tether_count; i++) {
    jp_tether_t *t = &world->tethers[i];
    jp_vec2_t diff = jp_vec2_sub(t->b->position, t->a->position);

    // Optimization: Quick AABB rejection first
    jag_fixed_t max_len = FIXED_SQRT(t->max_length_sqr);

    // Quick rejection: if either component > max_len, we are definitely outside
    if (FIXED_ABS(diff.x) <= max_len && FIXED_ABS(diff.y) <= max_len) {
      // Safe to use squared checks if we wanted, but sticking to safe length.
    }

    // Calculate precise safe length (64-bit friendly)
    jag_fixed_t dist = jp_vec2_len(diff);

    if (dist <= max_len)
      continue;

    jag_fixed_t excess = FIXED_SUB(dist, max_len);

    // Normalize diff to get direction: n = diff / dist
    jp_vec2_t n = jp_vec2_mul(diff, FIXED_DIV(JAG_FIXED_ONE, dist));

    // SOFT CONSTRAINT & STABILIZATION
    const jag_fixed_t stiffness = INT_TO_FIXED(5) / 10; // 0.5
    jag_fixed_t correction_mag = FIXED_MUL(excess, stiffness);

    // Clamp correction
    jag_fixed_t max_corr = INT_TO_FIXED(5);
    if (correction_mag > max_corr)
      correction_mag = max_corr;

    jp_vec2_t move = jp_vec2_mul(n, correction_mag);

    jag_fixed_t total_imass = FIXED_ADD(t->a->inv_mass, t->b->inv_mass);
    if (total_imass == 0)
      continue;

    if (!t->a->is_static) {
      jag_fixed_t share = FIXED_DIV(t->a->inv_mass, total_imass);
      t->a->position = jp_vec2_add(t->a->position, jp_vec2_mul(move, share));

      jag_fixed_t vrel =
          jp_vec2_dot(jp_vec2_sub(t->b->velocity, t->a->velocity), n);
      if (vrel > 0) {
        // Apply impulse to kill relative velocity
        // P = vrel / total_imass (magnitude of impulse)
        // dV = P * inv_mass * n
        jp_vec2_t P = jp_vec2_mul(n, FIXED_DIV(vrel, total_imass));
        t->a->velocity =
            jp_vec2_add(t->a->velocity, jp_vec2_mul(P, t->a->inv_mass));
      }
    }
    if (!t->b->is_static) {
      jag_fixed_t share = FIXED_DIV(t->b->inv_mass, total_imass);
      t->b->position = jp_vec2_sub(t->b->position, jp_vec2_mul(move, share));

      jag_fixed_t vrel =
          jp_vec2_dot(jp_vec2_sub(t->b->velocity, t->a->velocity), n);
      if (vrel > 0) {
        jp_vec2_t P = jp_vec2_mul(n, FIXED_DIV(vrel, total_imass));
        t->b->velocity =
            jp_vec2_sub(t->b->velocity, jp_vec2_mul(P, t->b->inv_mass));
      }
    }
  }
}

// --- Collision ---

typedef struct {
  jp_body_t *a;
  jp_body_t *b;
  jp_vec2_t normal;
  jag_fixed_t depth;
  int has_collision;
} jp_manifold_t;

jp_manifold_t SolveCircleCircle(jp_body_t *a, jp_body_t *b) {
  jp_manifold_t m = {a, b, {0, 0}, 0, 0};
  jp_vec2_t n = jp_vec2_sub(b->position, a->position);
  jag_fixed_t dist_sqr = jp_vec2_len_sqr(n);
  jag_fixed_t r =
      FIXED_ADD(a->shape.bounds.circle.radius, b->shape.bounds.circle.radius);

  if (dist_sqr >= FIXED_MUL(r, r))
    return m;
  if (dist_sqr == 0) {
    m.depth = r;
    m.normal = (jp_vec2_t){JAG_FIXED_ONE, 0};
    m.has_collision = 1;
    return m;
  }

  jag_fixed_t dist = FIXED_SQRT(dist_sqr);
  m.depth = FIXED_SUB(r, dist);
  m.normal = jp_vec2_mul(n, FIXED_DIV(JAG_FIXED_ONE, dist));
  m.has_collision = 1;
  return m;
}

jp_manifold_t SolveAABBAABB(jp_body_t *a, jp_body_t *b) {
  jp_manifold_t m = {a, b, {0, 0}, 0, 0};
  jp_vec2_t n = jp_vec2_sub(b->position, a->position);

  jag_fixed_t a_w = a->shape.bounds.aabb.width;
  jag_fixed_t a_h = a->shape.bounds.aabb.height;
  jag_fixed_t b_w = b->shape.bounds.aabb.width;
  jag_fixed_t b_h = b->shape.bounds.aabb.height;

  jag_fixed_t x_overlap = FIXED_SUB(FIXED_ADD(a_w, b_w), FIXED_ABS(n.x));
  if (x_overlap <= 0)
    return m;

  jag_fixed_t y_overlap = FIXED_SUB(FIXED_ADD(a_h, b_h), FIXED_ABS(n.y));
  if (y_overlap <= 0)
    return m;

  if (x_overlap < y_overlap) {
    m.depth = x_overlap;
    m.normal = (jp_vec2_t){n.x < 0 ? -JAG_FIXED_ONE : JAG_FIXED_ONE, 0};
  } else {
    m.depth = y_overlap;
    m.normal = (jp_vec2_t){0, n.y < 0 ? -JAG_FIXED_ONE : JAG_FIXED_ONE};
  }
  m.has_collision = 1;
  return m;
}

jp_manifold_t SolveCircleAABB(jp_body_t *circle, jp_body_t *aabb) {
  jp_manifold_t m = {circle, aabb, {0, 0}, 0, 0};

  jp_vec2_t diff = jp_vec2_sub(circle->position, aabb->position);
  jag_fixed_t half_w = aabb->shape.bounds.aabb.width;
  jag_fixed_t half_h = aabb->shape.bounds.aabb.height;
  jag_fixed_t clamped_x = FIXED_MAX(-half_w, FIXED_MIN(half_w, diff.x));
  jag_fixed_t clamped_y = FIXED_MAX(-half_h, FIXED_MIN(half_h, diff.y));

  jp_vec2_t closest = {clamped_x, clamped_y};
  jp_vec2_t n = jp_vec2_sub(diff, closest);
  jag_fixed_t dist_sqr = jp_vec2_len_sqr(n);
  jag_fixed_t r = circle->shape.bounds.circle.radius;

  if (dist_sqr > FIXED_MUL(r, r))
    return m;

  m.has_collision = 1;

  if (dist_sqr == 0) {
    if (FIXED_ABS(diff.x) > FIXED_ABS(diff.y)) {
      m.depth = FIXED_ADD(r, half_w);
      // Normal from Circle to AABB (A->B)
      // Diff is Circle - AABB. If diff.x > 0, Circle is to the Right. A->B
      // should be Left (-1).
      m.normal = (jp_vec2_t){diff.x > 0 ? -JAG_FIXED_ONE : JAG_FIXED_ONE, 0};
    } else {
      m.depth = FIXED_ADD(r, half_h);
      m.normal = (jp_vec2_t){0, diff.y > 0 ? -JAG_FIXED_ONE : JAG_FIXED_ONE};
    }
    m.depth = r;
  } else {
    jag_fixed_t dist = FIXED_SQRT(dist_sqr);
    m.depth = FIXED_SUB(r, dist);
    // n is Box->Circle. We want A->B (Circle->Box). So negate.
    m.normal = jp_vec2_mul(n, -FIXED_DIV(JAG_FIXED_ONE, dist));
  }

  return m;
}

void ResolveCollision(jp_manifold_t *m) {
  if (!m->has_collision)
    return;

  jp_vec2_t rv = jp_vec2_sub(m->b->velocity, m->a->velocity);
  jag_fixed_t vel_along_normal = jp_vec2_dot(rv, m->normal);

  if (vel_along_normal > 0)
    return;

  jag_fixed_t e = FIXED_MIN(m->a->restitution, m->b->restitution);
  jag_fixed_t j = FIXED_MUL(-(JAG_FIXED_ONE + e), vel_along_normal);
  jag_fixed_t den = FIXED_ADD(m->a->inv_mass, m->b->inv_mass);

  if (den == 0)
    return;

  j = FIXED_DIV(j, den);

  jp_vec2_t impulse = jp_vec2_mul(m->normal, j);

  if (!m->a->is_static)
    m->a->velocity =
        jp_vec2_sub(m->a->velocity, jp_vec2_mul(impulse, m->a->inv_mass));
  if (!m->b->is_static)
    m->b->velocity =
        jp_vec2_add(m->b->velocity, jp_vec2_mul(impulse, m->b->inv_mass));

  const jag_fixed_t percent = INT_TO_FIXED(1) / 5; // 0.2
  const jag_fixed_t slop = INT_TO_FIXED(1) / 100;  // 0.01
  jag_fixed_t correction_mag = FIXED_MAX(FIXED_SUB(m->depth, slop), 0);
  jag_fixed_t corr_num = FIXED_MUL(correction_mag, percent);
  correction_mag = FIXED_DIV(corr_num, den);
  jp_vec2_t correction = jp_vec2_mul(m->normal, correction_mag);

  if (!m->a->is_static)
    m->a->position =
        jp_vec2_sub(m->a->position, jp_vec2_mul(correction, m->a->inv_mass));
  if (!m->b->is_static)
    m->b->position =
        jp_vec2_add(m->b->position, jp_vec2_mul(correction, m->b->inv_mass));
}

void jp_world_step(jp_world_t *world, jag_fixed_t dt) {
  for (int i = 0; i < world->body_count; i++) {
    jp_body_t *b = &world->bodies[i];
    if (b->is_static)
      continue;

    // Apply gravity
    b->force = jp_vec2_add(
        b->force,
        jp_vec2_mul(world->gravity, FIXED_DIV(JAG_FIXED_ONE, b->inv_mass)));

    // Integrate Velocity
    jp_vec2_t acceleration = jp_vec2_mul(b->force, b->inv_mass);
    b->velocity = jp_vec2_add(b->velocity, jp_vec2_mul(acceleration, dt));

    // Integrate Position
    b->position = jp_vec2_add(b->position, jp_vec2_mul(b->velocity, dt));

    // Reset force
    b->force = (jp_vec2_t){0, 0};
  }

  // Collisions
  for (int i = 0; i < world->body_count; i++) {
    for (int j = i + 1; j < world->body_count; j++) {
      jp_manifold_t m = {0};
      jp_body_t *a = &world->bodies[i];
      jp_body_t *b = &world->bodies[j];

      if (a->is_static && b->is_static)
        continue;

      if (a->shape.type == JP_SHAPE_CIRCLE &&
          b->shape.type == JP_SHAPE_CIRCLE) {
        m = SolveCircleCircle(a, b);
      } else if (a->shape.type == JP_SHAPE_AABB &&
                 b->shape.type == JP_SHAPE_AABB) {
        m = SolveAABBAABB(a, b);
      } else if (a->shape.type == JP_SHAPE_CIRCLE &&
                 b->shape.type == JP_SHAPE_AABB) {
        m = SolveCircleAABB(a, b);
      } else if (a->shape.type == JP_SHAPE_AABB &&
                 b->shape.type == JP_SHAPE_CIRCLE) {
        m = SolveCircleAABB(b, a);
        m.normal = jp_vec2_mul(m.normal, -JAG_FIXED_ONE);
        m.a = a;
        m.b = b;
      }

      if (m.has_collision) {
        ResolveCollision(&m);
      }
    }
  }

  // Tethers
  ResolveTethers(world);
}
