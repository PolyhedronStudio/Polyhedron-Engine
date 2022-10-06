/***
*
*	License here.
*
*	@file
*
*	'Non-Wired' Local Entities Frame Processing.
* 
***/
// Basic Client.
#include "../Client.h"
#include "../GameModule.h"
#include "../World.h"

// Need Refresh and Sound Headers.
#include "Refresh/Models.h"
#include "../Sound/Sound.h"

// LocalEntities.
#include "LocalEntities.h"


/**
*
*
*	'State' updating for 'Non-Wired' Local Entities.
*
*
**/
/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static void LocalEntity_UpdateNew(PODEntity *clEntity, const EntityState *state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
    
	clEntity->clientEntityNumber = state->number; //MAX_WIRED_POD_ENTITIES + (++entity_ctr); //state.number; // used to be: clEntity->id = ++entity_ctr;
    clEntity->trailCount = 1024;

    // Notify the client game module that we've acquired from the server a fresh new entity to spawn.
    //CL_GM_CreateFromNewState(clEntity, state);
    
    // Duplicate the current state into the previous one, this way lerping won't hurt anything.
    clEntity->previousState = *state;

	// Link entity in.
	//CL_PF_World_LinkEntity( clEntity );

    // Ensure that when the entity has been teleported we adjust its lerp origin.
    if (state->eventID == EntityEvent::PlayerTeleport || state->eventID == EntityEvent::OtherTeleport
       || (state->renderEffects & (RenderEffects::FrameLerp | RenderEffects::Beam))) 
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
*   @brief  Updates an existing entity and ensures that if it needs to lerp it can do so safely.
**/
static void LocalEntity_UpdateExisting(PODEntity *clEntity, const EntityState *state, const vec_t *origin)
{
    // Fetch event ID.
    int32_t eventID = state->eventID;
	
	// Link entity in.
	//CL_PF_World_LinkEntity( clEntity );

    if (//state->hashedClassname != clEntity->previousState.hashedClassname
		state->modelIndex != clEntity->previousState.modelIndex
        || state->modelIndex2 != clEntity->previousState.modelIndex2
        || state->modelIndex3 != clEntity->previousState.modelIndex3
        || state->modelIndex4 != clEntity->previousState.modelIndex4
        || eventID == EntityEvent::PlayerTeleport
        || eventID == EntityEvent::OtherTeleport
        || fabsf(origin[0] - clEntity->previousState.origin[0]) > 512
        || fabsf(origin[1] - clEntity->previousState.origin[1]) > 512
        || fabsf(origin[2] - clEntity->previousState.origin[2]) > 512
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
	clEntity->lerpOrigin = origin;
}

/**
*   @brief  Checks whether the parsed entity is a newcomer or has been around 
*           in previous frames.
*   @return True when either of these conditions are met: 
*               - Last frame was invalid, meaning we can't compare to any previous state.
*               - Last frame was dropped, once again, can't compare to any previous state.
*               - cl_nolerp 2, forced by developer option.
*           False when none of the above conditions are met or cl_nolerp is set to 3:
**/
static inline qboolean LocalEntity_IsNew(const PODEntity *clEntity)
{
	// Always new on the first frame.
    if ( cl.clientFrame.number == 0 ) {
        return true;
    }

	// Wasn't in last local frame.
    //if ( clEntity->serverFrame != cl.frame.number ) {
	if ( clEntity->clientFrame != cl.clientFrame.number - 1 ) {
        return true;
    }

	// Hashname changed.
	if ( clEntity->currentState.hashedClassname != clEntity->previousState.hashedClassname ) {
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

    // No conditions met, so it wasn't in our previous frame.
    return false;
}

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
void LocalEntity_Update(const EntityState *state)
{
	// Ensure that its state number is > MAX_WIRED_POD_ENTITIES
	if ( state->number < MAX_WIRED_POD_ENTITIES ) {
		Com_DPrintf( "(%s): state.number(#%i) < MAX_WIRED_POD_ENTITIES\n", __func__, state->number );
		return;
	}

	// Make sure to have a valid pointer to the actual array. (It is the IGameWorld who manages it.)
	PODEntity *podEntities = CL_GM_GetClientPODEntities();

	if ( !podEntities ) {
		return;
	}

    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    PODEntity *clEntity = &podEntities[ state->number ];
	
	// Ensure client entity number matches the state.
	if ( clEntity->clientEntityNumber != state->number ) {
		Com_DPrintf( "(%s): (clEntity->clientEntityNumber != state.number): Correcting clEntity->clientEntityNumber. \n" );
		clEntity->clientEntityNumber = state->number;
	}

	//  // Add entity to the solids list if it has a solid.

	// Make sure to fetch and adjust solids here.
 //   if ( state->solid && state->number != cl.frame.clientNumber + 1 ) {
	//	if ( state->solid == PACKED_BSP ) {
	//		clEntity->solid = Solid::BSP;
	//		const mmodel_t *model = &cl.cm.cache->models[ clEntity->currentState.modelIndex - 1 ];
	//		clEntity->mins = model->mins;
	//		clEntity->maxs = model->maxs;
	//	} else {
	//		clEntity->solid = state->solid;

	//		// These get set when linking in the entity to our client areagrid world.
	//		//clEntity->mins = state->mins;
	//		//clEntity->maxs = state->maxs;
	//	}
	//} else {
	//	if ( state->number != cl.frame.clientNumber + 1 ) {
	//		//clEntity->mins = vec3_zero();
	//		//clEntity->mins = vec3_zero();
	//		clEntity->solid = Solid::Not;
	//	}
	//}

    // Was this entity in our previous frame, or not?
    if ( LocalEntity_IsNew( clEntity ) ) {
        // Wasn't in last update, so initialize some things.
        LocalEntity_UpdateNew( clEntity, state, state->origin );
    } else {
        // Updates the current and previous state so lerping won't hurt.
        LocalEntity_UpdateExisting( clEntity, state, state->origin );
    }
	
	//// Update the local entities for possible hashed name changes.	
	//LocalEntity_SetHashedClassname (clEntity, state );

	//// Ensure it gets done properly.
	//CL_GM_PacketNewHashedClassname( clEntity, state );

    // Update the serverFrame score to keep track of.
    clEntity->serverFrame = cl.frame.number;
	clEntity->clientFrame = cl.clientFrame.number;

    // Assign the fresh new received state as the entity's current.
	clEntity->currentState = *state;

	// Link entity.
	//CL_PF_World_LinkEntity( clEntity );
}

/**
*   @brief  Ensures its hashedClassname is updated accordingly to that which matches the Game Entity.
**/
void LocalEntity_SetHashedClassname(PODEntity* podEntity, const EntityState* state) {
	// Only continue IF we got a podEntity.
	if (!podEntity || !state) {
		return;
	}

	// No matter what, ensure that the previous frame hashed classname is set to our current.
	podEntity->previousState.hashedClassname = podEntity->currentState.hashedClassname;

	// Retreive and update its current/(possibly, new) hashedClassname after this frame.
	podEntity->currentState.hashedClassname = podEntity->hashedClassname;
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
void LocalEntity_FireEvent(EntityState *state) {
	// Let the LocalEntities react to events.
	CL_GM_LocalEntityEvent(state->number);
	
	// Reset the actual entities eventID.
	state->eventID = 0;
}