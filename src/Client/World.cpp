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
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"
#include "GameModule.h"
#include "World.h"

// TODO: REMOVE
const bbox3_t CM_Matrix_TransformBounds( const glm::mat4 &matrix, const bbox3_t &bounds );
const bbox3_t CM_EntityBounds( const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds );


/**
*	@return	Returns a headNode that can be used for testing or clipping an
*			object of mins/maxs size.
**/
mnode_t *CL_World_HullForEntity( Entity *ent ) {
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		return nullptr;
	}

    if ( ent->solid == Solid::BSP || ent->solid == PACKED_BSP ) {//Solid::BSP ) {
        const int32_t i = ent->currentState.modelIndex - 1;

		if (i < 0) {
			return nullptr;
		}

        // Explicit hulls in the BSP model.
        if ( i < 0 || i >= cl.cm.cache->nummodels ) {
            Com_Error(ErrorType::Drop, "%s: inline model %d out of range", __func__, i);
		}
		
        return cl.cm.cache->models[i].headNode;
    }

	// Determine brush contents to use:
	// - Defaults to Solid.
	// - Client and Monsters together share the Monster brush contents.
	// - DeadMonster flagged entities override (hence, we seek for them first.)
	int32_t brushContents = BrushContents::Solid;
	// Ensure DeadMonster flags override possible Monster brush contents by checking for DeadMonster first.
	if ( ent->serverFlags & EntityServerFlags::DeadMonster ) {
		brushContents = BrushContents::DeadMonster;
	} else if ( ent->client ) {
		brushContents = BrushContents::Monster;
	} else if ( ent->serverFlags & EntityServerFlags::Monster ) {
		brushContents = BrushContents::Monster;
	}

	// Otherwise, generate a temporary hull for the entity's bounds to trace with.
	if (ent->solid == Solid::Not) {
		return nullptr;
	// Octagon Hull:
	} else if (ent->solid == Solid::OctagonBox) {
		return CM_HeadnodeForOctagon( ent->bounds, brushContents );
	// Capsule Hull:
	} else if ( ent->solid == Solid::Capsule ) {
		return CM_HeadnodeForCapsule( ent->bounds, brushContents );
	// Sphere Hull:
	} else if ( ent->solid == Solid::Sphere ) {
		return CM_HeadnodeForSphere( ent->bounds, brushContents );
	// Default BoundingBox Hull:
    } else {
        return CM_HeadnodeForBox( ent->bounds, brushContents  );
    }

	return nullptr;
}

/**
*
*	World Entity Area Grid
*
*	Note: this use of "area" is different from the bsp file use
*
**/
// Area Grid Node.
struct ClientWorldAreaNode {
	//! Axis describing this area node.
    int32_t axis = 0;       // -1 = leaf node
	//! Distance of specified area.
    float dist = 0;

	//! Child nodes, containing yet another level of subdivided areas.
    ClientWorldAreaNode *children[2];

	//! Lists per registered entity type in specified node.
    list_t  triggerEdicts;
    list_t  solidEdicts;
    list_t  solidLocalClientEdicts;
};

/**
*	Stores all entity's in areas that define a 'grid like structure', for a lack of better words.
**/
struct ClientWorld {
	// Area Grid configuration.
	static constexpr int32_t AREA_DEPTH	= 4;
	static constexpr int32_t AREA_NODES	= 32;

	//! Area nodes.
	ClientWorldAreaNode areaNodes[AREA_NODES] = {};
	//! Number of total area nodes.
	int32_t numberOfAreaNodes = 0;

	/**
	*	Contains the actual stats of the area being tested for entities.
	**/
	struct Area {
		//! Bounding box of area to test for.
		bbox3_t areaBounds;

		//! Current area count.
		int32_t areaEntityCount = 0;
		//! Max area count.
		int32_t areaEntityMaxCount = 0;
		//! Area type.
		int32_t areaType = 0;

		//! Actual entity list.
		PODEntity **areaEntityList = nullptr;
	} areaTest;
} clientWorld;

//// Area nodes array.
//static ClientWorldAreaNode cl_areanodes[AREA_NODES];
//static int32_t cl_numareanodes = 0;
//
//// Area Mins/Maxs.
//static vec3_t areaMins	= vec3_zero();
//static vec3_t areaMaxs	= vec3_zero();
//
//// List of entities in an area.
//static Entity  **areaList	= nullptr;
//
//// Area stats.
//static int32_t areaCount	= 0;
//static int32_t areaMaxCount	= 0;
//static int32_t areaType		= 0;

/**
*	@brief Builds a uniformly subdivided tree for the given world size
**/
static ClientWorldAreaNode *CL_CreateAreaNode( const int32_t depth, const bbox3_t &bounds ) {
	// Acquire us the current free area node.
	ClientWorldAreaNode *areaNode = &clientWorld.areaNodes[ clientWorld.numberOfAreaNodes ];
	// Increment num of nodes.
	clientWorld.numberOfAreaNodes++;

	// Initialize trigger entities list.
	List_Init( &areaNode->triggerEdicts );
	// Initialize solid entities list.
	List_Init( &areaNode->solidEdicts );

	// If we've reached the area's depth, stop subdividing and ensure we 
	// return a node with -1 axis and no children.
	if ( depth == clientWorld.AREA_DEPTH ) {
		areaNode->axis = -1;
		areaNode->children[0] = areaNode->children[1] = nullptr;

		return areaNode;
	}

	const vec3_t areaSize = bbox3_size( bounds );
    
	if ( areaSize.x > areaSize.y ) {
		areaNode->axis = 0;
	} else {
		areaNode->axis = 1;
	}

	areaNode->dist = 0.5 * ( bounds.maxs[ areaNode->axis ] + bounds.mins[ areaNode->axis ] );

	bbox3_t bounds1 = bounds;
	bbox3_t bounds2 = bounds;
	bounds1.maxs[ areaNode->axis ] = bounds2.mins[ areaNode->axis ] = areaNode->dist;

	areaNode->children[0] = CL_CreateAreaNode( depth + 1, bounds2 );
	areaNode->children[1] = CL_CreateAreaNode( depth + 1, bounds1 );

	return areaNode;
}

