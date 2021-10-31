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

// Clamp.// These need to be replaced.
#define clamp(a,b,c)    ((a)<(b)?(a)=(b):(a)>(c)?(a)=(c):(a))
#define cclamp(a,b,c)   ((b)>(c)?clamp(a,c,b):clamp(a,b,c))

// These need to be replaced.
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

// These need to be replaced.
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// These need to be replaced.
#define Q_IsBitSet(data, bit)   (((data)[(bit) >> 3] & (1 << ((bit) & 7))) != 0)
#define Q_SetBit(data, bit)     ((data)[(bit) >> 3] |= (1 << ((bit) & 7)))
#define Q_ClearBit(data, bit)   ((data)[(bit) >> 3] &= ~(1 << ((bit) & 7)))

// Conversion (Degrees, Radians) scalar values.
#define DegreesScalar ((float) (180.0f / std::numbers::pi_v<float>))
#define RadiansScalar ((float) (std::numbers::pi_v<float> / 180.0f))


//
//===============
// Degrees
// 
// Returns the radians in degrees.
//===============
//
static inline float Degrees(float radians) {
    return radians * DegreesScalar;
}

//
//===============
// Radians
// 
// Returns the degrees in radians.
//===============
//
static inline float Radians(float degrees) {
    return degrees * RadiansScalar;
}

//
//===============
// Radians
// 
// Does a sinf, cosf, for the radians, useful utility. (used in vec3_vectors)
//===============
//
static inline void SinCosRadians(const float radians, float  &s, float &c) {
    s = std::sinf(radians);
    c = std::cosf(radians);
}

//
//===============
// InitRandomNumberGenerator
//
// Initializes the Random Number generator for use with generating random
// vector coordinates.
//===============
//
static inline void InitRandomNumberGenerator() {
    static qboolean isInitialized = false;
    if (!isInitialized) {
        std::srand(std::time(NULL));
        isInitialized = true;
    }
}

//
//===============
// Randomi
//
// Returns a psuedo random unsigned integral value between `0` and `UINT_MAX`.
//===============
//
static inline uint32_t Randomui(void) {
    // Make sure the random number generator is initialized.
    InitRandomNumberGenerator();

    return std::rand();
}

//
//===============
// Randomf
//
// Returns a psuedo random float value between `0` and 1/
//===============
//
static inline uint32_t Randomf(void) {
    // Make sure the random number generator is initialized.
    InitRandomNumberGenerator();

    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}


//
//===============
// RandomRangef
//
// Returns a psuedo random float value between 'begin' and 'end'.
//===============
//
static inline float RandomRangef(float begin, float end) {
    // Make sure the random number generator is initialized.
    InitRandomNumberGenerator();

    return begin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (begin - end)));
}

//
//===============
// RandomRangei
//
// Returns a psuedo random unsigned integral value between 'begin' and 'end'.
//===============
//
static inline uint32_t RandomRangeui(uint32_t begin, uint32_t end) {
    // Make sure the random number generator is initialized.
    InitRandomNumberGenerator();

    return begin + (rand() % (end - begin) + begin);
}

//
//===============
// Minf
// 
// Returns the minimim of 'a' and 'b'.
//===============
//
static inline float Minf(float a, float b) {
    return a < b ? a : b;
}

//
//===============
// Mini
// 
// Returns the minimim of 'a' and 'b'.
//===============
//
static inline int32_t Mini(int32_t a, int32_t b) {
    return a < b ? a : b;
}

//
//===============
// Maxf
// 
// Returns the maximum of 'a' and 'b'.
//===============
//
static inline float Maxf(float a, float b) {
    return a > b ? a : b;
}


//
//===============
// Maxi
// 
// Returns the maximum int of 'a' and 'b'.
//===============
//
static inline int32_t Maxi(int32_t a, int32_t b) {
    return a > b ? a : b;
}

//
//===============
// Mixf
// 
// Returns the linear interpolation of 'a' and 'b' using the specified fraction.
//===============
//
static inline float Mixf(float a, float b, float mix) {
    return std::fmaf(a, mix, b - a);
}

//
//===============
// Clampf
// 
// Returns the value 'f', clamped to the specified 'min' and 'max'.
//===============
//
static inline float Clampf(float f, float min = 0.f, float max = 1.f) {
    return Minf(Maxf(f, min), max);
}

//
//===============
// Clampi
// 
// Returns the value 'i', clamped to the specified 'min' and 'max'.
//===============
//
static inline int Clampi(int i, int min, int max) {
    return Mini(Maxi(i, min), max);
}


//
//===============
// ClampEuler
// 
// The angle `theta` circularly clamped.
//===============
//
static inline float ClampEuler(float theta) {
    while (theta >= 360.f)
        theta -= 360.f;
    while (theta < 0.f)
        theta += 360.f;

    return theta;
}

//
//===============
// Smoothf
// 
// Returns the Hermite interpolation of 'f'.
//===============
//
static inline float Smoothf(float f, float min, float max) {
    const float s = Clampf((f - min) / (max - min), 0.f, 1.f);
    return s * s * (3.f - 2.f * s);
}

//
//===============
// EqualEpsilonf
// 
// Returns true if `std""fabsf(a - b) <= epsilon`.
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
// AngleMod
// 
// Short angle modular.
//===============
//
static inline float AngleMod(float a) {
    a = fmodf(a, 360.f);// (360.0 / 65536) * ((int32_t) (a * (65536 / 360.0)) & 65535);

    if (a < 0) {
        return a + (((int32_t)(a / 360.f) + 1) * 360.f);
    }

    return a;
}
//static inline float AngleMod(float a)
//{
//    return (360.f * a) & 360; //(360.0f / 65536) * ((int)(a * (65536 / 360.0f)) & 65535);
//}

//
//===============
// AngleMod
// 
// Short angle modular.
//===============
//
static inline int rand_byte(void)
{
    // Make sure the random number generator is initialized.
    InitRandomNumberGenerator();

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

// These need to be replaced.


#define frand()     ((rand() & 32767) * (1.0 / 32767))
//#define frand()     Randomf()
#define crand()     ((rand() & 32767) * (2.0 / 32767) - 1)
//#define crand()     Randomui()
#define Q_rint(x)   ((x) < 0 ? ((int)((x) - 0.5f)) : ((int)((x) + 0.5f)))

#endif // __INC_SHARED_MATH_UTILITIES_H__
