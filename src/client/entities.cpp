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

#include "client.h"
#include "client/gamemodule.h"
#include "refresh/models.h"

extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;
extern qhandle_t cl_sfx_footsteps[4];

/*
=========================================================================

FRAME PARSING

=========================================================================
*/

static inline qboolean entity_optimized(const entity_state_t *state)
{
    if (cls.serverProtocol != PROTOCOL_VERSION_NAC)
        return false;

    if (state->number != cl.frame.clientNum + 1)
        return false;

    if (cl.frame.playerState.pmove.type >= PM_DEAD)
        return false;

    return true;
}

static inline void
entity_update_new(cl_entity_t *ent, const entity_state_t *state, const vec_t *origin)
{
    static int entity_ctr;
    ent->id = ++entity_ctr;
    ent->trailcount = 1024;     // for diminishing rocket / grenade trails

    // duplicate the current state so lerping doesn't hurt anything
    ent->prev = *state;

    if (state->event == EV_PLAYER_TELEPORT ||
        state->event == EV_OTHER_TELEPORT ||
        (state->renderfx & (RenderEffects::FrameLerp | RenderEffects::Beam))) {
        // no lerping if teleported
        VectorCopy(origin, ent->lerp_origin);
        return;
    }

    // old_origin is valid for new entities,
    // so use it as starting point for interpolating between
    VectorCopy(state->old_origin, ent->prev.origin);
    VectorCopy(state->old_origin, ent->lerp_origin);
}

static inline void
entity_update_old(cl_entity_t *ent, const entity_state_t *state, const vec_t *origin)
{
    int event = state->event;

    if (state->modelindex != ent->current.modelindex
        || state->modelindex2 != ent->current.modelindex2
        || state->modelindex3 != ent->current.modelindex3
        || state->modelindex4 != ent->current.modelindex4
        || event == EV_PLAYER_TELEPORT
        || event == EV_OTHER_TELEPORT
        || fabsf(origin[0] - ent->current.origin[0]) > 512
        || fabsf(origin[1] - ent->current.origin[1]) > 512
        || fabsf(origin[2] - ent->current.origin[2]) > 512
        || cl_nolerp->integer == 1) {
        // some data changes will force no lerping
        ent->trailcount = 1024;     // for diminishing rocket / grenade trails

        // duplicate the current state so lerping doesn't hurt anything
        ent->prev = *state;

        // no lerping if teleported or morphed
        VectorCopy(origin, ent->lerp_origin);
        return;
    }

    // shuffle the last state to previous
    ent->prev = ent->current;
}

static inline qboolean entity_new(const cl_entity_t *ent)
{
    if (!cl.oldframe.valid)
        return true;   // last received frame was invalid

    if (ent->serverframe != cl.oldframe.number)
        return true;   // wasn't in last received frame

    if (cl_nolerp->integer == 2)
        return true;   // developer option, always new

    if (cl_nolerp->integer == 3)
        return false;  // developer option, lerp from last received frame

    if (cl.oldframe.number != cl.frame.number - 1)
        return true;   // previous server frame was dropped

    return false;
}

static void entity_update(const entity_state_t *state)
{
    cl_entity_t *ent = &cs.entities[state->number];
    const vec_t *origin;
    vec3_t origin_v;

    // if entity is solid, decode mins/maxs and add to the list
    if (state->solid && state->number != cl.frame.clientNum + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = ent;
        if (state->solid != PACKED_BSP) {
            // 32 bit encoded bbox
            MSG_UnpackSolid32(state->solid, ent->mins, ent->maxs);
        }
    }

    // work around Q2PRO server bandwidth optimization
    if (entity_optimized(state)) {
        // N&C: FF Precision.
        VectorCopy(cl.frame.playerState.pmove.origin, origin_v);
        //VectorScale(cl.frame.playerState.pmove.origin, 0.125f, origin_v);
        origin = origin_v;
    } else {
        origin = state->origin;
    }

    if (entity_new(ent)) {
        // wasn't in last update, so initialize some things
        entity_update_new(ent, state, origin);
    } else {
        entity_update_old(ent, state, origin);
    }

    ent->serverframe = cl.frame.number;
    ent->current = *state;

    // work around Q2PRO server bandwidth optimization
    if (entity_optimized(state)) {
        Com_PlayerToEntityState(&cl.frame.playerState, &ent->current);
    }
}

