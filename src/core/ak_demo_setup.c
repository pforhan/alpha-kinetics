#include "ak_demo_setup.h"

void ak_demo_create_standard_scene(ak_world_t *world) {
  ak_fixed_t scale;
  ak_fixed_t scaled_ref_width;
  ak_fixed_t offset_x;
  ak_body_t *anchor;
  ak_body_t *bob;
  ak_body_t *b1;
  ak_body_t *b2;
  ak_body_t *b3;
  ak_vec2_t gravity;
  ak_shape_t shape;

  // Use uniform scaling based on height (reference 240px)
  scale = AK_FIXED_DIV(world->height, AK_INT_TO_FIXED(240));

  // Determine horizontal offset to center the 320px-reference scene
  scaled_ref_width = AK_FIXED_MUL(AK_INT_TO_FIXED(320), scale);
  offset_x = (world->width - scaled_ref_width) / 2;

  ak_vec2_set(&gravity, 0, AK_FIXED_MUL(AK_INT_TO_FIXED(50), scale));
  ak_world_init(world, world->width, world->height, &gravity);

  // 1. Ground (Static AABB)
  ak_shape_aabb_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale),
                    AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale));
  ak_world_add_body(world, &shape,
                    offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale),
                    AK_FIXED_MUL(AK_INT_TO_FIXED(230), scale), 0);

  // 2. Anchored Pendulum (Center)
  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(2), scale));
  anchor = ak_world_add_body(
      world, &shape, offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale),
      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale), 0); // Static

  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale));
  bob = ak_world_add_body(
      world, &shape, offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(220), scale),
      AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale), AK_INT_TO_FIXED(5)); // Mass 5

  ak_world_add_tether(world, anchor, bob,
                      AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale));

  // 3. Free Falling Box (Left)
  ak_shape_aabb_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale),
                    AK_FIXED_MUL(AK_INT_TO_FIXED(10), scale));
  ak_world_add_body(world, &shape,
                    offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale),
                    AK_FIXED_MUL(AK_INT_TO_FIXED(50), scale), AK_INT_TO_FIXED(2));

  // 4. Free Falling Circle (Right)
  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(12), scale));
  ak_world_add_body(world, &shape,
                    offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(260), scale),
                    AK_FIXED_MUL(AK_INT_TO_FIXED(30), scale), AK_INT_TO_FIXED(2));

  // 5. Tethered trio (Bolas)
  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(8), scale));
  b1 = ak_world_add_body(
      world, &shape, offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(100), scale),
      AK_FIXED_MUL(AK_INT_TO_FIXED(80), scale), AK_INT_TO_FIXED(3));

  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(8), scale));
  b2 = ak_world_add_body(
      world, &shape, offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(130), scale),
      AK_FIXED_MUL(AK_INT_TO_FIXED(80), scale), AK_INT_TO_FIXED(3));

  ak_shape_circle_set(&shape, AK_FIXED_MUL(AK_INT_TO_FIXED(6), scale));
  b3 = ak_world_add_body(
      world, &shape, offset_x + AK_FIXED_MUL(AK_INT_TO_FIXED(160), scale),
      AK_FIXED_MUL(AK_INT_TO_FIXED(60), scale), AK_INT_TO_FIXED(2));

  b2->velocity.x = AK_FIXED_MUL(AK_INT_TO_FIXED(20), scale);

  ak_world_add_tether(world, b1, b2, AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale));
  ak_world_add_tether(world, b2, b3, AK_FIXED_MUL(AK_INT_TO_FIXED(40), scale));
}
