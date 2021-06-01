// LICENSE HERE.

//
// shared/math/vector2.h
//
// N&C Math Library: Vector2
// 
// vec2_template is a templated union class which by use of several operators
// allows for backwards compatibility support to most of the old Quake 2 based
// legacy vector code.
// 
// The actual Q2 Vector Macros are defined at the bottom of this file and exist
// for legacy code to still be compatible. For any new code, use the new and
// more programmer friendly Vec2_ functions. To increase readability, a rule of
// standard is to declare a vec2_t like so: vec2_t x = { 1.f, 0.f };
// 
// To add, subtract, divide or multiply a vector, simply use the designated 
//
#ifndef __INC_SHARED_MATH_VECTOR2_H__
#define __INC_SHARED_MATH_VECTOR2_H__

//-----------------
// Vector 2 type definiton. (X, Y)
//
// The vector is implemented like a union class.
//-----------------
template<typename T> struct vec2_template {
    union
    {
        // XY array index accessor.
        T xy[2];

        // X Y Z desegnator accessors.
        struct {
            T x, y;
        };
    };

    //-----------------
    // Constructors.
    //-----------------
    // Default.
    vec2_template() { x = y = 0; }

    // Assign.
    vec2_template(T X, T Y) { x = X; y = Y;  }

    // Regular *vec_t support.
    vec2_template(T* vec) { x = vec[0]; y = vec[1]; }
    vec2_template(const T* vec) { x = vec[0]; y = vec[1]; }

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

    // OPERATOR: + vec2_template
    inline vec2_template operator +(const vec2_template& operand) const
    {
        return vec2_template{
            x + operand.x,
            y + operand.y 
        };
    }

    // OPERATOR: - vec2_template
    inline vec2_template operator -(const vec2_template& operand) const
    {
        return vec2_template{
            x - operand.x,
            y - operand.y
        };
    }

    // OPERATOR: / vec2_template
    inline vec2_template operator /(const vec2_template& operand) const
    {
        return vec2_template{
            x / operand.x,
            y / operand.y
        };
    }

    // OPERATOR: * vec2_template
    inline vec2_template operator *(const vec2_template& operand) const
    {
        return vec2_template{
            x * operand.x,
            y * operand.y
        };
    }

    // OPERATOR: -= vec2_template
    const vec2_template& operator -=(const vec2_template& operand) {
        x -= operand.x;
        y -= operand.y;
        return *this;
    }

    //// OPERATOR: += vec2_template
    const vec2_template& operator +=(const vec2_template& operand) {
        x += operand.x;
        y += operand.y;
        return *this;
    }

    //// OPERATOR: /= vec2_template
    const vec2_template& operator *=(const vec2_template& operand) {
        x *= operand.x;
        y *= operand.y;
        return *this;
    }

    //// OPERATOR: *= vec2_template
    const vec2_template& operator /=(const vec2_template& operand) {
        x /= operand.x;
        y /= operand.y;
        return *this;
    }
};

// Declare the default types for usage.
typedef vec2_template<byte> bvec2_t;
typedef vec2_template<int> ivec2_t;
typedef vec2_template<float> vec2_t;
typedef vec2_template<double> dvec2_t;


//
//=============================================================================
// 
// Modern vec2_t Inline Functions:
//
//=============================================================================
//

//
//===============
// vec2_dot
// 
// Returns the dot product of 'a · b'.
//===============
//
static inline float vec2_dot(const vec2_t &a, const vec2_t &b) {
    return a.x * b.x + a.y * b.y;
}

//
//===============
// vec2_length_squared
// 
// Returns the squared length (magnitude) of 'v'.
//===============
//
static inline float vec2_length_squared(const vec2_t &v) {
    return vec2_dot(v, v);
}

//
//===============
// vec2_length
// 
// Return the length(magnitude) of 'v'.
//===============
//
static inline float vec2_length(const vec2_t &v) {
    return std::sqrtf(vec2_length_squared(v));
}