/**
*	@brief	Clear the server entity area grid world.
**/
void CL_ClearWorld() {
	// Clear nodes.
	memset( clientWorld.areaNodes, 0, sizeof( clientWorld.areaNodes ) );
	clientWorld.numberOfAreaNodes = 0;

	// If the world IS loaded up, recreate a fresh area grid to work from next time around.
	if ( cl.cm.cache && cl.cm.cache->nodes ) {
		mmodel_t *worldCollisionModel = &cl.cm.cache->models[ 0 ];
		CL_CreateAreaNode( 0, bbox3_t { worldCollisionModel->mins, worldCollisionModel->maxs } );
	}

	// Make sure all entities are unlinked
    for ( int32_t i = 0; i < MAX_CLIENT_POD_ENTITIES; i++ ) {
        PODEntity *podEntity = &cs.entities[ i ];
		if ( podEntity ) {
			podEntity->area.prev = podEntity->area.next = nullptr;
		}
    }
}

/**
*	@brief	Checks if edict is potentially visible from the given PVS row.
*	@return	True if visible.
**/
const bool CL_EntityIsVisible( cm_t *cm, Entity *ent, byte *mask ) {
    if (ent->numClusters == -1) {
        // too many leafs for individual check, go by headNode
        return CM_HeadnodeVisible( CM_NodeNum( cm, ent->headNode ), mask );
    }

    // check individual leafs
    for ( int32_t i = 0; i < ent->numClusters; i++ ) {
        if ( Q_IsBitSet( mask, ent->clusterNumbers[i] ) ) {
            return true;
        }
    }

    return true;  // not visible
}

/**
*	@brief	Links entity to PVS leafs.
**/
void CL_World_LinkEntity( cm_t *cm, PODEntity *podEntity, const bbox3_t &boxLeafsBounds ) {
    mleaf_t	*leafs[MAX_TOTAL_ENT_LEAFS];
    int32_t	clusters[MAX_TOTAL_ENT_LEAFS];
    int32_t area = 0;
    mnode_t *topnode = nullptr;
	int32_t i = 0, j = 0;

	/**
	*	Link to PVS leafs.
	**/
	// Reset cluster and area numbers first.
    podEntity->numClusters = 0;
    podEntity->areaNumber = 0;
    podEntity->areaNumber2 = 0;

    // Get all leafs, including solids.
    const int32_t numberOfLeafs = CM_BoxLeafs( cm, boxLeafsBounds, leafs, MAX_TOTAL_ENT_LEAFS, &topnode );
    
	// Set areas.
    for ( i = 0; i < numberOfLeafs; i++ ) {
        clusters[i] = CM_LeafCluster( leafs[i] );
        area = CM_LeafArea( leafs[i] );
        if ( area ) {
            // Doors may legally straggle two areas, but nothing should evern need more than that.
            if ( podEntity->areaNumber && podEntity->areaNumber != area) {
                if ( podEntity->areaNumber2 && podEntity->areaNumber2 != area && cl.serverState == ServerState::Loading ) {
                    Com_DPrintf( "Object touching 3 areas at %f %f %f\n",
                                podEntity->absoluteBounds.mins[0], podEntity->absoluteBounds.mins[1], podEntity->absoluteBounds.mins[2] );
                }
                podEntity->areaNumber2 = area;
            } else {
                podEntity->areaNumber = area;
			}
        }
    }

	// Set cluster/headnode.
    if ( numberOfLeafs >= MAX_TOTAL_ENT_LEAFS  ) {
        // assume we missed some leafs, and mark by headNode
        podEntity->numClusters = -1;
        podEntity->headNode = CM_NumNode( cm, topnode );
    } else {
        podEntity->numClusters = 0;
        for ( i = 0; i < numberOfLeafs; i++ ) {
            if ( clusters[i] == -1) {
                continue;        // not a visible leaf
			}
			j = 0;
            for ( j = 0; j < i; j++ ) {
                if ( clusters[j] == clusters[i] ) {
                    break;
				}
			}
            if ( j == i) {
                if ( podEntity->numClusters == MAX_ENT_CLUSTERS ) {
                    // assume we missed some leafs, and mark by headNode
                    podEntity->numClusters = -1;
                    podEntity->headNode = CM_NumNode( cm, topnode );
                    break;
                }

                podEntity->clusterNumbers[ podEntity->numClusters++ ] = clusters[ i ];
            }
        }
    }
}

/**
*	@brief	Removes the entity for collision testing.
**/
void CL_World_UnlinkEntity( PODEntity *podEntity ) {
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		return;
	}
    if ( !podEntity->area.prev ) {
        return;        // not linked in anywhere
	}
    List_Remove( &podEntity->area );
    podEntity->area.prev = podEntity->area.next = NULL;
}

