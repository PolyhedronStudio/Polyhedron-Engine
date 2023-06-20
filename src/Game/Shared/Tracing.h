/***
*
*	License here.
*
*	@file
*
*	SharedGame Tracing Code. Wraps around the engine interface.
*
***/
#pragma once

#include "Shared/Shared.h"

/**
*   ServerGame Trace Structure. Can consume a general Trace result.
**/
struct SGTraceResult {
	/**
	*	@brief Default constructors accepting common TraceResult data.
	**/
	SGTraceResult() = default;
	SGTraceResult(TraceResult &traceResult);
	SGTraceResult(const TraceResult &traceResult);

    //! If true, the trace startedand ended within the same solid.
    qboolean allSolid = false;
    //! If true, the trace started within a solid, but exited it.
    qboolean startSolid = false;
    
	//! The fraction of the desired distance traveled(0.0 - 1.0).If
    //! 1.0, no plane was impacted.
    float fraction = 0.f;
	//! The destination position.
    vec3_t endPosition = vec3_zero();
    //! [signBits][x] = either size[0][x] or size[1][x]
    vec3_t offsets[8] = {
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
    };
        
    //! The impacted plane, or empty. Note that a copy of the plane is returned, 
    //! rather than a pointer.This is because the plane may belong to an inline 
    //! BSP model or the box hull of a solid entity.
    // 
    //! If it is an inline BSP Model or a box hull of a solid entity the plane 
    //! must be transformed by the entity's current position.
    CollisionPlane plane = {};
    //! The impacted surface, or nullptr.
    CollisionSurface* surface = nullptr;
    //! The contents mask of the impacted brush, or 0.
    int32_t contents = 0;


	//! The impacted PODEntity, or a nullptr.
	PODEntity *podEntity = nullptr;
    //! The impacted GameEntity, or nullptr.
    GameEntity *gameEntity = nullptr;
};

/**
*	@brief	SharedGame Trace Functionality: Supports GameEntities :-)
**/
SGTraceResult SG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, GameEntity* skipGameEntity, const int32_t contentMask );
/**
*	@brief	
**/
SGTraceResult SG_SphereTrace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, const sphere_t &sphere, GameEntity* skipGameEntity, const int32_t contentMask );

/**
*	@brief	SharedGame PointContents Functionality: Supports GameEntities :-)
**/
const int32_t SG_PointContents(const vec3_t &point);

/**
*	@brief	Scans whether this entity is touching any others, and if so, dispatches their touch callback function.
**/
void SG_TouchTriggers(GameEntity* geToucher);

/**
*	@return	GameEntityVector filled with the entities that were residing inside the box. Will not exceed listCount limit.
**/
GameEntityVector SG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType);