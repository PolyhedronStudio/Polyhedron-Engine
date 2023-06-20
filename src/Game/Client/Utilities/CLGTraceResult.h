/***
*
*	License here.
*
*	@file
*
*	ClientGame Tracing Utility. Takes care of handling entities appropriately.
*
***/
#pragma once


/**
*   ClientGame Trace Structure. Can consume a general Trace result.
**/
struct CLGTraceResult {
	/**
	*	@brief Default constructors accepting common TraceResult data.
	**/
	CLGTraceResult() = default;
	CLGTraceResult(TraceResult &traceResult);
	CLGTraceResult(const TraceResult &traceResult);

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
    IClientGameEntity *gameEntity = nullptr;
};

/**
*	@brief	ClientGame Trace function. Supports Game Entities.
**/
CLGTraceResult CLG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, IClientGameEntity* passent, const int32_t& contentMask );
/**
*	@brief	
**/
CLGTraceResult CLG_SphereTrace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, const sphere_t &sphere, IClientGameEntity* skipGameEntity, const int32_t& contentMask );