/**
*	@brief	Determines what area an entity resides in and "links it in for collision testing".
*			Finds the area to link the entity in to and sets its bounding box in case it is an
*			actual inline bsp model.
**/
mmodel_t *CL_World_GetEntityModel(PODEntity *podEntity) {
	const int32_t i = podEntity->currentState.modelIndex - 1;

	// Explicit hulls in the BSP model.
	if ( i < 0 || i >= cl.cm.cache->nummodels ) {
		return nullptr;
	}
		
	return &cl.cm.cache->models[i];
}
void CL_PF_World_LinkEntity( PODEntity *podEntity ) {
	// Make sure world is loaded.
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		Com_DPrintf( "%s: no cl.cm.cache, cl.cm.cache.nodes, or cl.cm stuff\n", __func__ );
		return;
	}
	// Make sure the entity's valid.
	if ( !podEntity ) {
		Com_DPrintf( "%s: (nullptr) entity.\n", __func__ );
		return;
	}
	// Make sure we got a proper areagrid to link to.
	if ( !clientWorld.numberOfAreaNodes ) {
		return;
	}
	// Do NOT link world.
	const int32_t entityNumber = podEntity->clientEntityNumber;
	if ( podEntity == cs.entities || entityNumber == 0 ) {
		Com_DPrintf( "%s: ent == cs.entities\n", __func__ );
		return;
	}
	// Do not link client entity.
	//if ( entityNumber == cl.frame.clientNumber + 1 ) {
	//	//podEntity->currentState.mins = cl.predictedState.mins;
	//	//podEntity->currentState.maxs = cl.predictedState.maxs;
	//	return;
	//}
	
	// Unlink from previous old position.
	if ( podEntity->area.prev ) {
        CL_World_UnlinkEntity( podEntity );
	}

	// Make sure entity is in use.
	if ( podEntity->isLocal ) {
		if ( !podEntity->inUse && podEntity->clientEntityNumber > 21) {
			Com_DPrintf("%s: local entity %d is not in use\n", __func__, podEntity->clientEntityNumber);
		//	return;
		}
	} else {
		if ( !podEntity->inUse ) {
			Com_DPrintf("%s: packet entity %d is not in use\n", __func__, podEntity->clientEntityNumber);
			return;
		}
	}

	/**
	*	Decode the needed size data from the EntityState for proper client-side prediction needs.
	**/
	const int32_t testSolid = podEntity->currentState.solid;
	mmodel_t *brushModel = nullptr;
	switch ( testSolid ) {
		case Solid::BoundingBox:
			if ( ( podEntity->clientFlags & EntityClientFlags::DeadMonster ) || vec3_equal( podEntity->currentState.mins, podEntity->currentState.maxs ) ) {
				podEntity->solid = Solid::Not;
			} else {
				podEntity->solid = Solid::BoundingBox;
				podEntity->mins = podEntity->currentState.mins;
				podEntity->maxs = podEntity->currentState.maxs;
			}
			break;
		case Solid::Capsule:
			if ( ( podEntity->clientFlags & EntityClientFlags::DeadMonster ) || vec3_equal( podEntity->currentState.mins, podEntity->currentState.maxs ) ) {
				podEntity->solid = Solid::Not;
			} else {
				podEntity->solid = Solid::Capsule;
				podEntity->mins = podEntity->currentState.mins;
				podEntity->maxs = podEntity->currentState.maxs;
			}
			break;
		case Solid::Sphere:
			if ( ( podEntity->clientFlags & EntityClientFlags::DeadMonster ) || vec3_equal( podEntity->currentState.mins, podEntity->currentState.maxs ) ) {
				podEntity->solid = Solid::Not;
			} else {
				podEntity->solid = Solid::Sphere;
				podEntity->mins = podEntity->currentState.mins;
				podEntity->maxs = podEntity->currentState.maxs;
			}
			break;
		case Solid::OctagonBox:
			if ( ( podEntity->clientFlags & EntityClientFlags::DeadMonster ) || vec3_equal( podEntity->currentState.mins, podEntity->currentState.maxs ) ) {
				podEntity->solid = Solid::Not;
			} else {
				podEntity->solid = Solid::OctagonBox;
				podEntity->mins = podEntity->currentState.mins;
				podEntity->maxs = podEntity->currentState.maxs;
			}
			break;
		case Solid::BSP: {
			podEntity->currentState.solid = PACKED_BSP;
			podEntity->solid = Solid::BSP;

			brushModel = &cl.cm.cache->models[podEntity->currentState.modelIndex - 1];
			podEntity->mins = brushModel->mins;
			podEntity->maxs = brushModel->maxs;
		}
		break;
		case PACKED_BSP: {
			podEntity->currentState.solid = PACKED_BSP;
			podEntity->solid = Solid::BSP;

			brushModel = &cl.cm.cache->models[podEntity->currentState.modelIndex - 1];
			podEntity->mins = brushModel->mins;
			podEntity->maxs = brushModel->maxs;
		}
		break;
		case Solid::Trigger:
			podEntity->solid = podEntity->currentState.solid = Solid::Not;
			break;
		default:
			podEntity->solid = Solid::Not;
			podEntity->mins = vec3_zero();
			podEntity->maxs = vec3_zero();
			// MATRIX:
			podEntity->bounds = bbox3_zero();
			// EOF MATRIX:
			break;
    }

	/**
	*	Update the actual bounds member to match the mins/maxs.
	**/
	// Determine whether it is a brush entity or not, they take special treatment.
	const bool isBrushEntity = ( podEntity->solid == Solid::BSP || podEntity->solid == PACKED_BSP ? true : false );
	// Get access to the brush model if needed.

	// Get brush model origin if needed.
	const vec3_t brushModelOrigin = ( brushModel ? brushModel->origin : vec3_zero() );

	// Adjust bounds depending on final solid.
	switch( podEntity->solid ) {
		case Solid::BoundingBox:
		case Solid::Capsule:
		case Solid::Sphere:
		case Solid::OctagonBox:
		case Solid::Trigger:
			// Unset the bounds.
			podEntity->bounds = bbox3_t{ podEntity->mins, podEntity->maxs };
			break;
		case Solid::BSP:
			// Adjust entity bounds to that of brush if need be.
			if ( isBrushEntity && brushModel ) {
				bbox3_t brushBounds = { brushModel->mins, brushModel->maxs };
				//vec3_t brushBoundsSize = bbox3_size( brushBounds );
				//bbox3_t centeredBrushBounds = bbox3_from_center_size( brushBoundsSize, brushModel->origin );
				podEntity->bounds = brushBounds;
				podEntity->mins = podEntity->bounds.mins;
				podEntity->maxs = podEntity->bounds.maxs;
			} else {
				podEntity->bounds = bbox3_t{ podEntity->mins, podEntity->maxs };
			}
			break;
		//case Solid::Trigger:
		case Solid::Not:
			podEntity->bounds = bbox3_zero();
			break;
		default:
			//podEntity->bounds = bbox3_zero();
			break;
	}

	// Set the box its size.
	podEntity->size = bbox3_size( podEntity->bounds );

	/**
	*	Calculate the entity's transform and invTransformices + its absolute bounds.
	**/
	const bool isClientEntity = ( podEntity->client && cl.frame.clientNumber == cl.clientNumber ? true : false );
	// Get angles for the entity:
	// - Clients, we only take the yaw.
	// - Non-Brush entities, we only take the yaw.
	// - Brush entities, we take the full rotation angles into consideration.
	vec3_t entityAngles = vec3_zero();//( isBrushEntity || podEntity->solid == Solid::OctagonBox ? podEntity->currentState.angles : vec3_zero() );
	// Brush Entities:
	if (isBrushEntity) {
		entityAngles = vec3_clamp_euler( podEntity->currentState.angles );
	// Non-Brush Entities + Solid::OctagonBox
	} else if ( podEntity->solid == Solid::OctagonBox ) {
		//// Client Entity:
		//if ( isClientEntity ) {
		//	entityAngles = { 0.f, podEntity->client->playerState.pmove.viewAngles[ vec3_t::Yaw ], 0.f };
		//// Non-Client Entity:
		//} else {
		//	entityAngles = { 0.f, podEntity->currentState.angles[ vec3_t::Yaw ], 0.f };
		//}
	} else {
		entityAngles = vec3_zero();
	}
	// Get the origin for the entity:
	// - BSP Brushes: Take their model offset origin into consideration
	// - Client Entity: Take predicted player origin.
	vec3_t entityOrigin = podEntity->currentState.origin;
	//if ( isBrushEntity ) {
	//
	//} else if ( isClientEntity ) {
	//	entityOrigin = cl.playerEntityOrigin;
	//}

	// Convert to GLM vecs.
	glm::vec3 vecEntityOrigin = phvec_to_glmvec3( entityOrigin ); // Need for rotating properly.
	glm::vec3 vecNegatedEntityOrigin = phvec_to_glmvec3( vec3_negate( entityOrigin ) ); // Need for rotating properly.

	// Vec used for creating final translation mat.
	glm::vec3 vecEntityOriginOffset = glm::vec3( 0.0, 0.0, 0.0 );

	// Scale:
	glm::mat4 matScale = glm::scale( ph_mat_identity(), glm::vec3( 1.0, 1.0, 1.0 ) );
	
	// Rotate:
	glm::mat4 matRotation = glm::mat4( 1.0 );
	// Brush Route:
	if ( isBrushEntity && brushModel && !vec3_equal( entityAngles, vec3_zero() ) ) {
		// Get brush origin.
		const vec3_t brushOrigin = (brushModel ? brushModel->origin : vec3_zero() );
		// Clamp the entity angles.
		const vec3_t eulerAngles = vec3_clamp_euler( { entityAngles.x, entityAngles.y, entityAngles.z } );

		// Fill in entity offset.
		vecEntityOriginOffset = phvec_to_glmvec3( brushOrigin );

		// Translate to point matrices.
		const glm::mat4 matBoxOriginTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3( brushOrigin ) );
		const glm::mat4 matInvBoxOriginTranslate = glm::inverse( matBoxOriginTranslate ); // glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( entityOrigin + brushOrigin ) ) ); //

		// Create orientation quat and euler quat to rotate about.
		const glm::quat quatOrientation( ph_mat_identity() );
		const glm::quat quatEuler = glm_quat_from_ph_euler( eulerAngles );
		const glm::quat quatRotation = glm::normalize( quatOrientation * quatEuler );

		// Convert quat to local box rotation.
		const glm::mat4 matBoxRotation = glm::mat4_cast( quatRotation );
		
		// Create actual entity rotation matrix.
		matRotation = matBoxOriginTranslate * matBoxRotation * matInvBoxOriginTranslate;
		//matRotation = matInvBoxOriginTranslate * matBoxRotation * matBoxOriginTranslate;
	// Non-Brush Route:
	} else {
		matRotation = ph_mat_identity();

		//// Get brush origin.
		//const vec3_t boxCenter = bbox3_center( podEntity->bounds );
		//// Clamp the entity angles.
		//const vec3_t eulerAngles = vec3_zero(); //vec3_clamp_euler( entityAngles );

		//vecEntityOriginOffset = phvec_to_glmvec3( boxCenter );

		//if ( isBrushEntity && brushModel ) {
		//	// Translate to point matrices.
		//	const glm::mat4 matBoxOriginTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3( boxCenter ) );
		//	const glm::mat4 matInvBoxOriginTranslate = glm::inverse( matBoxOriginTranslate ); // glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( entityOrigin + brushOrigin ) ) ); //

		//	// Create orientation quat and euler quat to rotate about.
		//	const glm::quat quatOrientation( ph_mat_identity() );
		//	const glm::quat quatEuler = CM_EulerToQuaternion( vec3_clamp_euler( { entityAngles.x, entityAngles.y, entityAngles.z } ) );
		//	const glm::quat quatRotation = glm::normalize( quatOrientation * quatEuler );

		//	// Convert quat to local box rotation.
		//	const glm::mat4 matBoxRotation = glm::mat4_cast( quatRotation );
		//
		//	// Create actual entity rotation matrix.
		//	matRotation = matBoxOriginTranslate * matBoxRotation * matInvBoxOriginTranslate;
		//	//matRotation = matInvBoxOriginTranslate * matBoxRotation * matBoxOriginTranslate;
		//} else {
		//	// Translate to point matrices.
		//	const glm::mat4 matBoxOriginTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3( boxCenter ) );
		//	const glm::mat4 matInvBoxOriginTranslate = glm::inverse( matBoxOriginTranslate ); // glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( entityOrigin + brushOrigin ) ) ); //

		//	// Create orientation quat and euler quat to rotate about.
		//	const glm::quat quatOrientation( ph_mat_identity() );
		//	const glm::quat quatEuler = CM_EulerToQuaternion( vec3_clamp_euler( { entityAngles.x, entityAngles.y, entityAngles.z } ) );
		//	const glm::quat quatRotation = glm::normalize( quatOrientation * quatEuler );

		//	// Convert quat to local box rotation.
		//	const glm::mat4 matBoxRotation = glm::mat4_cast( quatRotation );
		//
		//	// Create actual entity rotation matrix.
		//	matRotation = matBoxOriginTranslate * matBoxRotation * matInvBoxOriginTranslate;
		//	//matRotation = matInvBoxOriginTranslate * matBoxRotation * matBoxOriginTranslate;
		//}
	}
	
	// Translate:
	glm::mat4 matTranslation = glm::translate( ph_mat_identity(), vecEntityOrigin ); //+ vecEntityOriginOffset );

	// Calculate actual Polyhedron entity transform and invTransform matrices.
	glm::mat4 matEntityTransform = matTranslation * matRotation * matScale;
	glm::mat4 matInvEntityTransform = glm::inverse( matEntityTransform );

	// Calculate the entity's absolute bounds(world space).
	bbox3_t leafCheckBounds = podEntity->absoluteBounds = CM_EntityBounds( podEntity->solid, matEntityTransform, podEntity->bounds );

	// TODO: These are temporarily set, should be removed after making sure we are using absoluteBounds everywhere.	
	podEntity->absMin = podEntity->absoluteBounds.mins;
	podEntity->absMax = podEntity->absoluteBounds.maxs;

	// Transform our bounds if needed. (BSP Brush entities can rotate.)
	if ( matEntityTransform != ph_mat_identity() ) {// ph_mat_identity() ) {
		leafCheckBounds = CM_Matrix_TransformBounds( matInvEntityTransform, leafCheckBounds );
	}

	//if ( podEntity->currentState.number && podEntity->solid == Solid::OctagonBox ) {
	//	Com_LPrintf( PrintType::DeveloperWarning, fmt::format( "CL Entity(#{}, {}, {}, {} ) seated at ({}, {}, {}) angled at({}, {}, {})\n",
	//				podEntity->currentState.number,
	//				podEntity->currentState.origin.x,
	//				podEntity->currentState.origin.y,
	//				podEntity->currentState.origin.z,
	//				matEntityTransform[3][0],
	//				matEntityTransform[3][1],
	//				matEntityTransform[3][2],
	//				podEntity->currentState.angles.x,
	//				podEntity->currentState.angles.y,
	//				podEntity->currentState.angles.z
	//	).c_str());
	//}
	/**
	*	Link entity to our PVS Leafs, and exit early out of this function if we're dealing
	*	with a Solid::Not entity. If we don't, we'd be adding it to the list of solids.
	**/
    CL_World_LinkEntity( &cl.cm, podEntity, leafCheckBounds );

    // If first time, make sure oldOrigin is valid.
    if ( !podEntity->linkCount ) {
        podEntity->currentState.oldOrigin = podEntity->currentState.origin;
    }
    podEntity->linkCount++;

	// Only local entities are truly Solid::Not tested.
    if ( /*podEntity->isLocal && */podEntity->solid == Solid::Not ) {
		if ( podEntity->inUse == true ) {
		//	Com_LPrintf( PrintType::DeveloperWarning, "%s\n", fmt::format( "ent(#{}): solid(Solid::Not), linkCount({})", podEntity->clientEntityNumber, podEntity->linkCount).c_str());
		}
        return;
	}

	/**
	*	Link entity into our World Areas for tracing efficiency.
	**/
	// Find the first node that the ent's box crosses.
    ClientWorldAreaNode *areaNode = clientWorld.areaNodes;
	// Loop until we've found it.
    while ( 1 ) {
		// If we're in the last node, depth == - 1, break out, we can't go on any further.
		if ( areaNode->axis == -1 ) {
            break;
		}
		// See if it's in front the distance of said node.
        if ( podEntity->absoluteBounds.mins[ areaNode->axis ] > areaNode->dist ) {
            areaNode = areaNode->children[0];
		} else if ( podEntity->absoluteBounds.maxs[ areaNode->axis ] < areaNode->dist ) {
            areaNode = areaNode->children[1];
		} else {
            break; // Crosses the node
		}
    }

    // Link it in
	if ( podEntity->isLocal ) {
		// TODO: Might need separate lists some day.
	    if ( podEntity->solid == Solid::Trigger ) {
		    List_Append( &areaNode->triggerEdicts, &podEntity->area );
		} else {
	        List_Append( &areaNode->solidEdicts, &podEntity->area);
		}
	} else {
	    if ( podEntity->solid == Solid::Trigger ) {
		    List_Append( &areaNode->triggerEdicts, &podEntity->area );
		} else {
	        List_Append( &areaNode->solidEdicts, &podEntity->area );
		}
	}

	/**
	*	And last but not least, assign the newly calculated transform and inverted transform matrices.
	**/
	podEntity->translateMatrix = matEntityTransform;
	podEntity->invTranslateMatrix = matInvEntityTransform;

	//if ( !podEntity->client && podEntity->solid == Solid::OctagonBox ) {
	//	Com_LPrintf( PrintType::Warning, fmt::format( "CL Entity(#{}, {},{},{} ) seated at ({}, {}, {}) angled at({}, {}, {})\n",
	//				podEntity->currentState.number,
	//				podEntity->currentState.origin.x,
	//				podEntity->currentState.origin.y,
	//				podEntity->currentState.origin.z,
	//				podEntity->translateMatrix[3][0],
	//				podEntity->translateMatrix[3][1],
	//				podEntity->translateMatrix[3][2],
	//				podEntity->currentState.angles.x,
	//				podEntity->currentState.angles.y,
	//				podEntity->currentState.angles.z
	//	).c_str());
	//}
}


