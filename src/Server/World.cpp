/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// world.c -- world query functions

#include "Server.h"
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"


// TODO: REMOVE
const bbox3_t CM_Matrix_TransformBounds(const glm::mat4 &matrix, const bbox3_t &bounds);
const bbox3_t CM_EntityBounds(const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds);

/**
*	@return	Returns a headNode that can be used for testing or clipping an
*			object of mins/maxs size.
**/
static mnode_t *SV_HullForEntity(Entity *ent) {
	// In case of a Solid::BSP type, check if the model exists in our precached BSP data.
	if (ent->solid == Solid::BSP) {
		int32_t i = ent->currentState.modelIndex - 1;

		// Explicit hulls in the BSP model.
		if (i <= 0 || i >= sv.cm.cache->nummodels) {
			Com_Error(ErrorType::Drop, "%s: inline model %d out of range", __func__, i);
		}

		return sv.cm.cache->models[i].headNode;
	}

	// Determine brush contents to use:
	// - Defaults to Solid.
	// - Client and Monsters together share the Monster brush contents.
	// - DeadMonster flagged entities override (hence, we seek for them first.)
	int32_t brushContents = BrushContents::Solid;
	// Ensure DeadMonster flags override possible Monster brush contents by checking for DeadMonster first.
	if (ent->serverFlags & EntityServerFlags::DeadMonster) {
		brushContents = BrushContents::DeadMonster;
	} else if (ent->client) {
		brushContents = BrushContents::Monster;
	} else if (ent->serverFlags & EntityServerFlags::Monster) {
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
		return CM_HeadnodeForSphere( ent->bounds, ent->boundsSphere, brushContents );
	// Default BoundingBox Hull:
    } else {
        return CM_HeadnodeForBox( ent->bounds, brushContents  );
    }

	return nullptr;
}


/**
*	@brief	Checks if edict is potentially visible from the given PVS row.
*	@return	True if visible.
**/
qboolean SV_EntityIsVisible(cm_t *cm, Entity *ent, byte *mask)
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

    return false;  // not visible
}



/**
*
*	World Entity Area Grid
*
*	Note: this use of "area" is different from the bsp file use
*
**/
/**
*	Defines the data of each areagrid subsection node.
**/
struct ServerWorldAreaNode {
	//! Axis describing this area node.
    int32_t axis = 0;       // -1 = leaf node
	//! Distance of specified area.
    float dist = 0;

	//! Child nodes, containing yet another level of subdivided areas.
    ServerWorldAreaNode *children[2];

	//! Lists per registered entity type in specified node.
    list_t  triggerEdicts;
    list_t  solidEdicts;
    list_t  solidLocalClientEdicts;
};

/**
*	Stores all entity's in areas that define a 'grid like structure', for a lack of better words.
**/
struct ServerWorld {
	// Area Grid configuration.
	static constexpr int32_t AREA_DEPTH	= 4;
	static constexpr int32_t AREA_NODES	= 32;

	//! Area nodes.
	ServerWorldAreaNode areaNodes[AREA_NODES] = {};
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
} serverWorld;

