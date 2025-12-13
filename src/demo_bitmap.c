#include "demo_bitmap.h"
#include <stdlib.h>

void demo_bitmap_clear(demo_bitmap_t *bmp, uint16_t color) {
  int size = bmp->width * bmp->height;
  for (int i = 0; i < size; i++) {
    bmp->pixels[i] = color;
  }
}

void demo_bitmap_draw_pixel(demo_bitmap_t *bmp, int x, int y, uint16_t color) {
  if (x < 0 || x >= bmp->width || y < 0 || y >= bmp->height)
    return;
  bmp->pixels[y * bmp->width + x] = color;
}

void demo_bitmap_draw_rect(demo_bitmap_t *bmp, int x, int y, int w, int h,
                           uint16_t color) {
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      demo_bitmap_draw_pixel(bmp, i, j, color);
    }
  }
}

void demo_bitmap_draw_circle(demo_bitmap_t *bmp, int cx, int cy, int r,
                             uint16_t color) {
  int x = 0;
  int y = r;
  int d = 3 - 2 * r;

  while (y >= x) {
    demo_bitmap_draw_pixel(bmp, cx + x, cy + y, color);
    demo_bitmap_draw_pixel(bmp, cx - x, cy + y, color);
    demo_bitmap_draw_pixel(bmp, cx + x, cy - y, color);
    demo_bitmap_draw_pixel(bmp, cx - x, cy - y, color);
    demo_bitmap_draw_pixel(bmp, cx + y, cy + x, color);
    demo_bitmap_draw_pixel(bmp, cx - y, cy + x, color);
    demo_bitmap_draw_pixel(bmp, cx + y, cy - x, color);
    demo_bitmap_draw_pixel(bmp, cx - y, cy - x, color);

    x++;
    if (d > 0) {
      y--;
      d = d + 4 * (x - y) + 10;
    } else {
      d = d + 4 * x + 6;
    }
  }
}

void demo_bitmap_draw_line(demo_bitmap_t *bmp, int x0, int y0, int x1, int y1,
                           uint16_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    demo_bitmap_draw_pixel(bmp, x0, y0, color);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}
