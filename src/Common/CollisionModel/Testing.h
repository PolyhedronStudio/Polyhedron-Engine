/***
*
*	License here.
*
*	@file
*
*	Collision Model:	'Area Boxing', and 'Point Contents' API: Allows for acquiring all nodes residing in the specified bounds area.
*
***/
#pragma once

/**
*   @brief  Fills in a list of all the leafs touched
**/
void CM_BoxLeafs_r( mnode_t *node );

/**
*   @brief  
**/
const int32_t CM_BoxLeafs_headnode( const bbox3_t &bounds, mleaf_t **list, const int32_t listSize, mnode_t *headNode, mnode_t **topNode );

/**
*   @brief  
**/
const int32_t CM_BoxLeafs( cm_t *cm, const bbox3_t &bounds, mleaf_t **list, const int32_t listSize, mnode_t **topNode );
/**
*   @brief  Check for what brush contents reside at vec3 'p' inside given node list.
**/
int32_t CM_PointContents( cm_t *cm, const vec3_t &p, mnode_t *headNode );