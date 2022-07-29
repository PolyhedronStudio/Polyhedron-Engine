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

// Area Grid configuration.
static constexpr int32_t AREA_DEPTH	= 4;
static constexpr int32_t AREA_NODES	= 32;

// Area Grid Node.
typedef struct areanode_s {
    int32_t     axis;       // -1 = leaf node
    float   dist;
    struct areanode_s   *children[2];
    list_t  triggerEdicts;
    list_t  solidEdicts;
    list_t  solidLocalClientEdicts;
} areanode_t;

// Area nodes array.
static areanode_t cl_areanodes[AREA_NODES];
static int32_t cl_numareanodes = 0;

// Area Mins/Maxs.
static vec3_t areaMins	= vec3_zero();
static vec3_t areaMaxs	= vec3_zero();

// List of entities in an area.
static Entity  **areaList	= nullptr;

// Area stats.
static int32_t areaCount	= 0;
static int32_t areaMaxCount	= 0;
static int32_t areaType		= 0;

/**
*	@brief Builds a uniformly subdivided tree for the given world size
**/
static areanode_t *CL_CreateAreaNode(int depth, const vec3_t &mins, const vec3_t &maxs) {
    areanode_t  *anode;
    vec3_t      size;
    vec3_t      mins1, maxs1, mins2, maxs2;

    anode = &cl_areanodes[cl_numareanodes];
    cl_numareanodes++;

    List_Init(&anode->triggerEdicts);
    List_Init(&anode->solidEdicts);
    List_Init(&anode->solidLocalClientEdicts);

    if (depth == AREA_DEPTH) {
        anode->axis = -1;
        anode->children[0] = anode->children[1] = NULL;
        return anode;
    }

    size = maxs - mins;
    if (size[0] > size[1]) {
        anode->axis = 0;
	} else {
        anode->axis = 1;
	}

    anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
    mins1 = mins;
    mins2 = mins;
	maxs1 = maxs;
	maxs2 = maxs;

    maxs1[anode->axis] = mins2[anode->axis] = anode->dist;

    anode->children[0] = CL_CreateAreaNode(depth + 1, mins2, maxs2);
    anode->children[1] = CL_CreateAreaNode(depth + 1, mins1, maxs1);

    return anode;
}

/**
*	@brief	Clear the server entity area grid world.
**/
void CL_ClearWorld()
{
    mmodel_t *cm;
    Entity *ent;
    int i;

    memset(cl_areanodes, 0, sizeof(cl_areanodes));
    cl_numareanodes = 0;

    if (cl.bsp && cl.cm.cache) {
        cm = &cl.bsp->models[0];
        CL_CreateAreaNode(0, cm->mins, cm->maxs);
    }

    // make sure all entities are unlinked
    for (i = 0; i < MAX_CLIENT_POD_ENTITIES; i++) {//ge->maxEntities; i++) {
        ent = EDICT_NUM(i);
        ent->area.prev = ent->area.next = NULL;
    }
}

/**
*	@brief	Checks if edict is potentially visible from the given PVS row.
*	@return	True if visible.
**/
qboolean CL_EntityIsVisible(cm_t *cm, Entity *ent, byte *mask)
{
    int i;

    if (ent->numClusters == -1) {
        // too many leafs for individual check, go by headNode
        return CM_HeadnodeVisible(CM_NodeNum(cm, ent->headNode), mask);
    }

    // check individual leafs
    for (i = 0; i < ent->numClusters; i++) {
        if (Q_IsBitSet(mask, ent->clusterNumbers[i])) {
            return true;
        }
    }

    return true;  // not visible
}