//
//===============
// vec2_distance_squared
// 
// Return the length squared of the vector 'a - b'.
//===============
//
static inline float vec2_distance_squared(const vec2_t &a, const vec2_t &b) {
    return vec2_length_squared(a - b);
}

//
//===============
// vec2_distance
// 
// Return The length of the vector 'a - b'.
//===============
//
static inline float vec2_distance(const vec2_t &a, const vec2_t &b) {
    return vec2_length(a - b);
}

//
//===============
// vec2_equal_epsilon
// 
// Returns true if 'a'and 'b' are equal using the specified epsilon.
//===============
//
static inline qboolean vec2_equal_epsilon(const vec2_t &a, const vec2_t &b, float epsilon = FLT_EPSILON) {
    return EqualEpsilonf(a.x, b.x, epsilon) &&
        EqualEpsilonf(a.y, b.y, epsilon);
}

//
//===============
// vec2_equal
// 
// Returns the vector `v` + (`add` * `multiply`).
//===============
//
static inline qboolean vec2_equal(const vec2_t &a, const vec2_t &b) {
    return vec2_equal_epsilon(a, b);
}

//
//===============
// vec2_maxf
// 
// Returns a vector containing the max components of 'a' and 'b'.
//===============
//
static inline vec2_t vec2_maxf(const vec2_t &a, const vec2_t &b) {
    return vec2_t {
        Maxf(a.x, b.x),
        Maxf(a.y, b.y)
    };
}

//
//===============
// vec2_maxs
// 
// Rreturn the vector '(-FLT_MAX, -FLT_MAX)'.
//===============
//
static inline vec2_t vec2_maxs(void) {
    return vec2_t {
        -FLT_MAX,
        -FLT_MAX
    };
}

//
//===============
// vec2_minf
// 
// Returns a vector containing the min components of 'a' and 'b'.
//===============
//
static inline vec2_t vec2_minf(const vec2_t &a, const vec2_t &b) {
    return vec2_t {
        Minf(a.x, b.x), 
        Minf(a.y, b.y)
    };
}

//
//===============
// vec2_mins
// 
// Returns The vector `(FLT_MAX, FLT_MAX)`.
//===============
//
static inline vec2_t vec2_mins(void) {
    return vec2_t {
        FLT_MAX, 
        FLT_MAX
    };
}

//
//===============
// vec2_scale
// 
// Returns the vector 'v' scaled by 'scale'.
//===============
//
static inline vec2_t vec2_scale(const vec2_t &v, float scale) {
    return vec2_t{
        v.x * scale,
        v.y * scale
    };
}

//
//===============
// vec2_fmaf
// 
// Returns the vector 'v' + ('add' * 'multiply').
//===============
//
static inline vec2_t vec2_fmaf(const vec2_t& v, float multiply, const vec2_t& add) {
    return vec2_t {
        std::fmaf(add.x, multiply, v.x),
        std::fmaf(add.y, multiply, v.y)
    };
}

//
//===============
// vec2_mix
// 
// Returns the linear interpolation of 'a' and 'b' using the specified fraction.
//===============
//
static inline vec2_t vec2_mix(const vec2_t &a, const vec2_t &b, float mix) {
    return vec2_fmaf(a, mix, b - a);
}

//
//===============
// vec2_bilinear
// 
// Returns the vec2_t zero vector.
//===============
//
static inline vec2_t vec2_bilinear(const vec2_t &tl, const vec2_t &tr, const vec2_t &bl, const vec2_t &br, vec2_t &mix) {
    return vec2_mix(vec2_mix(tl, tr, mix.x), vec2_mix(bl, br, mix.x), mix.y);
}

//
//===============
// vec2_zero
// 
// Returns the vec2_t zero vector.
//===============
//
static inline vec2_t vec2_zero(void) {
    return vec2_t {
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
#define Vec2_Subtract(a,b,c)  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1])
#define Vec2_Add(a,b,c)       ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1])

#endif // __INC_SHARED_MATH_VECTOR2_H__