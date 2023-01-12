/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Octagon Box Hull API - Creates a headnode for tracing an octagonal shaped box
*						for non brush entities. Suits characters and barrels, or other circle like shaped
*						needs mostly.
*
***/
#pragma once

struct OctagonHull {
    CollisionPlane planes[20];
    mnode_t  nodes[10];
    mnode_t  *headNode;
    mbrush_t brush;
    mbrush_t *leafBrush;
    mbrushside_t brushSides[10];
    mleaf_t  leaf;
    mleaf_t  emptyLeaf;
};

/**
*   @brief  Set up the planes and nodes so that the 10 floats of an octagon box
*           can just be stored out and get a proper clipping hull structure.
**/
void CM_InitOctagonBoxHull( );
/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t* CM_HeadnodeForOctagon( const bbox3_t &bounds );