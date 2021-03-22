/*
Copyright (C) 1997-2001 Id Software, Inc.

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
//
// pmove.cpp
//
//
// Implements the player movement logic for both client and server game modules
// 
// The pml_t structure is used locally when processing the movement. Consider it
// a temporary structure. The origin, prev_origin, and velocity are copied over
// each frame from the player pmove state of the given entity.
// 
// Afterwards the actual player state is tested after applying the needed tran-
// slations. In case of success, the move is kept. Otherwise, reverted.
// 
// Finally, we copy the results over into the player pmove state. This should
// sort if sum up the method of action that's used for player movement.
//
#include "shared/shared.h"
#include "sharedgame/pmove.h"
#include "client/client.h"

//--------------------------------------------------
// Player Movement configuration.
//
// Most settings can be easily tweaked here to fine tune movement to custom
// desires.
//--------------------------------------------------
// Minimum step height for interpolation by the client.
#define PM_STEP_HEIGHT_MIN		4.f

// Maximum vertical distance for being able to climb a step.
#define PM_STEP_HEIGHT_MAX    18

// The minimum Z plane normal component required for standing.
#define PM_STEP_NORMAL			0.7f

//--------------------------------------------------
// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
//--------------------------------------------------
typedef struct {
    vec3_t      origin;
    vec3_t      velocity;

    vec3_t      forward, right, up;
    float       frametime;


    csurface_t* groundsurface;
    cplane_t    groundplane;
    int         groundcontents;

    vec3_t      previous_origin;
    qboolean    ladder;
} pml_t;


// Static locals.
static pmoveParams_t* pmp;  // Pointer to the player movement parameter settings.
static pml_t pml;           // Local movement state variables.
static pm_move_t* pm;         // Pointer to the actual player move structure.

// Player Movement Parameters
static const float  pm_stopspeed = 100;
static const float  pm_duckspeed = 100;
static const float  pm_accelerate = 10;
static const float  pm_wateraccelerate = 10;
static const float  pm_waterspeed = 400;



//
//=============================================================================
//
//	UTILITY FUNCTIONS.
//
//=============================================================================
//
//
//===============
// PM_ClipVelocity
//
// Walking up a step should kill some velocity.
//  
// Slide off of the impacting object
// returns the blocked flags(1 = floor, 2 = step / wall)
//===============
//
#define STOP_EPSILON    0.1
static void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
    float   backoff;
    float   change;
    int     i;

    backoff = Vec3_Dot(in, normal) * overbounce;

    for (i = 0; i < 3; i++) {
        change = normal[i] * backoff;
        out[i] = in[i] - change;
        if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
            out[i] = 0;
    }
}

//
//===============
// PM_TouchEntity
// 
// Marks the specified entity as touched.
//===============
//
static void PM_TouchEntity(struct edict_s* ent) {
    // Ensure it is valid.
    if (ent == NULL) {
        Com_LPrintf(PRINT_DEVELOPER, "Pm_TouchEntity: ent = NULL\n", MAXTOUCH);
        return;
    }

    // Only touch entity if we aren't at the maximum limit yet.
    if (pm->numtouch < MAXTOUCH && ent) {
        pm->touchents[pm->numtouch] = ent;
        pm->numtouch++;
    } else {
        // Developer print.
        Com_LPrintf(PRINT_DEVELOPER, "Pm_TouchEntity: MAXTOUCH(%i) amount of entities reached for this frame.\n", MAXTOUCH);
    }
}

//
//===============
// PM_ClampAngles
// 
// Clamp angles with deltas. Ensure they pitch doesn't exceed 90 or 270
//===============
//
static void PM_ClampAngles(void)
{
    short   temp;
    int     i;

    if (pm->s.flags & PMF_TIME_TELEPORT) {
        pm->viewangles[YAW] = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->s.delta_angles[YAW]);
        pm->viewangles[PITCH] = 0;
        pm->viewangles[ROLL] = 0;
    }
    else {
        // circularly clamp the angles with deltas
        for (i = 0; i < 3; i++) {
            temp = pm->cmd.angles[i] + pm->s.delta_angles[i];
            pm->viewangles[i] = SHORT2ANGLE(temp);
        }

        // don't let the player look up or down more than 90 degrees
        if (pm->viewangles[PITCH] > 89 && pm->viewangles[PITCH] < 180)
            pm->viewangles[PITCH] = 89;
        else if (pm->viewangles[PITCH] < 271 && pm->viewangles[PITCH] >= 180)
            pm->viewangles[PITCH] = 271;
    }
    AngleVectors(pm->viewangles, pml.forward, pml.right, pml.up);
}


//
//=============================================================================
//
//	STEP SLIDE MOVE
//
//=============================================================================
//
//
//===============
// PM_StepCheck
// 
// Check whether the player just stepped off of something, or not.
//===============
//
static bool PM_CheckStep(trace_t* trace) {

    if (!trace->allsolid) {
        if (trace->ent && trace->plane.normal[2] >= PM_STEP_NORMAL) {
            if (trace->ent != pm->groundentity || trace->plane.dist != pml.groundplane.dist) {
                return true;
            }
        }
    }

    return false;
}

//
//===============
// Pm_StepDown
// 
// Steps the player down, for slope/stair handling.
//===============
//
static void PM_StepDown(trace_t* trace) {

    // Copy the player move state origin 
    Vec3_Copy(pm->s.origin, trace->endpos);

    // Calculate step height.
    pm->step = pm->s.origin[2] - pml.previous_origin[2];

    // If we are above minimal step height, remove the PMF_ON_STAIRS flag.
    if (pm->step >= PM_STEP_HEIGHT_MIN) {
        pm->s.flags |= PMF_ON_STAIRS;
    }
    // If we are stepping down more rapidly than PM_STEP_HEIGHT_MIN then remove the stairs flag.
    else if (pm->step <= -PM_STEP_HEIGHT_MIN && (pm->s.flags & PMF_ON_GROUND)) {
        pm->s.flags |= PMF_ON_STAIRS;
    }
    // Nothing to deal with, set it to 0.
    else {
        pm->step = 0.0;
    }
}

//
//===============
// PM_StepSlideMove_
// 
// Each intersection will try to step over the obstruction instead of
// sliding along it.
//
// Returns a new origin, velocity, and contact entity
// Does not modify any world state ?
//===============
//
#define MIN_STEP_NORMAL 0.7     // can't step up onto very steep slopes
#define MAX_CLIP_PLANES 5
static void PM_StepSlideMove_(void)
{
    int         bumpcount, numbumps;
    vec3_t      dir;
    float       d;
    int         numplanes;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      primal_velocity;
    int         i, j;
    trace_t trace;
    vec3_t      end;
    float       time_left;

    numbumps = 4;

    Vec3_Copy(pml.velocity, primal_velocity);
    numplanes = 0;

    time_left = pml.frametime;

    for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
        for (i = 0; i < 3; i++)
            end[i] = pml.origin[i] + time_left * pml.velocity[i];

        trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

        if (trace.allsolid) {
            // entity is trapped in another solid
            pml.velocity[2] = 0;    // don't build up falling damage
            return;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            Vec3_Copy(trace.endpos, pml.origin);
            numplanes = 0;
        }

        if (trace.fraction == 1)
            break;     // moved the entire distance

        // Save entity for contact (touch) callbacks.
        PM_TouchEntity(trace.ent);

        time_left -= time_left * trace.fraction;

        // slide along this plane
        if (numplanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            Vec3_Copy(vec3_origin, pml.velocity);
            break;
        }

        Vec3_Copy(trace.plane.normal, planes[numplanes]);
        numplanes++;

        //
        // modify original_velocity so it parallels all of the clip planes
        //
        for (i = 0; i < numplanes; i++) {
            PM_ClipVelocity(pml.velocity, planes[i], pml.velocity, 1.01);
            for (j = 0; j < numplanes; j++)
                if (j != i) {
                    if (Vec3_Dot(pml.velocity, planes[j]) < 0)
                        break;  // not ok
                }
            if (j == numplanes)
                break;
        }

        if (i != numplanes) {
            // go along this plane
        }
        else {
            // go along the crease
            if (numplanes != 2) {
                //              Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
                Vec3_Copy(vec3_origin, pml.velocity);
                break;
            }
            Vec3_Cross(planes[0], planes[1], dir);
            d = Vec3_Dot(dir, pml.velocity);
            Vec3_Scale(dir, d, pml.velocity);
        }

        //
        // if velocity is against the original velocity, stop dead
        // to avoid tiny occilations in sloping corners
        //
        if (Vec3_Dot(pml.velocity, primal_velocity) <= 0) {
            Vec3_Copy(vec3_origin, pml.velocity);
            break;
        }
    }

    if (pm->s.time) {
        Vec3_Copy(primal_velocity, pml.velocity);
    }
}

//
//===============
// PM_StepSlideMove
//
// Executes the slide movement.
//===============
//
static void PM_StepSlideMove(void)
{
    vec3_t      start_o, start_v;
    vec3_t      down_o, down_v;
    trace_t     trace;
    float       down_dist, up_dist;
    //  vec3_t      delta;
    vec3_t      up, down;

    Vec3_Copy(pml.origin, start_o);
    Vec3_Copy(pml.velocity, start_v);

    PM_StepSlideMove_();

    Vec3_Copy(pml.origin, down_o);
    Vec3_Copy(pml.velocity, down_v);

    Vec3_Copy(start_o, up);
    up[2] += PM_STEP_HEIGHT_MAX;

    trace = pm->trace(up, pm->mins, pm->maxs, up);
    if (trace.allsolid)
        return;     // can't step up

    // try sliding above
    Vec3_Copy(up, pml.origin);
    Vec3_Copy(start_v, pml.velocity);

    PM_StepSlideMove_();

    // push down the final amount
    Vec3_Copy(pml.origin, down);
    down[2] -= PM_STEP_HEIGHT_MAX;
    trace = pm->trace(pml.origin, pm->mins, pm->maxs, down);
    if (!trace.allsolid) {
        Vec3_Copy(trace.endpos, pml.origin);
    }

    Vec3_Copy(pml.origin, up);

    // decide which one went farther
    down_dist = (down_o[0] - start_o[0]) * (down_o[0] - start_o[0])
        + (down_o[1] - start_o[1]) * (down_o[1] - start_o[1]);
    up_dist = (up[0] - start_o[0]) * (up[0] - start_o[0])
        + (up[1] - start_o[1]) * (up[1] - start_o[1]);

    if (down_dist > up_dist || trace.plane.normal[2] < PM_STEP_NORMAL) {
        Vec3_Copy(down_o, pml.origin);
        Vec3_Copy(down_v, pml.velocity);
        return;
    }
    //!! Special case
    // if we were walking along a plane, then we need to copy the Z over
    pml.velocity[2] = down_v[2];
}


//
//=============================================================================
//
//	ACCELERATION/FRICTION
//
//=============================================================================
//
//
//===============
// PM_Accelerate
//
// Handles both ground friction and water friction
//===============
//
static void PM_Friction(void)
{
    float* vel;
    float   speed, newspeed, control;
    float   friction;
    float   drop;

    vel = pml.velocity;

    speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
    if (speed < 1) {
        vel[0] = 0;
        vel[1] = 0;
        return;
    }

    drop = 0;

    // apply ground friction
    if ((pm->groundentity && pml.groundsurface && !(pml.groundsurface->flags & SURF_SLICK)) || (pml.ladder)) {
        friction = pmp->friction;
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * friction * pml.frametime;
    }

    // apply water friction
    if (pm->waterlevel && !pml.ladder)
        drop += speed * pmp->waterfriction * pm->waterlevel * pml.frametime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }
    newspeed /= speed;

    vel[0] = vel[0] * newspeed;
    vel[1] = vel[1] * newspeed;
    vel[2] = vel[2] * newspeed;
}

//
//===============
// PM_Accelerate
//
// Accelerate function for on-ground.
//===============
//
static void PM_Accelerate(vec3_t wishdir, float wishspeed, float accel)
{
    int         i;
    float       addspeed, accelspeed, currentspeed;

    currentspeed = Vec3_Dot(pml.velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0)
        return;
    accelspeed = accel * pml.frametime * wishspeed;
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    for (i = 0; i < 3; i++)
        pml.velocity[i] += accelspeed * wishdir[i];
}

//
//===============
// PM_AirAccelerate
//
// Accelerate function for in-air.
//===============
//
static void PM_AirAccelerate(vec3_t wishdir, float wishspeed, float accel)
{
    int         i;
    float       addspeed, accelspeed, currentspeed, wishspd = wishspeed;

    if (wishspd > 30)
        wishspd = 30;
    currentspeed = Vec3_Dot(pml.velocity, wishdir);
    addspeed = wishspd - currentspeed;
    if (addspeed <= 0)
        return;
    accelspeed = accel * wishspeed * pml.frametime;
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    for (i = 0; i < 3; i++)
        pml.velocity[i] += accelspeed * wishdir[i];
}

//
//===============
// PM_AddCurrents
//
// Adds all the active currents to our wished for velocity.
// - Ladders
// - Water
// - Conveyor Belts.
//===============
//
static void PM_AddCurrents(vec3_t wishvel)
{
    vec3_t  v;
    float   s;

    // Ladders Velocities.
    if (pml.ladder && fabs(pml.velocity[2]) <= 200) {
        if ((pm->viewangles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
            wishvel[2] = 200;
        else if ((pm->viewangles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
            wishvel[2] = -200;
        else if (pm->cmd.upmove > 0)
            wishvel[2] = 200;
        else if (pm->cmd.upmove < 0)
            wishvel[2] = -200;
        else
            wishvel[2] = 0;

        // Limit horizontal speed when on a ladder
        if (wishvel[0] < -25)
            wishvel[0] = -25;
        else if (wishvel[0] > 25)
            wishvel[0] = 25;

        if (wishvel[1] < -25)
            wishvel[1] = -25;
        else if (wishvel[1] > 25)
            wishvel[1] = 25;
    }


    // Water Current Velocities.
    if (pm->watertype & CONTENTS_MASK_CURRENT) {
        Vec3_Clear(v);

        if (pm->watertype & CONTENTS_CURRENT_0)
            v[0] += 1;
        if (pm->watertype & CONTENTS_CURRENT_90)
            v[1] += 1;
        if (pm->watertype & CONTENTS_CURRENT_180)
            v[0] -= 1;
        if (pm->watertype & CONTENTS_CURRENT_270)
            v[1] -= 1;
        if (pm->watertype & CONTENTS_CURRENT_UP)
            v[2] += 1;
        if (pm->watertype & CONTENTS_CURRENT_DOWN)
            v[2] -= 1;

        s = pm_waterspeed;
        if ((pm->waterlevel == 1) && (pm->groundentity))
            s /= 2;

        Vec3_MA(wishvel, s, v, wishvel);
    }

    // Conveyor Belt Velocities.
    if (pm->groundentity) {
        Vec3_Clear(v);

        if (pml.groundcontents & CONTENTS_CURRENT_0)
            v[0] += 1;
        if (pml.groundcontents & CONTENTS_CURRENT_90)
            v[1] += 1;
        if (pml.groundcontents & CONTENTS_CURRENT_180)
            v[0] -= 1;
        if (pml.groundcontents & CONTENTS_CURRENT_270)
            v[1] -= 1;
        if (pml.groundcontents & CONTENTS_CURRENT_UP)
            v[2] += 1;
        if (pml.groundcontents & CONTENTS_CURRENT_DOWN)
            v[2] -= 1;

        Vec3_MA(wishvel, 100 /* pm->groundentity->speed */, v, wishvel);
    }
}