/**
*	@brief	The inner workings of CL_AreaEntities.
**/
static void CL_World_AreaEntities_r( ClientWorldAreaNode *areaNode ) {
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		return;
	}

	// Determine what list to search in depending on the queried entity type.
	list_t *start = nullptr;
	if ( clientWorld.areaTest.areaType == AreaEntities::Solid ) {
		start = &areaNode->solidEdicts;
	} else {
		start = &areaNode->triggerEdicts;
	}

	PODEntity *podCheck = nullptr;
	LIST_FOR_EACH(PODEntity, podCheck, start, area ) {
		if ( podCheck->solid == Solid::Not ) {
			continue;        // deactivated
		}
		//if (check->absMin[0] > areaMaxs[0]
		//	|| check->absMin[1] > areaMaxs[1]
		//	|| check->absMin[2] > areaMaxs[2]
		//	|| check->absMax[0] < areaMins[0]
		//	|| check->absMax[1] < areaMins[1]
		//	|| check->absMax[2] < areaMins[2]) {
		//		continue;        // not touching
		//}

		if ( bbox3_intersects( podCheck->absoluteBounds, clientWorld.areaTest.areaBounds ) ) {
			clientWorld.areaTest.areaEntityList[ clientWorld.areaTest.areaEntityCount ] = podCheck;
			clientWorld.areaTest.areaEntityCount++;

			if ( clientWorld.areaTest.areaEntityCount == clientWorld.areaTest.areaEntityMaxCount ) {
				Com_WPrintf( "SV_AreaEntities: MAXCOUNT\n" );
				return;
			}
		}
	}

	// Terminal node.
	if ( areaNode->axis == -1 ) {
		return;
	}

	// Recurse down both sides.
	if ( clientWorld.areaTest.areaBounds.maxs[ areaNode->axis ] > areaNode->dist ) {
		CL_World_AreaEntities_r( areaNode->children[0] );
	}
	if ( clientWorld.areaTest.areaBounds.mins[ areaNode->axis ] < areaNode->dist ) {
		CL_World_AreaEntities_r( areaNode->children[1] );
	}
}

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int32_t CL_World_AreaEntities( const vec3_t &mins, const vec3_t &maxs, PODEntity **list, int32_t maxcount, int32_t areatype ) {
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		return 0;
	}

	clientWorld.areaTest.areaBounds = bbox3_t { mins, maxs };
    clientWorld.areaTest.areaEntityList = list;
    clientWorld.areaTest.areaEntityCount = 0;
    clientWorld.areaTest.areaEntityMaxCount = maxcount;
    clientWorld.areaTest.areaType = areatype;

    CL_World_AreaEntities_r( clientWorld.areaNodes );

	// Return the count of total entities found which now reside in our list.
    return clientWorld.areaTest.areaEntityCount;
} 



