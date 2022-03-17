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


/*
=========================================================================

FRAME PARSING

=========================================================================
*/

/**
*   @brief  Determines whether this entity comes from an optimized packet or not.
*   @return True if it, false otherwise.
**/
static inline qboolean Entity_IsOptimized(const EntityState* state) {
    if (cls.serverProtocol != PROTOCOL_VERSION_POLYHEDRON)
        return false;

    if (state->number != cl.frame.clientNumber + 1)
        return false;

    if (cl.frame.playerState.pmove.type >= EnginePlayerMoveType::Dead)
        return false;

    return true;
}

/**
*   @brief  Creates a new entity based on the newly received entity state.
**/
static inline void Entity_UpdateNew(ClientEntity *ent, const EntityState *state, const vec_t *origin)
{
    static int entity_ctr;
    ent->id = ++entity_ctr;
    ent->trailcount = 1024;

    // Duplicate the current state into the previous one, this way lerping won't hurt anything.
    ent->prev = *state;

    if (state->eventID == EntityEvent::PlayerTeleport || state->eventID == EntityEvent::OtherTeleport
       || (state->renderEffects & (RenderEffects::FrameLerp | RenderEffects::Beam))) 
    {
        // This entity has been teleported.
        ent->lerpOrigin = origin;
        return;
    }

    // oldOrigin is valid for new entities, so use it as starting point for interpolating between.
    ent->prev.origin = state->oldOrigin;
    ent->lerpOrigin = state->oldOrigin;
}

/**
*   @brief  Updates an existing entity using the newly received state for it.
**/
static inline void Entity_UpdateExisting(ClientEntity *player, const EntityState *state, const vec_t *origin)
{
    // Fetch event ID.
    int32_t eventID = state->eventID;

    if (state->modelIndex != player->current.modelIndex
        || state->modelIndex2 != player->current.modelIndex2
        || state->modelIndex3 != player->current.modelIndex3
        || state->modelIndex4 != player->current.modelIndex4
        || eventID == EntityEvent::PlayerTeleport
        || eventID == EntityEvent::OtherTeleport
        || fabsf(origin[0] - player->current.origin[0]) > 512
        || fabsf(origin[1] - player->current.origin[1]) > 512
        || fabsf(origin[2] - player->current.origin[2]) > 512
        || cl_nolerp->integer == 1) 
    {
        // Some data changes will force no lerping.
        player->trailcount = 1024;     // Used for diminishing rocket / grenade trails

        // Duplicate the current state so lerping doesn't hurt anything
        player->prev = *state;

        // No lerping if teleported or morphed
        player->lerpOrigin = origin;
        return;
    }

    // Shuffle the last state to previous
    player->prev = player->current;
}