//
//===============
// PM_WaterMove
//
// Handles in-water movement.
//===============
//
static void PM_WaterMove(void)
{
    int     i;
    vec3_t  wishvel;
    float   wishspeed;
    vec3_t  wishdir;

    //
    // user intentions
    //
    for (i = 0; i < 3; i++)
        wishvel[i] = pml.forward[i] * pm->cmd.forwardmove + pml.right[i] * pm->cmd.sidemove;

    if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
        wishvel[2] -= 60;       // drift towards bottom
    else
        wishvel[2] += pm->cmd.upmove;

    PM_AddCurrents(wishvel);

    Vec3_Copy(wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    if (wishspeed > pmp->maxspeed) {
        Vec3_Scale(wishvel, pmp->maxspeed / wishspeed, wishvel);
        wishspeed = pmp->maxspeed;
    }
    wishspeed *= pmp->watermult;

    PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);

    PM_StepSlideMove();
}

//
//===============
// PM_AirMove
//
// Handles in-air movement.
//===============
//
static void PM_AirMove(void)
{
    int         i;
    vec3_t      wishvel;
    float       fmove, smove;
    vec3_t      wishdir;
    float       wishspeed;
    float       maxspeed;

    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.sidemove;

    //!!!!! pitch should be 1/3 so this isn't needed??!
#if 0
    pml.forward[2] = 0;
    pml.right[2] = 0;
    VectorNormalize(pml.forward);
    VectorNormalize(pml.right);
#endif

    for (i = 0; i < 2; i++)
        wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
    wishvel[2] = 0;

    PM_AddCurrents(wishvel);

    Vec3_Copy(wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    maxspeed = (pm->s.flags & PMF_DUCKED) ? pm_duckspeed : pmp->maxspeed;

    if (wishspeed > maxspeed) {
        Vec3_Scale(wishvel, maxspeed / wishspeed, wishvel);
        wishspeed = maxspeed;
    }

    if (pml.ladder) {
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);
        if (!wishvel[2]) {
            if (pml.velocity[2] > 0) {
                pml.velocity[2] -= pm->s.gravity * pml.frametime;
                if (pml.velocity[2] < 0)
                    pml.velocity[2] = 0;
            }
            else {
                pml.velocity[2] += pm->s.gravity * pml.frametime;
                if (pml.velocity[2] > 0)
                    pml.velocity[2] = 0;
            }
        }
        PM_StepSlideMove();
    }
    else if (pm->groundentity) {
        // walking on ground
        pml.velocity[2] = 0; //!!! this is before the accel
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);


        // PGM  -- fix for negative trigger_gravity fields
        //      pml.velocity[2] = 0;
        if (pm->s.gravity > 0)
            pml.velocity[2] = 0;
        else
            pml.velocity[2] -= pm->s.gravity * pml.frametime;
        // PGM

        if (!pml.velocity[0] && !pml.velocity[1])
            return;
        PM_StepSlideMove();
    }
    else {
        // not on ground, so little effect on velocity
        if (pmp->airaccelerate)
            PM_AirAccelerate(wishdir, wishspeed, pm_accelerate);
        else
            PM_Accelerate(wishdir, wishspeed, 1);
        // add gravity
        pml.velocity[2] -= pm->s.gravity * pml.frametime;
        PM_StepSlideMove();
    }
}


