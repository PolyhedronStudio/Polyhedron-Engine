// LICENSE HERE.

//
// shared/math/vector2.h
//
// N&C Math Library: Vector2
//
#ifndef __INC_SHARED_MATH_VECTOR2_H__
#define __INC_SHARED_MATH_VECTOR2_H__

// Vector 2 type definiton. (X, Y)
typedef vec_t vec2_t[2];

#define Vec2_Subtract(a,b,c)  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1])
#define Vec2_Add(a,b,c)       ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1])

#endif // __INC_SHARED_MATH_VECTOR2_H__