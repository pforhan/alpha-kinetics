#include "ak_demo_setup.h"
#include "ak_physics.h"
#include "pd_api.h"

static ak_world_t world;
static PlaydateAPI *pd = NULL;

static int update(void *userdata) {
  pd->graphics->clear(kColorWhite);

  ak_fixed_t dt = AK_INT_TO_FIXED(1) / 30; // 30 FPS
  ak_world_step(&world, dt);

  PDButtons pushed;
  pd->system->getButtonState(NULL, &pushed, NULL);
  if (pushed & kButtonA) {
    ak_demo_create_standard_scene(&world);
  }

  for (int i = 0; i < world.body_count; i++) {
    ak_body_t *b = &world.bodies[i];
    int x = AK_FIXED_TO_INT(b->position.x);
    int y = AK_FIXED_TO_INT(b->position.y);

    if (b->shape.type == AK_SHAPE_CIRCLE) {
      int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius);
      pd->graphics->drawEllipse(x - r, y - r, r * 2, r * 2, 1, 0, 360,
                                kColorBlack);
    } else if (b->shape.type == AK_SHAPE_AABB) {
      int w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width);
      int h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height);
      pd->graphics->drawRect(x - w, y - h, w * 2, h * 2, kColorBlack);
    }
  }

  // Draw Tethers
  for (int i = 0; i < world.tether_count; i++) {
    ak_tether_t *t = &world.tethers[i];
    if (t->a == NULL || t->b == NULL)
      continue;

    int x1 = AK_FIXED_TO_INT(t->a->position.x);
    int y1 = AK_FIXED_TO_INT(t->a->position.y);
    int x2 = AK_FIXED_TO_INT(t->b->position.x);
    int y2 = AK_FIXED_TO_INT(t->b->position.y);

    pd->graphics->drawLine(x1, y1, x2, y2, 1, kColorBlack);

    // TODO: Draw curve when tether is slack (current_dist < length)
  }

  return 1;
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  if (event == kEventInit) {
    pd = playdate;
    ak_world_init(&world, AK_INT_TO_FIXED(400), AK_INT_TO_FIXED(240),
                  (ak_vec2_t){0, 0});
    ak_demo_create_standard_scene(&world);
    pd->system->setUpdateCallback(update, NULL);
  }
  return 0;
}
