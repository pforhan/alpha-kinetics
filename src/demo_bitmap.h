#ifndef DEMO_BITMAP_H
#define DEMO_BITMAP_H

#include "jag_platform.h"
#include <stdint.h>

typedef struct {
  uint16_t *pixels;
  int width;
  int height;
} demo_bitmap_t;

void demo_bitmap_clear(demo_bitmap_t *bmp, uint16_t color);
void demo_bitmap_draw_pixel(demo_bitmap_t *bmp, int x, int y, uint16_t color);
void demo_bitmap_draw_rect(demo_bitmap_t *bmp, int x, int y, int w, int h,
                           uint16_t color);
void demo_bitmap_draw_circle(demo_bitmap_t *bmp, int cx, int cy, int r,
                             uint16_t color);
void demo_bitmap_draw_line(demo_bitmap_t *bmp, int x0, int y0, int x1, int y1,
                           uint16_t color);

#endif // DEMO_BITMAP_H