/***
*
*
*	Client World: Point Contents Testing
*
*
***/
/**
*	@brief	Determines the shape to consider for trace testing with.
**/
struct ClientTraceShape {
	static constexpr int32_t Box = 0;
	static constexpr int32_t Sphere = 1;
};

/**
*	@brief	ClientTrace object.
**/
struct ClientTrace {
	//! The shape we're tracing with.
	int32_t traceShape = ClientTraceShape::Box;

	//! Start point of trace.
	vec3_t start = vec3_zero();
	//! End point of trace.
	vec3_t end = vec3_zero();
	//! The bounds of the hull being traced with.
	bbox3_t bounds = bbox3_zero();
	//! The absolute bounds of the entire move.
	bbox3_t absoluteBounds = bbox3_zero();

	//! The 'brush' contents mask we're tracing against.
	int32_t brushContentsMask = 0;
	//! The entity to skip.
	PODEntity *skipEntity = nullptr;

	//! The actual final traceresult acquired.
	TraceResult traceResult;
};

/**
*	@brief	Specialized server implementation of PointContents function.
**/
int32_t CL_World_PointContents( const vec3_t &point ) {
    static PODEntity *areaEntities[ MAX_CLIENT_POD_ENTITIES ];
    
	// Ensure all is sane.
    if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return 0;
	}

    // get base contents from world
    int32_t contents = CM_PointContents( &cl.cm, point, cl.cm.cache->nodes, ph_mat_identity() );

    // or in contents from all the other entities
    const int32_t numberOfAreaEntities = CL_World_AreaEntities( point, point, areaEntities, MAX_CLIENT_POD_ENTITIES, AreaEntities::Solid );

    for ( int32_t i = 0; i < numberOfAreaEntities; i++ ) {
		// Acquire touch entity.
        PODEntity *clipEntity = areaEntities[i];

		// Get our hull to trace with.
		mnode_t *clipEntityHull = CL_World_HullForEntity( clipEntity );
		if ( clipEntityHull ) {
			// Might intersect, so do an exact clip
			contents |= CM_PointContents( &cl.cm, point, clipEntityHull, clipEntity->invTranslateMatrix );
		}
	}

    return contents;
}



