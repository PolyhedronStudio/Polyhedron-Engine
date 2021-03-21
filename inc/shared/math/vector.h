// LICENSE HERE.

//
// shared/math/vector3.h
//
// Vector math library implementation.
//
#ifndef __INC__SHARED__MATH__VECTOR_H__
#define __INC__SHARED__MATH__VECTOR_H__

typedef float vec_t;
typedef vec_t vec2_t[2];

typedef union {
    // Xyz array index accessor.
    float xyz[3];

    // X Y Z desegnator accessor.
    struct {
        float x, y, z;
    };
} vec3_t;

typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

#endif // __INC__SHARED__MATH__VECTOR_H__