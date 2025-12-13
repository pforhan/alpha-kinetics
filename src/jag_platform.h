#ifndef JAG_PLATFORM_H
#define JAG_PLATFORM_H

#include <stdint.h>

// Memory Map
#define JAG_DRAM_BASE 0x00000000
#define JAG_DRAM_SIZE 0x00200000 // 2MB

// Register Bases
#define TOM_BASE 0xF00000
#define JERRY_BASE 0xF10000

// Video Interface (TOM)
#define VI_BASE (TOM_BASE + 0x0030)
#define VI_HWIDTH (*(volatile uint16_t *)(VI_BASE + 0x00))
#define VI_VHEIGHT (*(volatile uint16_t *)(VI_BASE + 0x02))
#define VI_CONTROL (*(volatile uint16_t *)(VI_BASE + 0x20)) // Video Control

// Object Processor (TOM)
#define OP_BASE (TOM_BASE + 0x0010)
#define OP_LIST_PTR                                                            \
  (*(volatile uint32_t *)(OP_BASE + 0x20)) // Object List Pointer

// Blitter (TOM)
#define BLIT_BASE (TOM_BASE + 0x3000)
#define BLIT_CMD (*(volatile uint32_t *)(BLIT_BASE + 0x00))
#define BLIT_A1_BASE (*(volatile uint32_t *)(BLIT_BASE + 0x04))

// GPU (TOM)
#define GPU_BASE (TOM_BASE + 0x2100)
#define GPU_PC (*(volatile uint32_t *)(GPU_BASE + 0x00))
#define GPU_CTRL (*(volatile uint32_t *)(GPU_BASE + 0x04))

// DSP (JERRY)
#define DSP_BASE (JERRY_BASE + 0x1000)
#define DSP_PC (*(volatile uint32_t *)(DSP_BASE + 0x00))
#define DSP_CTRL (*(volatile uint32_t *)(DSP_BASE + 0x04))

// Joypad (JERRY)
#define JOYSTICK (*(volatile uint16_t *)(JERRY_BASE + 0x4000))

// Screen Dimensions (NTSC)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Colors (16-bit CRY/RGB)
#define COL_BLACK 0x0000
#define COL_WHITE 0xFFFF
#define COL_RED 0xF800
#define COL_GREEN 0x07E0
#define COL_BLUE 0x001F

#endif // JAG_PLATFORM_H
