// LICENSE HERE.

//
// shared/math/vector3.h
//
// N&C Math Library: Vector3
// 
// vec3_template is a templated union class which by use of several operators
// allows for backwards compatibility support to most of the old Quake 2 based
// legacy vector code.
// 
// The actual Q2 Vector Macros are defined at the bottom of this file and exist
// for legacy code to still be compatible. For any new code, use the new and
// more programmer friendly Vec3_ functions. To increase readability, a rule of
// standard is to declare a vec3_t like so: vec3_t x = { 1.f, 0.f, 0.f };
// 
// To add, subtract, divide or multiply a vector, simply use the designated 
//
#ifndef __INC_SHARED_MATH_VECTOR3_H__
#define __INC_SHARED_MATH_VECTOR3_H__

//-----------------
// Vector 3 type definiton. (X, Y, Z)
//
// The vector is implemented like a union class.
//-----------------
template<typename T> struct vec3_template {
    union
    {
        // XYZ array index accessor.
        T xyz[3];

        // X Y Z desegnator accessors.
        struct {
            T x, y, z;
        };
    };

    //-----------------
    // Constructors.
    //-----------------
    // Default.
    vec3_template() { x = y = z = 0; }

    // Assign.
    vec3_template(T X, T Y, T Z) { x = X; y = Y; z = Z; }

    // Regular *vec_t support.
    vec3_template(T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }
    vec3_template(const T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }

    // Easy vec3_t array index accessors.
    enum PYR {
        Pitch = 0,
        Yaw = 1,
        Roll = 2
    };

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

    // OPERATOR: + vec3_template
    inline vec3_template operator +(const vec3_template& operand) const
    {
        return vec3_template{
            x + operand.x,
            y + operand.y,
            z + operand.z
        };
    }

    // OPERATOR: - vec3_template
    inline vec3_template operator -(const vec3_template& operand) const
    {
        return vec3_template{
            x - operand.x,
            y - operand.y,
            z - operand.z
        };
    }

    // OPERATOR: / vec3_template
    inline vec3_template operator /(const vec3_template& operand) const
    {
        return vec3_template{
            x / operand.x,
            y / operand.y,
            z / operand.z
        };
    }

    // OPERATOR: * vec3_template
    inline vec3_template operator *(const vec3_template& operand) const
    {
        return vec3_template{
            x * operand.x,
            y * operand.y,
            z * operand.z
        };
    }

    // OPERATOR: -= vec3_template
    inline const vec3_template& operator -=(const vec3_template& operand) {
        x -= operand.x;
        y -= operand.y;
        z -= operand.z;
        return *this;
    }

    //// OPERATOR: += vec3_template
    inline const vec3_template& operator +=(const vec3_template& operand) {
        x += operand.x;
        y += operand.y;
        z += operand.z;
        return *this;
    }

    //// OPERATOR: /= vec3_template
    inline const vec3_template& operator *=(const vec3_template& operand) {
        x *= operand.x;
        y *= operand.y;
        z *= operand.z;
        return *this;
    }

    //// OPERATOR: *= vec3_template
    inline const vec3_template& operator /=(const vec3_template& operand) {
        x /= operand.x;
        y /= operand.y;
        z /= operand.z;
        return *this;
    }
};
// OPERATOR: += float
//const vec3_template& operator +=(const float& other) {
//    x += other;
//    y += other;
//    z += other;
//    return *this;
//}
typedef vec3_template<byte> bvec3_t;
typedef vec3_template<int> ivec3_t;
typedef vec3_template<float> vec3_t;
typedef vec3_template<double> dvec35_t;

//
//=============================================================================
// 
// Modern vec3_t Inline Functions:
//
//=============================================================================
//


