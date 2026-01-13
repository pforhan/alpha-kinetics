#ifndef AK_PHYSICS_H
#define AK_PHYSICS_H

#include "ak_fixed.h"

#ifndef AK_MAX_BODIES
#define AK_MAX_BODIES 64
#endif

#ifndef AK_MAX_TETHERS
#define AK_MAX_TETHERS 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ak_fixed_t x, y;
} ak_vec2_t;

typedef enum { AK_SHAPE_CIRCLE, AK_SHAPE_AABB } ak_shape_type_t;

typedef struct {
  ak_shape_type_t type;
  union {
    struct {
      ak_fixed_t radius;
    } circle;
    struct {
      ak_fixed_t width, height;
    } aabb; // Half-width, Half-height
  } bounds;
} ak_shape_t;

typedef struct {
  int id;
  ak_vec2_t position;
  ak_vec2_t velocity;
  ak_vec2_t force;
  ak_fixed_t mass;
  ak_fixed_t inv_mass;    // 0 for static
  ak_fixed_t restitution; // Bounciness
  ak_shape_t shape;
  int is_static;
#ifdef JAGUAR
  int32_t padding[2]; // Pad to 64 bytes for 16-byte alignment (DMA friendly)
#endif
} ak_body_t;

typedef struct {
  ak_body_t *a;
  ak_body_t *b;
  ak_fixed_t max_length_sqr;
} ak_tether_t;

typedef struct {
  int body_a_id;
  int body_b_id;
  ak_vec2_t normal;
} ak_contact_t;

typedef struct {
  ak_vec2_t gravity;
  ak_body_t bodies[AK_MAX_BODIES];
  int body_count;
  ak_tether_t tethers[AK_MAX_TETHERS];
  int tether_count;
} ak_world_t;

// Vector Math
ak_vec2_t ak_vec2_add(ak_vec2_t a, ak_vec2_t b);
ak_vec2_t ak_vec2_sub(ak_vec2_t a, ak_vec2_t b);
ak_vec2_t ak_vec2_mul(ak_vec2_t v, ak_fixed_t s);
ak_fixed_t ak_vec2_dot(ak_vec2_t a, ak_vec2_t b);
ak_fixed_t ak_vec2_len_sqr(ak_vec2_t v);
ak_fixed_t ak_vec2_len(ak_vec2_t v);

// Physics API
void ak_world_init(ak_world_t *world, ak_vec2_t gravity);
ak_body_t *ak_world_add_body(ak_world_t *world, ak_shape_t shape, ak_fixed_t x,
                             ak_fixed_t y, ak_fixed_t mass);
void ak_world_add_tether(ak_world_t *world, ak_body_t *a, ak_body_t *b,
                         ak_fixed_t max_length);
void ak_world_step(ak_world_t *world, ak_fixed_t dt);

#ifdef __cplusplus
}
#endif
#endif // AK_PHYSICS_H
