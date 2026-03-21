#include "ak_physics.h"
#include <stddef.h>
#include <string.h>

// --- Vector Math ---

void ak_vec2_add(ak_vec2_t *out, const ak_vec2_t *a, const ak_vec2_t *b) {
  ak_vec2_set(out, AK_FIXED_ADD(a->x, b->x), AK_FIXED_ADD(a->y, b->y));
}

void ak_vec2_sub(ak_vec2_t *out, const ak_vec2_t *a, const ak_vec2_t *b) {
  ak_vec2_set(out, AK_FIXED_SUB(a->x, b->x), AK_FIXED_SUB(a->y, b->y));
}

void ak_vec2_mul(ak_vec2_t *out, const ak_vec2_t *v, ak_fixed_t s) {
  ak_vec2_set(out, AK_FIXED_MUL(v->x, s), AK_FIXED_MUL(v->y, s));
}

ak_fixed_t ak_vec2_dot(const ak_vec2_t *a, const ak_vec2_t *b) {
  return AK_FIXED_ADD(AK_FIXED_MUL(a->x, b->x), AK_FIXED_MUL(a->y, b->y));
}

// Safe length squared to prevent overflow
ak_fixed_t ak_vec2_len_sqr(const ak_vec2_t *v) {
  // A diagonal for 320x240 is 400. 400px = 26,214,400 raw fixed.
  // We use 64-bit to compute the square safely, but the result must fit in
  // 32-bit. x^2 >> 16 < 2^31 => x^2 < 2^47 => x < 11,863,283. Since we sum x^2
  // + y^2, we should limit to ~8M each.
  const ak_fixed_t LIMIT = 8000000;
  if (v->x > LIMIT || v->x < -LIMIT || v->y > LIMIT || v->y < -LIMIT) {
    return 2147483647; // INT32_MAX
  }
  return ak_vec2_dot(v, v);
}

// Safe length using 64-bit intermediates to support screen-width distances
// dist_sqr for >181px overflows 32-bit fixed point.
ak_fixed_t ak_vec2_len(const ak_vec2_t *v) {
#if defined(__CC65__) || defined(ARDUBOY)
  // For 8.8 fixed point, 32-bit intermediate is enough for squares
  int32_t x;
  int32_t y;
  int32_t sqr;
  uint32_t root;
  uint32_t rem;
  uint32_t place;

  x = v->x;
  y = v->y;
  sqr = (x * x) + (y * y);

  if (sqr <= 0)
    return 0;

  // Integer sqrt for 8.8
  root = 0;
  rem = (uint32_t)sqr;
  place = 1UL << 30;

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
  return (ak_fixed_t)root;
#else
  int64_t x;
  int64_t y;
  int64_t sqr;
  uint64_t root;
  uint64_t rem;
  uint64_t place;

  x = v->x;
  y = v->y;
  sqr = (x * x) + (y * y); // 32.32 format essentially

  if (sqr <= 0)
    return 0;

  // Sqrt logic for 64-bit input to 16.16 output
  root = 0;
  rem = (uint64_t)sqr;
  place = 1ULL << 62;

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
  return (ak_fixed_t)root;
#endif
}

// -- World --

void ak_world_init(ak_world_t *world, ak_fixed_t width, ak_fixed_t height,
                   const ak_vec2_t *gravity) {
  ak_fixed_t scale_y;
  world->width = width;
  world->height = height;
  world->gravity = *gravity;
  world->body_count = 0;
  world->tether_count = 0;

  // Scale constants relative to height (standard height 240)
  scale_y = AK_FIXED_DIV(height, AK_INT_TO_FIXED(240));
  world->slop = AK_FIXED_MUL(scale_y, AK_INT_TO_FIXED(1) / 100); // 0.01 scaled
  world->max_correction =
      AK_FIXED_MUL(scale_y, AK_INT_TO_FIXED(5)); // 5.0 scaled
}

