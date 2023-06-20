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
mnode_t *CM_HeadnodeForSphere( const bbox3_t &bounds, const sphere_t &sphere, const int32_t contents ) {
	// Set Brush and Leaf contents
	sphereHull.brush.contents = contents;
	sphereHull.leaf.contents = contents;
	
	// Calculate half size for mins and maxs.
	//const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );
	//const vec3_t size[2] = {
	//	bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
	//	bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
	//};
	//const vec3_t mins = size[0];
	//const vec3_t maxs = size[1];


	//const bbox3_t epsilonBounds = bbox3_expand( bounds, CM_RAD_EPSILON );
	//const bbox3_t symmetricBounds = bbox3_from_center_size( 
	//	bbox3_symmetrical( epsilonBounds ), 
	//	bbox3_center( epsilonBounds ) 
	//);
	//sphereHull.headNode->bounds = sphereHull.leaf.bounds = epsilonBounds;

	// Bounds
	const bbox3_t symmetricBounds = bounds;/*bbox3_from_center_size( 
		bbox3_symmetrical( bounds ), 
		bbox3_center( bounds ) 
	);*/
	sphereHull.headNode->bounds = sphereHull.leaf.bounds = bounds;

	// Sphere Shape.
	sphereHull.leaf.shapeType = CMHullType::Sphere;
	sphereHull.leaf.sphereShape = sphere;

    sphereHull.planes[0].dist = symmetricBounds.maxs[0];
    sphereHull.planes[1].dist = -symmetricBounds.maxs[0];
    sphereHull.planes[2].dist = symmetricBounds.mins[0];
    sphereHull.planes[3].dist = -symmetricBounds.mins[0];
    sphereHull.planes[4].dist = symmetricBounds.maxs[1];
    sphereHull.planes[5].dist = -symmetricBounds.maxs[1];
    sphereHull.planes[6].dist = symmetricBounds.mins[1];
    sphereHull.planes[7].dist = -symmetricBounds.mins[1];
    sphereHull.planes[8].dist = symmetricBounds.maxs[2];
    sphereHull.planes[9].dist = -symmetricBounds.maxs[2];
    sphereHull.planes[10].dist = symmetricBounds.mins[2];
    sphereHull.planes[11].dist = -symmetricBounds.mins[2];

    return sphereHull.headNode;
}

SphereHull CM_NewSphereHull( const bbox3_t &bounds, const sphere_t &sphere, const int32_t contents ) {
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
	
	// Bounds.
	const bbox3_t symmetricBounds = bbox3_from_size( bbox3_symmetrical( bounds ) );
	
	// Bounds
	newSphereHull.headNode->bounds = newSphereHull.leaf.bounds = symmetricBounds;

	// Sphere Shape.
	newSphereHull.leaf.shapeType = CMHullType::Sphere;
	newSphereHull.leaf.sphereShape = sphere;

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

	//newSphereHull.sphere = bbox3_to_sphere( bounds, bbox3_center( bounds ) );

	return newSphereHull;
}