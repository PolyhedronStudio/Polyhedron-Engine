// LICENSE HERE.

//
// shared/math/vector4.h
//
// N&C Math Library: Vector4
// 
// vec4_template is a templated union class which by use of several operators
// allows for backwards compatibility support to most of the old Quake 2 based
// legacy vector code.
// 
// The actual Q2 Vector Macros are defined at the bottom of this file and exist
// for legacy code to still be compatible. For any new code, use the new and
// more programmer friendly Vec4_ functions. To increase readability, a rule of
// standard is to declare a vec4_t like so: vec4_t x = { 1.f, 0.f, 0.f, 1.f };
// 
// To add, subtract, divide or multiply a vector, simply use the designated 
//
#ifndef __INC_SHARED_MATH_VECTOR4_H__
#define __INC_SHARED_MATH_VECTOR4_H__

//-----------------
// Vector 4 type definiton. (X, Y, Z, W)
//
// The vector is implemented like a union class.
//-----------------
template<typename T> struct vec4_template {
    union
    {
        // XYZW array index accessor.
        T xyzw[3];

        // X Y Z W desegnator accessors.
        struct {
            T x, y, z, w;
        };
    };

    //-----------------
    // Constructors.
    //-----------------
    // Default.
    vec4_template() { x = y = z = w = 0; }

    // Assign.
    vec4_template(T X, T Y, T Z, T W) { x = X; y = Y; z = Z; w = W; }

    // Regular *vec_t support.
    vec4_template(T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; w = vec[3]; }
    vec4_template(const T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; w = vec[3];  }

    //-----------------
    // Operators.
    //-----------------
    // Pointer.
    inline operator T* () {
        return &x;
    }

    // Pointer cast to const float*
    inline operator const T* () const {
        return &x;
    }

    // OPERATOR: + vec4_template
    inline vec4_template operator +(const vec4_template& operand) const
    {
        return vec4_template{
            x + operand.x,
            y + operand.y,
            z + operand.z,
            w + operand.w
        };
    }

    // OPERATOR: - vec4_template
    inline vec4_template operator -(const vec4_template& operand) const
    {
        return vec4_template{
            x - operand.x,
            y - operand.y,
            z - operand.z,
            w - operand.w
        };
    }

    // OPERATOR: / vec4_template
    inline vec4_template operator /(const vec4_template& operand) const
    {
        return vec4_template{
            x / operand.x,
            y / operand.y,
            z / operand.z,
            w / operand.w
        };
    }

    // OPERATOR: * vec4_template
    inline vec4_template operator *(const vec4_template& operand) const
    {
        return vec4_template{
            x * operand.x,
            y * operand.y,
            z * operand.z,
            w * operand.w
        };
    }

    // OPERATOR: -= vec4_template
    const vec4_template& operator -=(const vec4_template& operand) {
        x -= operand.x;
        y -= operand.y;
        z -= operand.z;
        w -= operand.w;
        return *this;
    }

    //// OPERATOR: += vec4_template
    const vec4_template& operator +=(const vec4_template& operand) {
        x += operand.x;
        y += operand.y;
        z += operand.z;
        w += operand.w;
        return *this;
    }

    //// OPERATOR: /= vec4_template
    const vec4_template& operator *=(const vec4_template& operand) {
        x *= operand.x;
        y *= operand.y;
        z *= operand.z;
        w += operand.w;
        return *this;
    }

    //// OPERATOR: *= vec4_template
    const vec4_template& operator /=(const vec4_template& operand) {
        x /= operand.x;
        y /= operand.y;
        z /= operand.z;
        w /= operand.w;
        return *this;
    }
};
// OPERATOR: += float
//const vec4_template& operator +=(const float& other) {
//    x += other;
//    y += other;
//    z += other;
//    return *this;
//}
typedef vec4_template<byte> bvec4_t;
typedef vec4_template<int> ivec4_t;
typedef vec4_template<float> vec4_t;
typedef vec4_template<double> dvec4_t;


//
//=============================================================================
// 
// Modern vec4_t Inline Functions:
//
//=============================================================================
//

