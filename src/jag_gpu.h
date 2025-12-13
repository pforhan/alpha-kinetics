#ifndef JAG_GPU_H
#define JAG_GPU_H

#include "jag_platform.h"
#include <stdint.h>


// Function pointer type for code to run on GPU
typedef void (*jag_gpu_func_t)(void *data);

// Command structure to send to GPU
typedef struct {
  jag_gpu_func_t code_ptr;
  void *data_ptr;
  uint32_t data_size;
} jag_gpu_cmd_t;

// Interface
void jag_gpu_init(void);
void jag_gpu_run(jag_gpu_func_t func, void *data, uint32_t size);
void jag_gpu_wait(void);

#endif // JAG_GPU_H
