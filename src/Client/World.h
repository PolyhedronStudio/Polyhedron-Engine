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
