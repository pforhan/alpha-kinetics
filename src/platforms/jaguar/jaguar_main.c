#include "ak_demo_setup.h"
#include "ak_physics.h"
#include "demo_bitmap.h"
#include "jag_gpu.h"
#include "jag_platform.h"
#include <stddef.h>
#include <stdint.h>

demo_bitmap_t main_screen;

void InitVideo() {
  main_screen.width = SCREEN_WIDTH;
  main_screen.height = SCREEN_HEIGHT;

#ifdef JAGUAR
  // TODO: jaguar-sdk 2D video initialization goes here.
  // We no longer use rmvlib, so a new frame buffer needs to be set up.
  main_screen.pixels = NULL; // Stub
#endif
}

void RenderWorld(ak_world_t *world) {
  demo_bitmap_clear(&main_screen, COL_BLACK);

  for (int i = 0; i < world->body_count; i++) {
    ak_body_t *b = &world->bodies[i];
    int x = AK_FIXED_TO_INT(b->position.x);
    int y = AK_FIXED_TO_INT(b->position.y);

    if (b->shape.type == AK_SHAPE_CIRCLE) {
      int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius);
      demo_bitmap_draw_circle(&main_screen, x, y, r,
                              b->is_static ? COL_BLUE : COL_RED);
    } else if (b->shape.type == AK_SHAPE_AABB) {
      int w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width);
      int h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height);
      demo_bitmap_draw_rect(&main_screen, x - w, y - h, w * 2, h * 2,
                            b->is_static ? COL_GREEN : COL_WHITE);
    }
  }

  for (int i = 0; i < world->tether_count; i++) {
    ak_tether_t *t = &world->tethers[i];
    int x1 = AK_FIXED_TO_INT(t->a->position.x);
    int y1 = AK_FIXED_TO_INT(t->a->position.y);
    int x2 = AK_FIXED_TO_INT(t->b->position.x);
    int y2 = AK_FIXED_TO_INT(t->b->position.y);
    demo_bitmap_draw_line(&main_screen, x1, y1, x2, y2, COL_WHITE);
  }
}

typedef struct {
  ak_world_t *world;
  ak_fixed_t dt;
} PhysicsArgs;

void PhysicsWrapper(void *data) {
  PhysicsArgs *args = (PhysicsArgs *)data;
  ak_world_step(args->world, args->dt);
}

int main() {
#ifdef JAGUAR
  InitVideo();
  jag_gpu_init();

  ak_world_t world;
  ak_world_init(&world, AK_INT_TO_FIXED(320), AK_INT_TO_FIXED(240),
                (ak_vec2_t){0, 0});
  ak_demo_create_standard_scene(&world);

  ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60;
  PhysicsArgs args = {&world, dt};

  while (1) {
    jag_gpu_run(PhysicsWrapper, &args, sizeof(PhysicsArgs));
    jag_gpu_wait();
    RenderWorld(&world);
  }
#endif
  return 0;
}