ak_body_t *ak_world_add_body(ak_world_t *world, const ak_shape_t *shape,
                             ak_fixed_t x, ak_fixed_t y, ak_fixed_t mass) {
  ak_body_t *b;
  if (world->body_count >= AK_MAX_BODIES) {
    return 0;
  }
  b = &world->bodies[world->body_count++];
  ak_vec2_set(&b->position, x, y);
  ak_vec2_set(&b->velocity, 0, 0);
  ak_vec2_set(&b->force, 0, 0);
  b->shape = *shape;
  b->inv_mass = (mass > 0) ? AK_FIXED_DIV(AK_FIXED_ONE, mass) : 0;
  b->restitution = AK_FIXED_DIV(AK_INT_TO_FIXED(7), AK_INT_TO_FIXED(10)); // 0.7
  b->is_static = (mass == 0);
  return b;
}

void ak_world_add_tether(ak_world_t *world, ak_body_t *a, ak_body_t *b,
                         ak_fixed_t max_length) {
  ak_tether_t *t;
  if (world->tether_count >= AK_MAX_TETHERS)
    return;
  t = &world->tethers[world->tether_count++];
  t->a = a;
  t->b = b;
  t->max_length_sqr = AK_FIXED_MUL(max_length, max_length);
}

static void ResolveTethers(ak_world_t *world) {
  int i;
  ak_tether_t *t;
  ak_vec2_t diff;
  ak_fixed_t max_len;
  ak_fixed_t dist;
  ak_fixed_t excess;
  ak_vec2_t n;
  ak_fixed_t stiffness;
  ak_fixed_t correction_mag;
  ak_fixed_t max_corr;
  ak_vec2_t move;
  ak_fixed_t total_imass;
  ak_fixed_t share;
  ak_fixed_t vrel;
  ak_vec2_t P;
  ak_vec2_t tmp_v;

  stiffness = AK_INT_TO_FIXED(5) / 10; // 0.5

  for (i = 0; i < world->tether_count; i++) {
    t = &world->tethers[i];
    ak_vec2_sub(&diff, &t->b->position, &t->a->position);

    // Optimization: Quick AABB rejection first
    max_len = AK_FIXED_SQRT(t->max_length_sqr);

    // Quick rejection: if either component > max_len, we are definitely outside
    if (AK_FIXED_ABS(diff.x) <= max_len && AK_FIXED_ABS(diff.y) <= max_len) {
      // Safe to use squared checks if we wanted, but sticking to safe length.
    }

    // Calculate precise safe length (64-bit friendly)
    dist = ak_vec2_len(&diff);

    if (dist <= max_len)
      continue;

    excess = AK_FIXED_SUB(dist, max_len);

    // Normalize diff to get direction: n = diff / dist
    ak_vec2_mul(&n, &diff, AK_FIXED_DIV(AK_FIXED_ONE, dist));

    // SOFT CONSTRAINT & STABILIZATION
    correction_mag = AK_FIXED_MUL(excess, stiffness);

    // Clamp correction
    max_corr = world->max_correction;
    if (correction_mag > max_corr)
      correction_mag = max_corr;

    ak_vec2_mul(&move, &n, correction_mag);

    total_imass = AK_FIXED_ADD(t->a->inv_mass, t->b->inv_mass);
    if (total_imass == 0)
      continue;

    if (!t->a->is_static) {
      share = AK_FIXED_DIV(t->a->inv_mass, total_imass);
      ak_vec2_mul(&tmp_v, &move, share);
      ak_vec2_add(&t->a->position, &t->a->position, &tmp_v);

      ak_vec2_sub(&tmp_v, &t->b->velocity, &t->a->velocity);
      vrel = ak_vec2_dot(&tmp_v, &n);
      if (vrel > 0) {
        // Apply impulse to kill relative velocity
        // P = vrel / total_imass (magnitude of impulse)
        // dV = P * inv_mass * n
        ak_vec2_mul(&P, &n, AK_FIXED_DIV(vrel, total_imass));
        ak_vec2_mul(&tmp_v, &P, t->a->inv_mass);
        ak_vec2_add(&t->a->velocity, &t->a->velocity, &tmp_v);
      }
    }
    if (!t->b->is_static) {
      share = AK_FIXED_DIV(t->b->inv_mass, total_imass);
      ak_vec2_mul(&tmp_v, &move, share);
      ak_vec2_sub(&t->b->position, &t->b->position, &tmp_v);

      ak_vec2_sub(&tmp_v, &t->b->velocity, &t->a->velocity);
      vrel = ak_vec2_dot(&tmp_v, &n);
      if (vrel > 0) {
        ak_vec2_mul(&P, &n, AK_FIXED_DIV(vrel, total_imass));
        ak_vec2_mul(&tmp_v, &P, t->b->inv_mass);
        ak_vec2_sub(&t->b->velocity, &t->b->velocity, &tmp_v);
      }
    }
  }
}