//
//===============
// vec3_cross
// 
// Returns the cross product of 'a x b'.
//===============
//
inline vec3_t vec3_cross(const vec3_t &a, const vec3_t &b) {
    return vec3_t{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

//
//===============
// vec3_dot
// 
// Returns the dot product of 'a · b'.
//===============
//
inline float vec3_dot(const vec3_t &a, const vec3_t &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

//
//===============
// vec3_scale
// 
// Returns the vector 'v' scaled by 'scale'.
//===============
//
inline vec3_t vec3_scale(const vec3_t &v, float scale) {
    return vec3_t{
        v.x * scale,
        v.y * scale,
        v.z * scale
    };
}

//
//===============
// vec3_negate
// 
// Returns the negated vector 'v'.
//===============
//
inline vec3_t vec3_negate(const vec3_t &v) {
    return vec3_scale(v, -1.f);
}


//
//===============
// vec3_one
// 
// Returns an identity vector:
// `vec3_t { 1.f, 1.f, 1.f }`
//===============
//
inline vec3_t vec3_one(void) {
    return vec3_t{
        1.f,
        1.f,
        1.f
    };
}

//
//===============
// vec3_zero
// 
// Returns a zero point origin vector:
// `vec3_t { 0.f, 0.f, 0.f }`
//===============
//
inline const vec3_t& vec3_zero(void) {
    static const vec3_t vec = vec3_t{ 
        0.f, 
        0.f, 
        0.f
    };
    return vec;
}

//
//===============
// vec3_up
//
// Returns the up vector:
// `vec3_t { 0.f, 0.f, 1.f }`
//===============
//
inline const vec3_t vec3_up(void) {
    return vec3_t{
        0.f,
        0.f,
        1.f
    };
}

//
//===============
// vec3_down
//
// Returns the down vector - presumably vec_up, in its negated form is:
// `vec3_t { 0.f, 0.f, -1.f }`
//===============
//
inline const vec3_t vec3_down(void) {
    return vec3_negate(vec3_up());
}

//
//===============
// vec3_equalepsilon
//
// Returns true if 'a'and 'b' are equal using the specified epsilon.
//===============
//
inline qboolean vec3_equal_epsilon(const vec3_t &a, const vec3_t &b, float epsilon = FLT_EPSILON) {
    return EqualEpsilonf(a.x, b.x, epsilon) &&
        EqualEpsilonf(a.y, b.y, epsilon) &&
        EqualEpsilonf(a.z, b.z, epsilon);
}

//
//===============
// vec3_equal
//
// Returns true if 'a' and 'b' are equal.
//===============
//
inline qboolean vec3_equal(const vec3_t &a, const vec3_t &b) {
    return vec3_equal_epsilon(a, b);
}

//
//===============
// vec3_euler
//
// Returns the euler angles, in radians, for the directional vector `dir`.
//===============
//
inline const vec3_t vec3_euler(const vec3_t &dir) {
    float pitch, yaw;

    if (dir.y == 0.f && dir.x == 0.f) {
        yaw = 0.f;
        if (dir.z > 0.f) {
            pitch = 90.f;
        }
        else {
            pitch = 270.f;
        }
    }
    else {
        if (dir.x) {
            yaw = Degrees(std::atan2f(dir.y, dir.x));
        }
        else if (dir.y > 0.f) {
            yaw = 90.f;
        }
        else {
            yaw = 270.f;
        }

        if (yaw < 0.f) {
            yaw += 360.f;
        }

        const float forward = std::sqrtf(dir.x * dir.x + dir.y * dir.y);
        pitch = Degrees(std::atan2f(dir.z, forward));

        if (pitch < 0.f) {
            pitch += 360.f;
        }
    }

    return vec3_t{
        -pitch,
        yaw,
        0
    };
}

//
//===============
// vec3_to_yaw
//
// Returns the yaw angles, in in degrees for the directional vector.
//===============
//
inline const float vec3_to_yaw(const vec3_t& dir) {
    float   yaw;

    if (/*vec[YAW] == 0 &&*/ dir[vec3_t::PYR::Pitch] == 0) {
        yaw = 0.f;

        if (dir[vec3_t::PYR::Yaw] > 0) {
            yaw = 90.f;
        } else if (dir[vec3_t::PYR::Yaw] < 0) {
            yaw = -90.f;
        }
    }
    else {
        yaw = (int)(atan2(dir[vec3_t::PYR::Yaw], dir[vec3_t::PYR::Pitch]) * 180.f / M_PI);
        if (yaw < 0)
            yaw += 360.f;
    }

    return yaw;
}

//
//===============
// vec3_fabsf
//
// Returns a vector containing the absolute values of 'v'.
//===============
//
inline const vec3_t vec3_fabsf(const vec3_t &v) {
    return vec3_t{
        std::fabsf(v.x),
        std::fabsf(v.y),
        std::fabsf(v.z)
    };
}

//
//===============
// vec3_floorf
//
// Returns a vector containing the components of 'v', rounded to the nearest lower integer.
//===============
//
inline const vec3_t vec3_floorf(const vec3_t &v) {
    return vec3_t{
        std::floorf(v.x),
        std::floorf(v.y),
        std::floorf(v.z)
    };
}

//
//===============
// vec3_ceilf
//
// Returns a vector containing the components of `v`, rounded to the nearest higher integer.
//===============
//
inline const vec3_t vec3_ceilf(const vec3_t &v) {
    return vec3_t{
        ceilf(v.x),
        ceilf(v.y),
        ceilf(v.z)
    };
}

//
//===============
// vec3_clamp_euler
//
// Returns the specified Euler angles circularly clamped to '0.f - 360.f'.
//===============
//
inline const vec3_t vec3_clamp_euler(const vec3_t &euler) {
    return vec3_t{
        ClampEuler(euler.x),
        ClampEuler(euler.y),
        ClampEuler(euler.z)
    };
}

//
//===============
// vec3_fmaf
//
// Returns The vector 'v' + ('add' * 'multiply').
//===============
//
inline const vec3_t vec3_fmaf(const vec3_t &v, float multiply, const vec3_t &add) {
    return vec3_t{
        std::fmaf(add.x, multiply, v.x),
        std::fmaf(add.y, multiply, v.y),
        std::fmaf(add.z, multiply, v.z)
    };
}

//
//===============
// vec3_maxf
//
// Returns a vector containing the max components of 'a' and 'b'.
//===============
//
inline const vec3_t vec3_maxf(const vec3_t &a, const vec3_t &b) {
    return vec3_t{
        Maxf(a.x, b.x), 
        Maxf(a.y, b.y),
        Maxf(a.z, b.z)
    };
}

//
//===============
// vec3_maxs
//
// Returns the vector 'vec3_t { -FLT_MAX, -FLT_MAX, -FLT_MAX }'.
//===============
//
inline const vec3_t vec3_maxs(void) {
    return vec3_t{
        -FLT_MAX,
        -FLT_MAX,
        -FLT_MAX
    };
}

//
//
//===============
// vec3_minf
//
// Return a vector containing the min components of 'a' and 'b'.
//===============
//
inline const vec3_t vec3_minf(const vec3_t &a, const vec3_t &b) {
    return vec3_t{
        Minf(a.x, b.x),
        Minf(a.y, b.y),
        Minf(a.z, b.z)
    };
}

//
//===============
// vec3_mins
//
// Returns the vector 'vec3_t { FLT_MAX, FLT_MAX, FLT_MAX }'.
//===============
//
inline const vec3_t vec3_mins(void) {
    return vec3_t{
        FLT_MAX,
        FLT_MAX,
        FLT_MAX
    };
}

//
//===============
// vec3_mix
//
// Returns the linear interpolation of 'a' and 'b' using the specified fraction.
//===============
//
inline const vec3_t vec3_mix(const vec3_t &a, const vec3_t &b, float mix) {
    return vec3_fmaf(a, mix, b - a);
}

//
//===============
// vec3_mix_euler
//
// Returns the linear interpolation of 'a' and 'b' using the specified fraction.
//===============
//
inline const vec3_t vec3_mix_euler(const vec3_t &a, const vec3_t &b, float mix) {

    vec3_t _a = a;
    vec3_t _b = b;

    for (std::size_t i = 0; i < 3; i++) {
        if (_b.xyz[i] - _a.xyz[i] >= 180.f) {
            _a.xyz[i] += 360.f;
        } else if (_b.xyz[i] - _a.xyz[i] <= -180.f) {
            _b.xyz[i] += 360.f;
        }
    }

    return vec3_mix(_a, _b, mix);
}

//
//===============
// vec3_mix3
//
// Returns the linear interpolation of `a` and `b` using the specified fractions.
//===============
//
inline const vec3_t vec3_mix3(const vec3_t &a, const vec3_t &b, const vec3_t &mix) {
    return a + ((b - a) * mix);
}

//
//===============
// vec3_roundf
//
// Returns the vector `v` rounded to the nearest integer values.
//===============
//
inline const vec3_t vec3_roundf(const vec3_t &v) {
    return vec3_t{
        std::roundf(v.x),
        std::roundf(v.y),
        std::roundf(v.z)
    };
}

//
//===============
// vec3_clamp
//
// Returns the clamped vector `v` between vector 'min', and vector 'max'.
//===============
//
inline const vec3_t vec3_clamp(const vec3_t &v, const vec3_t &min, const vec3_t &max) {
    return vec3_t{
        Clampf(v.x, min.x, max.x),
        Clampf(v.y, min.z, max.y),
        Clampf(v.z, min.z, max.z)
    };
}

//
//===============
// vec3_clampf
//
// Returns the clamped vector `v` between float 'min', and float 'max'.
//===============
//
inline const vec3_t vec3_clampf(const vec3_t& v, float min, float max) {
    return vec3_t{
        Clampf(v.x, min, max),
        Clampf(v.y, min, max),
        Clampf(v.z, min, max)
    };
}

//
//===============
// vec3_clamp01
//
// Returns the clamped vector `v` between ranges 0. and 1.
//===============
//
inline const vec3_t vec3_clamp01(const vec3_t &v) {
    return vec3_t{
        Clampf(v.x, 0.0, 1.0),
        Clampf(v.y, 0.0, 1.0),
        Clampf(v.z, 0.0, 1.0)
    };
}

//
//===============
// vec3_reflect
//
// Returns the reflected vector of 'a' and 'b'.
//===============
//
inline const vec3_t vec3_reflect(const vec3_t &a, const vec3_t &b) {
    return a + vec3_scale(b, -2.f * vec3_dot(a, b));
}


//
//===============
// vec3_length_squared
//
// Returns the squared length (magnitude) of 'v'.
//===============
//
inline float vec3_length_squared(const vec3_t &v) {
    return vec3_dot(v, v);
}

//
//===============
// vec3_length_squared
//
// Returns the length (magnitude) of 'v'.
//===============
//
inline float vec3_length(const vec3_t &v) {
    return std::sqrtf(vec3_length_squared(v));
}

//
//===============
// vec3_normalize_length
//
// Returns the normalized vector 'v', and sets the length reference value to
// the vector 'length'.
//===============
//
inline vec3_t vec3_normalize_length(const vec3_t &v, float &length) {
    length = vec3_length(v);
    if (length > 0.f) {
        return vec3_scale(v, 1.f / length);
    }
    else {
        return vec3_zero();
    }
}

//
//===============
// vec3_normalize
//
// Returns the normalized vector 'v'.
//===============
//
inline vec3_t vec3_normalize(const vec3_t &v) {
    float length;
    return vec3_normalize_length(v, length);
}

//
//===============
// vec3_distance_direction
//
// Returns the length of `a - b` as well as the normalized directional vector.
//===============
//
inline float vec3_distance_direction(const vec3_t &a, const vec3_t &b, vec3_t& dir) {
    float length;

    dir = vec3_normalize_length(a - b, length);

    return length;
}

//
//===============
// vec3_distance_direction
//
// Returns the normalized direction vector between points a and b.
//===============
//
inline vec3_t vec3_direction(const vec3_t &a, const vec3_t &b) {
    return vec3_normalize(a - b);
}


//
//===============
// vec3_distance_squared
//
// Returns the squared length of the vector `a - b`.
//===============
//
inline float vec3_distance_squared(const vec3_t &a, const vec3_t &b) {
    return vec3_length_squared(a - b);
}

//
//===============
// vec3_distance_squared
//
// Returns the length of the vector `a - b`.
//===============
//
inline float vec3_distance(const vec3_t &a, const vec3_t &b) {
    return vec3_length(a - b);
}

//
//===============
// vec3_distance_squared
//
// Returns the forward, right and up vectors for the euler angles in radians.
//===============
//
inline void vec3_vectors(const vec3_t &euler, vec3_t *forward, vec3_t *right, vec3_t *up) {
    float sr, sp, sy, cr, cp, cy;

    SinCosRadians(Radians(euler.x), sp, cp);
    SinCosRadians(Radians(euler.y), sy, cy);
    SinCosRadians(Radians(euler.z), sr, cr);

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }

    if (right) {
        right->x = (-1.f * sr * sp * cy + -1.f * cr * -sy);
        right->y = (-1.f * sr * sp * sy + -1.f * cr * cy);
        right->z = -1.f * sr * cp;
    }

    if (up) {
        up->x = (cr * sp * cy + -sr * -sy);
        up->y = (cr * sp * sy + -sr * cy);
        up->z = cr * cp;
    }
}

//
//===============
// vec3_to_str
//
// Slight convenience function, for simplicity.
//===============
//
inline const std::string vec3_to_str(const vec3_t& v, qboolean rounded = true)
{
    std::ostringstream sstream;

    if (rounded)
        sstream << "(" << static_cast<int>(v.x) << " " << static_cast<int>(v.y) << " " << static_cast<int>(v.z) << ")";
    else
        sstream << "(" << v.x << " " << v.y << " " << v.z << ")";

    return sstream.str();
}

//
//===============
// vec3_to_cstr
//
// Converts a vec3 into a C string
//===============
//
inline const char* vec3_to_cstr( const vec3_t& v, qboolean rounded = true )
{
    return vec3_to_str( v, rounded ).c_str();
}


//
//=============================================================================
// 
// Legacy Vector Macro Functions:
//
//=============================================================================
//
#define DotProduct(x,y)         ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define CrossProduct(v1,v2,cross) \
        ((cross)[0]=(v1)[1]*(v2)[2]-(v1)[2]*(v2)[1], \
         (cross)[1]=(v1)[2]*(v2)[0]-(v1)[0]*(v2)[2], \
         (cross)[2]=(v1)[0]*(v2)[1]-(v1)[1]*(v2)[0])
#define VectorSubtract(a,b,c) \
        ((c)[0]=(a)[0]-(b)[0], \
         (c)[1]=(a)[1]-(b)[1], \
         (c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c) \
        ((c)[0]=(a)[0]+(b)[0], \
         (c)[1]=(a)[1]+(b)[1], \
         (c)[2]=(a)[2]+(b)[2])
#define VectorAdd3(a,b,c,d) \
        ((d)[0]=(a)[0]+(b)[0]+(c)[0], \
         (d)[1]=(a)[1]+(b)[1]+(c)[1], \
         (d)[2]=(a)[2]+(b)[2]+(c)[2])
#define VectorCopy(a,b)     ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define VectorClear(a)      ((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)   ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorInverse(a)    ((a)[0]=-(a)[0],(a)[1]=-(a)[1],(a)[2]=-(a)[2])
#define VectorSet(v, x, y, z)   ((v)[0]=(x),(v)[1]=(y),(v)[2]=(z))
#define VectorAverage(a,b,c) \
        ((c)[0]=((a)[0]+(b)[0])*0.5f, \
         (c)[1]=((a)[1]+(b)[1])*0.5f, \
         (c)[2]=((a)[2]+(b)[2])*0.5f)
#define VectorMA(a,b,c,d) \
        ((d)[0]=(a)[0]+(b)*(c)[0], \
         (d)[1]=(a)[1]+(b)*(c)[1], \
         (d)[2]=(a)[2]+(b)*(c)[2])
#define VectorVectorMA(a,b,c,d) \
        ((d)[0]=(a)[0]+(b)[0]*(c)[0], \
         (d)[1]=(a)[1]+(b)[1]*(c)[1], \
         (d)[2]=(a)[2]+(b)[2]*(c)[2])
#define VectorEmpty(v) ((v)[0]==0&&(v)[1]==0&&(v)[2]==0)
#define VectorCompare(v1,v2)    ((v1)[0]==(v2)[0]&&(v1)[1]==(v2)[1]&&(v1)[2]==(v2)[2])
#define VectorLength(v)     (std::sqrtf(DotProduct((v),(v))))
#define VectorLengthSquared(v)      (DotProduct((v),(v)))
#define VectorScale(in,scale,out) \
        ((out)[0]=(in)[0]*(scale), \
         (out)[1]=(in)[1]*(scale), \
         (out)[2]=(in)[2]*(scale))
#define VectorVectorScale(in,scale,out) \
        ((out)[0]=(in)[0]*(scale)[0], \
         (out)[1]=(in)[1]*(scale)[1], \
         (out)[2]=(in)[2]*(scale)[2])
#define DistanceSquared(v1,v2) \
        (((v1)[0]-(v2)[0])*((v1)[0]-(v2)[0])+ \
        ((v1)[1]-(v2)[1])*((v1)[1]-(v2)[1])+ \
        ((v1)[2]-(v2)[2])*((v1)[2]-(v2)[2]))
#define Distance(v1,v2) (std::sqrtf(DistanceSquared(v1,v2)))
#define LerpAngles(a,b,c,d) \
        ((d)[0]=LerpAngle((a)[0],(b)[0],c), \
         (d)[1]=LerpAngle((a)[1],(b)[1],c), \
         (d)[2]=LerpAngle((a)[2],(b)[2],c))
#define LerpVector(a,b,c,d) \
    ((d)[0]=(a)[0]+(c)*((b)[0]-(a)[0]), \
     (d)[1]=(a)[1]+(c)*((b)[1]-(a)[1]), \
     (d)[2]=(a)[2]+(c)*((b)[2]-(a)[2]))
#define LerpVector2(a,b,c,d,e) \
    ((e)[0]=(a)[0]*(c)+(b)[0]*(d), \
     (e)[1]=(a)[1]*(c)+(b)[1]*(d), \
     (e)[2]=(a)[2]*(c)+(b)[2]*(d))
#define PlaneDiff(v,p)   (DotProduct(v,(p)->normal)-(p)->dist)

#define Vector2Subtract(a,b,c)  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1])
#define Vector2Add(a,b,c)       ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1])

//---------------------------------------------------------------------------------

void SetupRotationMatrix(vec3_t* matrix, const vec3_t& dir, float degrees);
void RotatePointAroundVector(vec3_t& dst, const vec3_t& dir, const vec3_t& point, float degrees);
void ProjectPointOnPlane(vec3_t& dst, const vec3_t& p, const vec3_t& normal);
void PerpendicularVector(vec3_t& dst, const vec3_t& src);

void AngleVectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up);
vec_t VectorNormalize(vec3_t &v);        // returns vector length
vec_t VectorNormalize2(const vec3_t& v, vec3_t& out);
vec_t VectorNormalize2(const vec3_t& v, vec_t *out);
void ClearBounds(vec3_t& mins, vec3_t& maxs);
void AddPointToBounds(const vec3_t& v, vec3_t& mins, vec3_t& maxs);
vec_t RadiusFromBounds(const vec3_t& mins, const vec3_t& maxs);
void UnionBounds(vec3_t* a, vec3_t* b, vec3_t* c);


void vectoangles2(const vec3_t& value1, vec3_t& angles);

void MakeNormalVectors(const vec3_t& forward, vec3_t& right, vec3_t& up);

//
//===============
// AnglesToAxis
// 
// Set angles to axis.
//===============
//
inline void AnglesToAxis(const vec3_t& angles, vec3_t* axis)
{
    AngleVectors(angles, &axis[0], &axis[1], &axis[2]);
    VectorInverse(axis[1]);
}

//
//===============
// TransposeAxis
// 
// Transpoes the vector axis.
//===============
//
inline void TransposeAxis(vec3_t *axis)
{
    vec_t temp;

    temp = axis[0].xyz[1];
    axis[0].xyz[1] = axis[1].xyz[0];
    axis[1].xyz[0] = temp;

    temp = axis[0].xyz[2];
    axis[0].xyz[2] = axis[2].xyz[0];
    axis[2].xyz[0] = temp;

    temp = axis[1].xyz[2];
    axis[1].xyz[2] = axis[2].xyz[1];
    axis[2].xyz[1] = temp;
}

//
//===============
// RotatePoint
// 
// Rotate point around axis.
//===============
//
inline void RotatePoint(vec3_t& point, vec3_t* axis)
{
    vec3_t temp = point;

    VectorCopy(point, temp);
    point.xyz[0] = DotProduct(temp, axis[0]);
    point.xyz[1] = DotProduct(temp, axis[1]);
    point.xyz[2] = DotProduct(temp, axis[2]);
}

#endif // __INC__SHARED__MATH__VECTOR3_H__