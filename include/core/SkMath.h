
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMath_DEFINED
#define SkMath_DEFINED

#include "SkTypes.h"

/**
 *  Computes numer1 * numer2 / denom in full 64 intermediate precision.
 *  It is an error for denom to be 0. There is no special handling if
 *  the result overflows 32bits.
 */
int32_t SkMulDiv(int32_t numer1, int32_t numer2, int32_t denom);

/**
 *  Computes (numer1 << shift) / denom in full 64 intermediate precision.
 *  It is an error for denom to be 0. There is no special handling if
 *  the result overflows 32bits.
 */
int32_t SkDivBits(int32_t numer, int32_t denom, int shift);

/**
 *  Return the integer square root of value, with a bias of bitBias
 */
int32_t SkSqrtBits(int32_t value, int bitBias);

/** Return the integer square root of n, treated as a SkFixed (16.16)
 */
#define SkSqrt32(n)         SkSqrtBits(n, 15)

///////////////////////////////////////////////////////////////////////////////

//! Returns the number of leading zero bits (0...32)
int SkCLZ_portable(uint32_t);

#if defined(SK_CPU_ARM)
    #define SkCLZ(x)    __builtin_clz(x)
#endif

#ifndef SkCLZ
    #define SkCLZ(x)    SkCLZ_portable(x)
#endif

/**
 *  Returns (value < 0 ? 0 : value) efficiently (i.e. no compares or branches)
 */
static inline int SkClampPos(int value) {
    return value & ~(value >> 31);
}

/** Given an integer and a positive (max) integer, return the value
 *  pinned against 0 and max, inclusive.
 *  @param value    The value we want returned pinned between [0...max]
 *  @param max      The positive max value
 *  @return 0 if value < 0, max if value > max, else value
 */
static inline int SkClampMax(int value, int max) {
    // ensure that max is positive
    SkASSERT(max >= 0);
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }
    return value;
}

/**
 *  Returns the smallest power-of-2 that is >= the specified value. If value
 *  is already a power of 2, then it is returned unchanged. It is undefined
 *  if value is <= 0.
 */
static inline int SkNextPow2(int value) {
    SkASSERT(value > 0);
    return 1 << (32 - SkCLZ(value - 1));
}

/**
 *  Returns the log2 of the specified value, were that value to be rounded up
 *  to the next power of 2. It is undefined to pass 0. Examples:
 *  SkNextLog2(1) -> 0
 *  SkNextLog2(2) -> 1
 *  SkNextLog2(3) -> 2
 *  SkNextLog2(4) -> 2
 *  SkNextLog2(5) -> 3
 */
static inline int SkNextLog2(uint32_t value) {
    SkASSERT(value != 0);
    return 32 - SkCLZ(value - 1);
}

/**
 *  Returns true if value is a power of 2. Does not explicitly check for
 *  value <= 0.
 */
static inline bool SkIsPow2(int value) {
    return (value & (value - 1)) == 0;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  SkMulS16(a, b) multiplies a * b, but requires that a and b are both int16_t.
 *  With this requirement, we can generate faster instructions on some
 *  architectures.
 */
#ifdef SK_ARM_HAS_EDSP
    static inline int32_t SkMulS16(S16CPU x, S16CPU y) {
        SkASSERT((int16_t)x == x);
        SkASSERT((int16_t)y == y);
        int32_t product;
        asm("smulbb %0, %1, %2 \n"
            : "=r"(product)
            : "r"(x), "r"(y)
            );
        return product;
    }
#else
    #ifdef SK_DEBUG
        static inline int32_t SkMulS16(S16CPU x, S16CPU y) {
            SkASSERT((int16_t)x == x);
            SkASSERT((int16_t)y == y);
            return x * y;
        }
    #else
        #define SkMulS16(x, y)  ((x) * (y))
    #endif
#endif

/**
 *  Return a*b/((1 << shift) - 1), rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767 and shift is > 0 and <= 8
 */
static inline unsigned SkMul16ShiftRound(U16CPU a, U16CPU b, int shift) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    SkASSERT(shift > 0 && shift <= 8);
    unsigned prod = SkMulS16(a, b) + (1 << (shift - 1));
    return (prod + (prod >> shift)) >> shift;
}

/**
 *  Return a*b/255, rounding any fractional bits.
 *  Only valid if a and b are unsigned and <= 32767.
 */
static inline U8CPU SkMulDiv255Round(U16CPU a, U16CPU b) {
    SkASSERT(a <= 32767);
    SkASSERT(b <= 32767);
    unsigned prod = SkMulS16(a, b) + 128;
    return (prod + (prod >> 8)) >> 8;
}

#endif