// an entity has just been parsed that has an event value
static void entity_event(int number)
{
    // N&C: Let the CG Module handle this.
    CL_GM_EntityEvent(number);
}

static void set_active_state(void)
{
    cls.state = ca_active;

    cl.serverdelta = Q_align(cl.frame.number, CL_FRAMEDIV);
    cl.time = cl.servertime = 0; // set time, needed for demos

    // initialize oldframe so lerping doesn't hurt anything
    cl.oldframe.valid = false;
    cl.oldframe.playerState = cl.frame.playerState;
    cl.frameflags = 0;

    if (cls.netchan) {
        cl.initialSeq = cls.netchan->outgoingSequence;
    }

    if (cls.demo.playback) {
        // init some demo things
        CL_FirstDemoFrame();
    } else {
        // set initial cl.predicted_origin and cl.predicted_angles
        VectorCopy(cl.frame.playerState.pmove.origin, cl.predicted_origin);
        VectorCopy(cl.frame.playerState.pmove.velocity, cl.predicted_velocity);

        if (cl.frame.playerState.pmove.type < PM_DEAD) {
            // enhanced servers don't send viewAngles
            // N&C: Let the client game module predict angles.
            CL_GM_PredictAngles();
        } else {
            // just use what server provided
            VectorCopy(cl.frame.playerState.viewAngles, cl.predicted_angles);
        }
    }

    SCR_EndLoadingPlaque();     // get rid of loading plaque
    SCR_LagClear();
    Con_Close(false);          // get rid of connection screen

    CL_CheckForPause();

    CL_UpdateFrameTimes();

    if (!cls.demo.playback) {
        EXEC_TRIGGER(cl_beginmapcmd);
        Cmd_ExecTrigger("#cl_enterlevel");
    }
}

