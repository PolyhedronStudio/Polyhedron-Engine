/***
*
*	License here.
*
*	@file
*
*	Collision Model: AreaPortal API.
*
***/
#pragma once

/**
*   @brief
**/
void FloodAreaConnections( cm_t *cm );
/**
*   @brief
**/
void CM_SetAreaPortalState( cm_t *cm, int32_t portalnum, qboolean open );

/**
*   @brief
**/
qboolean CM_AreasConnected( cm_t *cm, int32_t area1, int32_t area2 );
/**
*   @brief  Writes a length byte followed by a bit vector of all the areas
*           that area in the same flood as the area parameter
*
*           This is used by the client refreshes to cull visibility
**/
int CM_WriteAreaBits( cm_t *cm, byte *buffer, int32_t area );

/**
*   @brief
**/
int CM_WritePortalBits( cm_t *cm, byte *buffer );
/**
*   @brief
**/
void CM_SetPortalStates( cm_t *cm, byte *buffer, int32_t bytes );

/**
*   @return True if any leaf under headNode has a cluster that is potentially visible
**/
qboolean CM_HeadnodeVisible( mnode_t *node, byte *visbits );

/**
*   @brief  The client will interpolate the view position, so we can't use a single PVS point
**/
byte *CM_FatPVS( cm_t *cm, byte *mask, const vec3_t &org, int32_t vis);