/***
*
*
*	Client World: World/Entity Clipping Traces
*
*
***/
/**
*	@brief	Inspects whether the entity should be skipped from trace clip testing.
*	@return	True if it should, false otherwise.
**/
static const bool CL_World_ClipTraceSkipEntity( ClientTrace &clientTrace, PODEntity *clipEntity ) {
	if ( clientTrace.skipEntity ) {
		// Skip testing the entity if it is identical to our 'skipEntity'.
		if ( clipEntity == clientTrace.skipEntity ) {
			return true;
		}

		// Skip testing the entity if we are its owner.
		if ( clipEntity->owner == clientTrace.skipEntity ) {
			return true;
		}

		// If the skip entity has an owner:
		if ( clientTrace.skipEntity->owner ) {
			// Skip testing if it is our clip entity.
			if ( clipEntity == clientTrace.skipEntity->owner ) {
				return true;
			}
			// Visa versa.
			if ( clipEntity->owner == clientTrace.skipEntity->owner ) {
				return true;
			}
		}

		// Ensure triggers ONLY clip to the world brushes, allow others to 'occupy' them.
		if ( clientTrace.skipEntity->solid == Solid::Trigger ) {
			if ( clipEntity->solid != Solid::BSP && clipEntity->solid != 31 ) {
				return true;
			}
		}
	}

	// Skip dead monster contentmasked and serverflagged entities.
    if ( !( clientTrace.brushContentsMask & BrushContents::DeadMonster ) && ( clipEntity->clientFlags & EntityClientFlags::DeadMonster ) ) {
        return true;
	}
		
	// Don't skip.
	return false;
}

