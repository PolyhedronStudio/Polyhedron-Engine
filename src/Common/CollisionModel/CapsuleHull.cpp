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
#include "Shared/Shared.h"

#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CVar.h"
#include "Common/CollisionModel.h"
#include "Common/Common.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

#include "Common/CollisionModel/SphereHull.h"
#include "Common/CollisionModel/CapsuleHull.h"
#include "Common/CollisionModel/Tracing.h"



//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
CapsuleHull capsuleHull = {};

// TODO: Obvious, clean this up
sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin = vec3_zero() );
sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );
sphere_t bbox3_to_capsule( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );

/**
*   @brief   
**/
void CM_InitCapsuleHull( ) {
    capsuleHull.headNode = &capsuleHull.nodes[0];

    capsuleHull.brush.numsides = 6;
    capsuleHull.brush.firstbrushside = &capsuleHull.brushSides[0];
    capsuleHull.brush.contents = BrushContents::Monster;

    capsuleHull.leaf.contents = BrushContents::Monster;
    capsuleHull.leaf.firstleafbrush = &capsuleHull.leafBrush;
    capsuleHull.leaf.numleafbrushes = 1;

    capsuleHull.leafBrush = &capsuleHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &capsuleHull.brushSides[i];
        brushSide->plane = &capsuleHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &capsuleHull.nodes[i];
        node->plane = &capsuleHull.planes[i * 2];
        node->children[side] = (mnode_t *)&capsuleHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &capsuleHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&capsuleHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &capsuleHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &capsuleHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }
}

/**
*   @brief   
**/
mnode_t *CM_HeadnodeForCapsule( const bbox3_t &bounds, const int32_t contents ) {
	// Get the radius from the bounds.
	const float radius = bbox3_radius( bounds );

	// Set Brush and Leaf contents
	capsuleHull.brush.contents = contents;
	capsuleHull.leaf.contents = contents;
	
	// Calculate half size for mins and maxs.
	const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

    const vec3_t size[2] = {
        bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
        bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
    };
	
	const vec3_t mins = size[0];
	const vec3_t maxs = size[1];
	
	capsuleHull.headNode->bounds = capsuleHull.leaf.bounds = bounds;

    capsuleHull.planes[0].dist = bounds.maxs[0];
    capsuleHull.planes[1].dist = -bounds.maxs[0];
    capsuleHull.planes[2].dist = bounds.mins[0];
    capsuleHull.planes[3].dist = -bounds.mins[0];
    capsuleHull.planes[4].dist = bounds.maxs[1];
    capsuleHull.planes[5].dist = -bounds.maxs[1];
    capsuleHull.planes[6].dist = bounds.mins[1];
    capsuleHull.planes[7].dist = -bounds.mins[1];
    capsuleHull.planes[8].dist = bounds.maxs[2];
    capsuleHull.planes[9].dist = -bounds.maxs[2];
    capsuleHull.planes[10].dist = bounds.mins[2];
    capsuleHull.planes[11].dist = -bounds.mins[2];

    return capsuleHull.headNode;
}

/**
*	@return	A standalone CapsuleHull
**/
CapsuleHull CM_NewCapsuleHull( const bbox3_t &bounds, const int32_t contents ) {
	CapsuleHull newCapsuleHull;

    newCapsuleHull.headNode = &newCapsuleHull.nodes[0];

    newCapsuleHull.brush.numsides = 6;
    newCapsuleHull.brush.firstbrushside = &newCapsuleHull.brushSides[0];
    newCapsuleHull.brush.contents = BrushContents::Monster;

    newCapsuleHull.leaf.contents = BrushContents::Monster;
    newCapsuleHull.leaf.firstleafbrush = &newCapsuleHull.leafBrush;
    newCapsuleHull.leaf.numleafbrushes = 1;

    newCapsuleHull.leafBrush = &newCapsuleHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &newCapsuleHull.brushSides[i];
        brushSide->plane = &newCapsuleHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &newCapsuleHull.nodes[i];
        node->plane = &newCapsuleHull.planes[i * 2];
        node->children[side] = (mnode_t *)&newCapsuleHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &newCapsuleHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&newCapsuleHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &newCapsuleHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &newCapsuleHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }

	// Set Brush and Leaf contents
	newCapsuleHull.brush.contents = contents;
	newCapsuleHull.leaf.contents = contents;
	
	// Calculate a properly centered box for generating our planes with.
	//const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

 //   const vec3_t size[2] = {
 //       bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
 //       bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
 //   };
	//
	//const vec3_t mins = size[0];
	//const vec3_t maxs = size[1];

	const vec3_t mins = bounds.mins;
	const vec3_t maxs = bounds.maxs;
	
	// Setup head and leaf -node bounds.
	newCapsuleHull.headNode->bounds = newCapsuleHull.leaf.bounds = bounds;

	// Now generate our actual 'box' plane data.
    newCapsuleHull.planes[0].dist = maxs[0];
    newCapsuleHull.planes[1].dist = -maxs[0];
    newCapsuleHull.planes[2].dist = mins[0];
    newCapsuleHull.planes[3].dist = -mins[0];
    newCapsuleHull.planes[4].dist = maxs[1];
    newCapsuleHull.planes[5].dist = -maxs[1];
    newCapsuleHull.planes[6].dist = mins[1];
    newCapsuleHull.planes[7].dist = -mins[1];
    newCapsuleHull.planes[8].dist = maxs[2];
    newCapsuleHull.planes[9].dist = -maxs[2];
    newCapsuleHull.planes[10].dist = mins[2];
    newCapsuleHull.planes[11].dist = -mins[2];

	// Calculate the spherical data for the Capsule.
	newCapsuleHull.sphere = bbox3_to_capsule( bounds, bbox3_center( bounds ) );
	return newCapsuleHull;
}