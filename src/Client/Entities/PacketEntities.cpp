/***
*
*	License here.
*
*	@file
*
*	'Wired' Packet Entities Frame Parsing and Processing.
* 
***/
// Basic Client.
#include "../Client.h"
#include "../GameModule.h"
#include "../World.h"

// Need Refresh and Sound Headers.
#include "Refresh/Models.h"
#include "../Sound/Sound.h"

// PacketEntities.
#include "PacketEntities.h"


/**
*
*
*	'State' updating for 'Wired' Packet Entities.
*
*
**/
/**
*   @brief  The client's player entity is send as an "optimized" packet. Several
*           variables are sent over the player state instead of the entity state.
* 
*   @return True if the entity comes from an optimized packet, false otherwise.
**/
static inline qboolean PacketEntity_IsPlayer( const EntityState* state ) {
    if ( state->number != cl.frame.clientNumber + 1 )
        return false;

    if ( cl.frame.playerState.pmove.type >= EnginePlayerMoveType::Dead )
        return false;

    return true;
}

/**
*	@brief	Restores entity origin and angles from player state
**/
void PacketEntity_PlayerToEntityState(const PlayerState *ps, EntityState *es) {
    vec_t pitch;

    // Take the predicted state origin in case we are extrapolating ground movers.
	/*if ( cl.frame.playerState.pmove.flags & PMF_EXTRAPOLATING_GROUND_MOVER ) {*/
	if ( cl.predictedState.flags & PMF_EXTRAPOLATING_GROUND_MOVER ) {
		es->origin = cl.predictedState.viewOrigin;
	} else {
	    es->origin = ps->pmove.origin;
	}

    pitch = ps->pmove.viewAngles[vec3_t::Pitch];
    if (pitch > 180) {
        pitch -= 360;
    }
    es->angles[vec3_t::Pitch] = pitch / 3;
    es->angles[vec3_t::Yaw] = ps->pmove.viewAngles[vec3_t::Yaw];
    es->angles[vec3_t::Roll] = 0;
}

/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static inline void PacketEntity_UpdateNew(PODEntity *clEntity, const EntityState *state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
	// Set its inUse.
	clEntity->inUse = true;
	// Ensure it is not local anymore.
	clEntity->isLocal = false;
	// Update the client entity number to match the state's number.
    //clEntity->clientEntityNumber = state->number; // used to be: clEntity->id = ++entity_ctr;

	// Reset trail count for particles.
    clEntity->trailCount = 1024;
    
    // Duplicate the current state into the previous one, this way lerping won't hurt anything.
    clEntity->previousState = *state;

	// Link it in.
    const bool isPlayerEntity = PacketEntity_IsPlayer( state );
	if ( !isPlayerEntity ) {
		CL_PF_World_LinkEntity( clEntity );
	}

    // Ensure that when the entity has been teleported we adjust its lerp origin.
    if ( state->eventID == EntityEvent::PlayerTeleport || state->eventID == EntityEvent::OtherTeleport
       || ( state->renderEffects & ( RenderEffects::FrameLerp | RenderEffects::Beam ) ) ) 
    {
        // This entity has been teleported.
        clEntity->lerpOrigin = origin;
        return;
    }

    // oldOrigin is valid for new entities, so use it as starting point for interpolating between.
    clEntity->previousState.origin = state->oldOrigin;
    clEntity->lerpOrigin = state->oldOrigin;
}

