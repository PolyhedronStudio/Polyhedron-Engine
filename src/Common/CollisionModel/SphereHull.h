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
mnode_t *CM_HeadnodeForSphere( const vec3_t &mins, const sphere_t &sphere, const vec3_t &maxs );

/**
*	@return	A standalone CapsuleHull
**/
SphereHull CM_NewSphereHull( const bbox3_t &bounds, const sphere_t &sphere, const int32_t contents );