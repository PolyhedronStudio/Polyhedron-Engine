/***
*
*	License here.
*
*	@file
*
*	Collision Model:	General Capsule Hull API - Creates a headnode for capsule tracing general non
*						brush entities that are capsule shaped.
*
***/
#pragma once

struct sphere_t;
struct bisphere_t;
//// Sphere containing start and end radius.
//struct bisphere_t {
//	float startRadius = 0.f;
//	float endRadius = 0.f;
//};
//// The actual sphere, used for oriented capsule collision detection.
//struct sphere_t {
//	float radius = 0.f;
//	float halfHeight = 0.f;
//	vec3_t offset = vec3_zero();
//};

// CapsuleHull containing a Bounding Box as well as the actual sphere/bisphere parts.
struct CapsuleHull {
	//
	// The 'Box' surrounding this capsule. General Box Hull structure.
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
	// Actual Capsule aspect.
	//
	sphere_t sphere;
	bisphere_t biSphere;
};

// BiSphereHull - Supplements capsulehull
struct BiSphereHull {
	//
	// The 'Box' surrounding this BiSphere. General Box Hull structure.
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
	// Actual BiSphere aspect.
	//
	bisphere_t biSphere;
};

/**
*   @brief  
**/
void CM_InitCapsuleHull( );
/**
*   @brief  
**/
mnode_t *CM_HeadnodeForCapsule( const vec3_t &mins , const vec3_t &maxs );