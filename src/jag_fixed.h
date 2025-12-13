#ifndef JAG_FIXED_H
#define JAG_FIXED_H

#include <stdint.h>

// 16.16 Fixed Point Arithmetic
typedef int32_t jag_fixed_t;

#define JAG_FIXED_SHIFT 16
#define JAG_FIXED_ONE (1 << JAG_FIXED_SHIFT)
#define JAG_FIXED_HALF (1 << (JAG_FIXED_SHIFT - 1))

// Conversion
#define INT_TO_FIXED(x) ((jag_fixed_t)((x) << JAG_FIXED_SHIFT))
#define FIXED_TO_INT(x) ((int)((x) >> JAG_FIXED_SHIFT))
#define FLOAT_TO_FIXED(x) ((jag_fixed_t)((x) * JAG_FIXED_ONE))
#define FIXED_TO_FLOAT(x) ((float)(x) / JAG_FIXED_ONE)

// Arithmetic
#define FIXED_ADD(a, b) ((a) + (b))
#define FIXED_SUB(a, b) ((a) - (b))

// Multiplication: (a * b) >> 16
// We cast to int64_t to prevent overflow before shifting
#define FIXED_MUL(a, b) ((jag_fixed_t)(((int64_t)(a) * (b)) >> JAG_FIXED_SHIFT))

// Division: (a << 16) / b
#define FIXED_DIV(a, b) ((jag_fixed_t)(((int64_t)(a) << JAG_FIXED_SHIFT) / (b)))

// Absolute value
#define FIXED_ABS(a) ((a) < 0 ? -(a) : (a))

// Min/Max
#define FIXED_MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIXED_MAX(a, b) ((a) > (b) ? (a) : (b))

static inline jag_fixed_t FIXED_SQRT(jag_fixed_t x) {
  if (x <= 0)
    return 0;
  // Simple integer sqrt for 16.16 fixed point
  // We want sqrt(x * 65536) = sqrt(x) * 256
  // So we treat x as a large integer, take integer sqrt, then shift
  // appropriately? No, sqrt(x_fp) = sqrt(x_int * 2^16) = sqrt(x_int) * 2^8
  // Result should be in 16.16, so we need result * 2^16.
  // So we need sqrt(x_fp) * 2^8.
  // Let's just use a standard integer sqrt on the raw value and adjust.
  // sqrt(val) where val is x * 2^16.
  // We want res = sqrt(x) * 2^16.
  // res = sqrt(val / 2^16) * 2^16 = (sqrt(val) / 2^8) * 2^16 = sqrt(val) * 2^8.
  // So we take sqrt(x) and shift left by 8.

  uint64_t root = 0;
  uint64_t rem = (uint64_t)x;
  uint64_t place = 1ULL << 62; // Start high

  while (place > rem)
    place >>= 2;

  while (place) {
    if (rem >= root + place) {
      rem -= root + place;
      root += place * 2;
    }
    root >>= 1;
    place >>= 2;
  }
  return (jag_fixed_t)(root << 8);
}

#endif // JAG_FIXED_H