//
//=============================================================================
//
//	SPECIAL MOVEMENT HANDLING
//
//=============================================================================
//
//
//===============
// PM_CheckJump
//
// Tests for whether we can jump. If so, set the appropriate velocity values.
//===============
//
static void PM_CheckJump(void)
{
    if (pm->s.flags & PMF_TIME_LAND) {
        // hasn't been long enough since landing to jump again
        return;
    }

    if (pm->cmd.upmove < 10) {
        // not holding jump
        pm->s.flags &= ~PMF_JUMP_HELD;
        return;
    }

    // must wait for jump to be released
    if (pm->s.flags & PMF_JUMP_HELD)
        return;

    if (pm->s.type == PM_DEAD)
        return;

    if (pm->waterlevel >= 2) {
        // swimming, not jumping
        pm->groundentity = NULL;

        if (pmp->waterhack)
            return;

        if (pml.velocity[2] <= -300)
            return;

        // FIXME: makes velocity dependent on client FPS,
        // even causes prediction misses
        if (pm->watertype == CONTENTS_WATER)
            pml.velocity[2] = 100;
        else if (pm->watertype == CONTENTS_SLIME)
            pml.velocity[2] = 80;
        else
            pml.velocity[2] = 50;
        return;
    }

    if (pm->groundentity == NULL)
        return;     // in air, so no effect

    pm->s.flags |= PMF_JUMP_HELD;

    pm->groundentity = NULL;
    pm->s.flags &= ~PMF_ON_GROUND;
    pml.velocity[2] += 270;
    if (pml.velocity[2] < 270)
        pml.velocity[2] = 270;
}

