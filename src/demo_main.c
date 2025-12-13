#include "demo_bitmap.h"
#include "jag_physics.h"
#include "jag_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Define JAGUAR to compile for the console
// #define JAGUAR

#ifdef JAGUAR
// Jaguar specific setup
uint16_t *video_buffer = (uint16_t *)JAG_DRAM_BASE; // Simplified
#else
// PC Simulation setup
uint16_t video_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
#endif

demo_bitmap_t screen;

void InitVideo() {
  screen.pixels = video_buffer;
  screen.width = SCREEN_WIDTH;
  screen.height = SCREEN_HEIGHT;

#ifdef JAGUAR
// Setup VI (Video Interface) registers here
// This is pseudo-code for what you'd actually do
// VI_HWIDTH = SCREEN_WIDTH;
// VI_VHEIGHT = SCREEN_HEIGHT;
// VI_CONTROL = ...;
#endif
}

void RenderWorld(jp_world_t *world) {
  demo_bitmap_clear(&screen, COL_BLACK);

  // Draw Ground (Static bodies)
  // Draw Dynamic bodies

  for (int i = 0; i < world->body_count; i++) {
    jp_body_t *b = &world->bodies[i];
    int x = FIXED_TO_INT(b->position.x);
    int y = FIXED_TO_INT(b->position.y);

    if (b->shape.type == JP_SHAPE_CIRCLE) {
      int r = FIXED_TO_INT(b->shape.bounds.circle.radius);
      demo_bitmap_draw_circle(&screen, x, y, r,
                              b->is_static ? COL_BLUE : COL_RED);
    } else if (b->shape.type == JP_SHAPE_AABB) {
      int w = FIXED_TO_INT(b->shape.bounds.aabb.width);
      int h = FIXED_TO_INT(b->shape.bounds.aabb.height);
      // AABB is half-width/height, so draw full box
      demo_bitmap_draw_rect(&screen, x - w, y - h, w * 2, h * 2,
                            b->is_static ? COL_GREEN : COL_WHITE);
    }
  }
}

// Simple ASCII renderer for PC terminal
void PrintASCII(jp_world_t *world) {
  char canvas[20][41]; // +1 for null terminator or just padding

  // Clear canvas
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 40; x++) {
      canvas[y][x] = '.';
    }
    canvas[y][40] = '\0';
  }

  for (int i = 0; i < world->body_count; i++) {
    jp_body_t *b = &world->bodies[i];

    // Convert world coords to canvas coords (World: 320x240, Canvas: 40x20)
    // Scale X: / 8, Scale Y: / 12
    int cx = FIXED_TO_INT(b->position.x) / 8;
    int cy = FIXED_TO_INT(b->position.y) / 12;

    if (b->shape.type == JP_SHAPE_AABB) {
      int half_w = FIXED_TO_INT(b->shape.bounds.aabb.width) / 8;
      int half_h = FIXED_TO_INT(b->shape.bounds.aabb.height) / 12;
      // Ensure at least 1x1
      if (half_w == 0)
        half_w = 0; // AABB is half-width, so total width 0+1+0 = 1? No, from
                    // x-w to x+w.
      // Let's iterate bounds
      int x1 = cx - half_w;
      int x2 = cx + half_w;
      int y1 = cy - half_h;
      int y2 = cy + half_h;

      char fill = b->is_static ? '#' : '[';

      for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
          if (x >= 0 && x < 40 && y >= 0 && y < 20) {
            canvas[y][x] = fill;
          }
        }
      }
    } else if (b->shape.type == JP_SHAPE_CIRCLE) {
      int r = FIXED_TO_INT(b->shape.bounds.circle.radius) / 8;
      if (r < 1)
        r = 0; // Point

      // Simple circle raster - check distance for pixels in box
      for (int y = cy - r; y <= cy + r; y++) {
        for (int x = cx - r; x <= cx + r; x++) {
          if (x >= 0 && x < 40 && y >= 0 && y < 20) {
            // Check dist (very roughly for ascii, or just box it for small
            // circles) Let's just draw the center if small, or a small 3x3
            canvas[y][x] = 'O';
          }
        }
      }
    }
  }

  printf("\033[H\033[J"); // Clear screen
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 40; x++) {
      putchar(canvas[y][x]);
    }
    putchar('\n');
  }
  printf("Bodies: %d\n", world->body_count);
}

