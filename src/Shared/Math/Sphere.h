/***
*
*	License here.
*
*	@file
*
*	Sphere(vec3_t based) class and operation implementations.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"


/**
*	@brief	Sphere Data Type: Contains the parameters of a sphere, set in 'World Space' at its 'origin',
*			applying a 'Model Space' offset from its 'origin' in order to also be able to apply for use
*			with 'Capsules/Pill Shaped' maths.
**/
struct sphere_t {
	//! Radius of the sphere.
	float radius = 0.f;
	//! Radius used for calculating offset of the sphere's with.
	float offsetRadius = 0.f;

	//! The halfHeight from offset.
	float halfHeight = 0.f;
	//! The halfWidth from offset.
	float halfWidth = 0.f;

	//! The the sphere 'World Space' from its origin.
	vec3_t origin = vec3_zero();

	//! The 'offset' of the sphere in 'Model Space' from its origin.
	vec3_t offset = vec3_zero();
};



/**
*
*	Testing:
*
**/
/**
*	@brief	Calculates a spherical collision shape from a 'size' vector, for use with sphere/capsule hull tracing.
**/
const sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin );
/**
*	@brief	Appropriately centers a sphere's offset to rotation.
**/
void sphere_calculate_offset_rotation( const glm::mat4 &matTransform, const glm::mat4 &matInvTransform, sphere_t &sphere, const bool isTransformed  );

/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with capsule hull tracing.
**/
const sphere_t capsule_sphere_from_size( const vec3_t &size, const vec3_t &origin );
/**
*	@brief	Appropriately centers a capsule's offset to rotation.
**/
void capsule_calculate_offset_rotation( const glm::mat4 &matTransform, const glm::mat4 &matInvTransform, sphere_t &sphere, const bool isTransformed );




/**
*
*
*	Sphere Function Implementations:
*
*
**/
/**
*	@brief	Returns true if the point lies within the sphere.
**/
inline const bool sphere_contains_point( const sphere_t &sphere, const vec3_t &point ) {
	const vec3_t sphereOffsetOrigin = sphere.origin + sphere.offset;
	return vec3_dot( sphereOffsetOrigin - point ) <= flt_square( sphere.radius );
}

/**
*	@brief	Returns true if the point lies within the circular 2D(X/Y) of the sphere sphere.
**/
inline const bool sphere_contains_point_2d( const sphere_t &sphere, const vec3_t &point ) {
	const vec3_t sphereOffsetOrigin = sphere.origin + sphere.offset;
	return vec3_dot( vec3_xy( sphereOffsetOrigin ) - vec3_xy( point ) ) <= flt_square( sphere.radius );
}
















/***
*
*
*	TODO: These are used for "Method A".
*
*
***/
/**
*	@brief
**/
inline const float plane_distance( const CollisionPlane &p, const vec3_t &point, const float extraDistanceOffset = 0.f ) {
	const vec3_t planeOrigin = vec3_scale( p.normal, p.dist + extraDistanceOffset );
	return vec3_dot( point - planeOrigin, p.normal );
}
/**
*	@brief
**/
inline const bool sphere_inside_plane( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius ) {
	return -plane_distance( plane, sphereOrigin ) > sphereRadius;
}
/**
*	@brief
**/
inline const bool sphere_outside_plane( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius ) {
	return plane_distance( plane, sphereOrigin ) > sphereRadius;
}
/**
*	@brief
**/
inline const bool sphere_intersects_plane_point( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius ) {
	return fabs( plane_distance( plane, sphereOrigin ) ) <= sphereRadius;
}


/***
*
*
*	Sphere VS Plane Method B:
*
*
***/
/**
*	@brief	Projects the sphere onto the plane.
**/
inline const float sphere_plane_project( const vec3_t &spherePos, const CollisionPlane & plane ) {
	return vec3_dot( spherePos, plane.normal ) + plane.dist;
}
/**
*	@brief	Returns the distance between the sphere and the plane.
**/
inline const float sphere_plane_collision_distance( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane ) {
		return fabs( sphere_plane_project( point, plane ) / sqrtf( vec3_dot( plane.normal, plane.normal ) ) );
		// plane.normal.x * plane.normal.x + plane.normal.y * plane.normal.y + plane.normal.z * plane.normal.z ) 	
}
/**
*	@brief	Uses the distances between the sphere and the plane to return true/false.
**/
inline const bool sphere_plane_collision( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane ) {
		return sphere_plane_collision_distance( point, sphere, plane ) < sphere.radius;
}

/**
*	@brief
**/
//const bool sphere_intersects_plane_point( const sphere_t &sphere, const CollisionPlane &p, vec3_t &point, float &radius ) {
inline const bool sphere_intersects_plane_point( const vec3_t &sphereOrigin, const float sphereRadius, const CollisionPlane &plane, vec3_t &hitPoint, float &hitRadius, float &hitDistance ) {
	//float d = plane_distance( p, sphereOrigin );
	//vec3 proj = vec3_mul(p.direction, d);
	//*point = vec3_sub(s.center, proj);
	//*radius = sqrtf(max(s.radius * s.radius - d * d, 0));
	//return fabs(d) <= s.radius; 

	const float distance = plane_distance( plane, sphereOrigin );
	const vec3_t projection = vec3_scale( plane.normal, distance );
	hitPoint = sphereOrigin - projection;
	hitRadius = sqrtf( Maxf( sphereRadius * sphereRadius - distance * distance, 0 ) );
	hitDistance = distance;
	return fabs( distance ) <= sphereRadius;
}