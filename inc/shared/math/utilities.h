// LICENSE HERE.

//
// shared/math/utilities.h
//
// N&C Math Library: Utilities
// 
// Contains utility functions used elsewhere by the math library, and/or in 
// game and engine code. 
//
#ifndef __INC_SHARED_MATH_UTILITIES_H__
#define __INC_SHARED_MATH_UTILITIES_H__

// NAN Macro - Use for checking NAN errors.
#define nanmask (255<<23)
#define IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

// Easy array index accessors.
#define PITCH   0       // Up / Down
#define YAW     1       // Left / Right
#define ROLL    2       // Fall over

// Conversion (Degrees, Radians) scalar values.
#define DegreesScalar ((float) (180.0f / std::numbers::pi_v<float>))
#define RadiansScalar ((float) (std::numbers::pi_v<float> / 180.0f))

//
//===============
// EqualEpsilonf
// 
// Returns true if `fabsf(a - b) <= epsilon`.
//===============
//
static inline float Degrees(float radians) {
    return radians * DegreesScalar;
}

//
//===============
// EqualEpsilonf
// 
// Returns true if `fabsf(a - b) <= epsilon`.
//===============
//
static inline float Radians(float degrees) {
    return degrees * RadiansScalar;
}

//
//===============
// EqualEpsilonf
// 
// Returns true if `fabsf(a - b) <= epsilon`.
//===============
//
static inline bool EqualEpsilonf(float a, float b, float epsilon = FLT_EPSILON) {
    return std::fabsf(a - b) <= epsilon;
}

//
//===============
// npo32
// 
// Returns whether the value of k is npot32
//===============
//
static inline unsigned npot32(unsigned k)
{
    if (k == 0)
        return 1;

    k--;
    k = k | (k >> 1);
    k = k | (k >> 2);
    k = k | (k >> 4);
    k = k | (k >> 8);
    k = k | (k >> 16);

    return k + 1;
}

//
//===============
// LerpAngle
// 
// Linearly interpolate the fraction between a2 and a1.
//===============
//
static inline float LerpAngle(float a2, float a1, float frac)
{
    if (a1 - a2 > 180)
        a1 -= 360;
    if (a1 - a2 < -180)
        a1 += 360;
    return a2 + frac * (a1 - a2);
}

//
//===============
// anglemod
// 
// Short angle modular.
//===============
//
static inline float anglemod(float a)
{
    a = (360.0f / 65536) * ((int)(a * (65536 / 360.0f)) & 65535);
    return a;
}

//
//===============
// anglemod
// 
// Short angle modular.
//===============
//
static inline int rand_byte(void)
{
    int r = rand();

    int b1 = (r >> 24) & 255;
    int b2 = (r >> 16) & 255;
    int b3 = (r >> 8) & 255;
    int b4 = (r) & 255;

    return b1 ^ b2 ^ b3 ^ b4;
}

//
//===============
// Q_align
// 
// Returns the aligned value.
//===============
//
static inline int Q_align(int value, int align)
{
    int mod = value % align;
    return mod ? value + align - mod : value;
}

//
//===============
// Q_gcd
// 
// Returns the gcd.
//===============
//
static inline int Q_gcd(int a, int b)
{
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

#define clamp(a,b,c)    ((a)<(b)?(a)=(b):(a)>(c)?(a)=(c):(a))
#define cclamp(a,b,c)   ((b)>(c)?clamp(a,c,b):clamp(a,b,c))

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define frand()     ((rand() & 32767) * (1.0 / 32767))
#define crand()     ((rand() & 32767) * (2.0 / 32767) - 1)

#define Q_rint(x)   ((x) < 0 ? ((int)((x) - 0.5f)) : ((int)((x) + 0.5f)))

#define Q_IsBitSet(data, bit)   (((data)[(bit) >> 3] & (1 << ((bit) & 7))) != 0)
#define Q_SetBit(data, bit)     ((data)[(bit) >> 3] |= (1 << ((bit) & 7)))
#define Q_ClearBit(data, bit)   ((data)[(bit) >> 3] &= ~(1 << ((bit) & 7)))

#endif // __INC_SHARED_MATH_UTILITIES_H__
