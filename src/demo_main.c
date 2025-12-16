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

  // Draw Tethers
  for (int i = 0; i < world->tether_count; i++) {
    jp_tether_t *t = &world->tethers[i];
    int x1 = FIXED_TO_INT(t->a->position.x);
    int y1 = FIXED_TO_INT(t->a->position.y);
    int x2 = FIXED_TO_INT(t->b->position.x);
    int y2 = FIXED_TO_INT(t->b->position.y);
    demo_bitmap_draw_line(&screen, x1, y1, x2, y2, COL_WHITE);
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
}

#include "jag_gpu.h"

#include <stddef.h> // for NULL

// Wrapper for GPU execution
typedef struct {
  jp_world_t *world;
  jag_fixed_t dt;
} PhysicsArgs;

void PhysicsWrapper(void *data) {
  PhysicsArgs *args = (PhysicsArgs *)data;
  jp_world_step(args->world, args->dt);
}

int main() {
  InitVideo();
  jag_gpu_init();

  jp_world_t world;
  jp_world_init(
      &world,
      (jp_vec2_t){0, INT_TO_FIXED(50)}); // Gravity down (lighter for pendulum)

  // Scenario: Complex Demo

  // 1. Ground (Static AABB)
  // Positioned at bottom center (160, 230), Width 320 (Half 160), Height 20
  // (Half 10)
  jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_AABB,
                   .bounds.aabb = {INT_TO_FIXED(160), INT_TO_FIXED(10)}},
      INT_TO_FIXED(160), INT_TO_FIXED(230), 0);

  // 2. Anchored Pendulum (Center)
  jp_body_t *anchor = jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_CIRCLE, .bounds.circle = {INT_TO_FIXED(2)}},
      INT_TO_FIXED(160), INT_TO_FIXED(40), 0); // Static

  jp_body_t *bob = jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_CIRCLE,
                   .bounds.circle = {INT_TO_FIXED(10)}},
      INT_TO_FIXED(220), INT_TO_FIXED(40), INT_TO_FIXED(5)); // Mass 5

  jp_world_add_tether(&world, anchor, bob, INT_TO_FIXED(60));

  // 3. Free Falling Box (Left)
  jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_AABB,
                   .bounds.aabb = {INT_TO_FIXED(10), INT_TO_FIXED(10)}},
      INT_TO_FIXED(60), INT_TO_FIXED(50), INT_TO_FIXED(2));
  jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_AABB,
                   .bounds.aabb = {INT_TO_FIXED(10), INT_TO_FIXED(10)}},
      INT_TO_FIXED(45), INT_TO_FIXED(25), INT_TO_FIXED(2));

  // 4. Free Falling Circle (Right)
  jp_world_add_body(&world,
                    (jp_shape_t){.type = JP_SHAPE_CIRCLE,
                                 .bounds.circle = {INT_TO_FIXED(12)}},
                    INT_TO_FIXED(260), INT_TO_FIXED(30), INT_TO_FIXED(2));

  // 5. Tethered trio (Bolas) - Falling Mid-Air
  jp_body_t *b1 = jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_CIRCLE, .bounds.circle = {INT_TO_FIXED(8)}},
      INT_TO_FIXED(100), INT_TO_FIXED(80), INT_TO_FIXED(3));

  jp_body_t *b2 = jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_CIRCLE, .bounds.circle = {INT_TO_FIXED(8)}},
      INT_TO_FIXED(130), INT_TO_FIXED(80), INT_TO_FIXED(3));

  // 3rd body for dramatic effect (Tri-Bolas / Chain)
  jp_body_t *b3 = jp_world_add_body(
      &world,
      (jp_shape_t){.type = JP_SHAPE_CIRCLE, .bounds.circle = {INT_TO_FIXED(6)}},
      INT_TO_FIXED(160), INT_TO_FIXED(60),
      INT_TO_FIXED(2)); // Smaller, lighter, off to side

  // Give them some spin! b1 moves left, b2 moves right
  b2->velocity.x = INT_TO_FIXED(20);

  jp_world_add_tether(&world, b1, b2, INT_TO_FIXED(40));
  jp_world_add_tether(&world, b2, b3, INT_TO_FIXED(40)); // Chain: b1-b2-b3

  jag_fixed_t dt = INT_TO_FIXED(1) / 60; // 1/60th second

  PhysicsArgs args = {&world, dt};

  // Run infinite loop
  while (1) {
    // Offload physics
    jag_gpu_run(PhysicsWrapper, &args, sizeof(PhysicsArgs));
    jag_gpu_wait();

#ifdef JAGUAR
    RenderWorld(&world);
    // VSYNC would go here or just wait
#else
    // PC Simulation: Render (ASCII)
    // Clear
    printf("\033[H\033[J");

    // Actually PrintASCII does the drawing. Let's rely on that.
    PrintASCII(&world);

    printf("Bodies: %d, Tethers: %d\n", world.body_count, world.tether_count);

    // 60 FPS delay
    usleep(16666);
#endif
  }

  return 0;
}
