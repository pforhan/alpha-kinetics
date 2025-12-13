#include "jag_gpu.h"
#include <stddef.h>

// Mock GPU state for simulation
static int gpu_running = 0;

void jag_gpu_init(void) {
  // In a real implementation, this would reset the GPU (RISC) processor
  // and perhaps upload a small kernel/dispatcher.
  gpu_running = 0;
}

void jag_gpu_run(jag_gpu_func_t func, void *data, uint32_t size) {
#ifdef JAGUAR
  // Real Jaguar Implementation Logic:
  // TODO: Wait for GPU to be idle (check GPU_CTRL)
  // TODO: Copy 'func' code to GPU RAM (0xF03000) using Blitter
  // TODO: Copy 'data' to GPU RAM (or ensure it's in a shared space)
  // TODO: Set GPU PC (Program Counter) to start address
  // TODO: Trigger GPU Start (GPU_CTRL register)

  // For this simple example, we just run it on the 68k synchronously
  // because we don't have the actual RISC binary code here.
  func(data);

#else
  // PC Simulation
  // Just run the function immediately
  func(data);
  gpu_running = 1; // Simulate running state
#endif
}

void jag_gpu_wait(void) {
#ifdef JAGUAR
// TODO Real Jaguar Logic:
// while (GPU_CTRL & 1); // Wait for GPU stopped bit
#else
  // PC Simulation
  gpu_running = 0;
#endif
}
