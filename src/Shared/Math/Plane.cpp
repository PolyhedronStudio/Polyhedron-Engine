// LICENSE HERE.

//
// plane.cpp
//
#include "Shared/Shared.h"

void SetPlaneType(cplane_t* plane)
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

void SetPlaneSignbits(cplane_t* plane)
{
    int bits = 0;

    if (plane->normal.xyz[0] < 0) {
        bits |= 1;
    }
    if (plane->normal.xyz[1] < 0) {
        bits |= 2;
    }
    if (plane->normal.xyz[2] < 0) {
        bits |= 4;
    }

    plane->signbits = bits;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide(const vec3_t& emins, const vec3_t& emaxs, cplane_t* p)
{
    const vec_t* bounds[2] = { emins, emaxs };
    int     i = p->signbits & 1;
    int     j = (p->signbits >> 1) & 1;
    int     k = (p->signbits >> 2) & 1;

#define P(i, j, k) \
    p->normal[0] * bounds[i][0] + \
    p->normal[1] * bounds[j][1] + \
    p->normal[2] * bounds[k][2]

    vec_t   dist1 = P(i ^ 1, j ^ 1, k ^ 1);
    vec_t   dist2 = P(i, j, k);
    int     sides = 0;

#undef P

    if (dist1 >= p->dist)
        sides = BOX_INFRONT;
    if (dist2 < p->dist)
        sides |= BOX_BEHIND;

    return sides;
}