#include "jag_gpu.h"

// Wrapper for GPU execution
typedef struct {
  jp_body_t *bodies;
  int body_count;
  jp_vec2_t gravity;
  jag_fixed_t dt;
  jp_contact_t *contacts;
  int *contact_count;
  int max_contacts;
} PhysicsArgs;

void PhysicsWrapper(void *data) {
  PhysicsArgs *args = (PhysicsArgs *)data;
  jp_world_step(args->bodies, args->body_count, args->gravity, args->dt,
                args->contacts, args->contact_count, args->max_contacts);
}

void HandleDemoCollisions(jp_world_t *world, jp_contact_t *contacts,
                          int contact_count, int circle_id, int ground_id) {
  for (int k = 0; k < contact_count; k++) {
    jp_contact_t *c = &contacts[k];
    // Check if circle hit the ground
    if ((c->body_a_id == circle_id && c->body_b_id == ground_id) ||
        (c->body_b_id == circle_id && c->body_a_id == ground_id)) {

      printf("Collision Event: Circle hit Ground! Deleting Circle...\n");

      // Find and Delete Circle
      for (int b = 0; b < world->body_count; b++) {
        if (world->bodies[b].id == circle_id) {
          // Swap with last and pop
          world->bodies[b] = world->bodies[world->body_count - 1];
          world->body_count--;
          break;
        }
      }
    } else if (c->body_a_id == circle_id || c->body_b_id == circle_id) {
      printf("Collision Event: Circle hit Body ID %d or %d!\n", c->body_a_id,
             c->body_b_id);
    }
  }
}

int main() {
  InitVideo();
  jag_gpu_init();

  jp_world_t world;
  jp_world_init(&world, (jp_vec2_t){0, INT_TO_FIXED(100)}); // Gravity down

  // Ground
  jp_body_t *ground = jp_world_add_body(
      &world,
      (jp_shape_t){JP_SHAPE_AABB,
                   .bounds.aabb = {INT_TO_FIXED(100), INT_TO_FIXED(10)}},
      INT_TO_FIXED(160), INT_TO_FIXED(220), 0);
  int ground_id = ground->id;

  // Falling Circle
  jp_body_t *circle = jp_world_add_body(
      &world,
      (jp_shape_t){JP_SHAPE_CIRCLE, .bounds.circle = {INT_TO_FIXED(10)}},
      INT_TO_FIXED(169), INT_TO_FIXED(0), INT_TO_FIXED(1));
  // circle->velocity.y = INT_TO_FIXED(50); // Initial push to catch the box
  int circle_id = circle->id;

  // Falling Box
  jp_world_add_body(
      &world,
      (jp_shape_t){JP_SHAPE_AABB,
                   .bounds.aabb = {INT_TO_FIXED(10), INT_TO_FIXED(10)}},
      INT_TO_FIXED(180), INT_TO_FIXED(30), INT_TO_FIXED(100));

  jag_fixed_t dt = INT_TO_FIXED(1) / 60; // 1/60th second

  jp_contact_t contacts[16];
  int contact_count = 0;
  PhysicsArgs args = {world.bodies,
                      world.body_count,
                      world.gravity,
                      dt,
                      contacts,
                      &contact_count,
                      16};

#ifdef JAGUAR
  while (1) {
    // Offload physics to GPU (simulated on 68k for now)
    args.body_count = world.body_count; // Update count if changed
    jag_gpu_run(PhysicsWrapper, &args, sizeof(PhysicsArgs));

    // While GPU runs, we could do other things (like audio setup)
    jag_gpu_wait();

    // Process Collisions
    HandleDemoCollisions(&world, contacts, contact_count, circle_id, ground_id);

    RenderWorld(&world);
    // Wait for VSync
  }
#else
  // Run infinite loop on PC for demo
  while (1) {
    // Offload physics
    args.body_count = world.body_count;
    jag_gpu_run(PhysicsWrapper, &args, sizeof(PhysicsArgs));
    jag_gpu_wait();

    // Process Collision Events
    HandleDemoCollisions(&world, contacts, contact_count, circle_id, ground_id);

    // RenderWorld(&world); // We don't see this on PC but it tests the code
    PrintASCII(&world);

    usleep(16666); // ~60 FPS (16.6ms)
  }
#endif

  return 0;
}
