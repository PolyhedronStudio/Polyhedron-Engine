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
static inline void LocalEntity_UpdateNew(PODEntity *clEntity, const EntityState &state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
    clEntity->clientEntityNumber = MAX_WIRED_POD_ENTITIES + (++entity_ctr); //state.number; // used to be: clEntity->id = ++entity_ctr;
    clEntity->trailCount = 1024;

    // Notify the client game module that we've acquired from the server a fresh new entity to spawn.
    //CL_GM_CreateFromNewState(clEntity, state);
    
    // Duplicate the current state into the previous one, this way lerping won't hurt anything.
    clEntity->previousState = state;

    // Ensure that when the entity has been teleported we adjust its lerp origin.
    if (state.eventID == EntityEvent::PlayerTeleport || state.eventID == EntityEvent::OtherTeleport
       || (state.renderEffects & (RenderEffects::FrameLerp | RenderEffects::Beam))) 
    {
        // This entity has been teleported.
        clEntity->lerpOrigin = origin;
        return;
    }

    // oldOrigin is valid for new entities, so use it as starting point for interpolating between.
    clEntity->previousState.origin = state.oldOrigin;
    clEntity->lerpOrigin = state.oldOrigin;
}

/**
*   @brief  Updates an existing entity using the newly received state for it.
**/
static inline void LocalEntity_UpdateExisting(PODEntity *clEntity, const EntityState &state, const vec_t *origin)
{
    // Fetch event ID.
    int32_t eventID = state.eventID;

    if (state.modelIndex != clEntity->currentState.modelIndex
        || state.modelIndex2 != clEntity->currentState.modelIndex2
        || state.modelIndex3 != clEntity->currentState.modelIndex3
        || state.modelIndex4 != clEntity->currentState.modelIndex4
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
        clEntity->previousState = state;

        // No lerping if teleported or morphed
        clEntity->lerpOrigin = origin;
        return;
    }

    // Shuffle the last state to previous
    clEntity->previousState = clEntity->currentState;
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
    //// Last received frame was invalid.
    //if (!cl.oldframe.valid) {
    //    return true;
    //}

    //// Wasn't in last received frame.
    //if (clEntity->serverFrame != cl.oldframe.number) {
    //    return true;
    //}

    //// Developer option, always new.
    //if (cl_nolerp->integer == 2) {
    //    return true;
    //}

    //// Developer option, lerp from last received frame.
    //if (cl_nolerp->integer == 3) {
    //    return false;
    //}

    //// Previous server frame was dropped.
    //if (cl.oldframe.number != cl.frame.number - 1) {
    //    return true;
    //}

    // No conditions met, so it wasn't in our previous frame.
    return false;
}

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
void LocalEntity_Update(const EntityState &state)
{
	// Ensure that its state number is between MAX_SERVER_POD_ENTITIES and MAX_WIRED_POD_ENTITIES
	if (state.number < MAX_WIRED_POD_ENTITIES) {
		Com_DPrintf("Warning (%s): state.number(#%i) < MAX_WIRED_POD_ENTITIES\n", __func__, state.number);
		return;
	}

    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    PODEntity *clEntity = &cs.entities[state.number];

    // Add entity to the solids list if it has a solid.
    if (state.solid && state.number != cl.frame.clientNumber + 1 && cl.numSolidLocalEntities < MAX_PACKET_ENTITIES) {
        // Increment num solid local entities.
		cl.numSolidLocalEntities++;

		// Remember to subtract MAX_WIRED_POD_ENTITIES to get the actual array index.
		const int32_t solidLocalEntityIndex = cl.numSolidLocalEntities;
		cl.solidLocalEntities[solidLocalEntityIndex] = clEntity;

		// For non BRUSH models...
        if (state.solid != PACKED_BBOX) {
            // Update the actual bounding box.
            clEntity->mins = state.mins;
			clEntity->maxs = state.maxs;
        }
    }

	// Assign its clientEntity number.
	clEntity->clientEntityNumber = state.number;

    // Was this entity in our previous frame, or not?
    //if (LocalEntity_IsNew(clEntity)) {
    //    // Wasn't in last update, so initialize some things.
    //    LocalEntity_UpdateNew(clEntity, state, state.origin);
    //} else {
        // It already exists, update it accordingly.
        LocalEntity_UpdateExisting(clEntity, state, state.origin);
    //}

    // Assign the fresh new received server frame number that belongs to this frame.
    clEntity->serverFrame = cl.frame.number;

    // Assign the fresh new received state as the entity's current.
	//clEntity->previousState = clEntity->currentState;
	clEntity->currentState = state;
	clEntity->clientEntityNumber = state.number;
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
void LocalEntity_FireEvent(int32_t number) {
    CL_GM_LocalEntityEvent(number);
}