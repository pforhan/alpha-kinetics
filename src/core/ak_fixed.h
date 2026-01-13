#ifndef AK_FIXED_H
#define AK_FIXED_H

#include <stdint.h>

// 16.16 Fixed Point Arithmetic
typedef int32_t ak_fixed_t;

#define AK_FIXED_SHIFT 16
#define AK_FIXED_ONE ((ak_fixed_t)1L << AK_FIXED_SHIFT)
#define AK_FIXED_HALF ((ak_fixed_t)1L << (AK_FIXED_SHIFT - 1))

// Conversion
#define AK_INT_TO_FIXED(x) ((ak_fixed_t)((ak_fixed_t)(x) << AK_FIXED_SHIFT))
#define AK_FIXED_TO_INT(x) ((int)((ak_fixed_t)(x) >> AK_FIXED_SHIFT))
#define AK_FLOAT_TO_FIXED(x) ((ak_fixed_t)((x) * AK_FIXED_ONE))
#define AK_FIXED_TO_FLOAT(x) ((float)(x) / AK_FIXED_ONE)

// Arithmetic
#define AK_FIXED_ADD(a, b) ((ak_fixed_t)(a) + (ak_fixed_t)(b))
#define AK_FIXED_SUB(a, b) ((ak_fixed_t)(a) - (ak_fixed_t)(b))

// Multiplication: (a * b) >> 16
// We cast to int64_t to prevent overflow before shifting
#define AK_FIXED_MUL(a, b)                                                     \
  ((ak_fixed_t)(((int64_t)(a) * (b)) >> AK_FIXED_SHIFT))

// Division: (a << 16) / b
#define AK_FIXED_DIV(a, b)                                                     \
  ((ak_fixed_t)(((int64_t)(a) << AK_FIXED_SHIFT) / (b)))

// Absolute value
#define AK_FIXED_ABS(a) ((a) < 0 ? -(a) : (a))

// Min/Max
#define AK_FIXED_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AK_FIXED_MAX(a, b) ((a) > (b) ? (a) : (b))

static inline ak_fixed_t AK_FIXED_SQRT(ak_fixed_t x) {
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
  return (ak_fixed_t)(root << 8);
}

#endif // AK_FIXED_H