static void
player_update(server_frame_t *oldframe, server_frame_t *frame, int framediv)
{
    player_state_t *ps, *ops;
    cl_entity_t *ent;
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
    ent = &cs.entities[frame->clientNum + 1];
    if (ent->serverframe > oldnum &&
        ent->serverframe <= frame->number &&
        (ent->current.event == EV_PLAYER_TELEPORT
         || ent->current.event == EV_OTHER_TELEPORT)) {
        goto dup;
    }

    // no lerping if teleport bit was flipped
    if ((ops->pmove.flags ^ ps->pmove.flags) & PMF_TIME_TELEPORT)
        goto dup;
    // no lerping if POV number changed
    if (oldframe->clientNum != frame->clientNum)
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
    cl_entity_t           *ent;
    entity_state_t      *state;
    int                 i, j;
    int                 framenum;
    int                 prevstate = cls.state;

    // getting a valid frame message ends the connection process
    if (cls.state == ca_precached)
        set_active_state();

    // set server time
    framenum = cl.frame.number - cl.serverdelta;
    cl.servertime = framenum * CL_FRAMETIME;

    // rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;

    // initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing it's origin (such as MuzzleFlashType::Login, etc)
    ent = &cs.entities[cl.frame.clientNum + 1];
    Com_PlayerToEntityState(&cl.frame.playerState, &ent->current);

    for (i = 0; i < cl.frame.numEntities; i++) {
        j = (cl.frame.firstEntity + i) & PARSE_ENTITIES_MASK;
        state = &cl.entityStates[j];

        // set current and prev
        entity_update(state);

        // fire events
        entity_event(state->number);
    }

    if (cls.demo.recording && !cls.demo.paused && !cls.demo.seeking && CL_FRAMESYNC) {
        CL_EmitDemoFrame();
    }

    if (cls.demo.playback) {
        // this delta has nothing to do with local viewAngles,
        // clear it to avoid interfering with demo freelook hack
        VectorClear(cl.frame.playerState.pmove.delta_angles);
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
    cl_entity_t *e;

    if (entnum == cl.frame.clientNum + 1) {
        return; // player entity = current
    }

    e = &cs.entities[entnum];
    if (e->serverframe == cl.frame.number) {
        return; // current
    }

    if (e->serverframe) {
        Com_LPrintf(PRINT_DEVELOPER,
                    "SERVER BUG: %s on entity %d last seen %d frames ago\n",
                    what, entnum, cl.frame.number - e->serverframe);
    } else {
        Com_LPrintf(PRINT_DEVELOPER,
                    "SERVER BUG: %s on entity %d never seen before\n",
                    what, entnum);
    }
}
#endif


/*
==========================================================================

INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS

==========================================================================
*/

// Use a static entity ID on some things because the renderer relies on eid to match between meshes
// on the current and previous frames.
#define RESERVED_ENTITIY_GUN 1
#define RESERVED_ENTITIY_SHADERBALLS 2
#define RESERVED_ENTITIY_COUNT 3

static int adjust_shell_fx(int renderfx)
{
	// PMM - at this point, all of the shells have been handled
	// if we're in the rogue pack, set up the custom mixing, otherwise just
	// keep going
	if (!strcmp(fs_game->string, "rogue")) {
		// all of the solo colors are fine.  we need to catch any of the combinations that look bad
		// (double & half) and turn them into the appropriate color, and make double/quad something special
		if (renderfx & RenderEffects::HalfDamShell) {
			// ditch the half damage shell if any of red, blue, or double are on
			if (renderfx & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::DoubleShell))
				renderfx &= ~RenderEffects::HalfDamShell;
		}

		if (renderfx & RenderEffects::DoubleShell) {
			// lose the yellow shell if we have a red, blue, or green shell
			if (renderfx & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::GreenShell))
				renderfx &= ~RenderEffects::DoubleShell;
			// if we have a red shell, turn it to purple by adding blue
			if (renderfx & RenderEffects::RedShell)
				renderfx |= RenderEffects::BlueShell;
			// if we have a blue shell (and not a red shell), turn it to cyan by adding green
			else if (renderfx & RenderEffects::BlueShell) {
				// go to green if it's on already, otherwise do cyan (flash green)
				if (renderfx & RenderEffects::GreenShell)
					renderfx &= ~RenderEffects::BlueShell;
				else
					renderfx |= RenderEffects::GreenShell;
			}
		}
	}

	return renderfx;
}

#if USE_SMOOTH_DELTA_ANGLES
static inline float LerpShort(int a2, int a1, float frac)
{
    if (a1 - a2 > 32768)
        a1 &= 65536;
    if (a2 - a1 > 32768)
        a1 &= 65536;
    return a2 + frac * (a1 - a2);
}
#endif

/*
===============
CL_AddEntities

Emits all entities, particles, and lights to the refresh
===============
*/
void CL_AddEntities(void)
{
    // CL_CalcViewValues(); // N&C: Moved to V_RenderView so CG Module can use these too.
   // CL_FinishViewValues();
    //CL_AddPacketEntities();
   // CL_AddTEnts();
//   // CL_AddParticles();
//#if USE_DLIGHTS
//    CL_AddDLights();
//#endif
//#if USE_LIGHTSTYLES
//    CL_AddLightStyles();
//#endif
    LOC_AddLocationsToScene();
}

/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
vec3_t CL_GetEntitySoundOrigin(int entnum) {
    // Pointers.
    cl_entity_t   *ent;
    mmodel_t    *cm;

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
    LerpVector(ent->prev.origin, ent->current.origin, cl.lerpfrac, org);

    // offset the origin for BSP models
    if (ent->current.solid == PACKED_BSP) {
        cm = cl.model_clip[ent->current.modelindex];
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
	cl_entity_t *old;
    vec3_t vel = vec3_zero();
	if ((ent < 0) || (ent >= MAX_EDICTS))
	{
		Com_Error(ERR_DROP, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];
    
    vel = old->current.origin - old->prev.origin;

    return vel;
}