/**
*   @brief  Updates an existing entity using the newly received state for it.
**/
static inline void PacketEntity_UpdateExisting( PODEntity *clEntity, const EntityState *state, const vec_t *origin ) {
    // Get event ID.
    const int32_t eventID = state->eventID;

	// Link it in.
    const bool isPlayerEntity = PacketEntity_IsPlayer( state );
	if ( !isPlayerEntity ) {
		CL_PF_World_LinkEntity( clEntity );
	}

    if (state->modelIndex != clEntity->currentState.modelIndex
        || state->modelIndex2 != clEntity->currentState.modelIndex2
        || state->modelIndex3 != clEntity->currentState.modelIndex3
        || state->modelIndex4 != clEntity->currentState.modelIndex4
        || eventID == EntityEvent::PlayerTeleport
        || eventID == EntityEvent::OtherTeleport
        || fabsf(origin[0] - clEntity->currentState.origin[0]) > 512
        || fabsf(origin[1] - clEntity->currentState.origin[1]) > 512
        || fabsf(origin[2] - clEntity->currentState.origin[2]) > 512
        || cl_nolerp->integer == 1) 
    {
        // Some data changes will force no lerping.
        clEntity->trailCount = 1024;     // Used for diminishing rocket / grenade trails

        // Duplicate the current state so lerping doesn't hurt anything
        clEntity->previousState = *state;

        // No lerping if teleported or morphed
        clEntity->lerpOrigin = origin;
        return;
    }

    // Shuffle the last state to previous
    clEntity->previousState = clEntity->currentState;

	// Update entity number.
	clEntity->clientEntityNumber = state->number;
}

