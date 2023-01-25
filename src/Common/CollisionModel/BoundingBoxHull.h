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
#pragma once

struct BoxHull {
    CollisionPlane planes[12];
    mnode_t  nodes[6];
    mnode_t  *headNode;
    mbrush_t brush;
    mbrush_t *leafBrush;
    mbrushside_t brushSides[6];
    mleaf_t  leaf;
    mleaf_t  emptyLeaf;
};

/**
*   @brief  Set up the planes and nodes so that the six floats of a bounding box
*           can just be stored out and get a proper clipping hull structure.
**/
void CM_InitBoxHull( );
/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t *CM_HeadnodeForBox( const vec3_t &mins , const vec3_t &maxs );

/**
*	@return	A standalone BoxHull
**/
BoxHull CM_NewBoundingBoxHull( const bbox3_t &bounds, const int32_t contents );