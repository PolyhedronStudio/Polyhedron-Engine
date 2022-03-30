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
#include "Client/GameModule.h"
#include "refresh/models.h"

extern ClientShared cs;
/*
=========================================================================

FRAME PARSING

=========================================================================
*/

/**
*   @brief  The client's player entity is send as an "optimized" packet. Several
*           variables are sent over the player state instead of the entity state.
* 
*   @return True if the entity comes from an optimized packet, false otherwise.
**/
static inline qboolean Entity_IsPlayer(const EntityState& state) {
    if (state.number != cl.frame.clientNumber + 1)
        return false;

    if (cl.frame.playerState.pmove.type >= EnginePlayerMoveType::Dead)
        return false;

    return true;
}

/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static inline void Entity_UpdateNew(ClientEntity *clEntity, const EntityState &state, const vec3_t &origin)
{
    static int32_t entity_ctr = 0;
    clEntity->clientEntityNumber = ++entity_ctr; //state.number; // used to be: clEntity->id = ++entity_ctr;
    clEntity->trailcount = 1024;

    // Notify the client game module that we've acquired from the server a fresh new entity to spawn.
    CL_GM_UpdateFromState(clEntity, state);
    
    // Duplicate the current state into the previous one, this way lerping won't hurt anything.
    clEntity->prev = state;

    // Ensure that when the entity has been teleported we adjust its lerp origin.
    if (state.eventID == EntityEvent::PlayerTeleport || state.eventID == EntityEvent::OtherTeleport
       || (state.renderEffects & (RenderEffects::FrameLerp | RenderEffects::Beam))) 
    {
        // This entity has been teleported.
        clEntity->lerpOrigin = origin;
        return;
    }

    // oldOrigin is valid for new entities, so use it as starting point for interpolating between.
    clEntity->prev.origin = state.oldOrigin;
    clEntity->lerpOrigin = state.oldOrigin;
}