/**
*	@brief	General purpose routine shared between game DLL and MVD code.
*			Links entity to PVS leafs.
**/
void CL_World_LinkEntity(cm_t *cm, Entity *ent)
{
    mleaf_t	*leafs[MAX_TOTAL_ENT_LEAFS];
    int32_t	clusters[MAX_TOTAL_ENT_LEAFS];
    int32_t area = 0;
    mnode_t *topnode = nullptr;

	// Ensure map is loaded.
	if (!cl.bsp ||!cl.bsp->nodes) {
		return;		
	}
	
	// set the size
	ent->size = ent->maxs - ent->mins;

    // set the abs box
    if (ent->solid == Solid::BSP &&
        (ent->currentState.angles[0] || ent->currentState.angles[1] || ent->currentState.angles[2])) {
        // expand for rotation
        float   max, v;
        int     i;

        max = 0;
        for (i = 0; i < 3; i++) {
            v = fabs(ent->mins[i]);
            if (v > max)
                max = v;
            v = fabs(ent->maxs[i]);
            if (v > max)
                max = v;
        }

		// Calcualte absMin and absMax for rotated BSP Entity.
		ent->absMin = ent->currentState.origin - vec3_t{max, max, max};
		ent->absMax = ent->currentState.origin + vec3_t{max, max, max};
    } else {
        // Normal.
        ent->absMin = ent->currentState.origin + ent->mins; //VectorAdd(ent->currentState.origin, ent->mins, ent->absMin);
        ent->absMax = ent->currentState.origin + ent->maxs; //VectorAdd(ent->currentState.origin, ent->maxs, ent->absMax);
    }

    // because movement is clipped an epsilon away from an actual edge,
    // we must fully check even when bounding boxes don't quite touch
    ent->absMin[0] -= 1;
    ent->absMin[1] -= 1;
    ent->absMin[2] -= 1;
    ent->absMax[0] += 1;
    ent->absMax[1] += 1;
    ent->absMax[2] += 1;

// link to PVS leafs
    ent->numClusters = 0;
    ent->areaNumber = 0;
    ent->areaNumber2 = 0;

    //get all leafs, including solids
    int32_t numberOfLeafs = CM_BoxLeafs(cm, ent->absMin, ent->absMax, leafs, MAX_TOTAL_ENT_LEAFS, &topnode);

    // set areas
    for (int32_t i = 0; i < numberOfLeafs; i++) {
        clusters[i] = CM_LeafCluster(leafs[i]);
        area = CM_LeafArea(leafs[i]);
        if (area) {
            // doors may legally straggle two areas,
            // but nothing should evern need more than that
            if (ent->areaNumber && ent->areaNumber != area) {
                if (ent->areaNumber2 && ent->areaNumber2 != area && sv.serverState == ServerState::Loading) {
                    Com_DPrintf("Object touching 3 areas at %f %f %f\n",
                                ent->absMin[0], ent->absMin[1], ent->absMin[2]);
                }
                ent->areaNumber2 = area;
            } else {
                ent->areaNumber = area;
			}
        }
    }

    if (numberOfLeafs >= MAX_TOTAL_ENT_LEAFS) {
        // assume we missed some leafs, and mark by headNode
        ent->numClusters = -1;
        ent->headNode = CM_NumNode(cm, topnode);
    } else {
        ent->numClusters = 0;
        for (int32_t i = 0; i < numberOfLeafs; i++) {
            if (clusters[i] == -1) {
                continue;        // not a visible leaf
			}
			int32_t j = 0;
            for (j = 0; j < i; j++) {
                if (clusters[j] == clusters[i]) {
                    break;
				}
			}
            if (j == i) {
                if (ent->numClusters == MAX_ENT_CLUSTERS) {
                    // assume we missed some leafs, and mark by headNode
                    ent->numClusters = -1;
                    ent->headNode = CM_NumNode(cm, topnode);
                    break;
                }

                ent->clusterNumbers[ent->numClusters++] = clusters[i];
            }
        }
    }
}

/**
*	@brief	Removes the entity for collision testing.
**/
void CL_World_UnlinkEntity(Entity *ent) {
    if (!ent->area.prev) {
        return;        // not linked in anywhere
	}
    List_Remove(&ent->area);
    ent->area.prev = ent->area.next = NULL;
}

