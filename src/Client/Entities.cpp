/***
*
*	License here.
*
*	@file
*
*	Entity Frame Parsing and Frame Processing.
* 
***/

// Client & GM.
#include "Client.h"
#include "GameModule.h"

// Refresh & Sound.
#include "Refresh/Models.h"
#include "Sound/Sound.h"

// Packet Entities.
#include "Entities/PacketEntities.h"

// Packet Entities.
#include "Entities/LocalEntities.h"

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
        // Set the client to an active connection state.
        CL_SetActiveState();
    }

    // Set server time
    int64_t frameNumber = cl.frame.number - cl.serverDelta;
    cl.serverTime = frameNumber * CL_FRAMETIME_UI64;

    // Rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;

    // Initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing its origin (such as MuzzleFlashType::Login, etc)
    PODEntity *playerClientEntity = &cs.entities[cl.frame.clientNumber + 1];
    Com_PlayerToEntityState(&cl.frame.playerState, &playerClientEntity->currentState);

	// Process the entities that are 'in-frame' of the received server game frame packet data.
    for (int32_t i = 0; i < cl.frame.numEntities; i++) {
		// Calculate the actual state index of this entity.
        const int32_t stateIndex = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        EntityState *state = &cl.entityStates[stateIndex];

        // Update the entity state. (Updates current and previous state.)
        PacketEntity_UpdateState(state);
		
		// Get entity pointer.
		PODEntity *podEntity = &cs.entities[state->number];

		// Run the Client Game entity for a frame.		
		PacketEntity_SetHashedClassname(podEntity, &podEntity->currentState);

        // Fire entity event.
        PacketEntity_FireEvent(state->number);
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
		
	// Call into client game module its delta frame function.
	// This gives packet entities a chance to "predict" the next frame before
	// the current data arrives.
	CL_GM_ClientPacketEntityDeltaFrame();

    // Check for prediction errors.
    //CL_CheckPredictionError();
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

    if (entnum < 0 || entnum >= MAX_CLIENT_POD_ENTITIES) {
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
	if ((ent < 0) || (ent >= MAX_CLIENT_POD_ENTITIES))
	{
		Com_Error(ErrorType::Drop, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];
    
    vel = old->currentState.origin - old->previousState.origin;

    return vel;
}