//
//===============
// PM_CheckSpecialMovements
//
// Checks for special movements such as:
// - Whether we are climbing a ladder.
// - Whether to jump out of the water, or not.
//===============
//
static void PM_CheckSpecialMovements(void)
{
    vec3_t  spot;
    int     cont;
    vec3_t  flatforward;
    trace_t trace;

    if (pm->s.time)
        return;

    pml.ladder = false;

    // check for ladder
    flatforward[0] = pml.forward[0];
    flatforward[1] = pml.forward[1];
    flatforward[2] = 0;
    VectorNormalize(flatforward);

    Vec3_MA(pml.origin, 1, flatforward, spot);
    trace = pm->trace(pml.origin, pm->mins, pm->maxs, spot);
    if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
        pml.ladder = true;

    // check for water jump
    if (pm->waterlevel != 2)
        return;

    Vec3_MA(pml.origin, 30, flatforward, spot);
    spot[2] += 4;
    cont = pm->pointcontents(spot);
    if (!(cont & CONTENTS_SOLID))
        return;

    spot[2] += 16;
    cont = pm->pointcontents(spot);
    if (cont)
        return;
    // jump out of water
    Vec3_Scale(flatforward, 50, pml.velocity);
    pml.velocity[2] = 350;

    pm->s.flags |= PMF_TIME_WATERJUMP;
    pm->s.time = 255;
}