// --- Collision ---

typedef struct {
  ak_body_t *a;
  ak_body_t *b;
  ak_vec2_t normal;
  ak_fixed_t depth;
  int has_collision;
} ak_manifold_t;

static void SolveCircleCircle(ak_manifold_t *out, ak_body_t *a, ak_body_t *b) {
  ak_vec2_t n;
  ak_fixed_t dist_sqr;
  ak_fixed_t r;
  ak_fixed_t dist;
  ak_fixed_t inv_dist;

  out->a = a;
  out->b = b;
  ak_vec2_set(&out->normal, 0, 0);
  out->depth = 0;
  out->has_collision = 0;
  ak_vec2_sub(&n, &b->position, &a->position);
  dist_sqr = ak_vec2_len_sqr(&n);
  r = AK_FIXED_ADD(a->shape.bounds.circle.radius,
                   b->shape.bounds.circle.radius);

  if (dist_sqr >= AK_FIXED_MUL(r, r))
    return;
  if (dist_sqr == 0) {
    out->depth = r;
    ak_vec2_set(&out->normal, AK_FIXED_ONE, 0);
    out->has_collision = 1;
    return;
  }

  dist = AK_FIXED_SQRT(dist_sqr);
  out->depth = AK_FIXED_SUB(r, dist);
  inv_dist = AK_FIXED_DIV(AK_FIXED_ONE, dist);
  ak_vec2_mul(&out->normal, &n, inv_dist);
  out->has_collision = 1;
}

static void SolveAABBAABB(ak_manifold_t *out, ak_body_t *a, ak_body_t *b) {
  ak_vec2_t n;
  ak_fixed_t a_w;
  ak_fixed_t a_h;
  ak_fixed_t b_w;
  ak_fixed_t b_h;
  ak_fixed_t x_overlap;
  ak_fixed_t y_overlap;

  out->a = a;
  out->b = b;
  ak_vec2_set(&out->normal, 0, 0);
  out->depth = 0;
  out->has_collision = 0;
  ak_vec2_sub(&n, &b->position, &a->position);

  a_w = a->shape.bounds.aabb.width;
  a_h = a->shape.bounds.aabb.height;
  b_w = b->shape.bounds.aabb.width;
  b_h = b->shape.bounds.aabb.height;

  x_overlap = AK_FIXED_SUB(AK_FIXED_ADD(a_w, b_w), AK_FIXED_ABS(n.x));
  if (x_overlap <= 0)
    return;

  y_overlap = AK_FIXED_SUB(AK_FIXED_ADD(a_h, b_h), AK_FIXED_ABS(n.y));
  if (y_overlap <= 0)
    return;

  if (x_overlap < y_overlap) {
    out->depth = x_overlap;
    ak_vec2_set(&out->normal, n.x < 0 ? -AK_FIXED_ONE : AK_FIXED_ONE, 0);
  } else {
    out->depth = y_overlap;
    ak_vec2_set(&out->normal, 0, n.y < 0 ? -AK_FIXED_ONE : AK_FIXED_ONE);
  }
  out->has_collision = 1;
}