/**
*	@brief	Performs a specific shape type trace for use with clip to entities functions below.
*	@return	The trace results.
**/
static TraceResult CL_World_TraceClipShapeToEntity( ClientTrace &clientTrace, mnode_t *headNode, PODEntity *clipEntity ) {
	if ( clipEntity->translateMatrix == ph_mat_identity() ) {
		// 'Sphere' Trace:
		if ( clientTrace.traceShape == ClientTraceShape::Sphere ) {
			return CM_SphereTrace( &cl.cm, clientTrace.start, clientTrace.end, clientTrace.bounds, headNode, clientTrace.brushContentsMask );
		// 'Box' Trace:
		} else {
			return CM_BoxTrace( &cl.cm, clientTrace.start, clientTrace.end, clientTrace.bounds, headNode, clientTrace.brushContentsMask );
		}
	} else {
		// 'Sphere' TransformedTrace:
		if ( clientTrace.traceShape == ClientTraceShape::Sphere ) {
			return CM_TransformedSphereTrace( &cl.cm, clientTrace.start, clientTrace.end, clientTrace.bounds, headNode, clientTrace.brushContentsMask, clipEntity->translateMatrix, clipEntity->invTranslateMatrix );
		// 'Box' TransformedTrace:
		} else {
			return CM_TransformedBoxTrace( &cl.cm, clientTrace.start, clientTrace.end, clientTrace.bounds, headNode, clientTrace.brushContentsMask, clipEntity->translateMatrix, clipEntity->invTranslateMatrix );
		}
	}
}
/**
*	@brief	Performs a clip trace to a single entity.
*	@brief	True in case we can't trace properly(i.e, we're in allSolid)
**/
static const bool CL_World_ClipTraceToEntity( ClientTrace &clientTrace, PODEntity *clipEntity ) {
	// Stores actual trace results.
	TraceResult trace;

	// Skip non solids.
	//if ( clipEntity->isLocal && clipEntity->solid == Solid::Not ) {
	if ( clipEntity->solid == Solid::Not ) {
        return false;
	}

	// See if we can skip testing this actual entity.
	if ( CL_World_ClipTraceSkipEntity( clientTrace, clipEntity ) == true ) {
		return false;
	}
    

	// Get the entity's headNode to use for clipping.
	mnode_t *headNode = CL_World_HullForEntity( clipEntity );
	if ( !headNode ) {
	//	return false;
	}

	// Use the desire sphere shape trace to clip to the clipEntity.
	trace = CL_World_TraceClipShapeToEntity( clientTrace, headNode, clipEntity );

	// Finalize trace results and clip to entity.
    //CM_ClipEntity( &serverTrace.traceResult, &trace, clipEntity );
	if ( trace.allSolid || trace.fraction < clientTrace.traceResult.fraction ) {
		clientTrace.traceResult = trace;
		clientTrace.traceResult.ent = clipEntity;
	} else if ( trace.startSolid ) {
		clientTrace.traceResult.startSolid = true;
	}

	// Exit if the trace resulted in an 'All-Solid' 'Brush'.
	if ( trace.allSolid ) {
		return true;
	}

	return false;
}

