// LICENSE HERE.

//
// shared/math/vector4.h
//
// N&C Math Library: Vector4
//
#ifndef __INC_SHARED_MATH_VECTOR4_H__
#define __INC_SHARED_MATH_VECTOR4_H__

// Vector 4 type definiton. (X, Y, Z, W)
typedef vec_t vec4_t[4];

#define Vec4_Subtract(a,b,c)  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define Vec4_Add(a,b,c)       ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define Vec4_Copy(a,b)        ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define Vec4_Clear(a)         ((a)[0]=(a)[1]=(a)[2]=(a)[3]=0)
#define Vec4_Negate(a,b)      ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2],(b)[3]=-(a)[3])
#define Vec4_Set(v, a, b, c, d)   ((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

#endif // __INC_SHARED_MATH_VECTOR4_H__