//
//===============
// PM_CheckDuck
//
// Sets the wished for values to crouch:
// pm->mins, pm->maxs, and pm->viewheight
//===============
//
static void PM_CheckDuck(void)
{
    trace_t trace;

    pm->mins[0] = -16;
    pm->mins[1] = -16;

    pm->maxs[0] = 16;
    pm->maxs[1] = 16;

    if (pm->s.type == PM_GIB) {
        pm->mins[2] = 0;
        pm->maxs[2] = 16;
        pm->viewheight = 8;
        return;
    }

    pm->mins[2] = -24;

    if (pm->s.type == PM_DEAD) {
        pm->s.flags |= PMF_DUCKED;
    }
    else if (pm->cmd.upmove < 0 && (pm->s.flags & PMF_ON_GROUND)) {
        // duck
        pm->s.flags |= PMF_DUCKED;
    }
    else {
        // stand up if possible
        if (pm->s.flags & PMF_DUCKED) {
            // try to stand up
            pm->maxs[2] = 32;
            trace = pm->trace(pml.origin, pm->mins, pm->maxs, pml.origin);
            if (!trace.allsolid)
                pm->s.flags &= ~PMF_DUCKED;
        }
    }

    if (pm->s.flags & PMF_DUCKED) {
        pm->maxs[2] = 4;
        pm->viewheight = -2;
    }
    else {
        pm->maxs[2] = 32;
        pm->viewheight = 22;
    }
}