static void SolveCircleAABB(ak_manifold_t *out, ak_body_t *circle,
                            ak_body_t *aabb) {
  ak_vec2_t diff;
  ak_fixed_t half_w;
  ak_fixed_t half_h;
  ak_fixed_t clamped_x;
  ak_fixed_t clamped_y;
  ak_vec2_t closest;
  ak_vec2_t n;
  ak_fixed_t dist_sqr;
  ak_fixed_t r;
  ak_fixed_t dist;
  ak_fixed_t inv_dist;

  out->a = circle;
  out->b = aabb;
  ak_vec2_set(&out->normal, 0, 0);
  out->depth = 0;
  out->has_collision = 0;

  ak_vec2_sub(&diff, &circle->position, &aabb->position);
  half_w = aabb->shape.bounds.aabb.width;
  half_h = aabb->shape.bounds.aabb.height;
  clamped_x = AK_FIXED_MAX(-half_w, AK_FIXED_MIN(half_w, diff.x));
  clamped_y = AK_FIXED_MAX(-half_h, AK_FIXED_MIN(half_h, diff.y));

  ak_vec2_set(&closest, clamped_x, clamped_y);
  ak_vec2_sub(&n, &diff, &closest);
  dist_sqr = ak_vec2_len_sqr(&n);
  r = circle->shape.bounds.circle.radius;

  if (dist_sqr > AK_FIXED_MUL(r, r))
    return;

  out->has_collision = 1;

  if (dist_sqr == 0) {
    if (AK_FIXED_ABS(diff.x) > AK_FIXED_ABS(diff.y)) {
      out->depth = AK_FIXED_ADD(r, half_w);
      // Normal from Circle to AABB (A->B)
      // Diff is Circle - AABB. If diff.x > 0, Circle is to the Right. A->B
      // should be Left (-1).
      ak_vec2_set(&out->normal, diff.x > 0 ? -AK_FIXED_ONE : AK_FIXED_ONE, 0);
    } else {
      out->depth = AK_FIXED_ADD(r, half_h);
      ak_vec2_set(&out->normal, 0, diff.y > 0 ? -AK_FIXED_ONE : AK_FIXED_ONE);
    }
    out->depth = r;
  } else {
    dist = AK_FIXED_SQRT(dist_sqr);
    out->depth = AK_FIXED_SUB(r, dist);
    // n is Box->Circle. We want A->B (Circle->Box). So negate.
    inv_dist = -AK_FIXED_DIV(AK_FIXED_ONE, dist);
    ak_vec2_mul(&out->normal, &n, inv_dist);
  }
}

