#ifndef AK_FIXED_H
#define AK_FIXED_H

#include <stdint.h>

#if defined(__CC65__) || defined(ARDUBOY)
// 8.8 Fixed Point Arithmetic for 8-bit systems
// Using 32-bit to avoid overflow for screen coordinates (e.g. 160px screen)
typedef int32_t ak_fixed_t;
#define AK_FIXED_SHIFT 8
#else
// 16.16 Fixed Point Arithmetic
typedef int32_t ak_fixed_t;
#define AK_FIXED_SHIFT 16
#endif

#define AK_FIXED_ONE ((ak_fixed_t)1 << AK_FIXED_SHIFT)
#define AK_FIXED_HALF ((ak_fixed_t)1 << (AK_FIXED_SHIFT - 1))

// Conversion
#define AK_INT_TO_FIXED(x) ((ak_fixed_t)((ak_fixed_t)(x) << AK_FIXED_SHIFT))
#define AK_FIXED_TO_INT(x) ((int)((ak_fixed_t)(x) >> AK_FIXED_SHIFT))
#define AK_FLOAT_TO_FIXED(x) ((ak_fixed_t)((x) * AK_FIXED_ONE))
#define AK_FIXED_TO_FLOAT(x) ((float)(x) / AK_FIXED_ONE)

// Arithmetic
#define AK_FIXED_ADD(a, b) ((ak_fixed_t)(a) + (ak_fixed_t)(b))
#define AK_FIXED_SUB(a, b) ((ak_fixed_t)(a) - (ak_fixed_t)(b))

// Multiplication
#if defined(__CC65__) || defined(ARDUBOY)
// 8.8 Multiplication: (a * b) >> 8
// Using 32-bit math.
#define AK_FIXED_MUL(a, b)                                                     \
  ((ak_fixed_t)(((ak_fixed_t)(a) * (b)) >> AK_FIXED_SHIFT))
// 8.8 Division: (a << 8) / b
#define AK_FIXED_DIV(a, b)                                                     \
  ((ak_fixed_t)(((ak_fixed_t)(a) << AK_FIXED_SHIFT) / (b)))
#else
// 16.16 Multiplication: (a * b) >> 16
// We cast to int64_t to prevent overflow before shifting
#define AK_FIXED_MUL(a, b)                                                     \
  ((ak_fixed_t)(((int64_t)(a) * (b)) >> AK_FIXED_SHIFT))
// 16.16 Division: (a << 16) / b
#define AK_FIXED_DIV(a, b)                                                     \
  ((ak_fixed_t)(((int64_t)(a) << AK_FIXED_SHIFT) / (b)))
#endif

// Absolute value
#define AK_FIXED_ABS(a) ((a) < 0 ? -(a) : (a))

// Min/Max
#define AK_FIXED_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AK_FIXED_MAX(a, b) ((a) > (b) ? (a) : (b))

#if defined(__CC65__)
#define AK_INLINE static
#else
#define AK_INLINE static inline
#endif

AK_INLINE ak_fixed_t AK_FIXED_SQRT(ak_fixed_t x) {
  if (x <= 0)
    return 0;

#if defined(__CC65__) || defined(ARDUBOY)
  // Simple integer sqrt for 8.8 fixed point
  // sqrt(x_fp) = sqrt(x_int * 2^8) = sqrt(x_int) * 2^4
  // To get result in 8.8, we need sqrt(x_fp) * 2^4
  {
    uint32_t root = 0;
    uint32_t rem = (uint32_t)x;
    uint32_t place = 1UL << 30;

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
    return (ak_fixed_t)(root << 4);
  }
#else
  {
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
#endif
}

#endif // AK_FIXED_H
