#include "lynx_platform.h"
#include "ak_demo_setup.h"
#include "ak_physics.h"

ak_world_t world;

int main() {
    // Initialize Lynx hardware
    lynx_init();

    // Initialize AK World
    ak_world_init(&world, AK_INT_TO_FIXED(160), AK_INT_TO_FIXED(102), 
                  (ak_vec2_t){0, 0});
    ak_demo_create_standard_scene(&world);

    while (1) {
        // Poll input
        lynx_poll_input();

        if (lynx_button_pressed(LYNX_BTN_A)) {
            ak_demo_create_standard_scene(&world);
        }

        // Physics step (60Hz)
        ak_fixed_t dt = AK_FIXED_DIV(AK_INT_TO_FIXED(1), AK_INT_TO_FIXED(60));
        ak_world_step(&world, dt);

        // Render
        lynx_clear_screen();

        for (int i = 0; i < world.body_count; i++) {
            ak_body_t *b = &world.bodies[i];
            int x = AK_FIXED_TO_INT(b->position.x);
            int y = AK_FIXED_TO_INT(b->position.y);

            if (b->shape.type == AK_SHAPE_CIRCLE) {
                int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius);
                lynx_draw_circle(x, y, r);
            } else if (b->shape.type == AK_SHAPE_AABB) {
                int w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width);
                int h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height);
                lynx_draw_rect(x - w, y - h, w * 2, h * 2);
            }
        }

        for (int i = 0; i < world.tether_count; i++) {
            ak_tether_t *t = &world.tethers[i];
            lynx_draw_line(AK_FIXED_TO_INT(t->a->position.x), 
                           AK_FIXED_TO_INT(t->a->position.y),
                           AK_FIXED_TO_INT(t->b->position.x), 
                           AK_FIXED_TO_INT(t->b->position.y));
        }

        lynx_present_screen();
    }
}
