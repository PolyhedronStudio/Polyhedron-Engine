// LICENSE HERE.

//
// shared/math/plane.h
//
// Plane math library implementation.
//
#ifndef __INC_SHARED_MATH_PLANE_H__
#define __INC_SHARED_MATH_PLANE_H__

//-----------------
// plane_t structure
//-----------------
typedef struct cplane_s {
    vec3_t  normal;
    float   dist;
    byte    type;           // for fast side tests
    byte    signbits;       // signx + (signy<<1) + (signz<<1)
    byte    pad[2];
} cplane_t;

//-----------------
// Planes.
//-----------------
// 0-2 are axial planes
#define PLANE_X         0 // TODO: Change to cplane_t::PLANE::X?
#define PLANE_Y         1
#define PLANE_Z         2
// 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYX      3
#define PLANE_ANYY      4
#define PLANE_ANYZ      5
// planes (x&~1) and (x&~1)+1 are always opposites
#define PLANE_NON_AXIAL 6

//-----------------
// Structure offset for asm code
//-----------------
#define CPLANE_NORMAL_X         0
#define CPLANE_NORMAL_Y         4
#define CPLANE_NORMAL_Z         8
#define CPLANE_DIST             12
#define CPLANE_TYPE             16
#define CPLANE_SIGNBITS         17
#define CPLANE_PAD0             18
#define CPLANE_PAD1             19

//-----------------
// Defined in shared/math/plane.cpp
//-----------------
void SetPlaneType(cplane_t* plane);
void SetPlaneSignbits(cplane_t* plane);

int BoxOnPlaneSide(const vec3_t& emins, const vec3_t& emaxs, cplane_t* p);

//
//===============
// Plane_Difference
// 
//===============
//
static inline vec_t Plane_Difference(const vec3_t& v, cplane_s* p) {
    return DotProduct(v, p->normal) - p->dist;
}

//
//===============
// Plane_FastDifference
// 
// Returns a fast difference if available for the plane.
//===============
//
static inline vec_t Plane_FastDifference(const vec3_t& v, cplane_t* p)
{
    // fast axial cases
    if (p->type < 3) {
        return v.xyz[p->type] - p->dist;
    }

    // slow generic case
    return Plane_Difference(v, p);
}

//
//===============
// BoxOnPlaneSideFast
// 
//===============
//
#define BOX_INFRONT     1 // TODO: Move into PLANE::BOX::INFRONT?
#define BOX_BEHIND      2
#define BOX_INTERSECTS  3

static inline int BoxOnPlaneSideFast(const vec3_t& emins, const vec3_t& emaxs, cplane_t* p)
{
    // fast axial cases
    if (p->type < 3) {
        if (p->dist <= emins.xyz[p->type])
            return BOX_INFRONT;
        if (p->dist >= emaxs.xyz[p->type])
            return BOX_BEHIND;
        return BOX_INTERSECTS;
    }

    // slow generic case
    return BoxOnPlaneSide(emins, emaxs, p);
}

// Wrapper for legacy code.
//static inline int BoxOnPlaneSideFast(const float *emins, const float *emaxs, cplane_t* p) {
//    
//
//}
#endif // __INC_SHARED_MATH_PLANE_H__