/**
*	@brief	Will clip the move of the bounding box to the world entities.
**/
static void CL_World_ClipTraceToEntities( ClientTrace &clientTrace ) {
		// Solid list.
	static PODEntity *areaEntityList[ MAX_CLIENT_POD_ENTITIES ];
    const int32_t numberOfAreaEntities = CL_World_AreaEntities( clientTrace.absoluteBounds.mins, clientTrace.absoluteBounds.maxs, areaEntityList, MAX_CLIENT_POD_ENTITIES, AreaEntities::Solid );

    // Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered)
    for ( int32_t i = 0; i < numberOfAreaEntities; i++ ) {
        // Fetch 'touched' entity
		PODEntity *clipEntity = areaEntityList[i];

		// If for whatever reason our clip entity is non solid, skip it.
        //if ( clipEntity->isLocal && clipEntity->solid == Solid::Not ) {
		if ( clipEntity->solid == Solid::Not ) {
            continue;
		}

		// See if we can skip testing this actual entity.
		if ( CL_World_ClipTraceSkipEntity( clientTrace, clipEntity ) == true ) {
			continue;
		}

		// Get the entity's headNode to use for clipping.
		mnode_t *headNode = CL_World_HullForEntity( clipEntity );
		if ( !headNode ) {
			continue;
		}

		// Use the desire sphere shape trace to clip to the clipEntity.
		TraceResult trace = CL_World_TraceClipShapeToEntity( clientTrace, headNode, clipEntity );

		// Finalize trace results and clip to entity.
        //CM_ClipEntity( &serverTrace.traceResult, &trace, clipEntity );
		if ( trace.allSolid || trace.fraction < clientTrace.traceResult.fraction ) {
			clientTrace.traceResult = trace;
			clientTrace.traceResult.ent = clipEntity;
		} else if ( trace.startSolid ) {
			clientTrace.traceResult.startSolid = true;
		}

		// Exit if the trace resulted in an 'All-Solid' 'Brush'.
		if ( trace.allSolid ) {
			return;
		}
    }
}


/**
*	@brief	Tests and clips the specified trace to a single specific entity.
**/
const TraceResult CL_World_Clip( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, Entity *skipEntity, Entity *clipEntity, const int32_t contentMask, const int32_t traceShape = ClientTraceShape::Box ) {
	// Initialize a server world trace context.
	ClientTrace clientTrace = {
		// Tracing shape to use.
		.traceShape = traceShape,
		// Start/End of trace.		
		.start = start,
		.end = end,
		// Set bounds.
		.bounds = bbox3_t { mins, maxs },
		// Contents mask to search trace clips with.
		.brushContentsMask = contentMask,
		// Default trace result.
		.traceResult = {
			.fraction = 1.f,
			//.ent = ge->entities,
			.ent = nullptr
		}
	};

	// Ensure that our client's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		// For the client, return an empty trace since game logic might already be firing up
		// and awaiting a load still.
        //Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return clientTrace.traceResult;
    }

	// TODO: Do we need a skip entity when trying to clip against a single entity?
	clientTrace.skipEntity = skipEntity;
	// TODO: Do we need absoluteBounds when no move is made, thus clipping against a single entity?
	//clientTrace.absoluteBounds = CM_CalculateTraceBounds( start, end, clientTrace.bounds );

	// If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
	CL_World_ClipTraceToEntity( clientTrace, clipEntity );

	// And blast off!
	return clientTrace.traceResult;
}
/**
*	@brief	Moves the given mins/maxs volume through the world from start to end.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult CL_World_Trace( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, PODEntity *skipEntity, const int32_t contentMask, const int32_t traceShape = ClientTraceShape::Box ) {
	// Ensure that our client's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !cl.cm.cache || !cl.cm.cache->nodes ) {
		// For the client, return an empty trace since game logic might already be firing up
		// and awaiting a load still.
        //Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
		return TraceResult();
    }

	// Initialize a server world trace context.
	ClientTrace clientTrace = {
		// Tracing shape to use.
		.traceShape = traceShape,
		// Set bounds.
		.bounds = bbox3_t { mins, maxs }
	};

	// First perform a clipping trace to our world( The actual BSP tree itself).
	// 'Sphere' shape:
	if ( clientTrace.traceShape == ClientTraceShape::Sphere ) {
		clientTrace.traceResult = CM_SphereTrace( &cl.cm, start, end, clientTrace.bounds, cl.cm.cache->nodes, contentMask );
		// 'Box' shape by default.
	} else {
		clientTrace.traceResult = CM_BoxTrace( &cl.cm, start, end, clientTrace.bounds, cl.cm.cache->nodes, contentMask );
	}
	
	// Blocked by the world if fraction < 1.0.
    if ( clientTrace.traceResult.fraction < 1.0 ) {		
		// Assign world entity to trace result.
		clientTrace.traceResult.ent = cs.entities;

		// Got blocked entirely, return early and skip entity testing.
		if ( clientTrace.traceResult.startSolid ) {
		//if ( clientTrace.traceResult.fraction == 0 ) {
			return clientTrace.traceResult;
		}
    }
	
	// Prepare the server trace for iterating, and possibly clip to, all entities residing in the clip move's bounds.
	clientTrace.start = start;
	clientTrace.end = end;
	clientTrace.skipEntity = skipEntity;
	clientTrace.brushContentsMask = contentMask;
	clientTrace.absoluteBounds = CM_CalculateBoxTraceBounds( start, end, clientTrace.bounds );

    // If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
    CL_World_ClipTraceToEntities( clientTrace );

	// And blast off!
    return clientTrace.traceResult;
}

