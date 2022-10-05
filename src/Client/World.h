/***
*
*	License here.
*
*	@file
*
*	Client World header. 
* 
***/
#pragma once

/***
*
*	License here.
*
*	@file
*
*	Client 'World' management. Similar to the server World.cpp file.
* 
***/
#include "Client.h"
#include "GameModule.h"
#include "World.h"

/**
*	@brief	Clears out the old areagrid, and generates a new one with the currently cached data.
**/
void CL_ClearWorld();

/**
*	@brief	Determines what area an entity resides in and "links it in for collision testing".
*			Finds the area to link the entity in to and sets its bounding box in case it is an
*			actual inline bsp model.
**/
mnode_t *CL_World_HullForEntity(Entity *ent);

/**
*	@brief	Removes the entity for collision testing.
**/
void CL_World_UnlinkEntity(Entity *ent);
/**
*	@brief	Links entity to PVS leafs.
**/
void CL_World_LinkEntity( cm_t *cm, Entity *ent );
void CL_PF_World_LinkEntity(Entity *ent);

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_World_AreaEntities(const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype);
/**
*	@brief	Specialized server implementation of PointContents function.
**/
int32_t CL_World_PointContents(const vec3_t &point);
/**
*	@brief	Moves the given mins/maxs volume through the world from start to end. This is used for local entities.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult CL_World_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *passedict, int32_t contentMask);
