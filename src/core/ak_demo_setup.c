#include "ak_demo_setup.h"

void ak_demo_create_standard_scene(ak_world_t *world, ak_demo_config_t config) {
  ak_fixed_t scale_x = AK_FIXED_DIV(config.width, AK_INT_TO_FIXED(320));
  ak_fixed_t scale_y = AK_FIXED_DIV(config.height, AK_INT_TO_FIXED(240));

  ak_world_init(world, (ak_vec2_t){0, AK_FIXED_MUL(AK_INT_TO_FIXED(50),
                                                   scale_y)}); // Gravity scaled

  // 1. Ground (Static AABB)
  ak_world_add_body(
      world,
      (ak_shape_t){.type = AK_SHAPE_AABB,
                   .bounds.aabb = {AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale_x),
                                   AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(230), scale_y), 0);

  // 2. Anchored Pendulum (Center)
  ak_body_t *anchor = ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(2), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale_y), 0); // Static

  ak_body_t *bob = ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(220), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale_y), AK_INT_TO_FIXED(5)); // Mass 5

  ak_world_add_tether(world, anchor, bob,
                      AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale_x));

  // 3. Free Falling Box (Left)
  ak_world_add_body(
      world,
      (ak_shape_t){.type = AK_SHAPE_AABB,
                   .bounds.aabb = {AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale_y),
                                   AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(50), scale_y), AK_INT_TO_FIXED(2));

  // 4. Free Falling Circle (Right)
  ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(12), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(260), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(30), scale_y), AK_INT_TO_FIXED(2));

  // 5. Tethered trio (Bolas)
  ak_body_t *b1 = ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(8), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(100), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(80), scale_y), AK_INT_TO_FIXED(3));

  ak_body_t *b2 = ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(8), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(130), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(80), scale_y), AK_INT_TO_FIXED(3));

  ak_body_t *b3 = ak_world_add_body(
      world,
      (ak_shape_t){
          .type = AK_SHAPE_CIRCLE,
          .bounds.circle = {AK_FIXED_MUL(AK_INT_TO_FIXED(6), scale_y)}},
      AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale_x),
      AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale_y), AK_INT_TO_FIXED(2));

  b2->velocity.x = AK_FIXED_MUL(AK_INT_TO_FIXED(20), scale_x);

  ak_world_add_tether(world, b1, b2,
                      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale_x));
  ak_world_add_tether(world, b2, b3,
                      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale_x));
}
