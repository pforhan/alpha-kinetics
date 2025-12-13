#include "demo_bitmap.h"

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
