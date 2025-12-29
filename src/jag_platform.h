#ifndef JAG_PLATFORM_H
#define JAG_PLATFORM_H

// Memory Map
#define JAG_DRAM_BASE 0x00000000
#define JAG_DRAM_SIZE 0x00200000 // 2MB

// Register Bases
#define TOM_BASE 0xF00000
#define JERRY_BASE 0xF10000

// Video Interface (TOM)
#define VI_BASE (TOM_BASE + 0x0000)
#define VI_VMODE (*(volatile uint16_t *)(VI_BASE + 0x28))
#define VI_VOBF (*(volatile uint16_t *)(VI_BASE + 0x40))
#define VI_HPERIOD (*(volatile uint16_t *)(VI_BASE + 0x04))
#define VI_HBB (*(volatile uint16_t *)(VI_BASE + 0x06))
#define VI_HBE (*(volatile uint16_t *)(VI_BASE + 0x08))
#define VI_HSYNC (*(volatile uint16_t *)(VI_BASE + 0x0A))
#define VI_HVS (*(volatile uint16_t *)(VI_BASE + 0x0C))
#define VI_VPERIOD (*(volatile uint16_t *)(VI_BASE + 0x10))
#define VI_VBB (*(volatile uint16_t *)(VI_BASE + 0x12))
#define VI_VBE (*(volatile uint16_t *)(VI_BASE + 0x14))
#define VI_VSYNC (*(volatile uint16_t *)(VI_BASE + 0x16))
#define VI_BCOLOR (*(volatile uint32_t *)(VI_BASE + 0x24))

// Object Processor (TOM)
#define OP_BASE (TOM_BASE + 0x0010)
#define OP_LIST_PTR                                                            \
  (*(volatile uint32_t *)(OP_BASE + 0x20)) // Object List Pointer
#define OP_STATUS (*(volatile uint32_t *)(OP_BASE + 0x24))

// Palette (TOM)
#define PALETTE_BASE (TOM_BASE + 0x0400)
#define CLUT ((volatile uint16_t *)PALETTE_BASE)

// Blitter (TOM)
#define BLIT_BASE (TOM_BASE + 0x3000)
#define BLIT_CMD (*(volatile uint32_t *)(BLIT_BASE + 0x00))
#define BLIT_A1_BASE (*(volatile uint32_t *)(BLIT_BASE + 0x08))

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

// Screen Dimensions (NTSC 320x240)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// NTSC Video Timings (Standard 320 width)
#define NTSC_HPERIOD 0x018F
#define NTSC_HBB 0x015B
#define NTSC_HBE 0x012F
#define NTSC_HSYNC 0x014B
#define NTSC_VPERIOD 0x0103
#define NTSC_VBB 0x00EE
#define NTSC_VBE 0x0015
#define NTSC_VSYNC 0x0100

// Colors (16-bit RGB)
#define COL_BLACK 0x0000
#define COL_WHITE 0xFFFF
#define COL_RED 0xF800
#define COL_GREEN 0x07E0
#define COL_BLUE 0x001F

#endif // JAG_PLATFORM_H
