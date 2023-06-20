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