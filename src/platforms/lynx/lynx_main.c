#include "ak_demo_setup.h"
#include "ak_physics.h"
#include <6502.h>
#include <joystick.h>
#include <lynx.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgi.h>

static ak_world_t world;
static unsigned long frames = 0;

static void fast_draw_circle(int x, int y, int r) {
  tgi_line(x - r, y, x + r, y);
  tgi_line(x, y - r, x, y + r);
}

static void fast_draw_aabb(int x, int y, int hw, int hh) {
  tgi_line(x - hw, y - hh, x + hw, y - hh);
  tgi_line(x + hw, y - hh, x + hw, y + hh);
  tgi_line(x + hw, y + hh, x - hw, y + hh);
  tgi_line(x - hw, y + hh, x - hw, y - hh);
}

static void draw_unsigned(int x, int y, unsigned long val) {
  char buf[12];
  int i = 10;
  buf[11] = '\0';
  if (val == 0) {
    buf[10] = '0';
    i = 10;
  } else {
    while (val > 0 && i > 0) {
      buf[i--] = (val % 10) + '0';
      val /= 10;
    }
    i++;
  }
  tgi_outtextxy(x, y, &buf[i]);
}

void DrawWorld(ak_world_t *world) {
  int i;

  while (tgi_busy())
    ;

  tgi_clear();

  for (i = 0; i < world->body_count; i++) {
    ak_body_t *b = &world->bodies[i];
    int x = AK_FIXED_TO_INT(b->position.x);
    int y = AK_FIXED_TO_INT(b->position.y);

    if (b->is_static) {
      tgi_setcolor(COLOR_GREY);
    } else {
      tgi_setcolor(COLOR_WHITE);
    }

    if (b->shape.type == AK_SHAPE_AABB) {
      fast_draw_aabb(x, y, AK_FIXED_TO_INT(b->shape.bounds.aabb.width),
                     AK_FIXED_TO_INT(b->shape.bounds.aabb.height));
    } else if (b->shape.type == AK_SHAPE_CIRCLE) {
      fast_draw_circle(x, y, AK_FIXED_TO_INT(b->shape.bounds.circle.radius));
    }
  }

  // Draw Tethers
  tgi_setcolor(COLOR_RED);
  for (i = 0; i < world->tether_count; i++) {
    ak_tether_t *t = &world->tethers[i];
    tgi_line(
        AK_FIXED_TO_INT(t->a->position.x), AK_FIXED_TO_INT(t->a->position.y),
        AK_FIXED_TO_INT(t->b->position.x), AK_FIXED_TO_INT(t->b->position.y));
  }

  tgi_setcolor(COLOR_WHITE);
  tgi_outtextxy(0, 0, "AK Lynx");
  draw_unsigned(0, 92, frames++);

  tgi_updatedisplay();
}

int main() {
  ak_fixed_t dt;
  ak_vec2_t gravity;
  unsigned char joy;

  tgi_install(tgi_static_stddrv);
  tgi_init();
  tgi_clear();

  joy_install(joy_static_stddrv);

  // Enable interrupts AFTER drivers are installed
  CLI();

  while (tgi_busy())
    ;

  ak_vec2_set(&gravity, 0, 0);
  ak_world_init(&world, AK_INT_TO_FIXED(160), AK_INT_TO_FIXED(102), &gravity);
  ak_demo_create_standard_scene(&world);

  dt = AK_INT_TO_FIXED(1) / 60;

  while (1) {
    // Wait for the previous frame to finish before we start the next
    // simulation/draw
    while (tgi_busy())
      ;

    joy = joy_read(JOY_1);
    if (JOY_BTN_1(joy)) {
      ak_demo_create_standard_scene(&world);
    }

    ak_world_step(&world, dt);
    DrawWorld(&world);
  }

  return 0;
}
