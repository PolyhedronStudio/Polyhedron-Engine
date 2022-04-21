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

/*
===============================================================================

ENTITY AREA CHECKING

FIXME: this use of "area" is different from the bsp file use
===============================================================================
*/
// Area Grid Node.
//typedef struct areanode_s {
//    int32_t     axis;       // -1 = leaf node
//    float   dist;
//    struct areanode_s   *children[2];
//    list_t  triggerEdicts;
//    list_t  solidEdicts;
//} areanode_t;


//void CL_ClearWorld(void);
//qboolean CL_EntityIsVisible(cm_t *cm, Entity *ent, byte *mask);
/**
*	@brief	Removes the entity for collision testing.
**/
void CL_UnlinkEntity(Entity *ent);
/**
*	@brief	General purpose routine shared between game DLL and MVD code.
*			Links entity to PVS leafs.
**/
void CL_LinkEntity(cm_t *cm, Entity *ent);

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_AreaEntities(const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype);
/**
*	@brief	Specialized server implementation of PointContents function.
**/
int32_t CL_PointContents(const vec3_t &point);
/**
*	@brief	Moves the given mins/maxs volume through the world from start to end. This is used for local entities.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult CL_World_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *passedict, int32_t contentMask);

/*
===============
SV_LinkEdict

General purpose routine shared between game DLL and MVD code.
Links entity to PVS leafs.
===============
*/
//void CL_LinkEntity(cm_t *cm, Entity *ent);
//void PF_UnlinkEntity(Entity *ent);
//void PF_LinkEntity(Entity *ent);
//int32_t CL_AreaEntities(const vec3_t &mins, const vec3_t &maxs, Entity **list, int32_t maxCount, int32_t areaType) ; 
//int32_t CL_PointContents(const vec3_t &p);
//static void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end,
//                                  Entity *passedict, int contentmask, TraceResult *tr)
//const TraceResult q_gameabi CL_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, Entity *passedict, int32_t contentMask);