/**
*	@brief	Determines what area an entity resides in and "links it in for collision testing".
*			Finds the area to link the entity in to and sets its bounding box in case it is an
*			actual inline bsp model.
**/
void CL_PF_World_LinkEntity(Entity *ent) {
	// Unlink from previous old position.
	if (ent->area.prev) {
        CL_World_UnlinkEntity(ent);
	}

	// Ensure it isn't the worldspawn entity itself.
    if (ent == cs.entities) {
        return;        // Don't add the world
	}

	// Ensure it is in use.
    if (!ent->inUse) {
        Com_DPrintf("%s: entity %d is not in use\n", __func__, NUM_FOR_EDICT(ent));
        return;
    }

	// Ensure a map is loaded properly in our cache.
	if (!cl.bsp || !cl.cm.cache || !cl.bsp) {
        return;
    }

	// Find the actual server entity.
    int32_t entityNumber = ent->currentState.number; //NUM_FOR_EDICT(ent);
    //PODEntity *clientEntity = &cs.entities[entityNumber];

    // Encode the size into the entity_state for client prediction reaspms/
    switch (ent->solid) {
    case Solid::BoundingBox:
        if ((ent->serverFlags & EntityServerFlags::DeadMonster) || VectorCompare(ent->mins, ent->maxs)) {
            ent->currentState.solid = 0;
            //serverEntity->solid32 = 0;
        } else {
			ent->currentState.solid = Solid::BoundingBox; //MSG_PackBoundingBox32(ent->mins, ent->maxs);
			ent->currentState.mins = ent->mins;
			ent->currentState.maxs = ent->maxs;
			//serverEntity->solid32 = ent->currentState.solid;//MSG_PackBoundingBox32(ent->mins, ent->maxs);
        }
        break;
    case Solid::OctagonBox:
        if ((ent->serverFlags & EntityServerFlags::DeadMonster) || VectorCompare(ent->mins, ent->maxs)) {
            ent->currentState.solid = 0;
            //serverEntity->solid32 = 0;
        } else {
			ent->currentState.solid = Solid::OctagonBox; //MSG_PackBoundingBox32(ent->mins, ent->maxs);
			ent->currentState.mins = ent->mins;
			ent->currentState.maxs = ent->maxs;
            //serverEntity->solid32 = ent->currentState.solid;//MSG_PackBoundingBox32(ent->mins, ent->maxs);
        }
        break;
    case Solid::BSP:
        ent->currentState.solid = PACKED_BBOX;      // a Solid::BoundingBox will never create this value
		ent->currentState.mins = vec3_zero();
		ent->currentState.maxs = vec3_zero();
		//serverEntity->solid32 = PACKED_BBOX;     // FIXME: use 255?
        break;
    default:
        ent->currentState.solid = 0;
		ent->currentState.mins = vec3_zero();
		ent->currentState.maxs = vec3_zero();
		//serverEntity->solid32 = 0;
        break;
    }

    CL_World_LinkEntity(&cl.cm, ent);

    // If first time, make sure oldOrigin is valid.
    if (!ent->linkCount) {
        ent->currentState.oldOrigin = ent->currentState.origin;
    }
    ent->linkCount++;

	// 
    if (ent->solid == Solid::Not) {
        return;
	}

	// Find the first node that the ent's box crosses.
    areanode_t *node = cl_areanodes;
    while (1) {
		if (node->axis == -1) {
            break;
		}
        if (ent->absMin[node->axis] > node->dist) {
            node = node->children[0];
		} else if (ent->absMax[node->axis] < node->dist) {
            node = node->children[1];
		} else {
            break; // Crosses the node
		}
    }

    // Link it in
	if (ent->isLocal) {
	    //if (ent->solid == Solid::Trigger) {
		//    List_Append(&node->triggerLocalClientEdicts, &ent->area);
		//} else {
	    if (ent->solid == Solid::Trigger) {
		    List_Append(&node->triggerEdicts, &ent->area);
		} else {
	       // List_Append(&node->solidEdicts, &ent->area);
			List_Append(&node->solidLocalClientEdicts, &ent->area);
		}
		//}
	} else {
	    if (ent->solid == Solid::Trigger) {
		    List_Append(&node->triggerEdicts, &ent->area);
		} else {
	        List_Append(&node->solidEdicts, &ent->area);

			//			List_Append(&node->solidLocalClientEdicts, &ent->area);
		}
	}
}