/**
*   @brief  Checks whether the parsed entity is a newcomer or has been around 
*           in previous frames.
*   @return True if it has not been around in the previous frame. False if it
*           has not been around in previous frames. 
**/
static inline qboolean Entity_IsNew(const ClientEntity *player)
{
    if (!cl.oldframe.valid)
        return true;   // Last received frame was invalid.

    if (player->serverFrame != cl.oldframe.number)
        return true;   // Wasn't in last received frame.

    if (cl_nolerp->integer == 2)
        return true;   // Developer option, always new.

    if (cl_nolerp->integer == 3)
        return false;  // Developer option, lerp from last received frame.

    if (cl.oldframe.number != cl.frame.number - 1)
        return true;   // Previous server frame was dropped.

    return false;
}

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
static void Entity_UpdateState(const EntityState *state)
{
    ClientEntity *player = &cs.entities[state->number];
    vec3_t entityOrigin = vec3_zero();

    // If entity its solid is PACKED_BBOX, decode mins/maxs and add to the list
    if (state->solid && state->number != cl.frame.clientNumber + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = player;

        if (state->solid != PACKED_BBOX) {
            // 32 bit encoded bbox
            MSG_UnpackBoundingBox32(state->solid, player->mins, player->maxs);
        }
    }

    // Work around Q2PRO server bandwidth optimization.
    const bool isOptimizedEntity = Entity_IsOptimized(state);

    if (isOptimizedEntity) {
        entityOrigin = cl.frame.playerState.pmove.origin;
    } else {
        entityOrigin = state->origin;
    }

    if (Entity_IsNew(player)) {
        // Wasn't in last update, so initialize some things.
        Entity_UpdateNew(player, state, entityOrigin);
    } else {
        Entity_UpdateExisting(player, state, entityOrigin);
    }

    player->serverFrame = cl.frame.number;
    player->current = *state;

    // work around Q2PRO server bandwidth optimization
    if (isOptimizedEntity) {
        Com_PlayerToEntityState(&cl.frame.playerState, &player->current);
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
    if (previousFrame->number != previousFrameNumber)
	goto duplicate;

    // No lerping if player entity was teleported (origin check).
    if (std::fabsf((float)(previousPlayerState->pmove.origin[0] - currentPlayerState->pmove.origin[0])) > 256 ||
        std::fabsf((float)(previousPlayerState->pmove.origin[1] - currentPlayerState->pmove.origin[1])) > 256 ||
        std::fabsf((float)(previousPlayerState->pmove.origin[2] - currentPlayerState->pmove.origin[2])) > 256) {
        goto duplicate;
    }
    // no lerping if player entity was teleported (event check)
    player = &cs.entities[currentFrame->clientNumber + 1];
    if (player->serverFrame > previousFrameNumber && player->serverFrame <= currentFrame->number && 
       (player->current.eventID == EntityEvent::PlayerTeleport || player->current.eventID == EntityEvent::OtherTeleport)) 
    {
        goto duplicate;
    }

    // No lerping if teleport bit was flipped.
    if ((previousPlayerState->pmove.flags ^ currentPlayerState->pmove.flags) & PMF_TIME_TELEPORT)
        goto duplicate;
    // No lerping if POV number changed.
    if (previousFrame->clientNumber != currentFrame->clientNumber)
        goto duplicate;
    // Developer option.
    if (cl_nolerp->integer == 1)
        goto duplicate;

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
	    CL_GM_SpawnClassEntities(cl.bsp->entityString);

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
        EntityState *state = &cl.entityStates[stateIndex];

        // Update the entity state. (Sets current and previous state.)
        Entity_UpdateState(state);

        // Fire entity event.
        Entity_FireEvent(state->number);
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

#ifdef _DEBUG
// for debugging problems when out-of-date entity origin is referenced
void CL_CheckEntityPresent(int entnum, const char *what)
{
    ClientEntity *e;

    if (entnum == cl.frame.clientNumber + 1) {
        return; // player entity = current
    }

    e = &cs.entities[entnum];
    if (e->serverFrame == cl.frame.number) {
        return; // current
    }

    if (e->serverFrame) {
        Com_LPrintf(PrintType::Developer,
                    "SERVER BUG: %s on entity %d last seen %d frames ago\n",
                    what, entnum, cl.frame.number - e->serverFrame);
    } else {
        Com_LPrintf(PrintType::Developer,
                    "SERVER BUG: %s on entity %d never seen before\n",
                    what, entnum);
    }
}
#endif

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
void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, ClientEntity *skipEntity, const int32_t contentMask, trace_t *cmDstTrace) {
    // CM Source Trace.
    trace_t         cmSrcTrace;
    // Head Node used for testing.
    mnode_t*        headNode = nullptr;
    // Collision model for entity.
    mmodel_t*       cmodel = nullptr;
    // Client side entity.
    ClientEntity*   player = nullptr;

    // Actual start point of the trace. May modify during the loop.
    vec3_t traceOrigin = vec3_zero();
    // Actual angles for the trace. May modify during the loop.
    vec3_t traceAngles = vec3_zero();

    for (uint32_t i = 0; i < cl.numSolidEntities; i++) {
        // Fetch client entity.
        player = cl.solidEntities[i];

        // This check is likely redundent but let's make sure it is there anyway for possible future changes.
        if (player == nullptr) {
            continue;
        }

        // Should we skip it?
        if (skipEntity != nullptr && skipEntity->current.number == player->current.number) {
            continue;
        }

        if (player->current.solid == PACKED_BBOX) {
            // special value for bmodel
            cmodel = cl.clipModels[player->current.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            // Setup angles and origin for our trace.
            traceAngles = player->current.angles;
            traceOrigin = player->current.origin;
        } else {
            vec3_t entityMins = {0.f, 0.f, 0.f};
            vec3_t entityMaxs = {0.f, 0.f, 0.f};

            MSG_UnpackBoundingBox32(player->current.solid, entityMins, entityMaxs);
            headNode = CM_HeadnodeForBox(entityMins, entityMaxs);

            traceAngles = vec3_zero();
            traceOrigin = player->current.origin;
        }

        // We're done clipping against entities if we reached an allSolid aka world.
        if (cmDstTrace->allSolid)
            return;

        CM_TransformedBoxTrace(&cmSrcTrace, start, end,
                               mins, maxs, headNode, contentMask,
                               traceOrigin, traceAngles);

        CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct entity_s*)player);
    }
}

/*
===============
CL_Trace

Executes a client side trace on the world and its entities using the given contentMask.
Optionally one can pass a pointer to an entity in order to skip(ignore) it.
===============
*/
trace_t CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, entity_s* skipEntity, const int32_t contentMask) {
    trace_t trace;

    // Ensure we can pull of a proper trace.
    if (!cl.bsp || !cl.bsp->nodes) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
        return trace;
    }

    // Execute trace.
    CM_BoxTrace(&trace, start, end, mins, maxs, cl.bsp->nodes, contentMask);

    // Set trace entity.
    trace.ent = (struct entity_s*)&cl.solidEntities[0];

    // Clip to other solid entities.
    CL_ClipMoveToEntities(start, mins, maxs, end, (ClientEntity*)skipEntity, contentMask, &trace);

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
