#include <lynx.h>

// Mock implementation of Lynx hardware functions since the real SDK 
// header might be handled by the compiler.
void lynx_init() {
    // Hardware init
}

void lynx_poll_input() {
    // Poll buttons
}

int lynx_button_pressed(int btn) {
    return 0; 
}

void lynx_clear_screen() {
    // Clear frame buffer
}

void lynx_draw_circle(int x, int y, int r) {
    // Implementation for drawing circles on Lynx
}

void lynx_draw_rect(int x, int y, int w, int h) {
    // Implementation for drawing rectangles on Lynx
}

void lynx_draw_line(int x1, int y1, int x2, int y2) {
    // Implementation for drawing lines on Lynx
}

void lynx_present_screen() {
    // Swap buffers
}