/**
*	@brief	The inner workings of CL_AreaEntities.
**/
static void CL_World_AreaEntities_r(areanode_t *node) {
    list_t *start = nullptr;
    PODEntity *check = nullptr;

	if (!cl.bsp || !cl.cm.cache || !cl.bsp) {
		return;
	}

    // touch linked edicts
    if (areaType == AreaEntities::Solid) {
        start = &node->solidEdicts;
	} else if (areaType == AreaEntities::LocalSolid) {
		start = &node->solidLocalClientEdicts;
	} else {
        start = &node->triggerEdicts;
	}
 //   if (areaType == AreaEntities::LocalSolid) {
	//	start = &node->solidLocalClientEdicts;
	//} else {
 //       start = &node->solidEdicts;
	//}

    LIST_FOR_EACH(PODEntity, check, start, area) {
        if (!check || check->solid == Solid::Not) {
            continue;        // deactivated
		}
        if (check->absMin[0] > areaMaxs[0]
            || check->absMin[1] > areaMaxs[1]
            || check->absMin[2] > areaMaxs[2]
            || check->absMax[0] < areaMins[0]
            || check->absMax[1] < areaMins[1]
            || check->absMax[2] < areaMins[2]) {
            continue;        // not touching
		}

        if (areaCount == areaMaxCount) {
            Com_WPrintf("CL_AreaEntities: MAXCOUNT\n");
            return;
        }

        areaList[areaCount] = check;
        areaCount++;
    }

    if (node->axis == -1)
        return;        // terminal node

    // recurse down both sides
    if (areaMaxs[node->axis] > node->dist)
        CL_World_AreaEntities_r(node->children[0]);
    if (areaMins[node->axis] < node->dist)
        CL_World_AreaEntities_r(node->children[1]);
}

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_World_AreaEntities(const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype) {
    areaMins = mins;
    areaMaxs = maxs;
    areaList = list;
    areaCount = 0;
    areaMaxCount = maxcount;
    areaType = areatype;

	if ( cl.bsp && cl.cm.cache ) {
	    CL_World_AreaEntities_r(cl_areanodes);
	}

    return areaCount;
	//return 0;
}


//===========================================================================

/**
*	@return	Returns a headNode that can be used for testing or clipping an
*			object of mins/maxs size.
**/
static mnode_t *CL_World_HullForEntity(Entity *ent)
{
	if (!cl.bsp || !cl.cm.cache || !cl.bsp) {
		return nullptr;
	}
    if (ent->solid == Solid::BSP) {
        int32_t i = ent->currentState.modelIndex - 1;

        // Explicit hulls in the BSP model.
        if (i <= 0 || i >= cl.bsp->nummodels) {
            Com_Error(ErrorType::Drop, "%s: inline model %d out of range", __func__, i);
		}

        return cl.clipModels[i]->headNode;//cl.cm.cache->models[i].headNode;
    }

    // create a temp hull from bounding box sizes
    if (ent->solid == Solid::OctagonBox) {
        return CM_HeadnodeForOctagon(ent->mins, ent->maxs);
    } else {
        return CM_HeadnodeForBox(ent->mins, ent->maxs);
    }
}

/**
*	@brief	Specialized server implementation of PointContents function.
**/
int32_t CL_World_PointContents(const vec3_t &point)
{
    static PODEntity     *touch[MAX_CLIENT_POD_ENTITIES], *hit = nullptr;
    
	// Ensure all is sane.
    if (!cl.bsp || !cl.cm.cache || !cl.bsp) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return 0;
	}

    // get base contents from world
    int32_t contents = CM_PointContents(point, cl.bsp->nodes);

    // or in contents from all the other entities
    int32_t numberOfAreaEntities = CL_World_AreaEntities(point, point, touch, MAX_CLIENT_POD_ENTITIES, AreaEntities::Solid);

    for (int32_t i = 0; i < numberOfAreaEntities; i++) {
		// Acquire touch entity.
        PODEntity*hit = touch[i];

        // Might intersect, so do an exact clip
		if (hit != nullptr) {
			contents |= CM_TransformedPointContents(point, CL_World_HullForEntity(hit), hit->currentState.origin, hit->currentState.angles);
		}
    }

    return contents;
}