static void ResolveCollision(ak_world_t *world, ak_manifold_t *m) {
  ak_vec2_t rv;
  ak_fixed_t vel_along_normal;
  ak_fixed_t e;
  ak_fixed_t j;
  ak_fixed_t den;
  ak_vec2_t impulse;
  ak_fixed_t percent;
  ak_fixed_t slop;
  ak_fixed_t correction_mag;
  ak_fixed_t corr_num;
  ak_vec2_t correction;
  ak_vec2_t tmp_v;

  if (!m->has_collision)
    return;

  ak_vec2_sub(&rv, &m->b->velocity, &m->a->velocity);
  vel_along_normal = ak_vec2_dot(&rv, &m->normal);

  if (vel_along_normal > 0)
    return;

  e = AK_FIXED_MIN(m->a->restitution, m->b->restitution);
  j = AK_FIXED_MUL(-(AK_FIXED_ONE + e), vel_along_normal);
  den = AK_FIXED_ADD(m->a->inv_mass, m->b->inv_mass);

  if (den == 0)
    return;

  j = AK_FIXED_DIV(j, den);

  ak_vec2_mul(&impulse, &m->normal, j);

  if (!m->a->is_static) {
    ak_vec2_mul(&tmp_v, &impulse, m->a->inv_mass);
    ak_vec2_sub(&m->a->velocity, &m->a->velocity, &tmp_v);
  }
  if (!m->b->is_static) {
    ak_vec2_mul(&tmp_v, &impulse, m->b->inv_mass);
    ak_vec2_add(&m->b->velocity, &m->b->velocity, &tmp_v);
  }

  percent = AK_INT_TO_FIXED(2) / 10; // 0.2
  slop = world->slop;

  correction_mag = AK_FIXED_MAX(AK_FIXED_SUB(m->depth, slop), 0);
  corr_num = AK_FIXED_MUL(correction_mag, percent);
  correction_mag = AK_FIXED_DIV(corr_num, den);
  ak_vec2_mul(&correction, &m->normal, correction_mag);

  if (!m->a->is_static) {
    ak_vec2_mul(&tmp_v, &correction, m->a->inv_mass);
    ak_vec2_sub(&m->a->position, &m->a->position, &tmp_v);
  }
  if (!m->b->is_static) {
    ak_vec2_mul(&tmp_v, &correction, m->b->inv_mass);
    ak_vec2_add(&m->b->position, &m->b->position, &tmp_v);
  }
}

void ak_world_step(ak_world_t *world, ak_fixed_t dt) {
  int i, j;
  ak_body_t *b;
  ak_vec2_t acceleration;
  ak_manifold_t m;
  ak_body_t *a;
  ak_body_t *b_ptr;
  ak_vec2_t tmp_v;

  for (i = 0; i < world->body_count; i++) {
    b = &world->bodies[i];
    if (b->is_static)
      continue;

    // Apply gravity
    ak_vec2_mul(&tmp_v, &world->gravity,
                AK_FIXED_DIV(AK_FIXED_ONE, b->inv_mass));
    ak_vec2_add(&b->force, &b->force, &tmp_v);

    // Integrate Velocity
    ak_vec2_mul(&acceleration, &b->force, b->inv_mass);
    ak_vec2_mul(&tmp_v, &acceleration, dt);
    ak_vec2_add(&b->velocity, &b->velocity, &tmp_v);

    // Integrate Position
    ak_vec2_mul(&tmp_v, &b->velocity, dt);
    ak_vec2_add(&b->position, &b->position, &tmp_v);

    // Reset force
    ak_vec2_set(&b->force, 0, 0);
  }

  // Collisions
  for (i = 0; i < world->body_count; i++) {
    for (j = i + 1; j < world->body_count; j++) {
      memset(&m, 0, sizeof(m));
      a = &world->bodies[i];
      b_ptr = &world->bodies[j];

      if (a->is_static && b_ptr->is_static)
        continue;

      if (a->shape.type == AK_SHAPE_CIRCLE &&
          b_ptr->shape.type == AK_SHAPE_CIRCLE) {
        SolveCircleCircle(&m, a, b_ptr);
      } else if (a->shape.type == AK_SHAPE_AABB &&
                 b_ptr->shape.type == AK_SHAPE_AABB) {
        SolveAABBAABB(&m, a, b_ptr);
      } else if (a->shape.type == AK_SHAPE_CIRCLE &&
                 b_ptr->shape.type == AK_SHAPE_AABB) {
        SolveCircleAABB(&m, a, b_ptr);
      } else if (a->shape.type == AK_SHAPE_AABB &&
                 b_ptr->shape.type == AK_SHAPE_CIRCLE) {
        SolveCircleAABB(&m, b_ptr, a);
        ak_vec2_mul(&m.normal, &m.normal, -AK_FIXED_ONE);
        m.a = a;
        m.b = b_ptr;
      }

      if (m.has_collision) {
        ResolveCollision(world, &m);
      }
    }
  }

  // Tethers
  ResolveTethers(world);
}
