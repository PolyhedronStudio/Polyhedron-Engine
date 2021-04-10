// LICENSE HERE.

//
// shared/math/vector5.h
//
// N&C Math Library: Vector5
//
#ifndef __INC_SHARED_MATH_VECTOR5_H__
#define __INC_SHARED_MATH_VECTOR5_H__

//-----------------
// Vector 5 type definiton. (X, Y, Z, W, V)
//
// The vector is implemented like a union class.
//-----------------
template<typename T> struct vec5_template {
    union
    {
        // XYZWV array index accessor.
        T xyzwv[3];

        // X Y Z W V desegnator accessors.
        struct {
            T x, y, z, w, v;
        };
    };

    //-----------------
    // Constructors.
    //-----------------
    // Default.
    vec5_template() { x = y = z = w = v = 0; }

    // Assign.
    vec5_template(T X, T Y, T Z, T W, T V) { x = X; y = Y; z = Z; w = W; v = V;  }

    // Regular *vec_t support.
    vec5_template(T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; w = vec[3]; v = vec[4]; }
    vec5_template(const T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; w = vec[3]; v = vec[4];  }

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

    // OPERATOR: + vec5_template
    inline vec5_template operator +(const vec5_template& operand) const
    {
        return vec5_template{
            x + operand.x,
            y + operand.y,
            z + operand.z,
            w + operand.w,
            v + operand.v
        };
    }

    // OPERATOR: - vec5_template
    inline vec5_template operator -(const vec5_template& operand) const
    {
        return vec5_template{
            x - operand.x,
            y - operand.y,
            z - operand.z,
            w - operand.w,
            v - operand.v
        };
    }

    // OPERATOR: / vec5_template
    inline vec5_template operator /(const vec5_template& operand) const
    {
        return vec5_template{
            x / operand.x,
            y / operand.y,
            z / operand.z,
            w / operand.w,
            v / operand.v
        };
    }

    // OPERATOR: * vec5_template
    inline vec5_template operator *(const vec5_template& operand) const
    {
        return vec5_template{
            x * operand.x,
            y * operand.y,
            z * operand.z,
            w * operand.w,
            v * operand.v
        };
    }

    // OPERATOR: -= vec5_template
    const vec5_template& operator -=(const vec5_template& operand) {
        x -= operand.x;
        y -= operand.y;
        z -= operand.z;
        w -= operand.w;
        v -= operand.v;
        return *this;
    }

    //// OPERATOR: += vec4_template
    const vec5_template& operator +=(const vec5_template& operand) {
        x += operand.x;
        y += operand.y;
        z += operand.z;
        w += operand.w;
        v += operand.v;
        return *this;
    }

    //// OPERATOR: /= vec5_template
    const vec5_template& operator *=(const vec5_template& operand) {
        x *= operand.x;
        y *= operand.y;
        z *= operand.z;
        w += operand.w;
        v += operand.v;
        return *this;
    }

    //// OPERATOR: *= vec5_template
    const vec5_template& operator /=(const vec5_template& operand) {
        x /= operand.x;
        y /= operand.y;
        z /= operand.z;
        w /= operand.w;
        v /= operand.v;
        return *this;
    }
};

typedef vec5_template<byte> bvec5_t;
typedef vec5_template<int> ivec5_t;
typedef vec5_template<float> vec5_t;
typedef vec5_template<double> dvec5_t;

#endif // __INC_SHARED_MATH_VECTOR5_H__