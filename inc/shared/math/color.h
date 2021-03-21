// LICENSE HERE.

//
// shared/math/color.h
//
// N&C Math Library: Color
//
#ifndef __INC_SHARED_MATH_COLOR_H__
#define __INC_SHARED_MATH_COLOR_H__

// Color type definiton. (R8,G8,B8,A8) == uint32_t total.

typedef union {
    uint32_t u32;
    uint8_t u8[4];
} color_t;

#endif // __INC_SHARED_MATH_COLOR_H__