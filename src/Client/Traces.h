/***
*
*	License here.
*
*	@file
*
*	Client Tracing implementations, for Box and Octagon -hulls.
*	Tracing against entities is optional.
* 
***/
#pragma once


/**
*
**/
void CL_LinkEntity(PODEntity *entity);
void CL_UnlinkEntity(PODEntity *entity);

int32_t CL_PointContents(const vec3_t &point);

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_AreaEntities(const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype);

const TraceResult CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, PODEntity* skipEntity, const int32_t contentMask);