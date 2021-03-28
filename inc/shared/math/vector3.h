// LICENSE HERE.

//
// shared/math/vector3.h
//
// N&C Math Library: Vector3
// 
// Functions that state LEGACY: should not be used, they have a viable more
// preferenced alternative to use.
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

    // Copy from.
    vec3_template(const vec3_template<T>& v) { x = v.x; y = v.y; z = v.z; }

    // Regular *vec_t support.
    vec3_template(T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }
    vec3_template(const T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }

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

    //// OPERATOR: -= float
    //const vec3_template& operator -=(const float& other) {
    //    x += other;
    //    y += other;
    //    z += other;
    //    return *this;
    //}

    //// OPERATOR: -= vec3_template
    //const vec3_template& operator -=(const vec3_template& other) {
    //    x -= other.x;
    //    y -= other.y;
    //    z -= other.z;
    //    return *this;
    //}

    //// OPERATOR: += float
    //const vec3_template& operator +=(const float& other) {
    //    x += other;
    //    y += other;
    //    z += other;
    //    return *this;
    //}

    //// OPERATOR: += vec3_template
    //const vec3_template& operator +=(const vec3_template& other) {
    //    x += other.x;
    //    y += other.y;
    //    z += other.z;
    //    return *this;
    //}
};
typedef vec3_template<byte> bvec3_t;
typedef vec3_template<int> ivec3_t;
typedef vec3_template<float> vec3_t;
typedef vec3_template<double> dvec3_t;

// Legacy Vector Macro Functions:
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
#define VectorLength(v)     (sqrt(DotProduct((v),(v))))
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
#define Distance(v1,v2) (sqrt(DistanceSquared(v1,v2)))
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

//---------------------------------------------------------------------------------

void SetupRotationMatrix(vec3_t* matrix, const vec3_t& dir, float degrees);
void RotatePointAroundVector(vec3_t& dst, const vec3_t& dir, const vec3_t& point, float degrees);
void ProjectPointOnPlane(vec3_t& dst, const vec3_t& p, const vec3_t& normal);
void PerpendicularVector(vec3_t& dst, const vec3_t& src);

void AngleVectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up);
vec_t VectorNormalize(vec3_t& v);        // returns vector length
vec_t VectorNormalize2(const vec3_t& v, vec3_t& out);
vec_t VectorNormalize2(const vec3_t& v, vec_t *out);
void ClearBounds(vec3_t& mins, vec3_t& maxs);
void AddPointToBounds(const vec3_t& v, vec3_t& mins, vec3_t& maxs);
vec_t RadiusFromBounds(const vec3_t& mins, const vec3_t& maxs);
void UnionBounds(vec3_t* a, vec3_t* b, vec3_t* c);


void vectoangles2(const vec3_t& value1, vec3_t& angles);

void MakeNormalVectors(const vec3_t& forward, vec3_t& right, vec3_t& up);

int DirToByte(const vec3_t& dir);

//
//===============
// AnglesToAxis
// 
// Set angles to axis.
//===============
//
static inline void AnglesToAxis(const vec3_t& angles, vec3_t* axis)
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
static inline void TransposeAxis(vec3_t* axis)
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
static inline void RotatePoint(vec3_t& point, vec3_t* axis)
{
    vec3_t temp;

    VectorCopy(point, temp);
    point.xyz[0] = DotProduct(temp, axis[0]);
    point.xyz[1] = DotProduct(temp, axis[1]);
    point.xyz[2] = DotProduct(temp, axis[2]);
}

#endif // __INC__SHARED__MATH__VECTOR3_H__