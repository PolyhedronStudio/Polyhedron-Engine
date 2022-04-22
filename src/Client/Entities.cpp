/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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
// cl_ents.c -- entity parsing and management

#include "Client.h"
#include "GameModule.h"
#include "Refresh/Models.h"
#include "Sound/Sound.h"

//! Shared data, will be erased not far from now. I promise.
extern ClientShared cs;



/**
*
*
*	Packet State update parsing for received Frame Server Entities.
*
*
**/
/**
*   @brief  The client's player entity is send as an "optimized" packet. Several
*           variables are sent over the player state instead of the entity state.
* 
*   @return True if the entity comes from an optimized packet, false otherwise.
**/
static inline qboolean ServerEntity_IsPlayer(const EntityState& state) {
    if (state.number != cl.frame.clientNumber + 1)
        return false;

    if (cl.frame.playerState.pmove.type >= EnginePlayerMoveType::Dead)
        return false;

    return true;
}

/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static inline void ServerEntity_UpdateNew(PODEntity *clEntity, const EntityState &state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
    clEntity->clientEntityNumber = state.number; // used to be: clEntity->id = ++entity_ctr;
    clEntity->trailCount = 1024;

    // Notify the client game module that we've acquired from the server a fresh new entity to spawn.
    CL_GM_CreateFromNewState(clEntity, state);
    
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
static inline void ServerEntity_UpdateExisting(PODEntity *clEntity, const EntityState &state, const vec_t *origin)
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

	// Update entity number.
	clEntity->clientEntityNumber = state.number;
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
static inline qboolean ServerEntity_IsNew(const PODEntity *clEntity)
{
    // Last received frame was invalid.
    if (!cl.oldframe.valid) {
        return true;
    }

    // Wasn't in last received frame.
    if (clEntity->serverFrame != cl.oldframe.number) {
        return true;
    }

    // Developer option, always new.
    if (cl_nolerp->integer == 2) {
        return true;
    }

    // Developer option, lerp from last received frame.
    if (cl_nolerp->integer == 3) {
        return false;
    }

    // Previous server frame was dropped.
    if (cl.oldframe.number != cl.frame.number - 1) {
        return true;
    }

    // No conditions met, so it wasn't in our previous frame.
    return false;
}

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
static void ServerEntity_UpdateState(const EntityState &state)
{
    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    PODEntity *clEntity = &cs.entities[state.number];

    // Add entity to the solids list if it has a solid.
    if (state.solid && state.number != cl.frame.clientNumber + 1 && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = clEntity;

		// For non BRUSH models...
        if (state.solid != PACKED_BBOX) {
            // Update the actual bounding box.
            clEntity->mins = state.mins;
			clEntity->maxs = state.maxs; //MSG_UnpackBoundingBox32(state.solid, clEntity->mins, clEntity->maxs);
        }
    }

    // Work around Q2PRO server bandwidth optimization.
    const bool isPlayerEntity = ServerEntity_IsPlayer(state);

    // Fetch the entity's origin.
    vec3_t entityOrigin = state.origin;
    if (isPlayerEntity) {
        entityOrigin = cl.frame.playerState.pmove.origin;
    }

    // Was this entity in our previous frame, or not?
    if (ServerEntity_IsNew(clEntity)) {
        // Wasn't in last update, so initialize some things.
        ServerEntity_UpdateNew(clEntity, state, entityOrigin);
    } else {
        // It already exists, update it accordingly.
        ServerEntity_UpdateExisting(clEntity, state, entityOrigin);
    }

    // Assign the fresh new received server frame number that belongs to this frame.
    clEntity->serverFrame = cl.frame.number;

    // Assign the fresh new received state as the entity's current.
    clEntity->currentState = state;

	// Assign its clientEntity number.
	clEntity->clientEntityNumber = state.number;

    // work around Q2PRO server bandwidth optimization
    if (isPlayerEntity) {
        Com_PlayerToEntityState(&cl.frame.playerState, &clEntity->currentState);
    }
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
static void ServerEntity_FireEvent(int number) {
    CL_GM_PacketEntityEvent(number);
}



/**
*
*
*	'State' updating for client entities.
*
*
**/
/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static inline void LocalEntity_UpdateNew(PODEntity *clEntity, const EntityState &state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
    clEntity->clientEntityNumber = 2048 + (++entity_ctr); //state.number; // used to be: clEntity->id = ++entity_ctr;
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
static void LocalEntity_Update(const EntityState &state)
{
	// Ensure that its state number is higher than MAX_PACKET_ENTITIES
	if (state.number < MAX_PACKET_ENTITIES) {
		Com_DPrintf("Warning (%s): state.number(#%i) < MAX_PACKET_ENTITIES\n", __func__, state.number);
		return;
	}

    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    PODEntity *clEntity = &cs.entities[state.number];

    // Add entity to the solids list if it has a solid.
    if (state.solid && state.number != cl.frame.clientNumber + 1 && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.numSolidLocalEntities++;
		cl.solidLocalEntities[cl.numSolidLocalEntities++] = clEntity;

		// For non BRUSH models...
        if (state.solid != PACKED_BBOX) {
            // Update the actual bounding box.
            clEntity->mins = state.mins;
			clEntity->maxs = state.maxs;
        }
    }

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
	//clEntity->currentState = state;
	clEntity->clientEntityNumber = state.number;
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
static void LocalEntity_FireEvent(int number) {
    CL_GM_LocalEntityEvent(number);
}


/**
*
*
*	Player State Update.
*
*
**/
/**
*   @brief  Updates the player's state between previous and current frame.
**/
static void Player_UpdateStates(ServerFrame *previousFrame, ServerFrame *currentFrame, int framediv)
{
    // Fetch states to interpolate between.
    PlayerState *currentPlayerState  = &currentFrame->playerState;
    PlayerState *previousPlayerState = &previousFrame->playerState;

    // No lerping if previous frame was dropped or invalid
	if (!previousFrame->valid) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
	}

    // If the previous frame number we expected does not add up to the one we got
    // passed to this function, go to duplicate.
    int32_t previousFrameNumber = currentFrame->number - framediv;
    if (previousFrame->number != previousFrameNumber) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    // No lerping if player entity was teleported (origin check).
    if (fabs((float)(previousPlayerState->pmove.origin[0] - currentPlayerState->pmove.origin[0])) > 256 ||
        fabs((float)(previousPlayerState->pmove.origin[1] - currentPlayerState->pmove.origin[1])) > 256 ||
        fabs((float)(previousPlayerState->pmove.origin[2] - currentPlayerState->pmove.origin[2])) > 256) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    // No lerping if player entity was teleported (event check)
    PODEntity *playerEntity = &cs.entities[currentFrame->clientNumber + 1];
    if (playerEntity->serverFrame > previousFrameNumber && playerEntity->serverFrame <= currentFrame->number && 
       (playerEntity->currentState.eventID == EntityEvent::PlayerTeleport || playerEntity->currentState.eventID == EntityEvent::OtherTeleport)) 
    {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    // No lerping if teleport bit was flipped.
    if ((previousPlayerState->pmove.flags ^ currentPlayerState->pmove.flags) & PMF_TIME_TELEPORT) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    // No lerping if POV number changed.
    if (previousFrame->clientNumber != currentFrame->clientNumber) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    // Developer option.
    if (cl_nolerp->integer == 1) {
		// Duplicate the currentState as the previousState so lerping doesn't hurt anything.
		*previousPlayerState = *currentPlayerState;
		return;
    }

    return;
}



/**
*
*
*	State Check & DeltaFrame -Processing.
*
*
**/
/**
*   @brief  Set the client state to active after having precached all data.
**/
static void CL_SetActiveState(void) {
    // Switch to an active connection state.
    cls.connectionState = ClientConnectionState::Active;

    // Calculate server delta.
    cl.serverDelta = Q_align(cl.frame.number, CL_FRAMEDIV);
    cl.time = cl.serverTime = 0; // set time, needed for demos

    // Initialize oldframe so lerping doesn't hurt anything.
    cl.oldframe.valid = false;
    cl.oldframe.playerState = cl.frame.playerState;
    cl.frameFlags = 0;
    
    // Update sequences.
    if (cls.netChannel) {
        cl.initialSequence = cls.netChannel->outgoingSequence;
    }

    if (cls.demo.playback) {
        // Initialize first demo frame.
        CL_FirstDemoFrame();
    } else {
        // Set initial cl.predicted_origin and cl.predicted_angles.
        cl.predictedState.viewOrigin = cl.frame.playerState.pmove.origin;
        cl.predictedState.velocity = cl.frame.playerState.pmove.velocity;

        if (cl.frame.playerState.pmove.type < EnginePlayerMoveType::Dead) {
            // Enhanced servers don't send viewAngles
            // Let the client game module predict angles.
            CL_GM_PredictAngles();
        } else {
            // Just use what server provided.
            cl.predictedState.viewAngles = cl.frame.playerState.pmove.viewAngles;
        }
    }

    // Get rid of loading plaque.
    SCR_EndLoadingPlaque();
    SCR_LagClear();

    // Open the menu here after we're done loading the map properly.
    CL_OpenBSPMenu();

    // Close the console to get rid of connection screen.
    Con_Close(false);

    // Check for pauses.
    CL_CheckForPause();

    // Update frame times.
    CL_UpdateFrameTimes();

    if (!cls.demo.playback) {
        EXEC_TRIGGER(cl_beginmapcmd);
        Cmd_ExecTrigger("#cl_enterlevel");
    }
}

/**
*   @brief  A valid frame has been parsed.
**/
void CL_DeltaFrame(void)
{
    // Getting a valid frame message ends the connection process.
    if (cls.connectionState == ClientConnectionState::Precached) {
	    // Spawn all local class entities.
	    CL_GM_SpawnEntitiesFromBSPString(cl.bsp->entityString);

        // Set the client to an active connection state.
        CL_SetActiveState();
    }

    // Set server time
    int32_t frameNumber = cl.frame.number - cl.serverDelta;
    cl.serverTime = frameNumber * CL_FRAMETIME;

    // Rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;
	cl.numSolidLocalEntities = 0;

    // Initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing its origin (such as MuzzleFlashType::Login, etc)
    PODEntity *playerClientEntity = &cs.entities[cl.frame.clientNumber + 1];
    Com_PlayerToEntityState(&cl.frame.playerState, &playerClientEntity->currentState);

	// Process the entities that are 'in-frame' of the received server game frame packet data.
    for (int32_t i = 0; i < cl.frame.numEntities; i++) {
		// Calculate the actual state index of this entity.
        const int32_t stateIndex = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        EntityState &state = cl.entityStates[stateIndex];

        // Update the entity state. (Updates current and previous state.)
        ServerEntity_UpdateState(state);

        // Fire entity event.
        ServerEntity_FireEvent(state.number);
    }

	// The local entities start indexed from MAX_SERVER_POD_ENTITIES up to MAX_CLIENT_POD_ENTITIES.
	// We'll be processing them here.
	for (int32_t i = 2048; i < MAX_CLIENT_POD_ENTITIES; i++) {
		//if (i < totalLocalEntities) {}
		EntityState &localEntityState = cs.entities[i].currentState;

		// I guess it has to come from somewhere right?
		localEntityState.number = i;
		cs.entities[i].clientEntityNumber = i;
		localEntityState.eventID = 0;

		//// Update local entity.
		LocalEntity_Update(localEntityState);

		//// Fire local entity events.
		LocalEntity_FireEvent(localEntityState.number);
	}

    if (cls.demo.recording && !cls.demo.paused && !cls.demo.seeking) {
        CL_EmitDemoFrame();
    }

    if (cls.demo.playback) {
        // this delta has nothing to do with local viewAngles,
        // clear it to avoid interfering with demo freelook hack
        cl.frame.playerState.pmove.deltaAngles = vec3_zero();
    }

    // Activate user input.
    if (cl.oldframe.playerState.pmove.type != cl.frame.playerState.pmove.type) {
        IN_Activate();
    }

    // Update player state between previous and current frame.
    Player_UpdateStates(&cl.oldframe, &cl.frame, 1);

    // Check for prediction errors.
    CL_CheckPredictionError();

    // Call into client game its delta frame function.
    CL_GM_ClientPacketEntityDeltaFrame();
}



/***
*
*
*	Sound Origin and Velocity.
*
*
***/
/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
vec3_t CL_GetEntitySoundOrigin(int entnum) {
    // Pointers.
    PODEntity    *ent;
    mmodel_t        *cm;

    // Vectors.
    vec3_t mid = vec3_zero();
    vec3_t org = vec3_zero();

    if (entnum < 0 || entnum >= MAX_EDICTS) {
        Com_Error(ErrorType::Drop, "%s: bad entnum: %d", __func__, entnum);
    }

    if (!entnum || entnum == listener_entnum) {
        // should this ever happen?
        VectorCopy(listener_origin, org);
        return org;
    }

    // interpolate origin
    // FIXME: what should be the sound origin point for RenderEffects::Beam entities?
    ent = &cs.entities[entnum];
    LerpVector(ent->previousState.origin, ent->currentState.origin, cl.lerpFraction, org);

    // offset the origin for BSP models
    if (ent->currentState.solid == PACKED_BBOX) {
        cm = cl.clipModels[ent->currentState.modelIndex];
        if (cm) {
            VectorAverage(cm->mins, cm->maxs, mid);
            VectorAdd(org, mid, org);
        }
    }

    // return origin
    return org;
}

vec3_t CL_GetViewVelocity(void)
{
    return cl.frame.playerState.pmove.velocity;
}

vec3_t CL_GetEntitySoundVelocity(int ent)
{
	PODEntity *old;
    vec3_t vel = vec3_zero();
	if ((ent < 0) || (ent >= MAX_EDICTS))
	{
		Com_Error(ErrorType::Drop, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];
    
    vel = old->currentState.origin - old->previousState.origin;

    return vel;
}
