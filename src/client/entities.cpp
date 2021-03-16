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
    if (cls.serverProtocol != PROTOCOL_VERSION_Q2PRO)
        return qfalse;

    if (state->number != cl.frame.clientNum + 1)
        return qfalse;

    if (cl.frame.ps.pmove.type >= PM_DEAD)
        return qfalse;

    return qtrue;
}

static inline void
entity_update_new(centity_t *ent, const entity_state_t *state, const vec_t *origin)
{
    static int entity_ctr;
    ent->id = ++entity_ctr;
    ent->trailcount = 1024;     // for diminishing rocket / grenade trails

    // duplicate the current state so lerping doesn't hurt anything
    ent->prev = *state;
#if USE_FPS
    ent->prev_frame = state->frame;
    ent->event_frame = cl.frame.number;
#endif

    if (state->event == EV_PLAYER_TELEPORT ||
        state->event == EV_OTHER_TELEPORT ||
        (state->renderfx & (RF_FRAMELERP | RF_BEAM))) {
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
entity_update_old(centity_t *ent, const entity_state_t *state, const vec_t *origin)
{
    int event = state->event;

#if USE_FPS
    // check for new event
    if (state->event != ent->current.event)
        ent->event_frame = cl.frame.number; // new
    else if (cl.frame.number - ent->event_frame >= cl.framediv)
        ent->event_frame = cl.frame.number; // refreshed
    else
        event = 0; // duplicated
#endif

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
#if USE_FPS
        ent->prev_frame = state->frame;
#endif
        // no lerping if teleported or morphed
        VectorCopy(origin, ent->lerp_origin);
        return;
    }

#if USE_FPS
    // start alias model animation
    if (state->frame != ent->current.frame) {
        ent->prev_frame = ent->current.frame;
        ent->anim_start = cl.servertime - cl.frametime;
        Com_DDPrintf("[%d] anim start %d: %d --> %d [%d]\n",
                     ent->anim_start, state->number,
                     ent->prev_frame, state->frame,
                     cl.frame.number);
    }
#endif

    // shuffle the last state to previous
    ent->prev = ent->current;
}

static inline qboolean entity_new(const centity_t *ent)
{
    if (!cl.oldframe.valid)
        return qtrue;   // last received frame was invalid

    if (ent->serverframe != cl.oldframe.number)
        return qtrue;   // wasn't in last received frame

    if (cl_nolerp->integer == 2)
        return qtrue;   // developer option, always new

    if (cl_nolerp->integer == 3)
        return qfalse;  // developer option, lerp from last received frame

    if (cl.oldframe.number != cl.frame.number - 1)
        return qtrue;   // previous server frame was dropped

    return qfalse;
}

static void entity_update(const entity_state_t *state)
{
    centity_t *ent = &cs.entities[state->number];
    const vec_t *origin;
    vec3_t origin_v;

    // if entity is solid, decode mins/maxs and add to the list
    if (state->solid && state->number != cl.frame.clientNum + 1
        && cl.numSolidEntities < MAX_PACKET_ENTITIES) {
        cl.solidEntities[cl.numSolidEntities++] = ent;
        if (state->solid != PACKED_BSP) {
            // encoded bbox
            if (cl.esFlags & MSG_ES_LONGSOLID) {
                MSG_UnpackSolid32(state->solid, ent->mins, ent->maxs);
            } else {
                MSG_UnpackSolid16(state->solid, ent->mins, ent->maxs);
            }
        }
    }

    // work around Q2PRO server bandwidth optimization
    if (entity_optimized(state)) {
        // N&C: FF Precision.
        VectorCopy(cl.frame.ps.pmove.origin, origin_v);
        //VectorScale(cl.frame.ps.pmove.origin, 0.125f, origin_v);
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
        Com_PlayerToEntityState(&cl.frame.ps, &ent->current);
    }
}

// an entity has just been parsed that has an event value
static void entity_event(int number)
{
    // N&C: Let the CG Module handle this.
    CL_GM_EntityEvent(number);
//    centity_t *cent = &cs.entities[number];
//
//    // EF_TELEPORTER acts like an event, but is not cleared each frame
//    if ((cent->current.effects & EF_TELEPORTER) && CL_FRAMESYNC) {
//        CL_TeleporterParticles(cent->current.origin);
//    }
//
//#if USE_FPS
//    if (cent->event_frame != cl.frame.number)
//        return;
//#endif
//
//    switch (cent->current.event) {
//    case EV_ITEM_RESPAWN:
//        S_StartSound(NULL, number, CHAN_WEAPON, S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
//        CL_ItemRespawnParticles(cent->current.origin);
//        break;
//    case EV_PLAYER_TELEPORT:
//        S_StartSound(NULL, number, CHAN_WEAPON, S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
//        CL_TeleportParticles(cent->current.origin);
//        break;
//    case EV_FOOTSTEP:
//        // WatIsDeze: Commented, will be implemented later when we move this over to CG module.
//        //if (cl_footsteps->integer)
//        //    S_StartSound(NULL, number, CHAN_BODY, cl_sfx_footsteps[rand() & 3], 1, ATTN_NORM, 0);
//        break;
//    case EV_FALLSHORT:
//        S_StartSound(NULL, number, CHAN_AUTO, S_RegisterSound("player/land1.wav"), 1, ATTN_NORM, 0);
//        break;
//    case EV_FALL:
//        S_StartSound(NULL, number, CHAN_AUTO, S_RegisterSound("*fall2.wav"), 1, ATTN_NORM, 0);
//        break;
//    case EV_FALLFAR:
//        S_StartSound(NULL, number, CHAN_AUTO, S_RegisterSound("*fall1.wav"), 1, ATTN_NORM, 0);
//        break;
//    }
}

static void set_active_state(void)
{
    cls.state = ca_active;

    cl.serverdelta = Q_align(cl.frame.number, CL_FRAMEDIV);
    cl.time = cl.servertime = 0; // set time, needed for demos
#if USE_FPS
    cl.keytime = cl.keyservertime = 0;
    cl.keyframe = cl.frame; // initialize keyframe to make sure it's valid
#endif

    // initialize oldframe so lerping doesn't hurt anything
    cl.oldframe.valid = qfalse;
    cl.oldframe.ps = cl.frame.ps;
#if USE_FPS
    cl.oldkeyframe.valid = qfalse;
    cl.oldkeyframe.ps = cl.keyframe.ps;
#endif

    cl.frameflags = 0;

    if (cls.netchan) {
        cl.initialSeq = cls.netchan->outgoing_sequence;
    }

    if (cls.demo.playback) {
        // init some demo things
        CL_FirstDemoFrame();
    } else {
        // set initial cl.predicted_origin and cl.predicted_angles
        // N&C: FF Precision.
        VectorCopy(cl.frame.ps.pmove.origin, cl.predicted_origin);
        VectorCopy(cl.frame.ps.pmove.velocity, cl.predicted_velocity);
        //VectorScale(cl.frame.ps.pmove.origin, 0.125f, cl.predicted_origin);
        //VectorScale(cl.frame.ps.pmove.velocity, 0.125f, cl.predicted_velocity);
        if (cl.frame.ps.pmove.type < PM_DEAD &&
            cls.serverProtocol > PROTOCOL_VERSION_DEFAULT) {
            // enhanced servers don't send viewangles
            // N&C: Let the client game module predict angles.
            CL_GM_PredictAngles();
        } else {
            // just use what server provided
            VectorCopy(cl.frame.ps.viewangles, cl.predicted_angles);
        }
    }

    SCR_EndLoadingPlaque();     // get rid of loading plaque
    SCR_LagClear();
    Con_Close(qfalse);          // get rid of connection screen

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
    centity_t *ent;
    int oldnum;

    // find states to interpolate between
    ps = &frame->ps;
    ops = &oldframe->ps;

    // no lerping if previous frame was dropped or invalid
    if (!oldframe->valid)
        goto dup;

    oldnum = frame->number - framediv;
    if (oldframe->number != oldnum)
        goto dup;

    // no lerping if player entity was teleported (origin check)
    // N&C: FF Precision.
    // CPP: Added fabs instead of abs
    if (fabs((float)(ops->pmove.origin[0] - ps->pmove.origin[0])) > 256 ||
        fabs((float)(ops->pmove.origin[1] - ps->pmove.origin[1])) > 256 ||
        fabs((float)(ops->pmove.origin[2] - ps->pmove.origin[2])) > 256) {
        goto dup;
    }
    //if (abs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 ||
    //    abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 ||
    //    abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256) {
    //    goto dup;
    //}
    //if (abs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8 ||
    //    abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8 ||
    //    abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8) {
    //    goto dup;
    //}
 
    // no lerping if player entity was teleported (event check)
    ent = &cs.entities[frame->clientNum + 1];
    if (ent->serverframe > oldnum &&
        ent->serverframe <= frame->number &&
#if USE_FPS
        ent->event_frame > oldnum &&
        ent->event_frame <= frame->number &&
#endif
        (ent->current.event == EV_PLAYER_TELEPORT
         || ent->current.event == EV_OTHER_TELEPORT)) {
        goto dup;
    }

    // no lerping if teleport bit was flipped
    if ((ops->pmove.flags ^ ps->pmove.flags) & PMF_TELEPORT_BIT)
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
    centity_t           *ent;
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
#if USE_FPS
    cl.keyservertime = (framenum / cl.framediv) * BASE_FRAMETIME;
#endif

    // rebuild the list of solid entities for this frame
    cl.numSolidEntities = 0;

    // initialize position of the player's own entity from playerstate.
    // this is needed in situations when player entity is invisible, but
    // server sends an effect referencing it's origin (such as MZ_LOGIN, etc)
    ent = &cs.entities[cl.frame.clientNum + 1];
    Com_PlayerToEntityState(&cl.frame.ps, &ent->current);

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

    if (prevstate == ca_precached)
        CL_GTV_Resume();
    else
        CL_GTV_EmitFrame();

    if (cls.demo.playback) {
        // this delta has nothing to do with local viewangles,
        // clear it to avoid interfering with demo freelook hack
        VectorClear(cl.frame.ps.pmove.delta_angles);
    }

    if (cl.oldframe.ps.pmove.type != cl.frame.ps.pmove.type) {
        IN_Activate();
    }

    player_update(&cl.oldframe, &cl.frame, 1);

#if USE_FPS
    if (CL_FRAMESYNC)
        player_update(&cl.oldkeyframe, &cl.keyframe, cl.framediv);
#endif

    CL_CheckPredictionError();

    CL_GM_ClientDeltaFrame();
    //SCR_SetCrosshairColor();
}

#ifdef _DEBUG
// for debugging problems when out-of-date entity origin is referenced
void CL_CheckEntityPresent(int entnum, const char *what)
{
    centity_t *e;

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
		if (renderfx & RF_SHELL_HALF_DAM) {
			// ditch the half damage shell if any of red, blue, or double are on
			if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE))
				renderfx &= ~RF_SHELL_HALF_DAM;
		}

		if (renderfx & RF_SHELL_DOUBLE) {
			// lose the yellow shell if we have a red, blue, or green shell
			if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_GREEN))
				renderfx &= ~RF_SHELL_DOUBLE;
			// if we have a red shell, turn it to purple by adding blue
			if (renderfx & RF_SHELL_RED)
				renderfx |= RF_SHELL_BLUE;
			// if we have a blue shell (and not a red shell), turn it to cyan by adding green
			else if (renderfx & RF_SHELL_BLUE) {
				// go to green if it's on already, otherwise do cyan (flash green)
				if (renderfx & RF_SHELL_GREEN)
					renderfx &= ~RF_SHELL_BLUE;
				else
					renderfx |= RF_SHELL_GREEN;
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

static inline float lerp_client_fov(float ofov, float nfov, float lerp)
{
    if (cls.demo.playback) {
        float fov = info_fov->value;

        if (fov < 1)
            fov = 90;
        else if (fov > 160)
            fov = 160;

        if (info_uf->integer & UF_LOCALFOV)
            return fov;

        if (!(info_uf->integer & UF_PLAYERFOV)) {
            if (ofov >= 90)
                ofov = fov;
            if (nfov >= 90)
                nfov = fov;
        }
    }

    return ofov + lerp * (nfov - ofov);
}

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
void CL_GetEntitySoundOrigin(int entnum, vec3_t org)
{
    centity_t   *ent;
    mmodel_t    *cm;
    vec3_t      mid;

    if (entnum < 0 || entnum >= MAX_EDICTS) {
        Com_Error(ERR_DROP, "%s: bad entnum: %d", __func__, entnum);
    }

    if (!entnum || entnum == listener_entnum) {
        // should this ever happen?
        VectorCopy(listener_origin, org);
        return;
    }

    // interpolate origin
    // FIXME: what should be the sound origin point for RF_BEAM entities?
    ent = &cs.entities[entnum];
    LerpVector(ent->prev.origin, ent->current.origin, cl.lerpfrac, org);

    // offset the origin for BSP models
    if (ent->current.solid == PACKED_BSP) {
        cm = cl.model_clip[ent->current.modelindex];
        if (cm) {
            VectorAvg(cm->mins, cm->maxs, mid);
            VectorAdd(org, mid, org);
        }
    }
}

void CL_GetViewVelocity(vec3_t vel)
{
    // N&C: FF Precision.
    VectorCopy(cl.frame.ps.pmove.velocity, vel);
    // restore value from 12.3 fixed point
	//const float scale_factor = 1.0f / 8.0f;

	//vel[0] = (float)cl.frame.ps.pmove.velocity[0] * scale_factor;
	//vel[1] = (float)cl.frame.ps.pmove.velocity[1] * scale_factor;
	//vel[2] = (float)cl.frame.ps.pmove.velocity[2] * scale_factor;
}

void CL_GetEntitySoundVelocity(int ent, vec3_t vel)
{
	centity_t *old;

	if ((ent < 0) || (ent >= MAX_EDICTS))
	{
		Com_Error(ERR_DROP, "CL_GetEntitySoundVelocity: bad ent");
	}

	old = &cs.entities[ent];

	VectorSubtract(old->current.origin, old->prev.origin, vel);
}