//
//=============================================================================
//
//	POSITION TESTING
//
//=============================================================================
//

//
//===============
// PM_CategorizePosition
//
// + Tests for whether the player is on-ground or not:
//   - In case of the player its velocity being over 180, it will
//     assume it is off ground, and not test any further. 
// + End water jumps.
// + Test falling velocity.
//   - In case its falling velocity is too high, check the pmove landing flag.
// + Test for touching entities, and mark them.
// + Test whether the player view is inside water, or not, and set the
// waterlevel based on that accordingly. (1 to 3)
//===============
//
static void PM_CategorizePosition(void)
{
    vec3_t      point;
    int         cont;
    trace_t     trace;
    int         sample1;
    int         sample2;

    // if the player hull point one unit down is solid, the player
    // is on ground

    // see if standing on something solid
    point[0] = pml.origin[0];
    point[1] = pml.origin[1];
    point[2] = pml.origin[2] - 0.25f;
    if (pml.velocity[2] > 180) { //!!ZOID changed from 100 to 180 (ramp accel)
        pm->s.flags &= ~PMF_ON_GROUND;
        pm->groundentity = NULL;
    }
    else {
        trace = pm->trace(pml.origin, pm->mins, pm->maxs, point);
        pml.groundplane = trace.plane;
        pml.groundsurface = trace.surface;
        pml.groundcontents = trace.contents;

        // No ent, or place normal is under PM_STEP_NORMAL.
        if (!trace.ent || (trace.plane.normal[2] < PM_STEP_NORMAL && !trace.startsolid)) {
            pm->groundentity = NULL;
            pm->s.flags &= ~PMF_ON_GROUND;
        }
        else {
            pm->groundentity = trace.ent;

            // hitting solid ground will end a waterjump
            if (pm->s.flags & PMF_TIME_WATERJUMP) {
                pm->s.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
                pm->s.time = 0;
            }

            if (!(pm->s.flags & PMF_ON_GROUND)) {
                // just hit the ground
                pm->s.flags |= PMF_ON_GROUND;
                // don't do landing time if we were just going down a slope
                if (pml.velocity[2] < -200 && !pmp->strafehack) {
                    pm->s.flags |= PMF_TIME_LAND;
                    // don't allow another jump for a little while
                    if (pml.velocity[2] < -400)
                        pm->s.time = 25;
                    else
                        pm->s.time = 18;
                }
            }
        }

        // PMOVE: Touchentity.
        PM_TouchEntity(trace.ent);
    }

    //
    // get waterlevel, accounting for ducking
    //
    pm->waterlevel = 0;
    pm->watertype = 0;

    sample2 = pm->viewheight - pm->mins[2];
    sample1 = sample2 / 2;

    point[2] = pml.origin[2] + pm->mins[2] + 1;
    cont = pm->pointcontents(point);

    if (cont & CONTENTS_MASK_LIQUID) {
        pm->watertype = cont;
        pm->waterlevel = 1;
        point[2] = pml.origin[2] + pm->mins[2] + sample1;
        cont = pm->pointcontents(point);
        if (cont & CONTENTS_MASK_LIQUID) {
            pm->waterlevel = 2;
            point[2] = pml.origin[2] + pm->mins[2] + sample2;
            cont = pm->pointcontents(point);
            if (cont & CONTENTS_MASK_LIQUID)
                pm->waterlevel = 3;
        }
    }

}

//
//===============
// PM_TestPosition
// 
// Tests for whether the position is valid, or not.
// (In a wall, or object, etc.) 
//===============
//
static qboolean PM_TestPosition(void)
{
    trace_t trace;
    vec3_t  origin, end;
    int     i;

    // This check is not needed anymore. Whether to test for a position or not
    // can now be decided by calling PM_FinalizePosition with true as its arg. 
    //if (pm->s.type == PM_SPECTATOR)
    //    return true;

    // Copy over the s.origin to end and origin for trace testing.
    Vec3_Copy(pm->s.origin, origin);
    Vec3_Copy(pm->s.origin, end);

    // Do a trace test.
    trace = pm->trace(origin, pm->mins, pm->maxs, end);

    // Return whether not allsolid.
    return !trace.allsolid;
}

