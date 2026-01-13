#include "ak_demo_setup.h"
#include "ak_physics.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// Simple ASCII renderer for PC terminal
void PrintASCII(ak_world_t *world) {
  char canvas[20][41];

  // Clear canvas
  for (int y = 0; y < 20; y++) {
    for (int x = 0; x < 40; x++) {
      canvas[y][x] = '.';
    }
    canvas[y][40] = '\0';
  }

  for (int i = 0; i < world->body_count; i++) {
    ak_body_t *b = &world->bodies[i];

    // Convert world coords to canvas coords (World: 320x240, Canvas: 40x20)
    int cx = AK_FIXED_TO_INT(b->position.x) / 8;
    int cy = AK_FIXED_TO_INT(b->position.y) / 12;

    if (b->shape.type == AK_SHAPE_AABB) {
      int half_w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width) / 8;
      int half_h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height) / 12;

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
    } else if (b->shape.type == AK_SHAPE_CIRCLE) {
      int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius) / 8;
      if (r < 1)
        r = 0;

      for (int y = cy - r; y <= cy + r; y++) {
        for (int x = cx - r; x <= cx + r; x++) {
          if (x >= 0 && x < 40 && y >= 0 && y < 20) {
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

int main() {
  ak_world_t world;
  ak_world_init(
      &world, AK_INT_TO_FIXED(320), AK_INT_TO_FIXED(240),
      (ak_vec2_t){0, 0}); // Initialized with 0 gravity, demo setup will set it
  ak_demo_create_standard_scene(&world);

  ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60; // 1/60th second

  // Set non-blocking input
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  while (1) {
    int ch = getchar();
    if (ch == 'r' || ch == 'R') {
      ak_demo_create_standard_scene(&world);
    } else if (ch == 'q' || ch == 'Q') {
      break;
    }

    ak_world_step(&world, dt);
    PrintASCII(&world);
    printf("Alpha Kinetics PC Demo - Bodies: %d, Tethers: %d (R to reset, Q to "
           "quit)\n",
           world.body_count, world.tether_count);
    usleep(16666);
  }

  // Restore terminal
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  return 0;
}