//
//===============
// vec4_negate
// 
// Returns the negated vector 'v'.
//===============
//
static inline vec4_t vec4_negate(const vec4_t& v) {
    return vec4_t {
        -v.x,
        -v.y,
        -v.z,
        -v.w
    };
}

//
//===============
// vec4_equal_epsilon
// 
// Return true if `a` and `b` are equal using the specified epsilon.
//===============
//
static inline qboolean vec4_equal_epsilon(const vec4_t &a, const vec4_t &b, float epsilon = FLT_EPSILON) {
    return EqualEpsilonf(a.x, b.x, epsilon) &&
        EqualEpsilonf(a.y, b.y, epsilon) &&
        EqualEpsilonf(a.z, b.z, epsilon) &&
        EqualEpsilonf(a.w, b.w, epsilon);
}

//
//===============
// vec4_equal
// 
// Return true if `a` and `b` are equal.
//===============
//
static inline qboolean vec4_equal(const vec4_t &a, const vec4_t &b) {
    return vec4_equal_epsilon(a, b);
}

//
//===============
// vec4_scale
// 
// Return the vector 'v' scaled by 'scale'.
//===============
//
static inline vec4_t vec4_scale(const vec4_t &v, float scale) {
    return vec4_t {
        v.x * scale,
        v.y * scale,
        v.z * scale,
        v.w * scale
    };
}

//
//===============
// vec4_fmaf
// 
// Return the vector 'v' + ('add' * 'multiply').
//===============
//
static inline vec4_t vec4_fmaf(const vec4_t &v, float multiply, const vec4_t &add) {
    return vec4_t(std::fmaf(add.x, multiply, v.x), std::fmaf(add.y, multiply, v.y), std::fmaf(add.z, multiply, v.z), std::fmaf(add.w, multiply, v.w));
}

//
//===============
// vec4_mix
// 
// Return the linear interpolation of 'a' and 'b' using the specified fraction.
//===============
//
static inline vec4_t vec4_mix(const vec4_t &a, const vec4_t &b, float mix) {
    return vec4_fmaf(a, mix, b - a);
}

//
//===============
// vec4_one
// 
// Return the vector '{ 1.f, 1f., 1.f, .1f }'.
//===============
//
static inline vec4_t vec4_one(void) {
    return vec4_t {
        1.f, 
        1.f, 
        1.f, 
        1.f
    };
}

//
//===============
// vec4_random_range
// 
// Return a vector with random values between 'begin' and 'end'.
//===============
//
static inline vec4_t vec4_random_range(float begin, float end) {
    return vec4_t {
        RandomRangef(begin, end),
        RandomRangef(begin, end),
        RandomRangef(begin, end),
        RandomRangef(begin, end)
    };
}

//
//===============
// vec4_random
// 
// A vector with random values between '0' and '1'.
//===============
//
static inline vec4_t vec4_random(void) {
    return vec4_random_range(0.f, 1.f);
}

//
//===============
// vec4_xyz
// 
// The xyz swizzle of 'v'.
//===============
//
static inline vec3_t vec4_xyz(const vec4_t &v) {
    return vec3_t {
        v.x,
        v.y,
        v.z
    };
}

//
//===============
// vec4_zero
// 
// Return the vector '{ 0.f, 0.f, 0.f, 0.f }'.
//===============
//
static inline vec4_t vec4_zero(void) {
    return vec4_t {
        0.f,
        0.f,
        0.f,
        0.f
    };
}


//
//=============================================================================
// 
// Legacy Vector Macro Functions:
//
//=============================================================================
//
#define Vector4Subtract(a,b,c)  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define Vector4Add(a,b,c)       ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define Vector4Copy(a,b)        ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define Vector4Clear(a)         ((a)[0]=(a)[1]=(a)[2]=(a)[3]=0)
#define Vector4Negate(a,b)      ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2],(b)[3]=-(a)[3])
#define Vector4Set(v, a, b, c, d)   ((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))
#define Vector4MA(a,b,c,d) \
        ((d)[0]=(a)[0]+(b)*(c)[0], \
         (d)[1]=(a)[1]+(b)*(c)[1], \
         (d)[2]=(a)[2]+(b)*(c)[2], \
         (d)[3]=(a)[3]+(b)*(c)[3])

#define QuatCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#endif // __INC_SHARED_MATH_VECTOR4_H__