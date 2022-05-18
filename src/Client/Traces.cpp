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
static void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *skipEntity, const int32_t contentMask, TraceResult *cmDstTrace) {
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

#ifdef CFG_CM_ALLOW_ROTATING_BOXES
			traceAngles = solidEntity->currentState.angles;
#else
			traceAngles = vec3_zero();
#endif
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
*	@brief	Clips the trace against all entities resulting in a final trace result.
**/
void CL_ClipMoveToLocalClientEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *skipEntity, const int32_t contentMask, TraceResult *cmDstTrace) {
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

    for (uint32_t i = 0; i < cl.numSolidLocalEntities; i++) {
        // Fetch client entity.
        solidEntity = cl.solidLocalEntities[i];

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

#ifdef CFG_CM_ALLOW_ROTATING_BOXES
			traceAngles = solidEntity->currentState.angles;
#else
			traceAngles = vec3_zero();
#endif
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
const TraceResult CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, PODEntity* skipEntity, const int32_t contentMask) {
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
    trace.ent = reinterpret_cast<PODEntity*>(&cs.entities[0]);

	if (trace.fraction == 0) {
        return trace;   // Blocked by the world
    }

    // Clip to other solid entities.
    CL_ClipMoveToEntities(start, mins, maxs, end, reinterpret_cast<PODEntity*>(skipEntity), contentMask, &trace);
	CL_ClipMoveToLocalClientEntities(start, mins, maxs, end, reinterpret_cast<PODEntity*>(skipEntity), contentMask, &trace);

    return trace;
}

void CL_LinkEntity(PODEntity* entity) {


	// Ensure it is in use.
    if (!entity) {
        Com_DPrintf("%s: PODEntity == (nullptr)!\n", __func__);
        return;
    }
	if (!entity->inUse) {
        Com_DPrintf("%s: PODEntity(#i)->inUse == false!!\n", __func__, entity->clientEntityNumber);
		return;
	}

	// Unlink from previous old position.
	//if (ent->area.prev) {
	if (entity) {//}&& entity->linkCount) {
        CL_UnlinkEntity(entity);
	}

	// Ensure it isn't the worldspawn entity itself.
    if (entity->clientEntityNumber == 0) {
        return;        // Don't add the world
	}

	// Ensure a map is loaded properly in our cache.
    if (!cl.bsp || cl.bsp->nodes) {
        return;
    }

	// Find the actual server entity.
    //int32_t entityNumber = NUM_FOR_EDICT(ent);
    //server_entity_t *serverEntity = &sv.entities[entityNumber];

    // Encode the size into the entity_state for client prediction reaspms/
    switch (entity->solid) {
    case Solid::BoundingBox:
        if ((entity->serverFlags & EntityServerFlags::DeadMonster) || VectorCompare(entity->mins, entity->maxs)) {
            entity->currentState.solid = 0;
        } else {
			entity->currentState.mins = entity->mins;
			entity->currentState.maxs = entity->maxs;
        }
        break;
    case Solid::OctagonBox:
        if ((entity->serverFlags & EntityServerFlags::DeadMonster) || VectorCompare(entity->mins, entity->maxs)) {
            entity->currentState.solid = 0;
        } else {
			entity->currentState.solid = Solid::OctagonBox; //MSG_PackBoundingBox32(entity->mins, entity->maxs);
			entity->currentState.mins = entity->mins;
			entity->currentState.maxs = entity->maxs;
        }
        break;
    case Solid::BSP:
        entity->currentState.solid = PACKED_BBOX;      // a Solid::BoundingBox will never create this value
		entity->currentState.mins = vec3_zero();
		entity->currentState.maxs = vec3_zero();
        break;
    default:
        entity->currentState.solid = 0;
		entity->currentState.mins = vec3_zero();
		entity->currentState.maxs = vec3_zero();
        break;
    }

    //SV_LinkEntity(&sv.cm, ent);
	if (entity->solid == Solid::BSP &&
		(entity->currentState.angles[0] || entity->currentState.angles[1] || entity->currentState.angles[2])) {
		// expand for rotation
		float   max, v;
		int     i;

		max = 0;
		for (i = 0; i < 3; i++) {
			v = fabs(entity->mins[i]);
			if (v > max)
				max = v;
			v = fabs(entity->maxs[i]);
			if (v > max)
				max = v;
		}
		for (i = 0; i < 3; i++) {
			entity->absMin[i] = entity->currentState.origin[i] - max;
			entity->absMax[i] = entity->currentState.origin[i] + max;
		}
	} else {
		// normal
		entity->absMin = entity->currentState.origin + entity->mins; //VectorAdd(entity->currentState.origin, entity->mins, entity->absMin);
		entity->absMax = entity->currentState.origin + entity->maxs; //VectorAdd(entity->currentState.origin, entity->maxs, entity->absMax);
	}

    // If first time, make sure oldOrigin is valid.
    if (!entity->linkCount) {
        entity->currentState.oldOrigin = entity->currentState.origin;
    }
    entity->linkCount++;

	// 
    if (entity->solid == Solid::Not) {
        return;
	}

	if (entity->isLocal) {
		cl.solidLocalEntities[cl.numSolidLocalEntities++] = entity;
	}
}

/**
*	@brief	Unlinked the PODEntity.
**/
void CL_UnlinkEntity(PODEntity* entity) {
	if (entity) {
		entity->linkCount = 0;
	}
}

/**
*	@return	Returns a headNode that can be used for testing or clipping an
*			entity's BoundingBox or OctagonBox of mins/maxs size.
**/
mnode_t *CL_HullForEntity(PODEntity *podEntity) {
    if (podEntity->solid == Solid::BSP) {
        int32_t i = podEntity->currentState.modelIndex - 1;

        // Explicit hulls in the BSP model.
        if (i <= 0 || i >= cl.bsp->nummodels) {
            Com_Error(ErrorType::Drop, "%s: inline model %d out of range", __func__, i);
		}

		return cl.clipModels[i]->headNode;
        //return cl.bsp->models[i].headNode;
    }

    // create a temp hull from bounding box sizes
    if (podEntity->solid == Solid::OctagonBox) {
        return CM_HeadnodeForOctagon(podEntity->mins, podEntity->maxs);
    } else {
        return CM_HeadnodeForBox(podEntity->mins, podEntity->maxs);
    }
}

/**
*	@return	Returns the bit flags of what contents are there at position 'point'.
**/
int32_t CL_PointContents(const vec3_t& point) {
    static PODEntity     *touch[MAX_WIRED_POD_ENTITIES], *hit = nullptr;
    
	// Ensure all is sane.
    if (!cl.bsp || !cl.bsp->nodes) {
		//Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return 0;
	}

    // get base contents from world
    int32_t contents = CM_PointContents( point, cl.bsp->nodes );

    // or in contents from all the other entities
    int32_t numberOfAreaEntities = CL_AreaEntities( point, point, touch, MAX_WIRED_POD_ENTITIES, AreaEntities::Solid );

    for (int32_t i = 0; i < numberOfAreaEntities; i++) {
		// Acquire touch entity.
        Entity *hit = touch[i];

        // Might intersect, so do an exact clip
#ifdef CFG_CM_ALLOW_ROTATING_BOXES
			contents |= CM_TransformedPointContents( point, CL_HullForEntity(hit), hit->currentState.origin, hit->currentState.angles );
#else
			contents |= CM_TransformedPointContents(point, CL_HullForEntity(hit), hit->currentState.origin, vec3_zero());//hit->currentState.angles);
#endif
    }

    return contents;
}


/**
*	@brief	The inner workings of CL_AreaEntities.
**/
// Area Mins/Maxs.
static vec3_t areaMins	= vec3_zero();
static vec3_t areaMaxs	= vec3_zero();

// List of entities in an area.
static Entity  **areaList	= nullptr;

// Area stats.
static int32_t areaCount	= 0;
static int32_t areaMaxCount	= 0;
static int32_t areaType		= 0;

static void CL_AreaEntities_r() {
	// Packet Entities.
	if (areaType == AreaEntities::Solid) {
		for (int i = 0; i < cl.numSolidEntities; i++) {
			PODEntity *solidEntity = cl.solidEntities[i];

			if (!solidEntity || solidEntity->solid == Solid::Not) {
				continue;
			}

			if (solidEntity->absMin[0] > areaMaxs[0]
				|| solidEntity->absMin[1] > areaMaxs[1]
				|| solidEntity->absMin[2] > areaMaxs[2]
				|| solidEntity->absMax[0] < areaMins[0]
				|| solidEntity->absMax[1] < areaMins[1]
				|| solidEntity->absMax[2] < areaMins[2]) {
				continue;        // not touching
			}

			if (areaCount == areaMaxCount) {
				Com_WPrintf("CL_AreaEntities: MAXCOUNT\n");
				return;
			}

			areaList[areaCount] = solidEntity;
			areaCount++;
		}
	} 
	// Local Client Only entities that got Linked.
	else if (areaType == AreaEntities::LocalSolid) {
		for (int i = 0; i < cl.numSolidLocalEntities; i++) {
			PODEntity *solidEntity = cl.solidLocalEntities[i];

			if (!solidEntity || solidEntity->solid == Solid::Not) {
				continue;
			}

			if (solidEntity->absMin[0] > areaMaxs[0]
				|| solidEntity->absMin[1] > areaMaxs[1]
				|| solidEntity->absMin[2] > areaMaxs[2]
				|| solidEntity->absMax[0] < areaMins[0]
				|| solidEntity->absMax[1] < areaMins[1]
				|| solidEntity->absMax[2] < areaMins[2]) {
				continue;        // not touching
			}

			if (areaCount == areaMaxCount) {
				Com_WPrintf("CL_AreaEntities: MAXCOUNT\n");
				return;
			}

			areaList[areaCount] = solidEntity;
			areaCount++;
		}
	}
}

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_AreaEntities(const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype) {
    areaMins = mins;
    areaMaxs = maxs;
    areaList = list;
    areaCount = 0;
    areaMaxCount = maxcount;
    areaType = areatype;

    CL_AreaEntities_r();

    return areaCount;
	//return 0;
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