#ifndef LYNX_PLATFORM_H
#define LYNX_PLATFORM_H

void lynx_init();
void lynx_poll_input();
int lynx_button_pressed(int btn);
void lynx_clear_screen();
void lynx_draw_circle(int x, int y, int r);
void lynx_draw_rect(int x, int y, int w, int h);
void lynx_draw_line(int x1, int y1, int x2, int y2);
void lynx_present_screen();

#define LYNX_BTN_A 1

#endif
