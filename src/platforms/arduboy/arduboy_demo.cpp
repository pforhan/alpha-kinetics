/*
 * Alpha Kinetics - Arduboy FX Demo
 * Note: AK_MAX_BODIES must be reduced (e.g., 16) in the build flags
 * to fit in the 2.5KB RAM of the ATmega32u4.
 */

#include "ak_demo_setup.h"
#include "ak_physics.h"
#include <Arduboy2.h>

Arduboy2 arduboy;
ak_world_t world;

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(60);

  // Initialize AK World with standard scene
  ak_demo_create_standard_scene(&world);
}

void loop() {
  if (!arduboy.nextFrame())
    return;

  // Step Physics
  ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60;
  ak_world_step(&world, dt);

  // Render
  arduboy.clear();

  for (int i = 0; i < world.body_count; i++) {
    ak_body_t *b = &world.bodies[i];
    int x = AK_FIXED_TO_INT(b->position.x);
    int y = AK_FIXED_TO_INT(b->position.y);

    if (b->shape.type == AK_SHAPE_CIRCLE) {
      int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius);
      arduboy.drawCircle(x, y, r, WHITE);
    } else if (b->shape.type == AK_SHAPE_AABB) {
      int w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width);
      int h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height);
      arduboy.drawRect(x - w, y - h, w * 2, h * 2, WHITE);
    }
  }

  // Draw Tethers
  for (int i = 0; i < world.tether_count; i++) {
    ak_tether_t *t = &world.tethers[i];
    arduboy.drawLine(AK_FIXED_TO_INT(t->a->position.x),
                     AK_FIXED_TO_INT(t->a->position.y),
                     AK_FIXED_TO_INT(t->b->position.x),
                     AK_FIXED_TO_INT(t->b->position.y), WHITE);
  }

  arduboy.display();
}
