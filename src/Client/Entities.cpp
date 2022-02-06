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
static inline void Entity_UpdateExisting(ClientEntity *ent, const EntityState *state, const vec_t *origin)
{
    // Fetch event ID.
    int32_t eventID = state->eventID;

    if (state->modelIndex != ent->current.modelIndex
        || state->modelIndex2 != ent->current.modelIndex2
        || state->modelIndex3 != ent->current.modelIndex3
        || state->modelIndex4 != ent->current.modelIndex4
        || eventID == EntityEvent::PlayerTeleport
        || eventID == EntityEvent::OtherTeleport
        || fabsf(origin[0] - ent->current.origin[0]) > 512
        || fabsf(origin[1] - ent->current.origin[1]) > 512
        || fabsf(origin[2] - ent->current.origin[2]) > 512
        || cl_nolerp->integer == 1) 
    {
        // Some data changes will force no lerping.
        ent->trailcount = 1024;     // Used for diminishing rocket / grenade trails

        // Duplicate the current state so lerping doesn't hurt anything
        ent->prev = *state;

        // No lerping if teleported or morphed
        ent->lerpOrigin = origin;
        return;
    }

    // Shuffle the last state to previous
    ent->prev = ent->current;
}

/**
*   @brief  Checks whether the parsed entity is a newcomer or has been around 
*           in previous frames.
*   @return True if it has not been around in the previous frame. False if it
*           has not been around in previous frames. 
**/
static inline qboolean Entity_IsNew(const ClientEntity *ent)
{
    if (!cl.oldframe.valid)
        return true;   // Last received frame was invalid.

    if (ent->serverFrame != cl.oldframe.number)
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
static void entity_update(const EntityState *state)
{
    ClientEntity *ent = &cs.entities[state->number];
    const vec_t *origin;
    vec3_t origin_v;

    // If entity its solid is PACKED_BSP, decode mins/maxs and add to the list
    if (state->solid && state->number != cl.frame.clientNumber + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = ent;

        if (state->solid != PACKED_BSP) {
            // 32 bit encoded bbox
            MSG_UnpackBoundingBox32(state->solid, ent->mins, ent->maxs);
        }
    }

    // work around Q2PRO server bandwidth optimization
    const bool isOptimizedEntity = Entity_IsOptimized(state);

    if (isOptimizedEntity) {
        origin = origin_v = cl.frame.playerState.pmove.origin;
    } else {
        origin = state->origin;
    }

    if (Entity_IsNew(ent)) {
        // wasn't in last update, so initialize some things
        Entity_UpdateNew(ent, state, origin);
    } else {
        Entity_UpdateExisting(ent, state, origin);
    }

    ent->serverFrame = cl.frame.number;
    ent->current = *state;

    // work around Q2PRO server bandwidth optimization
    if (isOptimizedEntity) {
        Com_PlayerToEntityState(&cl.frame.playerState, &ent->current);
    }
}

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
static void Entity_ExecuteEvent(int number)
{
    // N&C: Let the CG Module handle this.
    CL_GM_EntityEvent(number);
}

static void set_active_state(void)
{
    cls.connectionState = ClientConnectionState::Active;

    cl.serverDelta = Q_align(cl.frame.number, CL_FRAMEDIV);
    cl.time = cl.serverTime = 0; // set time, needed for demos

    // initialize oldframe so lerping doesn't hurt anything
    cl.oldframe.valid = false;
    cl.oldframe.playerState = cl.frame.playerState;
    cl.frameFlags = 0;
    if (cls.netChannel) {
        cl.initialSequence = cls.netChannel->outgoingSequence;
    }
    if (cls.demo.playback) {
        // init some demo things
        CL_FirstDemoFrame();
    } else {
        // set initial cl.predicted_origin and cl.predicted_angles
        cl.predictedState.viewOrigin = cl.frame.playerState.pmove.origin;
        cl.predictedState.velocity = cl.frame.playerState.pmove.velocity;

        if (cl.frame.playerState.pmove.type < EnginePlayerMoveType::Dead) {
            // enhanced servers don't send viewAngles
            // N&C: Let the client game module predict angles.
            CL_GM_PredictAngles();
        } else {
            // just use what server provided
            cl.predictedState.viewAngles = cl.frame.playerState.pmove.viewAngles;
        }
    }

    SCR_EndLoadingPlaque();     // get rid of loading plaque
    SCR_LagClear();


    // Open the menu here iafter we're done loading the map properly.
    CL_OpenBSPMenu();
    Con_Close(false);          // get rid of connection screen

    CL_CheckForPause();

    CL_UpdateFrameTimes();

    if (!cls.demo.playback) {
        EXEC_TRIGGER(cl_beginmapcmd);
        Cmd_ExecTrigger("#cl_enterlevel");
    }
}

static void
player_update(ServerFrame *oldframe, ServerFrame *frame, int framediv)
{
    PlayerState *ps, *ops;
    ClientEntity *ent;
    int oldnum;

    // find states to interpolate between
    ps = &frame->playerState;
    ops = &oldframe->playerState;

    // no lerping if previous frame was dropped or invalid
    if (!oldframe->valid)
        goto dup;

    oldnum = frame->number - framediv;
    if (oldframe->number != oldnum)
        goto dup;

    // no lerping if player entity was teleported (origin check)
    // N&C: FF Precision.
    // CPP: Added fabs instead of abs
    if (std::fabsf((float)(ops->pmove.origin[0] - ps->pmove.origin[0])) > 256 ||
        std::fabsf((float)(ops->pmove.origin[1] - ps->pmove.origin[1])) > 256 ||
        std::fabsf((float)(ops->pmove.origin[2] - ps->pmove.origin[2])) > 256) {
        goto dup;
    }
    // no lerping if player entity was teleported (event check)
    ent = &cs.entities[frame->clientNumber + 1];
    if (ent->serverFrame > oldnum &&
        ent->serverFrame <= frame->number &&
        (ent->current.eventID == EntityEvent::PlayerTeleport
         || ent->current.eventID == EntityEvent::OtherTeleport)) {
        goto dup;
    }

    // no lerping if teleport bit was flipped
    if ((ops->pmove.flags ^ ps->pmove.flags) & PMF_TIME_TELEPORT)
        goto dup;
    // no lerping if POV number changed
    if (oldframe->clientNumber != frame->clientNumber)
        goto dup;
    // developer option
    if (cl_nolerp->integer == 1)
        goto dup;

    return;

dup:
    // duplicate the current state so lerping doesn't hurt anything
    *ops = *ps;
}

/*
==================
CL_DeltaFrame

A valid frame has been parsed.
==================
*/
void CL_DeltaFrame(void)
{
    ClientEntity    *ent;
    EntityState     *state;
    int32_t i, j;
    int32_t frameNumber;
    int32_t prevstate = cls.connectionState;

    // getting a valid frame message ends the connection process
    if (cls.connectionState == ClientConnectionState::Precached)
        set_active_state();

    // set server time
    frameNumber = cl.frame.number - cl.serverDelta;
    cl.serverTime = frameNumber * CL_FRAMETIME;

    // rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;

    // initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing it's origin (such as MuzzleFlashType::Login, etc)
    ent = &cs.entities[cl.frame.clientNumber + 1];
    Com_PlayerToEntityState(&cl.frame.playerState, &ent->current);

    for (i = 0; i < cl.frame.numEntities; i++) {
        j = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        state = &cl.entityStates[j];

        // set current and prev
        entity_update(state);

        // fire events
        Entity_ExecuteEvent(state->number);
    }

    if (cls.demo.recording && !cls.demo.paused && !cls.demo.seeking && CL_FRAMESYNC()) {
        CL_EmitDemoFrame();
    }

    if (cls.demo.playback) {
        // this delta has nothing to do with local viewAngles,
        // clear it to avoid interfering with demo freelook hack
        cl.frame.playerState.pmove.deltaAngles = vec3_zero();
    }

    if (cl.oldframe.playerState.pmove.type != cl.frame.playerState.pmove.type) {
        IN_Activate();
    }

    player_update(&cl.oldframe, &cl.frame, 1);

    CL_CheckPredictionError();

    CL_GM_ClientDeltaFrame();
    //SCR_SetCrosshairColor();
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
        Com_LPrintf(PRINT_DEVELOPER,
                    "SERVER BUG: %s on entity %d last seen %d frames ago\n",
                    what, entnum, cl.frame.number - e->serverFrame);
    } else {
        Com_LPrintf(PRINT_DEVELOPER,
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
    ClientEntity*   clientEntity = nullptr;

    // Actual start point of the trace. May modify during the loop.
    vec3_t traceOrigin = vec3_zero();
    // Actual angles for the trace. May modify during the loop.
    vec3_t traceAngles = vec3_zero();

    for (uint32_t i = 0; i < cl.numSolidEntities; i++) {
        // Fetch client entity.
        clientEntity = cl.solidEntities[i];

        // This check is likely redundent but let's make sure it is there anyway for possible future changes.
        if (clientEntity == nullptr) {
            continue;
        }

        // Should we skip it?
        if (skipEntity != nullptr && skipEntity->current.number == clientEntity->current.number) {
            continue;
        }

        if (clientEntity->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl.clipModels[clientEntity->current.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            // Setup angles and origin for our trace.
            traceAngles = clientEntity->current.angles;
            traceOrigin = clientEntity->current.origin;
        } else {
            vec3_t entityMins = {0.f, 0.f, 0.f};
            vec3_t entityMaxs = {0.f, 0.f, 0.f};
            MSG_UnpackBoundingBox32(clientEntity->current.solid, entityMins, entityMaxs);
            headNode = CM_HeadnodeForBox(entityMins, entityMaxs);
            traceAngles = vec3_zero();
            traceOrigin = clientEntity->current.origin;
        }

        // We're done clipping against entities if we reached an allSolid aka world.
        if (cmDstTrace->allSolid)
            return;

        CM_TransformedBoxTrace(&cmSrcTrace, start, end,
                               mins, maxs, headNode, contentMask,
                               traceOrigin, traceAngles);

        CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct entity_s*)clientEntity);
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
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
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
        Com_Error(ERR_DROP, "%s: bad entnum: %d", __func__, entnum);
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
    if (ent->current.solid == PACKED_BSP) {
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
		Com_Error(ERR_DROP, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];
    
    vel = old->current.origin - old->prev.origin;

    return vel;
}
