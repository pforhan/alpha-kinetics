#include "op.h"
#include <stddef.h>
#include <stdint.h>

// Globals expected by rmvlib
uint16_t a_vde = 0;
uint16_t a_vdb = 0;
op_stop_object stop_object;

// Stubs for stdlib functions
int abs(int i) { return i < 0 ? -i : i; }

void bcopy(const void *src, void *dest, size_t n) {
  char *d = dest;
  const char *s = src;
  while (n--) {
    *d++ = *s++;
  }
}

// Stubs for stdio functions
int printf(const char *format, ...) {
  (void)format;
  return 0;
}

int putchar(int c) {
  (void)c;
  return 0;
}
