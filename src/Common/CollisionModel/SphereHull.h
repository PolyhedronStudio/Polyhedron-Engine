/***
*
*	License here.
*
*	@file
*
*	Collision Model:	General Sphere Hull API - Creates a headnode for sphere tracing general non
*						brush entities that are sphere shaped.
*
***/
#pragma once



/***
*
*
*	Bi-Sphere Utilities
*
*
***/
struct bisphere_t {
	float startRadius = 0.f;
	float endRadius = 0.f;
};



/***
*
*
*	Sphere Utilities
*
*
***/
/**
*	@brief	Sphere data type, offset is origin from 0,0,0
**/
struct sphere_t {
	//! Actual sphere radius from offset.
	float radius = 0.f;
	//! The halfHeight from offset.
	float halfHeight = 0.f;
	//! The halfWidth from offset.
	float halfWidth = 0.f;

	//! The offset of the sphere. (i.e, its centered origin in world space.)
	vec3_t offset = vec3_zero();
};

/**
*	@brief	Returns true if the point lies within the sphere.
**/
inline const bool sphere_contains_point( const sphere_t &sphere, const vec3_t &point ) {
	return vec3_dot( sphere.offset - point ) <= flt_square( sphere.radius );
}

/**
*	@brief	Returns true if the point lies within the circular 2D(X/Y) of the sphere sphere.
**/
inline const bool sphere_contains_point_2d( const sphere_t &sphere, const vec3_t &point ) {
	return vec3_dot( vec3_xy( sphere.offset ) - vec3_xy( point ) ) <= flt_square( sphere.radius );
}



///////////////////////////
// Sphere containing start and end radius.

//// The actual sphere, used for oriented sphere collision detection.
//struct sphere_t {
//	float radius = 0.f;
//	float halfHeight = 0.f;
//	vec3_t offset = vec3_zero();
//};
///////////////////////////

// SphereHull containing a Bounding Box as well as the actual sphere/bisphere parts.
struct SphereHull {
	//
	// The 'Box' surrounding this sphere. General Box Hull structure.
	//
    CollisionPlane planes[12];
    mnode_t  nodes[6];
    mnode_t  *headNode;
    mbrush_t brush;
    mbrush_t *leafBrush;
    mbrushside_t brushSides[6];
    mleaf_t  leaf;
    mleaf_t  emptyLeaf;

	//
	// Actual Sphere aspect.
	//
	sphere_t sphere;
	//bisphere_t biSphere;
};

/**
*   @brief  
**/
void CM_InitSphereHull( );
/**
*   @brief  
**/
mnode_t *CM_HeadnodeForSphere( const vec3_t &mins , const vec3_t &maxs );