/**
*	@brief Builds a uniformly subdivided tree for the given world size
**/
static ServerWorldAreaNode *SV_CreateAreaNode( const int32_t depth, const bbox3_t &bounds ) {
	// Acquire us the current free area node.
	ServerWorldAreaNode *areaNode = &serverWorld.areaNodes[ serverWorld.numberOfAreaNodes ];
	// Increment num of nodes.
	serverWorld.numberOfAreaNodes++;

	// Initialize trigger entities list.
	List_Init( &areaNode->triggerEdicts );
	// Initialize solid entities list.
	List_Init( &areaNode->solidEdicts );

	// If we've reached the area's depth, stop subdividing and ensure we 
	// return a node with -1 axis and no children.
	if ( depth == serverWorld.AREA_DEPTH ) {
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

	areaNode->children[0] = SV_CreateAreaNode( depth + 1, bounds2 );
	areaNode->children[1] = SV_CreateAreaNode( depth + 1, bounds1 );

	return areaNode;
}

/**
*	@brief	Clear the server entity area grid world.
**/
void SV_ClearWorld(void) {
	// Clear nodes.
	memset( serverWorld.areaNodes, 0, sizeof( serverWorld.areaNodes ) );
	serverWorld.numberOfAreaNodes = 0;

	// If the world IS loaded up, recreate a fresh area grid to work from next time around.
	if ( sv.cm.cache ) {
		mmodel_t *worldCollisionModel = &sv.cm.cache->models[ 0 ];
		SV_CreateAreaNode( 0, bbox3_t{ worldCollisionModel->mins, worldCollisionModel->maxs } );
	}

	// Make sure all entities are unlinked
	for ( int32_t i = 0; i < MAX_SERVER_POD_ENTITIES; i++ ) {
		PODEntity *podEntity = &ge->entities[ i ];
		if ( podEntity ) {
			podEntity->area.prev = podEntity->area.next = nullptr;
		}
	}
}

/**
*	@brief	General purpose routine shared to the ServerGame module.
*			Links entity to PVS leafs.
**/
const bbox3_t CM_Matrix_TransformBounds( const glm::mat4 &matrix, const bbox3_t &bounds );
const bbox3_t CM_EntityBounds( const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds );

void SV_LinkEntityToPVSLeafs( cm_t *cm, PODEntity *podEntity, const bbox3_t &boxLeafsBounds ) {
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
                if ( podEntity->areaNumber2 && podEntity->areaNumber2 != area && sv.serverState == ServerState::Loading ) {
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
void PF_UnlinkEntity( PODEntity *podEntity ) {
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
void PF_LinkEntity( PODEntity *podEntity ) {
	// Unlink from previous old position.
	if ( podEntity->area.prev ) {
        PF_UnlinkEntity( podEntity );
	}
	
	// Ensure it isn't the worldspawn entity itself.
    if ( podEntity == ge->entities ) {
        return;        // Don't add the world
	}

	// Ensure it is in use.
    if ( !podEntity->inUse ) {
        Com_DPrintf( "%s: entity %d is not in use\n", __func__, NUM_FOR_EDICT( podEntity ) );
        return;
    }

	// Ensure a map is loaded properly in our cache.
    if ( !sv.cm.cache ) {
        return;
    }

	// Get the server entity.
    const int32_t entityNumber = NUM_FOR_EDICT( podEntity );
    server_entity_t *serverEntity = &sv.entities[ entityNumber ];
	
	// MATRIX:
	/**
	*	Update the actual bounds member to match the mins/maxs.
	**/
	// Determine whether it is a brush entity or not, they take special treatment.
	const bool isBrushEntity = ( podEntity->solid == Solid::BSP ? true : false );
	// Get access to the brush model if needed.
	mmodel_t *brushModel = ( isBrushEntity ? &sv.cm.cache->models[ podEntity->currentState.modelIndex - 1 ] : nullptr );
	// Get brush model origin if needed.
	const vec3_t brushModelOrigin = ( isBrushEntity && brushModel ? brushModel->origin : vec3_zero() );

	// Adjust bounds depending on final solid.
	switch( podEntity->solid ) {
		case Solid::BoundingBox:
		case Solid::OctagonBox:
		case Solid::Capsule:
		case Solid::Sphere:
		case Solid::Trigger:
			// Unset the bounds.
			podEntity->bounds = bbox3_t{ podEntity->mins, podEntity->maxs };

			break;
		case Solid::BSP:
			// Adjust entity bounds to that of brush if need be.
			//if ( isBrushEntity && brushModel ) {
			//	bbox3_t brushBounds = { brushModel->mins, brushModel->maxs };
			//	vec3_t brushBoundsSize = bbox3_size( brushBounds );
			//	bbox3_t centeredBrushBounds = bbox3_from_center_size( brushBoundsSize, bbox3_center( brushBounds ) );
			//	podEntity->bounds = brushBounds;
			////	podEntity->mins = podEntity->bounds.mins;
			////	podEntity->maxs = podEntity->bounds.maxs;
			//} else {
				podEntity->bounds = bbox3_t{ podEntity->mins, podEntity->maxs };
			//}
			break;
		//case Solid::Trigger:
		case Solid::Not:
			podEntity->bounds = bbox3_zero();
			break;
		default:
			//podEntity->bounds = bbox3_zero();
			break;
	}
	// EOF MATRIX:
    
	/**
	*	Encode the needed size data into the EntityState for proper client-side prediction needs.
	**/
	// Set the box its size.
	podEntity->size = bbox3_size( podEntity->bounds );

    switch ( podEntity->solid ) {
		case Solid::BoundingBox:
			if ( ( podEntity->serverFlags & EntityServerFlags::DeadMonster ) || vec3_equal( podEntity->mins, podEntity->maxs ) ) {
				podEntity->currentState.solid = 0;
				serverEntity->solid32 = 0;
			} else {
				podEntity->currentState.solid = Solid::BoundingBox;
				podEntity->currentState.mins = podEntity->mins;
				podEntity->currentState.maxs = podEntity->maxs;
				serverEntity->solid32 = podEntity->currentState.solid;
			}
			break;
		case Solid::Capsule:
			if ((podEntity->serverFlags & EntityServerFlags::DeadMonster ) || vec3_equal( podEntity->mins, podEntity->maxs ) ) {
				podEntity->currentState.solid = 0;
				serverEntity->solid32 = 0;
			} else {
				podEntity->currentState.solid = Solid::Capsule;
				podEntity->currentState.mins = podEntity->mins;
				podEntity->currentState.maxs = podEntity->maxs;
				serverEntity->solid32 = podEntity->currentState.solid;
			}
			break;
		case Solid::Sphere:
			if ((podEntity->serverFlags & EntityServerFlags::DeadMonster ) || vec3_equal( podEntity->mins, podEntity->maxs ) ) {
				podEntity->currentState.solid = 0;
				serverEntity->solid32 = 0;
			} else {
				podEntity->currentState.solid = Solid::Sphere;
				podEntity->currentState.mins = podEntity->mins;
				podEntity->currentState.maxs = podEntity->maxs;
				serverEntity->solid32 = podEntity->currentState.solid;
			}
			break;
		case Solid::OctagonBox:
			if ((podEntity->serverFlags & EntityServerFlags::DeadMonster ) || vec3_equal( podEntity->mins, podEntity->maxs ) ) {
				podEntity->currentState.solid = 0;
				serverEntity->solid32 = 0;
			} else {
				podEntity->currentState.solid = Solid::OctagonBox;
				podEntity->currentState.mins = podEntity->mins;
				podEntity->currentState.maxs = podEntity->maxs;
				serverEntity->solid32 = podEntity->currentState.solid;
			}
			break;
		case Solid::BSP:
			podEntity->currentState.solid = PACKED_BSP; // a Solid::BoundingBox will never create this value
			//podEntity->currentState.mins = vec3_zero();
			//podEntity->currentState.maxs = vec3_zero();
			serverEntity->solid32 = PACKED_BSP;     // FIXME: use 255?
			break;
		//case Solid::Not:
		//	podEntity->currentState.solid = Solid::Not;
		//	podEntity->currentState.mins = vec3_zero();
		//	podEntity->currentState.maxs = vec3_zero();
		//case Solid::Trigger:
		//	podEntity->currentState.solid = Solid::Trigger;
		//	podEntity->currentState.mins = podEntity->mins;
		//	podEntity->currentState.maxs = podEntity->maxs;
		//	break;
		default:
			podEntity->currentState.solid = 0;
			//podEntity->currentState.mins = vec3_zero();
			//podEntity->currentState.maxs = vec3_zero();
			serverEntity->solid32 = 0;

			// MATRIX:
			// Unset the bounds.
			//podEntity->bounds = bbox3_zero();
			// EOF MATRIX:
			break;
    }

	/**
	*	Calculate the entity's transform and invTransformices + its absolute bounds.
	**/
	const bool isClientEntity = ( podEntity->client ? true : false );
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
		entityAngles = vec3_zero();
	// Sphere testing for lulz:
	} else if ( podEntity->solid == Solid::Sphere ) {
		entityAngles = vec3_clamp_euler( podEntity->currentState.angles );
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
	//	entityOrigin = podEntity->client->playerState.pmove.origin;
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
	// Sphere Route:
	} else if ( podEntity->solid == Solid::Sphere ) {
		// Get brush origin.
		const vec3_t entityNodeOrigin = (brushModel ? brushModel->origin : vec3_zero() );
		// Clamp the entity angles.
		const vec3_t eulerAngles = vec3_clamp_euler( { entityAngles.x, entityAngles.y, entityAngles.z } );

		// Fill in entity offset.
		vecEntityOriginOffset = phvec_to_glmvec3( entityNodeOrigin );

		// Translate to point matrices.
		const glm::mat4 matBoxOriginTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3( entityNodeOrigin ) );
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
	}
	
	// Translate:
	glm::mat4 matTranslation = glm::translate( ph_mat_identity(), vecEntityOrigin ); //+ vecEntityOriginOffset );

	// Calculate actual Polyhedron entity transform and invTransform matrices.
	glm::mat4 matEntityTransform = matTranslation * matRotation * matScale;
	glm::mat4 matInvEntityTransform = glm::inverse( matEntityTransform );

	// Calculate the entity's absolute bounds(world space).
	bbox3_t leafCheckBounds = podEntity->absoluteBounds = CM_EntityBounds( podEntity->solid, matEntityTransform, podEntity->bounds );

	// Calculate the entity's bounds sphere(model space) and absolute bounds sphere(world space).
	const bool isTransformed = matEntityTransform != ph_mat_identity();
	CM_EntitySphere( podEntity->solid, matEntityTransform, matInvEntityTransform, podEntity->bounds, podEntity->boundsSphere, podEntity->boundsAbsoluteSphere, isTransformed );

	// TODO: These are temporarily set, should be removed after making sure we are using absoluteBounds everywhere.	
	podEntity->absMin = podEntity->absoluteBounds.mins;
	podEntity->absMax = podEntity->absoluteBounds.maxs;

	// Transform our bounds if needed. (BSP Brush entities can rotate.)
	if ( isTransformed ) {//if ( matEntityTransform != ph_mat_identity() ) {// ph_mat_identity() ) {
		leafCheckBounds = CM_Matrix_TransformBounds( matInvEntityTransform, leafCheckBounds );
	}

	//if ( podEntity->currentState.number == 37 ) {
	//	Com_DPrintf(fmt::format("SV Entity(#{}), (Solid::BSP), (Angles: {}, {}, {}), (bounds.Mins: {}, {}, {} bounds.Maxs: {}, {}, {})\n",
	//				podEntity->currentState.number,
	//				eulerAngles.x,
	//				eulerAngles.y,
	//				eulerAngles.z,
	//				podEntity->absoluteBounds.mins.x,
	//				podEntity->absoluteBounds.mins.y,
	//				podEntity->absoluteBounds.mins.z,
	//				podEntity->absoluteBounds.maxs.x,
	//				podEntity->absoluteBounds.maxs.y,
	//				podEntity->absoluteBounds.maxs.z).c_str()
	//	);
	//}
	/**
	*	Link entity to our PVS Leafs, and exit early out of this function if we're dealing
	*	with a Solid::Not entity. If we don't, we'd be adding it to the list of solids.
	**/
    SV_LinkEntityToPVSLeafs( &sv.cm, podEntity, leafCheckBounds );

    // If first time, make sure oldOrigin is valid.
    if ( !podEntity->linkCount ) {
        podEntity->currentState.oldOrigin = podEntity->currentState.origin;
    }
    podEntity->linkCount++;

	// Exit if we do not want this entity to ever be tested by traces.
    if ( podEntity->solid == Solid::Not ) {
        return;
	}

	/**
	*	Link entity into our World Areas for tracing efficiency.
	**/
	// Find the first node that the ent's box croses.
    ServerWorldAreaNode *areaNode = serverWorld.areaNodes;
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
    if ( podEntity->solid == Solid::Trigger ) {
        List_Append( &areaNode->triggerEdicts, &podEntity->area );
	} else {
        List_Append( &areaNode->solidEdicts, &podEntity->area );
	}

	/**
	*	And last but not least, assign the newly calculated transform and inverted transform matrices.
	**/
	podEntity->translateMatrix = matEntityTransform;
	podEntity->invTranslateMatrix = matInvEntityTransform;
}


/**
*	@brief	The inner workings of SV_AreaEntities.
**/
static void SV_AreaEntities_r( ServerWorldAreaNode *areaNode ) {
	// Determine what list to search in depending on the queried entity type.
	list_t *start = nullptr;
	if ( serverWorld.areaTest.areaType == AreaEntities::Solid ) {
		start = &areaNode->solidEdicts;
	} else {
		start = &areaNode->triggerEdicts;
	}

	PODEntity *podCheck = nullptr;
	LIST_FOR_EACH(PODEntity, podCheck, start, area ) {
		if ( podCheck->solid == Solid::Not ) {
			continue;        // deactivated
		}
	
		if ( bbox3_intersects( podCheck->absoluteBounds, serverWorld.areaTest.areaBounds ) ) {
			serverWorld.areaTest.areaEntityList[ serverWorld.areaTest.areaEntityCount ] = podCheck;
			serverWorld.areaTest.areaEntityCount++;

			if ( serverWorld.areaTest.areaEntityCount == serverWorld.areaTest.areaEntityMaxCount ) {
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
	if ( serverWorld.areaTest.areaBounds.maxs[ areaNode->axis ] > areaNode->dist ) {
		SV_AreaEntities_r( areaNode->children[0] );
	}
	if ( serverWorld.areaTest.areaBounds.mins[ areaNode->axis ] < areaNode->dist ) {
		SV_AreaEntities_r( areaNode->children[1] );
	}
}

/**
*	@brief	Looks up all areas residing in the mins/maxs box of said areaType (solid, or triggers).
*	@return	Number of entities found and stored in the list.
**/
int SV_AreaEntities(const vec3_t &mins, const vec3_t &maxs, Entity **list, int maxcount, int areatype) {
	serverWorld.areaTest.areaBounds = bbox3_t { mins, maxs };
    serverWorld.areaTest.areaEntityList = list;
    serverWorld.areaTest.areaEntityCount = 0;
    serverWorld.areaTest.areaEntityMaxCount = maxcount;
    serverWorld.areaTest.areaType = areatype;

    SV_AreaEntities_r( serverWorld.areaNodes );

	// Return the count of total entities found which now reside in our list.
    return serverWorld.areaTest.areaEntityCount;
}



/***
*
*
*	Server World: Point Contents Testing
*
*
***/
/**
*	@brief	Determines the shape to consider for trace testing with.
**/
struct ServerTraceShape {
	static constexpr int32_t Box = 0;
	static constexpr int32_t Sphere = 1;
};

/**
*	@brief	ServerTrace object.
**/
struct ServerTrace {
	//! The shape we're tracing with.
	int32_t traceShape = ServerTraceShape::Box;

	//! Start point of trace.
	vec3_t start = vec3_zero();
	//! End point of trace.
	vec3_t end = vec3_zero();

	// Box Bounds Trace:
	//! The 'bounds' box we're tracing from 'start' to 'end'.
	bbox3_t bounds = bbox3_zero();
	//! The 'absolute bounds' box, containing the entire 'bounds' clip move from 'start' to 'end'.
	bbox3_t absoluteBounds = bbox3_zero();

	// Sphere Trace:
	//! The bounds sphere we're tracing from 'start' to 'end'.
	sphere_t boundsSphere;
	//! The 'absolute bounds' containing the entire 'bounds sphere' clip move from 'start' to 'end'.
	sphere_t absoluteBoundsSphere;

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
int32_t SV_PointContents( const vec3_t &point ) {
    static PODEntity *areaEntities[MAX_WIRED_POD_ENTITIES];
    
	// Ensure all is sane.
    if ( !sv.cm.cache || !sv.cm.cache->nodes ) {
        Com_Error( ErrorType::Drop, "%s: no map loaded", __func__ );
		return 0;
	}

    // get base contents from world
    int32_t contents = CM_PointContents( &sv.cm, point, sv.cm.cache->nodes, ph_mat_identity() );

    // or in contents from all the other entities
    const int32_t numberOfAreaEntities = SV_AreaEntities( point, point, areaEntities, MAX_WIRED_POD_ENTITIES, AreaEntities::Solid );

    for ( int32_t i = 0; i < numberOfAreaEntities; i++ ) {
		// Acquire touch entity.
        PODEntity *clipEntity = areaEntities[ i ];
		
		// Get our hull to trace with.
		mnode_t *clipEntityHull = SV_HullForEntity( clipEntity );

		if ( clipEntityHull ) {
			// Might intersect, so do an exact clip
			contents |= CM_PointContents( &sv.cm, point, clipEntityHull , clipEntity->invTranslateMatrix );
		}
    }

    return contents;
}



/***
*
*
*	Server World: World/Entity Clipping Traces
*
*
***/
/**
*	@brief	Inspects whether the entity should be skipped from trace clip testing.
*	@return	True if it should, false otherwise.
**/
static const bool SV_ClipTraceSkipEntity( ServerTrace &serverTrace, PODEntity *clipEntity ) {
	// See if we can skip testing this actual entity.
    if ( serverTrace.skipEntity ) {
		// Skip testing the entity if it is identical to our 'skipEntity'.
		if ( clipEntity == serverTrace.skipEntity ) {
			return true;
		}

		// Skip testing the entity if we are its owner.
		if ( clipEntity->owner == serverTrace.skipEntity ) {
			return true;
		}

		// If the skip entity has an owner:
		if ( serverTrace.skipEntity->owner ) {
			// Skip testing if it is our clip entity.
			if ( clipEntity == serverTrace.skipEntity->owner ) {
				return true;
			}
			// Visa versa.
			if ( clipEntity->owner == serverTrace.skipEntity->owner ) {
				return true;
			}
		}

		// Ensure triggers ONLY clip to the world brushes, allow others to 'occupy' them.
		if ( serverTrace.skipEntity->solid == Solid::Trigger ) {
			if ( clipEntity->solid != Solid::BSP ) {
				return true;
			}
		}
    }

	// Unless specifically seeking 'DeadMonster' brush type entities, skip them as well.
    if ( !( serverTrace.brushContentsMask & BrushContents::DeadMonster ) && ( clipEntity->serverFlags & EntityServerFlags::DeadMonster ) ) {
        return true;
	}

	// Don't skip.
	return false;
}

/**
*	@brief	Performs a specific shape type trace for use with clip to entities functions below.
*	@return	The trace results.
**/
static TraceResult SV_TraceClipShapeToEntity( ServerTrace &serverTrace, mnode_t *headNode, PODEntity *clipEntity ) {
	if ( clipEntity->translateMatrix == ph_mat_identity() ) {
		// 'Sphere' Trace:
		if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
			return CM_SphereTrace( &sv.cm, serverTrace.start, serverTrace.end, serverTrace.bounds, serverTrace.boundsSphere, headNode, serverTrace.brushContentsMask );
		// 'Box' Trace:
		} else {
			return CM_BoxTrace( &sv.cm, serverTrace.start, serverTrace.end, serverTrace.bounds, headNode, serverTrace.brushContentsMask );
		}
	} else {
		// 'Sphere' TransformedTrace:
		if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
			return CM_TransformedSphereTrace( &sv.cm, serverTrace.start, serverTrace.end, serverTrace.bounds, serverTrace.boundsSphere, headNode, serverTrace.brushContentsMask, clipEntity->translateMatrix, clipEntity->invTranslateMatrix );
		// 'Box' TransformedTrace:
		} else {
			return CM_TransformedBoxTrace( &sv.cm, serverTrace.start, serverTrace.end, serverTrace.bounds, headNode, serverTrace.brushContentsMask, clipEntity->translateMatrix, clipEntity->invTranslateMatrix );
		}
	}
}

/**
*	@brief	Performs a clip trace to a single entity.
*	@brief	True in case we can't trace properly(i.e, we're in allSolid)
**/
static const bool SV_ClipTraceToEntity( ServerTrace &serverTrace, PODEntity *clipEntity ) {
	// If for whatever reason our clip entity is non solid, skip it.
    if ( clipEntity->solid == Solid::Not ) {
        return false;
	}

	// See if we can skip testing this actual entity.
	if ( SV_ClipTraceSkipEntity( serverTrace, clipEntity ) ) {
		return false;
	}

	// Get the entity's headNode to use for clipping.
	mnode_t *headNode = SV_HullForEntity( clipEntity );
	if ( !headNode ) {
		//return false;
	}

	// Use the desire sphere shape trace to clip to the clipEntity.
	TraceResult trace = SV_TraceClipShapeToEntity( serverTrace, headNode, clipEntity );

	// Finalize trace results and clip to entity.
    //CM_ClipEntity( &serverTrace.traceResult, &trace, clipEntity );
	if ( trace.allSolid || trace.fraction < serverTrace.traceResult.fraction ) {
		serverTrace.traceResult = trace;
		serverTrace.traceResult.ent = clipEntity;
	} else if ( trace.startSolid ) {
		serverTrace.traceResult.startSolid = true;
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
//static void SV_ClipTraceToEntities( const vec3_t &start, const bbox3_t &traceMoveBounds, const vec3_t &end, Entity *skipEntity, int32_t contentMask, TraceResult *tr ) {
static void SV_ClipTraceToEntities( ServerTrace &serverTrace ) {
	// Actual list that will contain the final results.
	static PODEntity *areaEntityList[MAX_WIRED_POD_ENTITIES];
    const int32_t numberOfAreaEntities = SV_AreaEntities( serverTrace.absoluteBounds.mins, serverTrace.absoluteBounds.maxs, areaEntityList, MAX_WIRED_POD_ENTITIES, AreaEntities::Solid );

    // Be careful, it is possible to have an entity in this list removed before we get to it (killtriggered)
    for (int32_t i = 0; i < numberOfAreaEntities; i++) {
        // Fetch 'touched' entity
		PODEntity *clipEntity = areaEntityList[i];

		// If for whatever reason our clip entity is non solid, skip it.
		if ( clipEntity->solid == Solid::Not ) {
			continue;
		}

		// See if we can skip testing this actual entity.
		if ( SV_ClipTraceSkipEntity( serverTrace, clipEntity ) ) {
			continue;
		}

		// Get the entity's headNode to use for clipping.
		mnode_t *headNode = SV_HullForEntity( clipEntity );
		if ( !headNode ) {
			continue;
		}

		// Use the desire sphere shape trace to clip to the clipEntity.
		TraceResult trace = SV_TraceClipShapeToEntity( serverTrace, headNode, clipEntity );

		// Finalize trace results and clip to entity.
        //CM_ClipEntity( &serverTrace.traceResult, &trace, clipEntity );
		if ( trace.allSolid || trace.fraction < serverTrace.traceResult.fraction ) {
			serverTrace.traceResult = trace;
			serverTrace.traceResult.ent = clipEntity;
		} else if ( trace.startSolid ) {
			serverTrace.traceResult.startSolid = true;
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
const TraceResult SV_Clip( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, Entity *skipEntity, Entity *clipEntity, const int32_t contentMask ) {
	// Initialize a server world trace context.
	ServerTrace serverTrace = {
		// Tracing shape to use.
		.traceShape = ServerTraceShape::Box,
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
	bbox3_t boundsEpsilon = bbox3_expand( serverTrace.bounds, CM_RAD_EPSILON );
	serverTrace.boundsSphere = sphere_from_size( bbox3_symmetrical( boundsEpsilon ), bbox3_center( boundsEpsilon ) );

	// Ensure that our server 's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !sv.cm.cache ) {
		Com_Error( ErrorType::Drop, "%s: no map loaded", __func__ );
		return serverTrace.traceResult;
	}

	// TODO: Do we need a skip entity when trying to clip against a single entity?
	serverTrace.skipEntity = skipEntity;
	// TODO: Do we need absoluteBounds when no move is made, thus clipping against a single entity?
	//serverTrace.absoluteBounds = CM_CalculateTraceBounds( start, end, serverTrace.bounds );

	// If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
	SV_ClipTraceToEntity( serverTrace, clipEntity );

	// And blast off!
	return serverTrace.traceResult;
}
/**
*	@brief	Tests and clips the specified trace to a single specific entity.
**/
const TraceResult SV_ClipSphere( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const sphere_t &sphere, Entity *skipEntity, Entity *clipEntity, const int32_t contentMask ) {
	// Initialize a server world trace context.
	ServerTrace serverTrace = {
		// Tracing shape to use.
		.traceShape = ServerTraceShape::Sphere,
		// Start/End of trace.		
		.start = start,
		.end = end,
		// Set bounds.
		.bounds = bbox3_t { mins, maxs },
		.boundsSphere = sphere,

		// Contents mask to search trace clips with.
		.brushContentsMask = contentMask,
		// Default trace result.
		.traceResult = {
			.fraction = 1.f,
			//.ent = ge->entities,
			.ent = nullptr
		}
	};

	// Ensure that our server 's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !sv.cm.cache ) {
		Com_Error( ErrorType::Drop, "%s: no map loaded", __func__ );
		return serverTrace.traceResult;
	}

	// TODO: Do we need a skip entity when trying to clip against a single entity?
	serverTrace.skipEntity = skipEntity;
	// TODO: Do we need absoluteBounds when no move is made, thus clipping against a single entity?
	//serverTrace.absoluteBounds = CM_CalculateTraceBounds( start, end, serverTrace.bounds );

	// If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
	SV_ClipTraceToEntity( serverTrace, clipEntity );

	// And blast off!
	return serverTrace.traceResult;
}

/**
*	@brief	Moves the given mins/maxs volume through the world from start to end.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult q_gameabi SV_Trace( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, Entity *skipEntity, const int32_t contentMask ) {
    // Ensure that our server's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !sv.cm.cache || !sv.cm.cache->nodes ) {
        Com_Error( ErrorType::Drop, "%s: no map loaded", __func__ );
		return TraceResult();
	}

	// Initialize a server world trace context.
	ServerTrace serverTrace = {
		// Tracing shape to use.
		.traceShape = ServerTraceShape::Box,
		// Set bounds.
		.bounds = bbox3_t { mins, maxs }
	};
	bbox3_t boundsEpsilon = bbox3_expand( serverTrace.bounds, CM_RAD_EPSILON );
	serverTrace.boundsSphere = sphere_from_size( bbox3_symmetrical( boundsEpsilon ), bbox3_center( boundsEpsilon ) );

	// First perform a clipping trace to our world( The actual BSP tree itself).
	// 'Sphere' shape:
	//if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
	//	// Create sphere from bounds.
	//	serverTrace.traceResult = CM_SphereTrace( &sv.cm, start, end, serverTrace.bounds, serverTrace.boundsSphere, sv.cm.cache->nodes, contentMask );
	//	// 'Box' shape by default.
	//} else {
		serverTrace.traceResult = CM_BoxTrace( &sv.cm, start, end, serverTrace.bounds, sv.cm.cache->nodes, contentMask );
	//}

	// Blocked by the world if fraction < 1.0.
    if ( serverTrace.traceResult.fraction < 1.0 ) {		
		// Assign world entity to trace result.
		serverTrace.traceResult.ent = ge->entities;

		// Got blocked entirely, return early and skip entity testing.
		if ( serverTrace.traceResult.startSolid ) {
		//if ( serverTrace.traceResult.fraction == 0 ) {
			return serverTrace.traceResult;
		}
    }
	
	// Prepare the server trace for iterating, and possibly clip to, all entities residing in the clip move's bounds.
	serverTrace.start = start;
	serverTrace.end = end;
	serverTrace.skipEntity = skipEntity;
	serverTrace.brushContentsMask = contentMask;
	//if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
	//	serverTrace.absoluteBounds = CM_Sphere_CalculateTraceBounds( start, end, serverTrace.bounds, serverTrace.boundsSphere.offset, serverTrace.boundsSphere.radius );
	//} else {
		serverTrace.absoluteBounds = CM_AABB_CalculateTraceBounds( start, end, serverTrace.bounds );
	//}

    // If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
    SV_ClipTraceToEntities( serverTrace );

	// And blast off!
    return serverTrace.traceResult;
}

/**
*	@brief	Moves the given mins/maxs volume through the world from start to end.
*			Passedict and edicts owned by passedict are explicitly skipped from being checked.
**/
const TraceResult q_gameabi SV_TraceSphere( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const sphere_t &sphere, Entity *skipEntity, const int32_t contentMask ) {
    // Ensure that our server's collision model BSP data is precached. (i.e, a map is loaded.)
	if ( !sv.cm.cache || !sv.cm.cache->nodes ) {
        Com_Error( ErrorType::Drop, "%s: no map loaded", __func__ );
		return TraceResult();
	}

	// Initialize a server world trace context.
	ServerTrace serverTrace = {
		// Tracing shape to use.
		.traceShape = ServerTraceShape::Sphere,
		// Set bounds.
		.bounds = bbox3_t { mins, maxs },
		.boundsSphere = sphere
	};
	
	// First perform a clipping trace to our world( The actual BSP tree itself).
	// 'Sphere' shape:
	//if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
		// Create sphere from bounds.
		serverTrace.traceResult = CM_SphereTrace( &sv.cm, start, end, serverTrace.bounds, serverTrace.boundsSphere, sv.cm.cache->nodes, contentMask );
		// 'Box' shape by default.
	//} else {
	//	serverTrace.traceResult = CM_BoxTrace( &sv.cm, start, end, serverTrace.bounds, sv.cm.cache->nodes, contentMask );
	//}

	// Blocked by the world if fraction < 1.0.
    if ( serverTrace.traceResult.fraction < 1.0 ) {		
		// Assign world entity to trace result.
		serverTrace.traceResult.ent = ge->entities;

		// Got blocked entirely, return early and skip entity testing.
		if ( serverTrace.traceResult.startSolid ) {
		//if ( serverTrace.traceResult.fraction == 0 ) {
			return serverTrace.traceResult;
		}
    }
	
	// Prepare the server trace for iterating, and possibly clip to, all entities residing in the clip move's bounds.
	serverTrace.start = start;
	serverTrace.end = end;
	serverTrace.skipEntity = skipEntity;
	serverTrace.brushContentsMask = contentMask;
	//if ( serverTrace.traceShape == ServerTraceShape::Sphere ) {
		serverTrace.absoluteBounds = CM_Sphere_CalculateTraceBounds( start, end, serverTrace.bounds, serverTrace.boundsSphere.offset, serverTrace.boundsSphere.radius );
	//} else {
	//	serverTrace.absoluteBounds = CM_AABB_CalculateTraceBounds( start, end, serverTrace.bounds );
	//}

    // If we didn't hit the world, iterate over all entities using our trace bounds and clip our move against their transforms.
    SV_ClipTraceToEntities( serverTrace );

	// And blast off!
    return serverTrace.traceResult;
}