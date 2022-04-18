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
#include "Client.h"
#include "Traces.h"
#include "GameModule.h"
#include "Refresh/Models.h"

extern ClientShared cs;


/**
*	@brief	Clips the trace against all entities resulting in a final trace result.
**/
void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *skipEntity, const int32_t contentMask, TraceResult *cmDstTrace) {
    // CM Source Trace.
    TraceResult         cmSrcTrace;
    // Head Node used for testing.
    mnode_t*        headNode = nullptr;
    // Collision model for entity.
    mmodel_t*       cmodel = nullptr;
    // Client side entity.
    PODEntity*   solidEntity = nullptr;

    // Actual start point of the trace. May modify during the loop.
    vec3_t traceOrigin = vec3_zero();
    // Actual angles for the trace. May modify during the loop.
    vec3_t traceAngles = vec3_zero();

    for (uint32_t i = 0; i < cl.numSolidEntities; i++) {
        // Fetch client entity.
        solidEntity = cl.solidEntities[i];

        // This check is likely redundent but let's make sure it is there anyway for possible future changes.
        if (solidEntity == nullptr) {
            continue;
        }

        // Should we skip it?
        if (skipEntity != nullptr && skipEntity->currentState.number == solidEntity->currentState.number) {
            continue;
        }

        if (solidEntity->currentState.solid == PACKED_BBOX) {
            // special value for bmodel
            cmodel = cl.clipModels[solidEntity->currentState.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            // Setup angles and origin for our trace.
            traceAngles = solidEntity->currentState.angles;
            traceOrigin = solidEntity->currentState.origin;
        } else {
            if (solidEntity->currentState.solid == Solid::OctagonBox) {
                headNode = CM_HeadnodeForOctagon(solidEntity->currentState.mins, solidEntity->currentState.maxs);
            } else {
                headNode = CM_HeadnodeForBox(solidEntity->currentState.mins, solidEntity->currentState.maxs);
            }

            traceAngles = solidEntity->currentState.angles;
            traceOrigin = solidEntity->currentState.origin;
        }

        // We're done clipping against entities if we reached an allSolid aka world.
        if (cmDstTrace->allSolid)
            return;

		// Execute source trace.
        cmSrcTrace = CM_TransformedBoxTrace(start, end, mins, maxs, headNode, contentMask, traceOrigin, traceAngles);

        CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct PODEntity*)solidEntity);
    }
}

/**
*	@brief	Executes a client side trace on the world and its entities using the given contentMask.
*			Optionally one can pass a pointer to an entity in order to skip(ignore) it.
*	@return	The result of said trace, in case of hittnig the world, ent == cl.solidEntities[0] == nullptr?
**/
TraceResult CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, PODEntity* skipEntity, const int32_t contentMask) {
	// Absolute clean trace result.
	TraceResult trace = {};

    // Ensure we can pull of a proper trace.
    if (!cl.bsp || !cl.bsp->nodes) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
        return trace;
    }

    // Execute trace.
    //CM_BoxTrace(&trace, start, end, mins, maxs, cl.bsp->nodes, contentMask);
	vec3_t traceOrigin = vec3_zero();
	vec3_t traceAngles = vec3_zero();

    trace = CM_TransformedBoxTrace(start, end, mins, maxs, cl.bsp->nodes, contentMask, traceOrigin, traceAngles);

    // Set trace entity.
    trace.ent = reinterpret_cast<PODEntity*>(&cl.solidEntities[0]);

	if (trace.fraction == 0) {
        return trace;   // Blocked by the world
    }

    // Clip to other solid entities.
    CL_ClipMoveToEntities(start, mins, maxs, end, reinterpret_cast<PODEntity*>(skipEntity), contentMask, &trace);

    return trace;
}





/*

/*
===============
CL_ClipMoveToEntities

Clips the trace against all entities resulting in a final trace result.
===============
void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *skipEntity, const int32_t contentMask, TraceResult *cmDstTrace) {
    // CM Source Trace.
    TraceResult         cmSrcTrace;
    // Head Node used for testing.
    mnode_t*        headNode = nullptr;
    // Collision model for entity.
    mmodel_t*       cmodel = nullptr;
    // Client side entity.
    PODEntity*   solidEntity = nullptr;

    // Actual start point of the trace. May modify during the loop.
    vec3_t traceOrigin = vec3_zero();
    // Actual angles for the trace. May modify during the loop.
    vec3_t traceAngles = vec3_zero();

    for (uint32_t i = 0; i < cl.numSolidEntities; i++) {
        // Fetch client entity.
        solidEntity = cl.solidEntities[i];

        // This check is likely redundent but let's make sure it is there anyway for possible future changes.
        if (solidEntity == nullptr) {
            continue;
        }

        // Should we skip it?
        if (skipEntity != nullptr && skipEntity->currentState.number == solidEntity->currentState.number) {
            continue;
        }

        if (solidEntity->currentState.solid == PACKED_BBOX) {
            // special value for bmodel
            cmodel = cl.clipModels[solidEntity->currentState.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            // Setup angles and origin for our trace.
            traceAngles = solidEntity->currentState.angles;
            traceOrigin = solidEntity->currentState.origin;
        } else {
            vec3_t entityMins = {0.f, 0.f, 0.f};
            vec3_t entityMaxs = {0.f, 0.f, 0.f};

            //MSG_UnpackBoundingBox32(solidEntity->currentState.solid, entityMins, entityMaxs);
            
            if (solidEntity->currentState.solid == Solid::OctagonBox) {

                headNode = CM_HeadnodeForOctagon(solidEntity->mins, solidEntity->maxs);
            } else {
                headNode = CM_HeadnodeForBox(solidEntity->mins, solidEntity->maxs);
            }

            traceAngles = vec3_zero();
            traceOrigin = solidEntity->currentState.origin;
        }

        // We're done clipping against entities if we reached an allSolid aka world.
        if (cmDstTrace->allSolid)
            return;

        CM_TransformedBoxTrace(&cmSrcTrace, start, end,
                               mins, maxs, headNode, contentMask,
                               traceOrigin, traceAngles);

        CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct PODEntity*)solidEntity);
    }
}

===============
CL_Trace

Executes a client side trace on the world and its entities using the given contentMask.
Optionally one can pass a pointer to an entity in order to skip(ignore) it.
===============
TraceResult CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, PODEntity* skipEntity, const int32_t contentMask) {
    TraceResult trace;

    // Ensure we can pull of a proper trace.
    if (!cl.bsp || !cl.bsp->nodes) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
        return trace;
    }

    // Execute trace.
    //CM_BoxTrace(&trace, start, end, mins, maxs, cl.bsp->nodes, contentMask);
    CM_TransformedBoxTrace(&trace, start, end, mins, maxs, cl.bsp->nodes, contentMask, vec3_zero(), vec3_zero());

    // Set trace entity.
    trace.ent = reinterpret_cast<PODEntity*>(&cl.solidEntities[0]);

    // Clip to other solid entities.
    CL_ClipMoveToEntities(start, mins, maxs, end, reinterpret_cast<PODEntity*>(skipEntity), contentMask, &trace);

    return trace;
}
*/