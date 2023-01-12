// LICENSE HERE.

//
// plane.cpp
//
#include "../Shared.h"

void SetPlaneType(CollisionPlane* plane)
{
	if (!plane) {
		return;
	}
	
	const vec3_t normal = plane->normal;

	const float x = fabsf(normal.x);
	if (x > 1.f - FLT_EPSILON) {
		plane->type = PLANE_X;
		return;
	}

	const float y = fabsf(normal.y);
	if (y > 1.f - FLT_EPSILON) {
		plane->type = PLANE_Y;
		return;
	}

	const float z = fabsf(normal.z);
	if (z > 1.f - FLT_EPSILON) {
		plane->type = PLANE_Z;
		return;
	}

	if (x >= y && x >= z) {
		plane->type = PLANE_ANYX;
		return;
	}
	if (y >= x && y >= z) {
		plane->type = PLANE_ANYY;
		return;
	}

	plane->type = PLANE_ANYZ;

    //if (plane->normal[0] == 1) {
    //    plane->type = PLANE_X;
    //    return;
    //}
    //if (plane->normal[1] == 1) {
    //    plane->type = PLANE_Y;
    //    return;
    //}
    //if (plane->normal[2] == 1) {
    //    plane->type = PLANE_Z;
    //    return;
    //}

    //plane->type = PLANE_NON_AXIAL;
}

void SetPlaneSignbits(CollisionPlane* plane)
{
	if (!plane) {
		return;
	}

    //if (plane->normal.xyz[0] < 0) {
    //    bits |= 1;
    //}
    //if (plane->normal.xyz[1] < 0) {
    //    bits |= 2;
    //}
    //if (plane->normal.xyz[2] < 0) {
    //    bits |= 4;
    //}

    //plane->signBits = bits;

	// For fast box on planeside test
    int32_t bits = 0;
	for(int32_t i = 0; i < 3; i++ ) {
		if( plane->normal[i] < 0.0f ) {
			bits |= (1 << i);
		}
	}
	plane->signBits = bits;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
const int32_t BoxOnPlaneSide( const bbox3_t& ebounds, CollisionPlane* p ) {
    // Fast axial cases.
    if ( p->type < 3 ) {
		if ( ebounds.mins.xyz[ p->type ] - p->dist >= 0.f ) {
			return BoxPlane::InFront;
		}
		if ( ebounds.maxs.xyz[ p->type ] - p->dist < 0.f ) {
			return BoxPlane::Behind;
		}

        return BoxPlane::Intersects;
    }

	// Slower, more complex case.
	float dist1 = 0.f;
	float dist2 = 0.f;
	switch ( p->signBits ) {
		case 0:
			dist1 = vec3_dot( p->normal, ebounds.maxs );
			dist2 = vec3_dot( p->normal, ebounds.mins );
			break;
		case 1:
			dist1 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.maxs.z;
			dist2 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.mins.z;
			break;
		case 2:
			dist1 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.maxs.z;
			dist2 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.mins.z;
			break;
		case 3:
			dist1 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.maxs.z;
			dist2 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.mins.z;
			break;
		case 4:
			dist1 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.mins.z;
			dist2 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.maxs.z;
			break;
		case 5:
			dist1 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.mins.z;
			dist2 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.maxs.z;
			break;
		case 6:
			dist1 = p->normal.x * ebounds.maxs.x + p->normal.y * ebounds.mins.y + p->normal.z * ebounds.mins.z;
			dist2 = p->normal.x * ebounds.mins.x + p->normal.y * ebounds.maxs.y + p->normal.z * ebounds.maxs.z;
			break;
		case 7:
			dist1 = vec3_dot(p->normal, ebounds.mins);
			dist2 = vec3_dot(p->normal, ebounds.maxs);
			break;
		default:
			dist1 = dist2 = 0.0; // shut up compiler
			break;
	}

	int32_t side = 0;

	if ( dist1 - p->dist >= 0.f ) {
		side = BoxPlane::InFront;
	}
	if ( dist2 - p->dist <  0.f ) {
		side |= BoxPlane::Behind;
	}

	return side;

//    const vec_t* bounds[2] = { ebounds.mins, ebounds.maxs };
//    int     i = p->signBits & 1;
//    int     j = (p->signBits >> 1) & 1;
//    int     k = (p->signBits >> 2) & 1;
//
//#define P(i, j, k) \
//    p->normal[0] * bounds[i][0] + \
//    p->normal[1] * bounds[j][1] + \
//    p->normal[2] * bounds[k][2]
//
//    vec_t   dist1 = P(i ^ 1, j ^ 1, k ^ 1);
//    vec_t   dist2 = P(i, j, k);
//    int     sides = 0;
//
//#undef P
//
//	if (p->signBits == 0) {
//		dist1 = vec3_dot( p->normal, ebounds.maxs );
//		dist2 = vec3_dot( p->normal, ebounds.mins );
//	}
//	if (p->signBits == 7) {
//		dist1 = vec3_dot( p->normal, ebounds.mins );
//		dist2 = vec3_dot( p->normal, ebounds.maxs );
//	}
//
//    if (dist1 >= p->dist)
//        sides = BoxPlane::InFront;
//    if (dist2 < p->dist)
//        sides |= BoxPlane::Behind;
//
//    return sides;
}