#include "lynx_platform.h"
#include <lynx.h>
#include <stdlib.h>
#include <stdint.h>

constexpr uint16_t SCREEN_WIDTH  = 160;
constexpr uint16_t SCREEN_HEIGHT = 102;
constexpr uint16_t SCREEN_STRIDE = 80; 

uint8_t* const SCREEN_BUFFER = reinterpret_cast<uint8_t*>(0xC000);
volatile uint8_t* const DISP_ADDR_LOW  = reinterpret_cast<volatile uint8_t*>(0xFD94);
volatile uint8_t* const DISP_ADDR_HIGH = reinterpret_cast<volatile uint8_t*>(0xFD95);

void plot_pixel(uint16_t x, uint16_t y, uint8_t color_index) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    uint16_t byte_offset = (y * SCREEN_STRIDE) + (x >> 1);
    if ((x & 1) == 0) {
        SCREEN_BUFFER[byte_offset] = (SCREEN_BUFFER[byte_offset] & 0x0F) | (color_index << 4);
    } else {
        SCREEN_BUFFER[byte_offset] = (SCREEN_BUFFER[byte_offset] & 0xF0) | (color_index & 0x0F);
    }
}

void lynx_init() {
    // Set display address to our buffer immediately
    *DISP_ADDR_LOW  = static_cast<uint8_t>(0xC000 & 0xFF);
    *DISP_ADDR_HIGH = static_cast<uint8_t>((0xC000 >> 8) & 0xFF);
}

void lynx_poll_input() {
}

int lynx_button_pressed(int btn) {
    unsigned char joy = SUZY.joystick;
    if (btn == LYNX_BTN_A) {
        return (joy & JOY_BTN_A_MASK) != 0;
    }
    return 0;
}

void lynx_clear_screen() {
    uint8_t packed_byte = 0x00; // Color index 0
    for (uint16_t i = 0; i < (SCREEN_STRIDE * SCREEN_HEIGHT); ++i) {
        SCREEN_BUFFER[i] = packed_byte;
    }
}

void lynx_draw_circle(int x, int y, int r) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                plot_pixel(x + dx, y + dy, 0x0F);
            }
        }
    }
}

void lynx_draw_rect(int x, int y, int w, int h) {
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            plot_pixel(px, py, 0x0F);
        }
    }
}

void lynx_draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        plot_pixel(x1, y1, 0x0F);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void lynx_present_screen() {
    // Buffer is already at 0xC000, so no flip needed unless we double buffer
}
