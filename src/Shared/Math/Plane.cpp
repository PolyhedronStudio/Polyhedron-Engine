// LICENSE HERE.

//
// plane.cpp
//
#include "Shared/Shared.h"

void SetPlaneType(CollisionPlane* plane)
{
    // VEC3_T: !! May be broken, pointer issue.
    vec3_t* normal = &plane->normal;

    if (normal->xyz[0] == 1) {
        plane->type = PLANE_X;
        return;
    }
    if (normal->xyz[1] == 1) {
        plane->type = PLANE_Y;
        return;
    }
    if (normal->xyz[2] == 1) {
        plane->type = PLANE_Z;
        return;
    }

    plane->type = PLANE_NON_AXIAL;
}

void SetPlaneSignbits(CollisionPlane* plane)
{
    int32_t bits = 0;

    if (plane->normal.xyz[0] < 0) {
        bits |= 1;
    }
    if (plane->normal.xyz[1] < 0) {
        bits |= 2;
    }
    if (plane->normal.xyz[2] < 0) {
        bits |= 4;
    }

    plane->signBits = bits;

	// For fast box on planeside test
    /*plane->signBits = 0;
	for(int32_t j = 0; j < 3; j++ ) {
		if( plane->normal[j] < 0 ) {
			plane->signBits |= (1 << j);
		}
	}*/
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide(const vec3_t& emins, const vec3_t& emaxs, CollisionPlane* p)
{
    const vec_t* bounds[2] = { emins, emaxs };
    int     i = p->signBits & 1;
    int     j = (p->signBits >> 1) & 1;
    int     k = (p->signBits >> 2) & 1;

#define P(i, j, k) \
    p->normal[0] * bounds[i][0] + \
    p->normal[1] * bounds[j][1] + \
    p->normal[2] * bounds[k][2]

    vec_t   dist1 = P(i ^ 1, j ^ 1, k ^ 1);
    vec_t   dist2 = P(i, j, k);
    int     sides = 0;

#undef P

    if (dist1 >= p->dist)
        sides = BoxPlane::InFront;
    if (dist2 < p->dist)
        sides |= BoxPlane::Behind;

    return sides;
}