/**
*   @brief  Checks whether the parsed entity from received from the server frame packet is
*			a new comer to our client's view or not. (ie, was it in previous frames?)
*   @return True when either of these conditions are met: 
*               - Last frame was invalid, meaning we can't compare to any previous state.
*               - Last frame was dropped, once again, can't compare to any previous state.
*               - cl_nolerp 2, forced by developer option.
*           False when none of the above conditions are met or cl_nolerp is set to 3:
**/
static inline qboolean PacketEntity_IsNew( const PODEntity *clEntity, const EntityState *state ) {
    // Last received frame was invalid.
    if ( !cl.oldframe.valid ) {
        return true;
    }

    // Wasn't in last received frame.
    if ( clEntity->serverFrame != cl.oldframe.number ) {
        return true;
    }

    // Developer option, always new.
    if ( cl_nolerp->integer == 2 ) {
        return true;
    }

    // Developer option, lerp from last received frame.
    if ( cl_nolerp->integer == 3 ) {
        return false;
    }

    // Previous server frame was dropped.
    if ( cl.oldframe.number != cl.frame.number - 1 ) {
        return true;
    }

    // No conditions met, so it wasn't in our previous frame.
    return false;
}

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
void PacketEntity_UpdateState( const EntityState *state ) {
    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    PODEntity *clEntity = &cs.entities[ state->number ];

	// Make sure to fetch and adjust solids here.
    if ( state->solid && state->number != cl.frame.clientNumber + 1 ) {
		if ( state->solid == PACKED_BSP ) {
			clEntity->solid = Solid::BSP;
			const mmodel_t *model = &cl.cm.cache->models[ clEntity->currentState.modelIndex - 1 ];
			clEntity->mins = model->mins;
			clEntity->maxs = model->maxs;
		} else {
			clEntity->solid = state->solid;

			// These get set when linking in the entity to our client areagrid world.
			//clEntity->mins = state->mins;
			//clEntity->maxs = state->maxs;
		}
	} else {
		if ( state->number != cl.frame.clientNumber + 1 ) {
			//clEntity->mins = vec3_zero();
			//clEntity->mins = vec3_zero();
			clEntity->solid = Solid::Not;
		}
	}

	//---------------- TODO: Move into a function for checking and setting extrapolated states?
    // Work around Q2PRO server bandwidth optimization.
    const bool isPlayerEntity = PacketEntity_IsPlayer( state );

    // Fetch the entity's origin.
    vec3_t entityOrigin = state->origin;
    if ( isPlayerEntity ) {
		/*if ( cl.frame.playerState.pmove.flags & PMF_EXTRAPOLATING_GROUND_MOVER ) {*/
		if ( cl.predictedState.flags & PMF_EXTRAPOLATING_GROUND_MOVER ) {
			entityOrigin = cl.predictedState.viewOrigin;
		} else {
			entityOrigin = cl.frame.playerState.pmove.origin;
		}
    }

	// FUNC_ROTATE:
	// Store its extrapolated state if we're extrapolating.
	struct ExtraPolatedStates {
		EntityState current;
		EntityState previous;
	} extraPolatedStates;
	if ( clEntity->linearMovement.isExtrapolating ) {
		extraPolatedStates.current = clEntity->currentState;
		extraPolatedStates.previous = clEntity->previousState;
	}
	// EOF FUNC_ROTATE:
	//----------------- END OF TODO.

	// Assign its clientEntity number.
	clEntity->clientEntityNumber = state->number;
	
	if (clEntity->clientEntityNumber == 14) {
		int x = 10; // FUNC_ROTATE bug
	}

    // Was this entity in our previous frame, or not?
    if ( PacketEntity_IsNew( clEntity, state ) ) {
        // Wasn't in last update, so initialize some things.
        PacketEntity_UpdateNew( clEntity, state, entityOrigin );
    } else {
        // It already exists, update it accordingly.
        PacketEntity_UpdateExisting( clEntity, state, entityOrigin );
    }
	
	// In case of non player entities, inspect whether their
	// game entity hashname changed.
	if ( !isPlayerEntity ) {
		CL_GM_PacketNewHashedClassname( clEntity, state );
	}

    // Assign the fresh new received server frame number that belongs to this frame.
    clEntity->serverFrame = cl.frame.number;

	// Do the same for the local client frame.
	clEntity->clientFrame = cl.clientFrame.number;

	// Assign the fresh new received state as the entity's current.
    clEntity->currentState = *state;

	//---------------- TODO: Move into a function for checking and setting origins?


    // work around Q2PRO server bandwidth optimization
    if ( isPlayerEntity ) {
        PacketEntity_PlayerToEntityState( &cl.frame.playerState, &clEntity->currentState );

		// We need to link it in here in order for each packet entity delta frame.
		CL_PF_World_LinkEntity( clEntity );
	}

	// FUNC_ROTATE:
	// Store its extrapolated state if we're extrapolating.
	if ( clEntity->linearMovement.isExtrapolating ) {
		clEntity->currentState.origin = extraPolatedStates.current.origin;
		clEntity->currentState.angles = extraPolatedStates.current.angles;
		clEntity->currentState.oldOrigin = extraPolatedStates.current.oldOrigin;

		//clEntity->previousState.origin = extraPolatedStates.previous.origin;
		//clEntity->previousState.angles = extraPolatedStates.previous.angles;
		//clEntity->previousState.oldOrigin = extraPolatedStates.previous.oldOrigin;

		//Com_DPrintf( "Extrapolating Entity(#%i): angles(%f,%f,%f), origin(%f,%f,%f), oldOrigin(%f,%f,%f)\n",
		//			clEntity->clientEntityNumber,
		//			extraPolatedStates.current.angles.x,
		//			extraPolatedStates.current.angles.y,
		//			extraPolatedStates.current.angles.z,
		//			extraPolatedStates.current.origin.x,
		//			extraPolatedStates.current.origin.y,
		//			extraPolatedStates.current.origin.z,
		//			extraPolatedStates.current.oldOrigin.x,
		//			extraPolatedStates.current.oldOrigin.y,
		//			extraPolatedStates.current.oldOrigin.z
		//			);
	}
	// EOF FUNC_ROTATE:
	//----------------- END OF TODO.
}

/**
*   @brief  Ensures its hashedClassname is updated accordingly to that which matches the Game Entity.
**/
void PacketEntity_SetHashedClassname(PODEntity* podEntity, const EntityState* state) {
	// Only continue IF we got a podEntity.
	if (!podEntity) {
		return;
	}
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
void PacketEntity_FireEvent( int32_t number ) {
    CL_GM_PacketEntityEvent(number);
}