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
#include "Shared/Shared.h"

#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CVar.h"
#include "Common/CollisionModel.h"
#include "Common/Common.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

#include "Common/CollisionModel/SphereHull.h"
#include "Common/CollisionModel/Tracing.h"



//! All round 'sphere hull' data, accessed in a few other CollisionModel files as extern.
SphereHull sphereHull = {};

// TODO: Obvious, clean this up
sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin = vec3_zero() );
sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );

/**
*   @brief   
**/
void CM_InitSphereHull( ) {
    sphereHull.headNode = &sphereHull.nodes[0];

    sphereHull.brush.numsides = 6;
    sphereHull.brush.firstbrushside = &sphereHull.brushSides[0];
    sphereHull.brush.contents = BrushContents::Monster;

    sphereHull.leaf.contents = BrushContents::Monster;
    sphereHull.leaf.firstleafbrush = &sphereHull.leafBrush;
    sphereHull.leaf.numleafbrushes = 1;

    sphereHull.leafBrush = &sphereHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &sphereHull.brushSides[i];
        brushSide->plane = &sphereHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &sphereHull.nodes[i];
        node->plane = &sphereHull.planes[i * 2];
        node->children[side] = (mnode_t *)&sphereHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &sphereHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&sphereHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &sphereHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &sphereHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }
}

/**
*   @brief   
**/
mnode_t *CM_HeadnodeForSphere( const bbox3_t &bounds, const int32_t contents ) {
	// Get the radius from the bounds.
	const float radius = bbox3_radius( bounds );

	// Set Brush and Leaf contents
	sphereHull.brush.contents = contents;
	sphereHull.leaf.contents = contents;
	
	// Calculate half size for mins and maxs.
	const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

    const vec3_t size[2] = {
        bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
        bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
    };
	
	const vec3_t mins = size[0];
	const vec3_t maxs = size[1];
	
	sphereHull.headNode->bounds = sphereHull.leaf.bounds = bounds;

    sphereHull.planes[0].dist = bounds.maxs[0];
    sphereHull.planes[1].dist = -bounds.maxs[0];
    sphereHull.planes[2].dist = bounds.mins[0];
    sphereHull.planes[3].dist = -bounds.mins[0];
    sphereHull.planes[4].dist = bounds.maxs[1];
    sphereHull.planes[5].dist = -bounds.maxs[1];
    sphereHull.planes[6].dist = bounds.mins[1];
    sphereHull.planes[7].dist = -bounds.mins[1];
    sphereHull.planes[8].dist = bounds.maxs[2];
    sphereHull.planes[9].dist = -bounds.maxs[2];
    sphereHull.planes[10].dist = bounds.mins[2];
    sphereHull.planes[11].dist = -bounds.mins[2];

    return sphereHull.headNode;
}

SphereHull CM_NewSphereHull( const bbox3_t &bounds, const int32_t contents ) {
	SphereHull newSphereHull;

    newSphereHull.headNode = &newSphereHull.nodes[0];

    newSphereHull.brush.numsides = 6;
    newSphereHull.brush.firstbrushside = &newSphereHull.brushSides[0];
    newSphereHull.brush.contents = BrushContents::Monster;

    newSphereHull.leaf.contents = BrushContents::Monster;
    newSphereHull.leaf.firstleafbrush = &newSphereHull.leafBrush;
    newSphereHull.leaf.numleafbrushes = 1;

    newSphereHull.leafBrush = &newSphereHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &newSphereHull.brushSides[i];
        brushSide->plane = &newSphereHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &newSphereHull.nodes[i];
        node->plane = &newSphereHull.planes[i * 2];
        node->children[side] = (mnode_t *)&newSphereHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &newSphereHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&newSphereHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &newSphereHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &newSphereHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }

// Set Brush and Leaf contents
	newSphereHull.brush.contents = contents;
	newSphereHull.leaf.contents = contents;
	
	//// Calculate half size for mins and maxs.
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
	
	newSphereHull.headNode->bounds = newSphereHull.leaf.bounds = bounds;

    newSphereHull.planes[0].dist = maxs[0];
    newSphereHull.planes[1].dist = -maxs[0];
    newSphereHull.planes[2].dist = mins[0];
    newSphereHull.planes[3].dist = -mins[0];
    newSphereHull.planes[4].dist = maxs[1];
    newSphereHull.planes[5].dist = -maxs[1];
    newSphereHull.planes[6].dist = mins[1];
    newSphereHull.planes[7].dist = -mins[1];
    newSphereHull.planes[8].dist = maxs[2];
    newSphereHull.planes[9].dist = -maxs[2];
    newSphereHull.planes[10].dist = mins[2];
    newSphereHull.planes[11].dist = -mins[2];

	newSphereHull.sphere = bbox3_to_sphere( bounds, bbox3_center( bounds ) );

	return newSphereHull;
}