//
//===============
// PM_FinalizePosition
// 
// Copies over the velocity and origin back into the player movement pmove
// state. 
// 
// If testForValid is true, it'll do some extra work. Where in case of a 
// good position, the function returns and is done. If invalid, it'll revert to
// the old origin. By doing so, we prevent from moving into objects and walls.
// 
// The PM_SpectatorMove for example does NOT test for a valid position, it is 
// free to move wherever it pleases.
//===============
//
static void PM_FinalizePosition(qboolean testForValid) {
    // Copy over velocity and origin.
    Vec3_Copy(pml.velocity, pm->s.velocity);
    Vec3_Copy(pml.origin, pm->s.origin);
    
    // Don't test for a valid position if not wished for.
    if (!testForValid)
        return;

    // Check to see if the position is valid.
    if (PM_TestPosition())
        return;

    // Revert back to the previous origin.
    Vec3_Copy(pml.previous_origin, pm->s.origin);
}

//
//===============
// PM_TestInitialPosition
// 
// In case the position has been changed outside of PMove, it'll test its new
// position and copy it over in case it is valid.
//===============
//
static void PM_TestInitialPosition(void)
{
    // Do 
    if (PM_TestPosition()) {
        // Copy over the state origin in case it is valid.
        Vec3_Copy(pm->s.origin, pml.origin);
        Vec3_Copy(pm->s.origin, pml.previous_origin);
        return;
    }
}