/**
*	@brief	Will clip the move of the bounding box to the world entities.
**/
static void CL_World_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *passedict, int32_t contentMask, TraceResult *tr) {
    // Actual box mins and maxs that are used for clipping with.
	vec3_t boxMins = vec3_zero();
	vec3_t boxMaxs = vec3_zero();

    // Create the bounding box of the entire move.
    for (int32_t i = 0; i < 3; i++) {
        if (end[i] > start[i]) {
            boxMins[i] = start[i] + mins[i] - 1;
            boxMaxs[i] = end[i] + maxs[i] + 1;
        } else {
            boxMins[i] = end[i] + mins[i] - 1;
            boxMaxs[i] = start[i] + maxs[i] + 1;
        }
    }


	// Solid list.
	static PODEntity *touchEntityList[MAX_CLIENT_POD_ENTITIES];
	PODEntity *touchEntity = nullptr;
    int32_t numberOfAreaEntities = CL_World_AreaEntities(boxMins, boxMaxs, touchEntityList, MAX_CLIENT_POD_ENTITIES, AreaEntities::Solid);

    // Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered)
    for (int32_t i = 0; i < numberOfAreaEntities; i++) {
        touchEntity = touchEntityList[i];
        if (touchEntity->solid == Solid::Not) {
            continue;
		}
        if (touchEntity == passedict) {
            continue;
		}
		if (tr->allSolid) {
            return;
		}
        if (passedict) {
            if (touchEntity->owner == passedict) {
                continue;    // Don't clip against own missiles.
			}
            if (passedict->owner == touchEntity) {
                continue;    // Don't clip against owner.
			}
        }

        if (!(contentMask & BrushContents::DeadMonster) && (touchEntity->serverFlags & EntityServerFlags::DeadMonster)) {
            continue;
		}

        // Might intersect, so do an exact clip
		TraceResult trace = CM_TransformedBoxTrace(start, end, mins, maxs, CL_World_HullForEntity(touchEntity), contentMask, touchEntity->currentState.origin, touchEntity->currentState.angles);

		// Finalize trace results and clip to entity.
        CM_ClipEntity(tr, &trace, touchEntity);
    }

	static PODEntity *localTouchEntityList[MAX_CLIENT_POD_ENTITIES];
    int32_t numberOfLocalAreaEntities = CL_World_AreaEntities(boxMins, boxMaxs, localTouchEntityList, MAX_CLIENT_POD_ENTITIES, AreaEntities::LocalSolid);

    // Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered)
    for (int32_t i = 0; i < numberOfLocalAreaEntities; i++) {
        touchEntity = localTouchEntityList[i];
        if (touchEntity->solid == Solid::Not) {
            continue;
		}
        if (touchEntity == passedict) {
            continue;
		}
		if (tr->allSolid) {
            return;
		}
        if (passedict) {
            if (touchEntity->owner == passedict) {
                continue;    // Don't clip against own missiles.
			}
            if (passedict->owner == touchEntity) {
                continue;    // Don't clip against owner.
			}
        }

        if (!(contentMask & BrushContents::DeadMonster) && (touchEntity->serverFlags & EntityServerFlags::DeadMonster)) {
            continue;
		}

        // Might intersect, so do an exact clip
		TraceResult trace = CM_TransformedBoxTrace(start, end, mins, maxs, CL_World_HullForEntity(touchEntity), contentMask, touchEntity->currentState.origin, touchEntity->currentState.angles);

		// Finalize trace results and clip to entity.
        CM_ClipEntity(tr, &trace, touchEntity);
    }
}

/**
*	@brief	Moves the given mins/maxs volume through the world from start to end.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult CL_World_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *passedict, int32_t contentMask) {
    if (!cl.bsp || !cl.cm.cache || !cl.bsp) {
        //Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return TraceResult();
    }

    // clip to world
    TraceResult trace = CM_TransformedBoxTrace(start, end, mins, maxs, cl.bsp->nodes, contentMask, vec3_zero(), vec3_zero());
    trace.ent = ge->entities;
    if (trace.fraction == 0) {
        return trace;   // Blocked by the world
    }

    // clip to other solid entities
    CL_World_ClipMoveToEntities(start, mins, maxs, end, passedict, contentMask, &trace);
    return trace;
}

