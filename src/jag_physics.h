#ifndef JAG_PHYSICS_H
#define JAG_PHYSICS_H

#include "jag_fixed.h"

#define JP_MAX_BODIES 64
#define JP_MAX_TETHERS 16

typedef struct {
  jag_fixed_t x, y;
} jp_vec2_t;

typedef enum { JP_SHAPE_CIRCLE, JP_SHAPE_AABB } jp_shape_type_t;

typedef struct {
  jp_shape_type_t type;
  union {
    struct {
      jag_fixed_t radius;
    } circle;
    struct {
      jag_fixed_t width, height;
    } aabb; // Half-width, Half-height
  } bounds;
} jp_shape_t;

typedef struct {
  int id;
  jp_vec2_t position;
  jp_vec2_t velocity;
  jp_vec2_t force;
  jag_fixed_t mass;
  jag_fixed_t inv_mass;    // 0 for static
  jag_fixed_t restitution; // Bounciness
  jp_shape_t shape;
  int is_static;
  int32_t padding[2]; // Pad to 64 bytes for 16-byte alignment (DMA friendly)
} jp_body_t;

typedef struct {
  jp_body_t *a;
  jp_body_t *b;
  jag_fixed_t max_length_sqr;
} jp_tether_t;

typedef struct {
  int body_a_id;
  int body_b_id;
  jp_vec2_t normal;
} jp_contact_t;

typedef struct {
  jp_vec2_t gravity;
  jp_body_t bodies[JP_MAX_BODIES];
  int body_count;
  jp_tether_t tethers[JP_MAX_TETHERS];
  int tether_count;
} jp_world_t;

// Vector Math
jp_vec2_t jp_vec2_add(jp_vec2_t a, jp_vec2_t b);
jp_vec2_t jp_vec2_sub(jp_vec2_t a, jp_vec2_t b);
jp_vec2_t jp_vec2_mul(jp_vec2_t v, jag_fixed_t s);
jag_fixed_t jp_vec2_dot(jp_vec2_t a, jp_vec2_t b);
jag_fixed_t jp_vec2_len_sqr(jp_vec2_t v);
jag_fixed_t jp_vec2_len(jp_vec2_t v);

// Physics API
void jp_world_init(jp_world_t *world, jp_vec2_t gravity);
jp_body_t *jp_world_add_body(jp_world_t *world, jp_shape_t shape, jag_fixed_t x,
                             jag_fixed_t y, jag_fixed_t mass);
void jp_world_add_tether(jp_world_t *world, jp_body_t *a, jp_body_t *b,
                         jag_fixed_t max_length);
void jp_world_step(jp_world_t *world, jag_fixed_t dt);
#endif // JAG_PHYSICS_H