//
//=============================================================================
//
//	PLAYER MOVEMENT STYLE IMPLEMENTATIONS
//
//=============================================================================
//
//
//===============
// PM_FlyMove
// 
// Executes fly movement.
//===============
//
static void PM_FlyMove(void)
{
    float   speed, drop, friction, control, newspeed;
    float   currentspeed, addspeed, accelspeed;
    int         i;
    vec3_t      wishvel;
    float       fmove, smove;
    vec3_t      wishdir;
    float       wishspeed;

    pm->viewheight = 22;

    // Friction
    speed = Vec3_Length(pml.velocity);
    if (speed < 1) {
        Vec3_Copy(vec3_origin, pml.velocity);
    }
    else {
        drop = 0;

        friction = pmp->flyfriction;
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * friction * pml.frametime;

        // scale the velocity
        newspeed = speed - drop;
        if (newspeed < 0)
            newspeed = 0;
        newspeed /= speed;

        Vec3_Scale(pml.velocity, newspeed, pml.velocity);
    }

    // accelerate
    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.sidemove;

    VectorNormalize(pml.forward);
    VectorNormalize(pml.right);

    for (i = 0; i < 3; i++)
        wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
    wishvel[2] += pm->cmd.upmove;

    Vec3_Copy(wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    if (wishspeed > pmp->maxspeed) {
        Vec3_Scale(wishvel, pmp->maxspeed / wishspeed, wishvel);
        wishspeed = pmp->maxspeed;
    }

    currentspeed = Vec3_Dot(pml.velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0) {
        if (!pmp->flyhack) {
            return; // original buggy behaviour
        }
    }
    else {
        accelspeed = pm_accelerate * pml.frametime * wishspeed;
        if (accelspeed > addspeed)
            accelspeed = addspeed;

        for (i = 0; i < 3; i++)
            pml.velocity[i] += accelspeed * wishdir[i];
    }

#if 0
    if (doclip) {
        for (i = 0; i < 3; i++)
            end[i] = pml.origin[i] + pml.frametime * pml.velocity[i];

        trace = pm->trace(pml.origin, pm->mins, pm->maxs, end);

        Vec3_Copy(trace.endpos, pml.origin);
    }
    else
#endif
    {
        // move
        Vec3_MA(pml.origin, pml.frametime, pml.velocity, pml.origin);
    }
}


//
//===============
// PM_DeadMove
// 
// Handles movement when dead movement.
//===============
//
static void PM_DeadMove(void)
{
    float   forward;

    if (!pm->groundentity)
        return;

    // extra friction

    forward = Vec3_Length(pml.velocity);
    forward -= 20;
    if (forward <= 0) {
        Vec3_Clear(pml.velocity);
    }
    else {
        VectorNormalize(pml.velocity);
        Vec3_Scale(pml.velocity, forward, pml.velocity);
    }
}

//
//===============
// PM_SpectatorMove
// 
// Handles special spectator movement.
//===============
//
static void PM_SpectatorMove(void)
{
    // Setup a different frametime for movement.
    pml.frametime = pmp->speedmult * pm->cmd.msec * 0.001f;

    // Execute typical fly movement.
    PM_FlyMove();
    
    // Finalize the position. Do no position testing, a spectator is free to
    // roam where he pleases.
    PM_FinalizePosition(false);
}


//
//===============
// PMove
// 
// Can be called by either the server or the client
//===============
//
void PMove(pm_move_t* pmove, pmoveParams_t* params)
{
    // Store pointers for local usage.
    pm = pmove;
    pmp = params;

    // clear results
    pm->numtouch = 0;
    Vec3_Clear(pm->viewangles);
    pm->viewheight = 0;
    pm->groundentity = NULL;
    pm->watertype = 0;
    pm->waterlevel = 0;

    // Reset the PMF_ON_STAIRS flag that we test for every move.
    pm->s.flags &= ~(PMF_ON_STAIRS);

    // clear all pmove local vars
    memset(&pml, 0, sizeof(pml));

    // Copy over the actual player state data we need into the
    // local player move data. This is where we'll be working with.
    Vec3_Copy(pm->s.origin, pml.origin);
    Vec3_Copy(pm->s.velocity, pml.velocity);
    Vec3_Copy(pm->s.origin, pml.previous_origin);  // Save old origin, just in case we get stuck.

    // Clamp angles.
    PM_ClampAngles();

    // Special spectator movement handling.
    if (pm->s.type == PM_SPECTATOR) {
        PM_SpectatorMove();
        return;
    }

    pml.frametime = pm->cmd.msec * 0.001f;

    if (pm->s.type >= PM_DEAD) {
        pm->cmd.forwardmove = 0;
        pm->cmd.sidemove = 0;
        pm->cmd.upmove = 0;
    }

    if (pm->s.type == PM_FREEZE)
        return;     // no movement at all

    // set mins, maxs, and viewheight
    PM_CheckDuck();

    // Check whether we need to test the initial position, in case it has been modified outside of
    // pmove.cpp
    if (pm->testInitial)
        PM_TestInitialPosition();

    // set groundentity, watertype, and waterlevel
    PM_CategorizePosition();

    // Check for whether we're dead, if so, call PM_DeadMove. It will stop
    // the player from keeping on moving forward.
    if (pm->s.type == PM_DEAD)
        PM_DeadMove();

    // Check for special movements to execute.
    PM_CheckSpecialMovements();

    // drop timing counter
    if (pm->s.time) {
        int     msec;

        msec = pm->cmd.msec >> 3;
        if (!msec)
            msec = 1;
        if (msec >= pm->s.time) {
            pm->s.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
            pm->s.time = 0;
        }
        else
            pm->s.time -= msec;
    }

    if (pm->s.flags & PMF_TIME_TELEPORT) {
        // teleport pause stays exactly in place
    }
    else if (pm->s.flags & PMF_TIME_WATERJUMP) {
        // waterjump has no control, but falls
        pml.velocity[2] -= pm->s.gravity * pml.frametime;
        if (pml.velocity[2] < 0) {
            // cancel as soon as we are falling down again
            pm->s.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
            pm->s.time = 0;
        }

        PM_StepSlideMove();
    }
    else {
        PM_CheckJump();

        PM_Friction();

        if (pm->waterlevel >= 2)
            PM_WaterMove();
        else {
            vec3_t  angles;

            Vec3_Copy(pm->viewangles, angles);
            if (angles[PITCH] > 180)
                angles[PITCH] = angles[PITCH] - 360;
            angles[PITCH] /= 3;

            AngleVectors(angles, pml.forward, pml.right, pml.up);

            PM_AirMove();
        }
    }

    // set groundentity, watertype, and waterlevel for final spot
    PM_CategorizePosition();

    //PM_UpdateClientSoundSpecialEffects();

    PM_FinalizePosition(true);
}


//
//=============================================================================
//
//	PMOVE PARAMETER
//
//=============================================================================
//
//
//===============
// PMoveInit
// 
// Initializes the pmp structure.
//===============
//
void PMoveInit(pmoveParams_t* pmp)
{
    // set up default pmove parameters
    memset(pmp, 0, sizeof(*pmp));

    pmp->speedmult = 1;
    pmp->watermult = 0.5f;
    pmp->maxspeed = 300;
    pmp->friction = 6;
    pmp->waterfriction = 1;
    pmp->flyfriction = 9;
}

//
//===============
// PMoveEnableQW
// 
// Enables QuakeWorld movement on the pmp.
//===============
//
void PMoveEnableQW(pmoveParams_t* pmp)
{
    pmp->qwmode = true;
    pmp->watermult = 0.7f;
    pmp->maxspeed = 320;
    //pmp->upspeed = (sv_qwmod->integer > 1) ? 310 : 350;
    pmp->friction = 4;
    pmp->waterfriction = 4;
    pmp->airaccelerate = true;
}
