/***
*
*	License here.
*
*	@file
*
*	Math Lib: Plane Implementation.
* 
***/
#pragma once

#include "Shared/Shared.h"

//-----------------
// plane_t structure
//-----------------
typedef struct cplane_s {
    vec3_t  normal = vec3_zero();
    float   dist = 0;
    byte    type = 0;           //! For fast side tests.
    byte    signBits = 0;       //! signx + (signy<<1) + (signz<<1)
    byte    pad[2] = {};
} CollisionPlane;

//-----------------
// Planes.
//-----------------
// 0-2 are axial planes
#define PLANE_X         0 // TODO: Change to CollisionPlane::PLANE::X?
#define PLANE_Y         1
#define PLANE_Z         2
// 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYX      3
#define PLANE_ANYY      4
#define PLANE_ANYZ      5
// planes (x&~1) and (x&~1)+1 are always opposites
#define PLANE_NON_AXIAL 6


//-----------------
// Defined in Shared/Math/plane.cpp
//-----------------
void SetPlaneType(CollisionPlane* plane);
void SetPlaneSignbits(CollisionPlane* plane);


const int32_t BoxOnPlaneSide( const bbox3_t& ebounds, CollisionPlane* p );

//
//===============
// Plane_Difference
// 
//===============
//
static inline vec_t Plane_Difference(const vec3_t& v, CollisionPlane* p) {
    return DotProduct(v, p->normal) - p->dist;
}

/**
*   @return  Returns a fast difference if available for the plane.
**/
static inline vec_t Plane_FastDifference(const vec3_t& v, CollisionPlane* p)
{
    // fast axial cases
    //if (p->type < 3) {
    //    return v.xyz[p->type] - p->dist;
    //}

    // slow generic case
    return Plane_Difference(v, p);
}

/**
*   Results for BoxPlane functions.
**/
struct BoxPlane {
    static constexpr int32_t InFront    = 1;
    static constexpr int32_t Behind     = 2;
    static constexpr int32_t Intersects = 3;
};

/**
*   @brief  Tries to perform fast axial tests before resorting to the slower more thorough Box On Plane Side function.
**/
static inline const int32_t BoxOnPlaneSideFast( const bbox3_t &ebounds, CollisionPlane* p)
{
    // Fast axial cases.
    //if (p->type < 3) {
    //    if (p->dist <= ebounds.mins.xyz[p->type])
    //        return BoxPlane::InFront;
    //    if (p->dist >= ebounds.maxs.xyz[p->type])
    //        return BoxPlane::Behind;

    //    return BoxPlane::Intersects;
    //}

    // Slow generic case.
    return BoxOnPlaneSide( ebounds, p );
}
