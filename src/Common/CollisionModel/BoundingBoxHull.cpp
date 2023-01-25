/***
*
*	License here.
*
*	@file
*
*	Collision Model:	General Box Hull API - Creates a headnode for box tracing general non
*						brush entities that are bounding box shaped.
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

#include "Common/CollisionModel/BoundingBoxHull.h"
#include "Common/CollisionModel/Tracing.h"



//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
BoxHull boxHull = {};



/**
*   @brief  Set up the planes and nodes so that the six floats of a bounding box
*           can just be stored out and get a proper clipping hull structure.
**/
void CM_InitBoxHull( ) {
    boxHull.headNode = &boxHull.nodes[0];

    boxHull.brush.numsides = 6;
    boxHull.brush.firstbrushside = &boxHull.brushSides[0];
    boxHull.brush.contents = BrushContents::Monster;

    boxHull.leaf.contents = BrushContents::Monster;
    boxHull.leaf.firstleafbrush = &boxHull.leafBrush;
    boxHull.leaf.numleafbrushes = 1;

    boxHull.leafBrush = &boxHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &boxHull.brushSides[i];
        brushSide->plane = &boxHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &boxHull.nodes[i];
        node->plane = &boxHull.planes[i * 2];
        node->children[side] = (mnode_t *)&boxHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &boxHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&boxHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &boxHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &boxHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }
}

/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t *CM_HeadnodeForBox( const bbox3_t &bounds, const int32_t contents ) {
// Set Brush and Leaf contents
	boxHull.brush.contents = contents;
	boxHull.leaf.contents = contents;
	
	// Calculate half size for mins and maxs.
	const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

    const vec3_t size[2] = {
        bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
        bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
    };
	
	const vec3_t mins = size[0];
	const vec3_t maxs = size[1];
	
	boxHull.headNode->bounds = boxHull.leaf.bounds = { size[0], size[1] };

    boxHull.planes[0].dist = maxs[0];
    boxHull.planes[1].dist = -maxs[0];
    boxHull.planes[2].dist = mins[0];
    boxHull.planes[3].dist = -mins[0];
    boxHull.planes[4].dist = maxs[1];
    boxHull.planes[5].dist = -maxs[1];
    boxHull.planes[6].dist = mins[1];
    boxHull.planes[7].dist = -mins[1];
    boxHull.planes[8].dist = maxs[2];
    boxHull.planes[9].dist = -maxs[2];
    boxHull.planes[10].dist = mins[2];
    boxHull.planes[11].dist = -mins[2];

    return boxHull.headNode;
}


/**
*	@return	A standalone CapsuleHull
**/
BoxHull CM_NewBoundingBoxHull( const bbox3_t &bounds, const int32_t contents ) {
	BoxHull newBoxHull;

    newBoxHull.headNode = &newBoxHull.nodes[0];

    newBoxHull.brush.numsides = 6;
    newBoxHull.brush.firstbrushside = &newBoxHull.brushSides[0];
    newBoxHull.brush.contents = BrushContents::Monster;

    newBoxHull.leaf.contents = BrushContents::Monster;
    newBoxHull.leaf.firstleafbrush = &newBoxHull.leafBrush;
    newBoxHull.leaf.numleafbrushes = 1;

    newBoxHull.leafBrush = &newBoxHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &newBoxHull.brushSides[i];
        brushSide->plane = &newBoxHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &newBoxHull.nodes[i];
        node->plane = &newBoxHull.planes[i * 2];
        node->children[side] = (mnode_t *)&newBoxHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &newBoxHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&newBoxHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &newBoxHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &newBoxHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }

// Set Brush and Leaf contents
	newBoxHull.brush.contents = contents;
	newBoxHull.leaf.contents = contents;
	
	// Calculate half size for mins and maxs.
	const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

    const vec3_t size[2] = {
        bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
        bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
    };
	
	const vec3_t mins = size[0];
	const vec3_t maxs = size[1];
	
	newBoxHull.headNode->bounds = newBoxHull.leaf.bounds = { size[0], size[1] };

    newBoxHull.planes[0].dist = maxs[0];
    newBoxHull.planes[1].dist = -maxs[0];
    newBoxHull.planes[2].dist = mins[0];
    newBoxHull.planes[3].dist = -mins[0];
    newBoxHull.planes[4].dist = maxs[1];
    newBoxHull.planes[5].dist = -maxs[1];
    newBoxHull.planes[6].dist = mins[1];
    newBoxHull.planes[7].dist = -mins[1];
    newBoxHull.planes[8].dist = maxs[2];
    newBoxHull.planes[9].dist = -maxs[2];
    newBoxHull.planes[10].dist = mins[2];
    newBoxHull.planes[11].dist = -mins[2];

    //newBoxHull.planes[0].dist = bounds.maxs[0];
    //newBoxHull.planes[1].dist = -bounds.maxs[0];
    //newBoxHull.planes[2].dist = bounds.mins[0];
    //newBoxHull.planes[3].dist = -bounds.mins[0];
    //newBoxHull.planes[4].dist = bounds.maxs[1];
    //newBoxHull.planes[5].dist = -bounds.maxs[1];
    //newBoxHull.planes[6].dist = bounds.mins[1];
    //newBoxHull.planes[7].dist = -bounds.mins[1];
    //newBoxHull.planes[8].dist = bounds.maxs[2];
    //newBoxHull.planes[9].dist = -bounds.maxs[2];
    //newBoxHull.planes[10].dist = bounds.mins[2];
    //newBoxHull.planes[11].dist = -bounds.mins[2];

	return newBoxHull;
}