/**
*   @brief  Updates an existing entity using the newly received state for it.
**/
static inline void Entity_UpdateExisting(ClientEntity *entity, const EntityState &state, const vec_t *origin)
{
    // Fetch event ID.
    int32_t eventID = state.eventID;

    if (state.modelIndex != entity->current.modelIndex
        || state.modelIndex2 != entity->current.modelIndex2
        || state.modelIndex3 != entity->current.modelIndex3
        || state.modelIndex4 != entity->current.modelIndex4
        || eventID == EntityEvent::PlayerTeleport
        || eventID == EntityEvent::OtherTeleport
        || fabsf(origin[0] - entity->current.origin[0]) > 512
        || fabsf(origin[1] - entity->current.origin[1]) > 512
        || fabsf(origin[2] - entity->current.origin[2]) > 512
        || cl_nolerp->integer == 1) 
    {
        // Some data changes will force no lerping.
        entity->trailcount = 1024;     // Used for diminishing rocket / grenade trails

        // Duplicate the current state so lerping doesn't hurt anything
        entity->prev = state;

        // No lerping if teleported or morphed
        entity->lerpOrigin = origin;
        return;
    }

    // Shuffle the last state to previous
    entity->prev = entity->current;
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
static inline qboolean Entity_IsNew(const ClientEntity *clEntity)
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
static void Entity_UpdateState(const EntityState &state)
{
    // Acquire a pointer to the client side entity that belongs to the state->number server entity.
    ClientEntity *clEntity = &cs.entities[state.number];

    // If entity its solid is PACKED_BBOX, decode mins/maxs and add to the list
    if (state.solid && state.number != cl.frame.clientNumber + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = clEntity;

        if (state.solid != PACKED_BBOX) {
            // 32 bit encoded bbox
            MSG_UnpackBoundingBox32(state.solid, clEntity->mins, clEntity->maxs);
        }
    }

    // Work around Q2PRO server bandwidth optimization.
    const bool isPlayerEntity = Entity_IsPlayer(state);

    // Fetch the entity's origin.
    vec3_t entityOrigin = state.origin;
    if (isPlayerEntity) {
        entityOrigin = cl.frame.playerState.pmove.origin;
    }

    // Was this entity in our previous frame, or not?
    if (Entity_IsNew(clEntity)) {
        // Wasn't in last update, so initialize some things.
        Entity_UpdateNew(clEntity, state, entityOrigin);
    } else {
        // It already exists, update it accordingly.
        Entity_UpdateExisting(clEntity, state, entityOrigin);
    }

    // Assign the fresh new received server frame number that belongs to this frame.
    clEntity->serverFrame = cl.frame.number;

    // Assign the fresh new received state as the entity's current.
    clEntity->current = state;

    // work around Q2PRO server bandwidth optimization
    if (isPlayerEntity) {
        Com_PlayerToEntityState(&cl.frame.playerState, &clEntity->current);
    }
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
static void Entity_FireEvent(int number) {
    CL_GM_EntityEvent(number);
}

/**
*   @brief  Updates the player's state between previous and current frame.
**/
static void Player_UpdateStates(ServerFrame *previousFrame, ServerFrame *currentFrame, int framediv)
{
    PlayerState *currentPlayerState = nullptr, *previousPlayerState = nullptr;
    ClientEntity *player = nullptr;
    int32_t previousFrameNumber = 0;

    // Fetch states to interpolate between.
    currentPlayerState  = &currentFrame->playerState;
    previousPlayerState = &previousFrame->playerState;

    // No lerping if previous frame was dropped or invalid
    if (!previousFrame->valid)
        goto duplicate;

    // If the previous frame number we expected does not add up to the one we got
    // passed to this function, go to duplicate.
    previousFrameNumber = currentFrame->number - framediv;
    if (previousFrame->number != previousFrameNumber) {
	    goto duplicate;
    }

    // No lerping if player entity was teleported (origin check).
    if (std::fabsf((float)(previousPlayerState->pmove.origin[0] - currentPlayerState->pmove.origin[0])) > 256 ||
        std::fabsf((float)(previousPlayerState->pmove.origin[1] - currentPlayerState->pmove.origin[1])) > 256 ||
        std::fabsf((float)(previousPlayerState->pmove.origin[2] - currentPlayerState->pmove.origin[2])) > 256) {
        goto duplicate;
    }

    // No lerping if player entity was teleported (event check)
    player = &cs.entities[currentFrame->clientNumber + 1];
    if (player->serverFrame > previousFrameNumber && player->serverFrame <= currentFrame->number && 
       (player->current.eventID == EntityEvent::PlayerTeleport || player->current.eventID == EntityEvent::OtherTeleport)) 
    {
        goto duplicate;
    }

    // No lerping if teleport bit was flipped.
    if ((previousPlayerState->pmove.flags ^ currentPlayerState->pmove.flags) & PMF_TIME_TELEPORT) {
        goto duplicate;
    }

    // No lerping if POV number changed.
    if (previousFrame->clientNumber != currentFrame->clientNumber) {
        goto duplicate;
    }

    // Developer option.
    if (cl_nolerp->integer == 1) {
        goto duplicate;
    }

    return;

duplicate:
    // duplicate the current state so lerping doesn't hurt anything
    *previousPlayerState= *currentPlayerState;
}

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

    // Initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing its origin (such as MuzzleFlashType::Login, etc)
    ClientEntity *playerClientEntity = &cs.entities[cl.frame.clientNumber + 1];
    Com_PlayerToEntityState(&cl.frame.playerState, &playerClientEntity->current);

    for (int32_t i = 0; i < cl.frame.numEntities; i++) {
        int32_t stateIndex = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        EntityState &state = cl.entityStates[stateIndex];

        // Update the entity state. (Updates current and previous state.)
        Entity_UpdateState(state);

        // Fire entity event.
        Entity_FireEvent(state.number);
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
    CL_GM_ClientDeltaFrame();
}

/*
==========================================================================

CLIENT ENTITY TRACING FUNCTIONALITY.

==========================================================================
*/

/*
===============
CL_ClipMoveToEntities

Clips the trace against all entities resulting in a final trace result.
===============
*/
void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, ClientEntity *skipEntity, const int32_t contentMask, TraceResult *cmDstTrace) {
    // CM Source Trace.
    TraceResult         cmSrcTrace;
    // Head Node used for testing.
    mnode_t*        headNode = nullptr;
    // Collision model for entity.
    mmodel_t*       cmodel = nullptr;
    // Client side entity.
    ClientEntity*   solidEntity = nullptr;

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
        if (skipEntity != nullptr && skipEntity->current.number == solidEntity->current.number) {
            continue;
        }

        if (solidEntity->current.solid == PACKED_BBOX) {
            // special value for bmodel
            cmodel = cl.clipModels[solidEntity->current.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            // Setup angles and origin for our trace.
            traceAngles = solidEntity->current.angles;
            traceOrigin = solidEntity->current.origin;
        } else {
            vec3_t entityMins = {0.f, 0.f, 0.f};
            vec3_t entityMaxs = {0.f, 0.f, 0.f};

            //MSG_UnpackBoundingBox32(solidEntity->current.solid, entityMins, entityMaxs);
            
            if (solidEntity->current.solid == Solid::OctagonBox) {

                headNode = CM_HeadnodeForOctagon(solidEntity->mins, solidEntity->maxs);
            } else {
                headNode = CM_HeadnodeForBox(solidEntity->mins, solidEntity->maxs);
            }

            traceAngles = vec3_zero();
            traceOrigin = solidEntity->current.origin;
        }

        // We're done clipping against entities if we reached an allSolid aka world.
        if (cmDstTrace->allSolid)
            return;

        CM_TransformedBoxTrace(&cmSrcTrace, start, end,
                               mins, maxs, headNode, contentMask,
                               traceOrigin, traceAngles);

        CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct entity_s*)solidEntity);
    }
}

/*
===============
CL_Trace

Executes a client side trace on the world and its entities using the given contentMask.
Optionally one can pass a pointer to an entity in order to skip(ignore) it.
===============
*/
TraceResult CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, entity_s* skipEntity, const int32_t contentMask) {
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
    trace.ent = reinterpret_cast<entity_s*>(&cl.solidEntities[0]);

    // Clip to other solid entities.
    CL_ClipMoveToEntities(start, mins, maxs, end, reinterpret_cast<ClientEntity*>(skipEntity), contentMask, &trace);

    return trace;
}

/*
==========================================================================

INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS

==========================================================================
*/


/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
vec3_t CL_GetEntitySoundOrigin(int entnum) {
    // Pointers.
    ClientEntity    *ent;
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
    LerpVector(ent->prev.origin, ent->current.origin, cl.lerpFraction, org);

    // offset the origin for BSP models
    if (ent->current.solid == PACKED_BBOX) {
        cm = cl.clipModels[ent->current.modelIndex];
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
	ClientEntity *old;
    vec3_t vel = vec3_zero();
	if ((ent < 0) || (ent >= MAX_EDICTS))
	{
		Com_Error(ErrorType::Drop, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];
    
    vel = old->current.origin - old->prev.origin;

    return vel;
}
