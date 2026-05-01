#include "ak_demo_setup.h"
#include "ak_physics.h"
#include "demo_bitmap.h"
#include "jag_gpu.h"
#include "jag_platform.h"
#include <jaguar.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// 64-bit Object List phrase (represented as two 32-bit halves for big-endian 68000)
typedef struct {
  uint32_t hi;
  uint32_t lo;
} OP_Phrase;

extern OP_Phrase opList[8];
extern uint16_t frameBuffer[];

demo_bitmap_t main_screen;

void InitVideo() {
  main_screen.width = 320;
  main_screen.height = 240;

#ifdef JAGUAR
  // Point the demo_bitmap structure to our framebuffer
  main_screen.pixels = frameBuffer;

  // Configure Video Registers
  int is_pal = (*CONFIG & VIDTYPE) == 0;
  int hmid = is_pal ? PAL_HMID : NTSC_HMID;
  int width = is_pal ? PAL_WIDTH : NTSC_WIDTH;
  int vmid = is_pal ? PAL_VMID : NTSC_VMID;
  int height = is_pal ? PAL_HEIGHT : NTSC_HEIGHT;

  int hdb = hmid - (width / 2) + 4;
  int hde = (width / 2) - 1 + 0x400;

  int screen_vdb = vmid - height;
  int screen_vde = vmid + height;

  *HDB1 = hdb;
  *HDB2 = hdb;
  *HDE = hde;
  *VDB = screen_vdb;
  *VDE = 0xFFFF;
  *BORD1 = 0;
  *BG = 0;

  // Construct Object List
  int opIndex = 0;
  uint32_t next_phrase;

  // 1. Branch if VC < VDB
  next_phrase = ((uint32_t)&opList[2]) >> 3;
  opList[opIndex].lo = (BRANCHOBJ) | (O_BRLT) | (screen_vdb << 3) | (next_phrase << 24);
  opList[opIndex].hi = (next_phrase >> 8);
  opIndex++;

  // 2. Branch if VC > VDE
  next_phrase = ((uint32_t)&opList[4]) >> 3;
  opList[opIndex].lo = (BRANCHOBJ) | (O_BRGT) | (screen_vde << 3) | (next_phrase << 24);
  opList[opIndex].hi = (next_phrase >> 8);
  opIndex++;

  // 3. Bitmap Object (2 phrases)
  next_phrase = ((uint32_t)&opList[4]) >> 3; // Link to Stop object
  uint32_t data_addr = (uint32_t)frameBuffer;
  int bmp_height = 240;
  int bmp_width = 320;
  
  // Vertically center
  int ypos = screen_vdb + (height * 2 - bmp_height) / 2;
  ypos &= ~1; // Must be even
  
  // Phrase 1
  opList[opIndex].lo = (BITOBJ) | (ypos << 3) | (bmp_height << 14) | (next_phrase << 24);
  opList[opIndex].hi = (next_phrase >> 8) | (data_addr << 8);
  opIndex++;

  // Phrase 2
  int xpos = ((width / 4) - bmp_width) / 2; // Horizontally center
  uint32_t iwidth = bmp_width / 4; // 16-bit depth = 4 pixels/phrase
  uint32_t dwidth = bmp_width / 4;

  opList[opIndex].lo = (O_DEPTH16) | (O_NOGAP) | (xpos & 0xFFF) | (dwidth << 18) | (iwidth << 28);
  opList[opIndex].hi = (iwidth >> 4);
  opIndex++;

  // 4. Stop Object
  opList[opIndex].lo = STOPOBJ | O_STOPINTS;
  opList[opIndex].hi = 0;
  opIndex++;

  // Set Object List Pointer (swapped for 68000)
  uint32_t olp_addr = (uint32_t)opList;
  olp_addr = ((olp_addr >> 16) & 0xFFFF) | ((olp_addr & 0xFFFF) << 16);
  *OLP = olp_addr;

  // Configure Video Mode (RGB16, Enable Video)
  *VMODE = 0x6C7; // CRY16/RGB16 + VIEN + BGEN + etc
#endif
}

void RenderWorld(ak_world_t *world) {
  demo_bitmap_clear(&main_screen, COL_BLACK);

  for (int i = 0; i < world->body_count; i++) {
    ak_body_t *b = &world->bodies[i];
    int x = AK_FIXED_TO_INT(b->position.x);
    int y = AK_FIXED_TO_INT(b->position.y);

    if (b->shape.type == AK_SHAPE_CIRCLE) {
      int r = AK_FIXED_TO_INT(b->shape.bounds.circle.radius);
      demo_bitmap_draw_circle(&main_screen, x, y, r,
                              b->is_static ? COL_BLUE : COL_RED);
    } else if (b->shape.type == AK_SHAPE_AABB) {
      int w = AK_FIXED_TO_INT(b->shape.bounds.aabb.width);
      int h = AK_FIXED_TO_INT(b->shape.bounds.aabb.height);
      demo_bitmap_draw_rect(&main_screen, x - w, y - h, w * 2, h * 2,
                            b->is_static ? COL_GREEN : COL_WHITE);
    }
  }

  for (int i = 0; i < world->tether_count; i++) {
    ak_tether_t *t = &world->tethers[i];
    int x1 = AK_FIXED_TO_INT(t->a->position.x);
    int y1 = AK_FIXED_TO_INT(t->a->position.y);
    int x2 = AK_FIXED_TO_INT(t->b->position.x);
    int y2 = AK_FIXED_TO_INT(t->b->position.y);
    demo_bitmap_draw_line(&main_screen, x1, y1, x2, y2, COL_WHITE);
  }
}

typedef struct {
  ak_world_t *world;
  ak_fixed_t dt;
} PhysicsArgs;

void PhysicsWrapper(void *data) {
  PhysicsArgs *args = (PhysicsArgs *)data;
  ak_world_step(args->world, args->dt);
}

int main() {
#ifdef JAGUAR
  InitVideo();
  jag_gpu_init();

  ak_world_t world;
  ak_world_init(&world, AK_INT_TO_FIXED(320), AK_INT_TO_FIXED(240),
                (ak_vec2_t){0, 0});
  ak_demo_create_standard_scene(&world);

  ak_fixed_t dt = AK_INT_TO_FIXED(1) / 60;
  PhysicsArgs args = {&world, dt};

  while (1) {
    jag_gpu_run(PhysicsWrapper, &args, sizeof(PhysicsArgs));
    jag_gpu_wait();
    RenderWorld(&world);
  }
#endif
  return 0;
}
