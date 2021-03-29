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
// The pm_locals_t structure is used locally when processing the movement. Consider it
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
} pm_locals_t;


// Static locals.
static pmoveParams_t* pmp;  // Pointer to the player movement parameter settings.
static pm_locals_t pml;           // Local movement state variables.
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
// vtos
//
// A convenience function for printing vectors.
//===============
//
//char* vtos(const vec3_t &v) {
//    static uint32_t index;
//    static char str[8][MAX_QPATH];
//
//    char* s = str[index++ % 8];
//    Q_scnprintf(s, MAX_QPATH, "(%4.2f %4.2f %4.2f)", v[0], v[1], v[2]);
//
//    return s;
//}

//
//===============
// PM_Debug
//
// Can be enabled on/off for client AND server indistinctively.
//===============
//
#ifdef CGAME_INCLUDE
#define DEBUG_CLIENT_PMOVE 1
#ifdef DEBUG_CLIENT_PMOVE
// Client debug output.
static void CLGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    std::string str = "[CLIENT -- PM_Debug]: ";
    str += func;
    str += "(";
    str += fmt;
    str += ")";
    Com_LPrintf(PRINT_DEVELOPER, str.c_str(), args);

    va_end(args);
}
#define PM_Debug(...) CLGM_Debug(__func__, __VA_ARGS__);
#else
#define PM_Debug () void(0)
#endif // PMOVE_DEBUG
#else
#define DEBUG_SERVER_PMOVE 1
#ifdef DEBUG_SERVER_PMOVE
// Server debug output.
static void SVGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    std::string str = "[SERVER PM_Debug:";
    str += func;
    str += "] ";
    str += fmt;
    str += ")\n";
    Com_LPrintf(PRINT_DEVELOPER, str.c_str(), args);

    va_end(args);
}
#define PM_Debug(...) SVGPM_Debug(__func__, __VA_ARGS__);
#else
#define PM_Debug () void(0)
#endif // PMOVE_DEBUG
#endif // CGAME_INCLUDE

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
static vec3_t PM_ClipVelocity(vec3_t& in, vec3_t& normal, float overbounce)
{
    vec3_t  result;
    float   backoff;
    float   change;
    int     i;

    backoff = DotProduct(in, normal) * overbounce;

    for (i = 0; i < 3; i++) {
        change = normal[i] * backoff;
        result[i] = in[i] - change;
        if (result[i] > -STOP_EPSILON && result[i] < STOP_EPSILON)
            result[i] = 0;
    }

    return result;
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
        PM_Debug("ent = NULL");
        return;
    }

    // Only touch entity if we aren't at the maximum limit yet.
    if (pm->numTouchedEntities < PM_MAX_TOUCH_ENTS && ent) {
        pm->touchedEntities[pm->numTouchedEntities] = ent;
        pm->numTouchedEntities++;
    }
    else {
        // Developer print.
        PM_Debug("PM_MAX_TOUCH_ENTS(%i) amount of entities reached for this frame.", PM_MAX_TOUCH_ENTS);
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

    // In case of teleporting, we wan't to reset pitch and roll, but maintain the yaw.
    if (pm->state.flags & PMF_TIME_TELEPORT) {
        pm->viewAngles[YAW] = SHORT2ANGLE(pm->cmd.angles[YAW] + pm->state.delta_angles[YAW]);
        pm->viewAngles[PITCH] = 0;
        pm->viewAngles[ROLL] = 0;
    }
    else {
        // circularly clamp the angles with deltas
        for (i = 0; i < 3; i++) {
            temp = pm->cmd.angles[i] + pm->state.delta_angles[i];
            pm->viewAngles[i] = SHORT2ANGLE(temp);
        }

        // don't let the player look up or down more than 90 degrees
        if (pm->viewAngles[PITCH] > 89 && pm->viewAngles[PITCH] < 180)
            pm->viewAngles[PITCH] = 89;
        else if (pm->viewAngles[PITCH] < 271 && pm->viewAngles[PITCH] >= 180)
            pm->viewAngles[PITCH] = 271;
    }

    // Calculate the angle vectors for movement.
    AngleVectors(pm->viewAngles, &pml.forward, &pml.right, &pml.up);
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
        if (trace->ent && trace->plane.normal.z >= PM_STEP_NORMAL) {
            if (trace->ent != pm->groundEntity || trace->plane.dist != pml.groundplane.dist) {
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
    VectorCopy(pm->state.origin, trace->endpos);

    // Calculate step height.
    pm->step = pm->state.origin[2] - pml.previous_origin[2];

    // If we are above minimal step height, remove the PMF_ON_STAIRS flag.
    if (pm->step >= PM_STEP_HEIGHT_MIN) {
        pm->state.flags |= PMF_ON_STAIRS;
    }
    // If we are stepping down more rapidly than PM_STEP_HEIGHT_MIN then remove the stairs flag.
    else if (pm->step <= -PM_STEP_HEIGHT_MIN && (pm->state.flags & PMF_ON_GROUND)) {
        pm->state.flags |= PMF_ON_STAIRS;
    }
    // Nothing to deal with, set it to 0.
    else {
        pm->step = 0.0;
    }
}

/**
 * Adapted from Quake III, this function adjusts a trace so that if it starts inside of a wall,
 * it is adjusted so that the trace begins outside of the solid it impacts.
 * @return The actual trace.
 */
const trace_t PM_TraceCorrectAllSolid(const vec3_t& start, const vec3_t& end, const vec3_t& mins, const vec3_t& maxs) {
    const int32_t offsets[] = { 0, 1, -1 };

    // Jitter around
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            for (uint32_t k = 0; k < 3; k++) {
                // Calculate start.
                vec3_t offsetVec = { (vec_t)offsets[i], (vec_t)offsets[j], (vec_t)offsets[k] };
                vec3_t point = start + offsetVec;

                // Execute trace.
                const trace_t trace = pm->Trace(point, end, mins, maxs);

                if (!trace.allsolid) {

                    if (i != 0 || j != 0 || k != 0) {
                        PM_Debug("Fixed all-solid");
                    }

                    return trace;
                }
            }
        }
    }

    PM_Debug("No good position");
    return pm->Trace(start, end, mins, maxs);
}

//
//===============
// PM_ImpactPlane
// 
// Return True if `plane` is unique to `planes` and should be impacted, 
// return false otherwise.
//===============
//
static bool PM_ImpactPlane(vec3_t* planes, int32_t num_planes, const vec3_t& plane) {

    for (int32_t i = 0; i < num_planes; i++) {
        if (DotProduct(plane, planes[i]) > 1.0f - PM_STOP_EPSILON) {
            return false;
        }
    }

    return true;
}

#define OLD_SLIDE_MOVE 1
#ifdef OLD_SLIDE_MOVE
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

    VectorCopy(pml.velocity, primal_velocity);
    numplanes = 0;

    time_left = pml.frametime;

    for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
        for (i = 0; i < 3; i++)
            end[i] = pml.origin[i] + time_left * pml.velocity[i];

        trace = pm->Trace(pml.origin, pm->mins, pm->maxs, end);

        if (trace.allsolid) {
            // entity is trapped in another solid
            pml.velocity[2] = 0;    // don't build up falling damage
            return;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            VectorCopy(trace.endpos, pml.origin);
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
            VectorCopy(vec3_origin, pml.velocity);
            break;
        }

        VectorCopy(trace.plane.normal, planes[numplanes]);
        numplanes++;

        //
        // modify original_velocity so it parallels all of the clip planes
        //
        for (i = 0; i < numplanes; i++) {
            pml.velocity = PM_ClipVelocity(pml.velocity, planes[i], 1.01f);
            for (j = 0; j < numplanes; j++)
                if (j != i) {
                    if (DotProduct(pml.velocity, planes[j]) < 0)
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
                VectorCopy(vec3_origin, pml.velocity);
                break;
            }
            CrossProduct(planes[0], planes[1], dir);
            d = DotProduct(dir, pml.velocity);
            VectorScale(dir, d, pml.velocity);
        }

        //
        // if velocity is against the original velocity, stop dead
        // to avoid tiny occilations in sloping corners
        //
        if (DotProduct(pml.velocity, primal_velocity) <= 0) {
            VectorCopy(vec3_origin, pml.velocity);
            break;
        }
    }

    if (pm->state.time) {
        VectorCopy(primal_velocity, pml.velocity);
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

    VectorCopy(pml.origin, start_o);
    VectorCopy(pml.velocity, start_v);

    PM_StepSlideMove_();

    VectorCopy(pml.origin, down_o);
    VectorCopy(pml.velocity, down_v);

    VectorCopy(start_o, up);
    up[2] += PM_STEP_HEIGHT_MAX;

    trace = pm->Trace(up, pm->mins, pm->maxs, up);
    if (trace.allsolid)
        return;     // can't step up

    // try sliding above
    VectorCopy(up, pml.origin);
    VectorCopy(start_v, pml.velocity);

    PM_StepSlideMove_();

    // push down the final amount
    VectorCopy(pml.origin, down);
    down[2] -= PM_STEP_HEIGHT_MAX;
    trace = pm->Trace(pml.origin, pm->mins, pm->maxs, down);
    if (!trace.allsolid) {
        VectorCopy(trace.endpos, pml.origin);
    }

    VectorCopy(pml.origin, up);

    // decide which one went farther
    down_dist = (down_o[0] - start_o[0]) * (down_o[0] - start_o[0])
        + (down_o[1] - start_o[1]) * (down_o[1] - start_o[1]);
    up_dist = (up[0] - start_o[0]) * (up[0] - start_o[0])
        + (up[1] - start_o[1]) * (up[1] - start_o[1]);

    if (down_dist > up_dist || trace.plane.normal[2] < PM_STEP_NORMAL) {
        VectorCopy(down_o, pml.origin);
        VectorCopy(down_v, pml.velocity);
        return;
    }
    //!! Special case
    // if we were walking along a plane, then we need to copy the Z over
    pml.velocity[2] = down_v[2];
}
#else
//
//===============
// PM_SlideMove
// 
// Calculates a new origin, velocity, and contact entities based on the
// movement commandand world state. Returns true if not blocked.
//===============
//
#define MIN_STEP_NORMAL 0.7     // can't step up onto very steep slopes
#define MAX_CLIP_PLANES 5
static bool PM_SlideMove(void)
{
    int         bumpcount, numbumps;
    vec3_t      dir;
    float       d;
    int         numplanes;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      primal_velocity;
    int         i, j;
    trace_t     trace;
    vec3_t      end;
    float       time_left;

    numbumps = 4;

    VectorCopy(pml.velocity, primal_velocity);
    numplanes = 0;

    time_left = pml.frametime;

    for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
        // Break in case we have run out of time for this frame.
        if (time_left <= 0.0f)
            break;

        // Project desired destination
        VectorMA(pml.origin, time_left, pml.velocity, end);

        // Trace to it.
        trace = pm->Trace(pml.origin, pm->mins, pm->maxs, end);

        // If the player is trapped in a solid, don't build up Z
        if (trace.allsolid) {
            // entity is trapped in another solid
            pml.velocity[2] = 0;    // don't build up falling damage
            return true;
        }


        // if the trace succeeded, move some distance
        if (trace.fraction > 0.0f) {
            VectorCopy(trace.endpos, pm->state.origin);

            // if the trace didn't hit anything, we're done
            if (trace.fraction == 1.0f) {
                break;
            }

            // update the movement time remaining
            time_left -= (time_left * trace.fraction);
        }

        // OLD FRACTION CODE.
        //--------------------------------------------
        //if (trace.fraction > 0) {
        //    // actually covered some distance
        //    VectorCopy(trace.endpos, pml.origin);
        //    numplanes = 0;
        //}

        //if (trace.fraction == 1)
        //    break;     // moved the entire distance
        //--------------------------------------------

        // Save entity for contact (touch) callbacks.
        PM_TouchEntity(trace.ent);

        time_left -= time_left * trace.fraction;

        // slide along this plane
        if (numplanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            VectorCopy(vec3_origin, pml.velocity);
            break;
        }

        // Record the impacted plane, or nudge velocity out along it
        if (PM_ImpactPlane(planes, numplanes, trace.plane.normal)) {
            VectorCopy(trace.plane.normal, planes[numplanes]);
            numplanes++;
        }
        else {
            // if we've seen this plane before, nudge our velocity out along it
            //pm->state.velocity = VectorAdd(pm->s.velocity, trace.plane.normal);
            VectorAdd(pm->state.velocity, trace.plane.normal, pm->state.velocity);
            continue;
        }

        // and modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numplanes; i++) {
            vec3_t vel;

            // If velocity doesn't impact this plane, skip it
            if (DotProduct(pm->state.velocity, planes[i]) >= 0.0f) {
                continue;
            }

            // Slide along the plane
            vel = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

            // See if there is a second plane that the new move enters
            for (int32_t j = 0; j < numplanes; j++) {
                vec3_t cross;

                if (j == i) {
                    continue;
                }

                // If the clipped velocity doesn't impact this plane, skip it
                if (DotProduct(vel, planes[j]) >= 0.0f) {
                    continue;
                }

                // We are now intersecting a second plane
                vel = PM_ClipVelocity(vel, planes[j], PM_CLIP_BOUNCE);

                // But if we clip against it without being deflected back, we're okay
                if (DotProduct(vel, planes[i]) >= 0.0f) {
                    continue;
                }

                // We must now slide along the crease (cross product of the planes)
                CrossProduct(planes[i], planes[j], cross);
                VectorNormalize(cross);//cross = Vec3_Normalize(cross);

                const float scale = DotProduct(cross, pm->state.velocity);
                VectorScale(cross, scale, vel);//vel = VectorScale(cross, scale);

                // See if there is a third plane the the new move enters
                for (int32_t k = 0; k < numplanes; k++) {

                    if (k == i || k == j) {
                        continue;
                    }

                    if (DotProduct(vel, planes[k]) >= 0.0f) {
                        continue;
                    }

                    // Stop dead at a triple plane interaction
                    VectorCopy(vec3_origin, pm->state.velocity);//pm->state.velocity = vec3_origin;
                    return true;
                }
            }

            // If we have fixed all interactions, try another move
            VectorCopy(vel, pm->state.velocity);//pm->s.velocity = vel;
            break;
        }
    }

    return bumpcount == 0;
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
    vec3_t upV = { 0.f, 0.f, 1.f };
    vec3_t downV = { 0.f, 0.f, -1.f };

    // Store pre-move parameters
    vec3_t org0;
    vec3_t vel0;
    VectorCopy(pm->state.origin, org0);
    VectorCopy(pm->state.velocity, vel0);

    // Attempt to move; if nothing blocks us, we're done
    if (PM_SlideMove()) {

        // Attempt to step down to remain on ground
        if ((pm->state.flags & PMF_ON_GROUND) && pm->cmd.upmove <= 0) {
            vec3_t down;

            // Vec3_FMAF
            for (int i = 0; i < 3; i++) down[i] = pm->state.origin[i] + (PM_STEP_HEIGHT + PM_GROUND_DIST) * downV[i];

            // Exceute trace for determining whether to step or not.
            trace_t step_down = pm->Trace(pm->state.origin, down, pm->mins, pm->maxs);

            // Step if needed.
            if (PM_CheckStep(&step_down)) {
                PM_StepDown(&step_down);
            }
        }

        return;
    }

    // We were blocked, so try to step over the obstacle
    vec3_t org1;
    vec3_t vel1;
    VectorCopy(pm->state.origin, org1);
    VectorCopy(pm->state.velocity, vel1);


    vec3_t up;
    for (int i = 0; i < 3; i++) up[i] = org0[i] + PM_STEP_HEIGHT * upV[i];

    // Execute trace.
    trace_t step_up = pm->Trace(org0, up, pm->mins, pm->maxs);

    if (!step_up.allsolid) {

        // Step from the higher position, with the original velocity
        VectorCopy(step_up.endpos, pm->state.origin);
        VectorCopy(vel0, pm->state.velocity);

        PM_SlideMove();

        // Settle to the new ground, keeping the step if and only if it was successful
        vec3_t down;            // Vec3_FMAF
        for (int i = 0; i < 3; i++) down[i] = org0[i] + PM_STEP_HEIGHT * upV[i];

        trace_t step_down = pm->Trace(pm->state.origin, down, pm->mins, pm->maxs);

        if (PM_CheckStep(&step_down)) {
            // Quake2 trick jump secret sauce
            if ((pm->state.flags & PMF_ON_GROUND) || vel0[2] < PM_SPEED_UP) {
                PM_StepDown(&step_down);
            }
            else {
                pm->step = pm->state.origin[2] - pml.previous_origin[2];
                pm->state.flags |= PMF_ON_STAIRS;
            }

            return;
        }
    }

    // Copy results into the actual state.
    VectorCopy(org1, pm->state.origin);
    VectorCopy(vel1, pm->state.velocity);
}
#endif // OLD_SLIDE_MOVE


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
    vec3_t  vel;
    float   speed, newspeed, control;
    float   friction;
    float   drop;

    vel = pml.velocity;

    speed = std::sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
    if (speed < 1) {
        vel[0] = 0;
        vel[1] = 0;
        return;
    }

    drop = 0;

    // apply ground friction
    if ((pm->groundEntity && pml.groundsurface && !(pml.groundsurface->flags & SURF_SLICK)) || (pml.ladder)) {
        friction = pmp->friction;
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * friction * pml.frametime;
    }

    // apply water friction
    if (pm->waterLevel && !pml.ladder)
        drop += speed * pmp->waterfriction * pm->waterLevel * pml.frametime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }
    newspeed /= speed;

    vel[0] = vel[0] * newspeed;
    vel[1] = vel[1] * newspeed;
    vel[2] = vel[2] * newspeed;

    // Apply new velocity to pml.
    pml.velocity = vel;
}

//
//===============
// PM_Accelerate
//
// Accelerate function for on-ground.
//===============
//
static void PM_Accelerate(vec3_t& wishdir, float wishspeed, float accel)
{
    int         i;
    float       addspeed, accelspeed, currentspeed;

    currentspeed = DotProduct(pml.velocity, wishdir);
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
static void PM_AirAccelerate(vec3_t& wishdir, float wishspeed, float accel)
{
    int         i;
    float       addspeed, accelspeed, currentspeed, wishspd = wishspeed;

    if (wishspd > 30)
        wishspd = 30;
    currentspeed = DotProduct(pml.velocity, wishdir);
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
// Returns the new velocity with all the active currents added to it:
// - Ladders
// - Water
// - Conveyor Belts.
//===============
//
static vec3_t PM_AddCurrents(const vec3_t& vel)
{
    vec3_t  v;
    float   s;

    // Working copy.
    vec3_t  wishvel = vel;

    // Ladders Velocities.
    if (pml.ladder && fabs(pml.velocity[2]) <= 200) {
        if ((pm->viewAngles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
            wishvel[2] = 200;
        else if ((pm->viewAngles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
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
    if (pm->waterType & CONTENTS_MASK_CURRENT) {
        VectorClear(v);

        if (pm->waterType & CONTENTS_CURRENT_0)
            v[0] += 1;
        if (pm->waterType & CONTENTS_CURRENT_90)
            v[1] += 1;
        if (pm->waterType & CONTENTS_CURRENT_180)
            v[0] -= 1;
        if (pm->waterType & CONTENTS_CURRENT_270)
            v[1] -= 1;
        if (pm->waterType & CONTENTS_CURRENT_UP)
            v[2] += 1;
        if (pm->waterType & CONTENTS_CURRENT_DOWN)
            v[2] -= 1;

        s = pm_waterspeed;
        if ((pm->waterLevel == 1) && (pm->groundEntity))
            s /= 2;

        VectorMA(wishvel, s, v, wishvel);
    }

    // Conveyor Belt Velocities.
    if (pm->groundEntity) {
        VectorClear(v);

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

        VectorMA(wishvel, 100 /* pm->groundEntity->speed */, v, wishvel);
    }

    return wishvel;
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

    wishvel = PM_AddCurrents(wishvel);

    VectorCopy(wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    if (wishspeed > pmp->maxspeed) {
        VectorScale(wishvel, pmp->maxspeed / wishspeed, wishvel);
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

    wishvel = PM_AddCurrents(wishvel);

    VectorCopy(wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    maxspeed = (pm->state.flags & PMF_DUCKED) ? pm_duckspeed : pmp->maxspeed;

    if (wishspeed > maxspeed) {
        VectorScale(wishvel, maxspeed / wishspeed, wishvel);
        wishspeed = maxspeed;
    }

    if (pml.ladder) {
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);
        if (!wishvel[2]) {
            if (pml.velocity[2] > 0) {
                pml.velocity[2] -= pm->state.gravity * pml.frametime;
                if (pml.velocity[2] < 0)
                    pml.velocity[2] = 0;
            }
            else {
                pml.velocity[2] += pm->state.gravity * pml.frametime;
                if (pml.velocity[2] > 0)
                    pml.velocity[2] = 0;
            }
        }
        PM_StepSlideMove();
    }
    else if (pm->groundEntity) {
        // walking on ground
        pml.velocity[2] = 0; //!!! this is before the accel
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);


        // PGM  -- fix for negative trigger_gravity fields
        //      pml.velocity[2] = 0;
        if (pm->state.gravity > 0)
            pml.velocity[2] = 0;
        else
            pml.velocity[2] -= pm->state.gravity * pml.frametime;
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
        pml.velocity[2] -= pm->state.gravity * pml.frametime;
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
    if (pm->state.flags & PMF_TIME_LAND) {
        // hasn't been long enough since landing to jump again
        return;
    }

    if (pm->cmd.upmove < 10) {
        // not holding jump
        pm->state.flags &= ~PMF_JUMP_HELD;
        return;
    }

    // must wait for jump to be released
    if (pm->state.flags & PMF_JUMP_HELD)
        return;

    if (pm->state.type == PM_DEAD)
        return;

    if (pm->waterLevel >= 2) {
        // swimming, not jumping
        pm->groundEntity = NULL;

        if (pmp->waterhack)
            return;

        if (pml.velocity[2] <= -300)
            return;

        // FIXME: makes velocity dependent on client FPS,
        // even causes prediction misses
        if (pm->waterType == CONTENTS_WATER)
            pml.velocity[2] = 100;
        else if (pm->waterType == CONTENTS_SLIME)
            pml.velocity[2] = 80;
        else
            pml.velocity[2] = 50;
        return;
    }

    if (pm->groundEntity == NULL)
        return;     // in air, so no effect

    pm->state.flags |= PMF_JUMP_HELD;

    pm->groundEntity = NULL;
    pm->state.flags &= ~PMF_ON_GROUND;
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

    if (pm->state.time)
        return;

    pml.ladder = false;

    // check for ladder
    flatforward[0] = pml.forward[0];
    flatforward[1] = pml.forward[1];
    flatforward[2] = 0;
    VectorNormalize(flatforward);

    VectorMA(pml.origin, 1, flatforward, spot);
    trace = pm->Trace(pml.origin, pm->mins, pm->maxs, spot);
    if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
        pml.ladder = true;

    // check for water jump
    if (pm->waterLevel != 2)
        return;

    VectorMA(pml.origin, 30, flatforward, spot);
    spot[2] += 4;
    cont = pm->PointContents(spot);
    if (!(cont & CONTENTS_SOLID))
        return;

    spot[2] += 16;
    cont = pm->PointContents(spot);
    if (cont)
        return;
    // jump out of water
    VectorScale(flatforward, 50, pml.velocity);
    pml.velocity[2] = 350;

    pm->state.flags |= PMF_TIME_WATERJUMP;
    pm->state.time = 255;
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

    if (pm->state.type == PM_GIB) {
        pm->mins[2] = 0;
        pm->maxs[2] = 16;
        pm->state.view_offset[2] = 8;
        return;
    }

    pm->mins[2] = -24;

    if (pm->state.type == PM_DEAD) {
        pm->state.flags |= PMF_DUCKED;
    }
    else if (pm->cmd.upmove < 0 && (pm->state.flags & PMF_ON_GROUND)) {
        // duck
        pm->state.flags |= PMF_DUCKED;
    }
    else {
        // stand up if possible
        if (pm->state.flags & PMF_DUCKED) {
            // try to stand up
            pm->maxs[2] = 32;
            trace = pm->Trace(pml.origin, pm->mins, pm->maxs, pml.origin);
            if (!trace.allsolid)
                pm->state.flags &= ~PMF_DUCKED;
        }
    }

    if (pm->state.flags & PMF_DUCKED) {
        pm->maxs[2] = 4;
        pm->state.view_offset[2] = -2;
    }
    else {
        pm->maxs[2] = 32;
        pm->state.view_offset[2] = 22;
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
// waterLevel based on that accordingly. (1 to 3)
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
        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntity = NULL;
    }
    else {
        trace = pm->Trace(pml.origin, pm->mins, pm->maxs, point);
        pml.groundplane = trace.plane;
        pml.groundsurface = trace.surface;
        pml.groundcontents = trace.contents;

        // No ent, or place normal is under PM_STEP_NORMAL.
        if (!trace.ent || (trace.plane.normal[2] < PM_STEP_NORMAL && !trace.startsolid)) {
            pm->groundEntity = NULL;
            pm->state.flags &= ~PMF_ON_GROUND;
        }
        else {
            pm->groundEntity = trace.ent;

            // hitting solid ground will end a waterjump
            if (pm->state.flags & PMF_TIME_WATERJUMP) {
                pm->state.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
                pm->state.time = 0;
            }

            if (!(pm->state.flags & PMF_ON_GROUND)) {
                // just hit the ground
                pm->state.flags |= PMF_ON_GROUND;
                // don't do landing time if we were just going down a slope
                if (pml.velocity[2] < -200 && !pmp->strafehack) {
                    pm->state.flags |= PMF_TIME_LAND;
                    // don't allow another jump for a little while
                    if (pml.velocity[2] < -400)
                        pm->state.time = 25;
                    else
                        pm->state.time = 18;
                }
            }
        }

        // PMOVE: Touchentity.
        PM_TouchEntity(trace.ent);
    }

    //
    // get waterLevel, accounting for ducking
    //
    pm->waterLevel = 0;
    pm->waterType = 0;

    sample2 = pm->state.view_offset[2] - pm->mins[2];
    sample1 = sample2 / 2;

    point[2] = pml.origin[2] + pm->mins[2] + 1;
    cont = pm->PointContents(point);

    if (cont & CONTENTS_MASK_LIQUID) {
        pm->waterType = cont;
        pm->waterLevel = 1;
        point[2] = pml.origin[2] + pm->mins[2] + sample1;
        cont = pm->PointContents(point);
        if (cont & CONTENTS_MASK_LIQUID) {
            pm->waterLevel = 2;
            point[2] = pml.origin[2] + pm->mins[2] + sample2;
            cont = pm->PointContents(point);
            if (cont & CONTENTS_MASK_LIQUID)
                pm->waterLevel = 3;
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

    // This check is not needed anymore. Whether to test for a position or not
    // can now be decided by calling PM_FinalizePosition with true as its arg. 
    //if (pm->state.type == PM_SPECTATOR)
    //    return true;

    // Copy over the s.origin to end and origin for trace testing.
    VectorCopy(pm->state.origin, origin);
    VectorCopy(pm->state.origin, end);

    // Do a trace test.
    trace = pm->Trace(origin, pm->mins, pm->maxs, end);

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
    // Copy over origin and velocity.
    pm->state.origin = pml.origin;
    pm->state.velocity = pml.velocity;

    // Don't test for a valid position if not wished for.
    if (!testForValid)
        return;

    // Check to see if the position is valid.
    if (PM_TestPosition())
        return;

    // Revert back to the previous origin.
    pm->state.origin = pml.previous_origin;
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
        VectorCopy(pm->state.origin, pml.origin);
        VectorCopy(pm->state.origin, pml.previous_origin);
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

    pm->state.view_offset.z = 22;

    // Friction
    speed = VectorLength(pml.velocity);
    if (speed < 1) {
        // Reset velocity.
        pml.velocity = { 0.f, 0.f, 0.f };
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

        VectorScale(pml.velocity, newspeed, pml.velocity);
    }

    // accelerate
    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.sidemove;

    VectorNormalize(pml.forward);
    VectorNormalize(pml.right);

    for (i = 0; i < 3; i++)
        wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
    wishvel[2] += pm->cmd.upmove;

    wishdir = wishvel;
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    if (wishspeed > pmp->maxspeed) {
        VectorScale(wishvel, pmp->maxspeed / wishspeed, wishvel);
        wishspeed = pmp->maxspeed;
    }

    currentspeed = DotProduct(pml.velocity, wishdir);
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

        // Apply to velocity.
        pml.velocity.x += accelspeed * wishdir.x;
        pml.velocity.y += accelspeed * wishdir.y;
        pml.velocity.z += accelspeed * wishdir.z;
    }

#if 0
    if (doclip) {
        for (i = 0; i < 3; i++)
            end[i] = pml.origin[i] + pml.frametime * pml.velocity[i];

        trace = pm->Trace(pml.origin, pm->mins, pm->maxs, end);

        VectorCopy(trace.endpos, pml.origin);
    }
    else
#endif
    {
        // move
        VectorMA(pml.origin, pml.frametime, pml.velocity, pml.origin);
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

    if (!pm->groundEntity)
        return;

    // extra friction

    forward = VectorLength(pml.velocity);
    forward -= 20;
    if (forward <= 0) {
        // Clear  velocity.
        pml.velocity = { 0.f, 0.f, 0.f };
    }
    else {
        // Normalize and scale towards direction.
        VectorNormalize(pml.velocity);
        VectorScale(pml.velocity, forward, pml.velocity);
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
    pm->numTouchedEntities = 0;
    pm->viewAngles = { 0.f, 0.f, 0.f };
    pm->state.view_offset.z = 0;
    pm->groundEntity = NULL;
    pm->waterType = 0;
    pm->waterLevel = 0;

    // Reset the PMF_ON_STAIRS flag that we test for every move.
    pm->state.flags &= ~(PMF_ON_STAIRS);

    // clear all pmove local vars
    memset(&pml, 0, sizeof(pml));

    // Copy over the actual player state data we need into the
    // local player move data. This is where we'll be working with.
    pml.origin = pm->state.origin;
    pml.velocity = pm->state.velocity;

    // Save in case we get stuck and wish to undo this move.
    pml.previous_origin = pm->state.origin;

    // Clamp angles.
    PM_ClampAngles();

    // Special spectator movement handling.
    if (pm->state.type == PM_SPECTATOR) {
        PM_SpectatorMove();
        return;
    }

    pml.frametime = pm->cmd.msec * 0.001f;

    if (pm->state.type >= PM_DEAD) {
        pm->cmd.forwardmove = 0;
        pm->cmd.sidemove = 0;
        pm->cmd.upmove = 0;
    }

    if (pm->state.type == PM_FREEZE)
        return;     // no movement at all

    // set mins, maxs, and viewheight
    PM_CheckDuck();

    // Check whether we need to test the initial position, in case it has been modified outside of
    // pmove.cpp
    if (pm->testInitial)
        PM_TestInitialPosition();

    // set groundEntity, waterType, and waterLevel
    PM_CategorizePosition();

    // Check for whether we're dead, if so, call PM_DeadMove. It will stop
    // the player from keeping on moving forward.
    if (pm->state.type == PM_DEAD)
        PM_DeadMove();

    // Check for special movements to execute.
    PM_CheckSpecialMovements();

    // Used for "dropping" the player, ie, landing after jumps or falling off a ledge/slope.
    if (pm->state.time) {
        int     msec;

        msec = pm->cmd.msec >> 3;
        if (!msec)
            msec = 1;
        if (msec >= pm->state.time) {
            pm->state.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
            pm->state.time = 0;
        }
        else
            pm->state.time -= msec;
    }

    if (pm->state.flags & PMF_TIME_TELEPORT) {
        // teleport pause stays exactly in place
    }
    else if (pm->state.flags & PMF_TIME_WATERJUMP) {
        // waterjump has no control, but falls
        pml.velocity[2] -= pm->state.gravity * pml.frametime;
        if (pml.velocity[2] < 0) {
            // cancel as soon as we are falling down again
            pm->state.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
            pm->state.time = 0;
        }

        PM_StepSlideMove();
    }
    else {
        PM_CheckJump();

        PM_Friction();

        if (pm->waterLevel >= 2)
            PM_WaterMove();
        else {
            // Fetch angles and create specific view vectors for air move.
            vec3_t  angles = pm->viewAngles;
            if (angles[PITCH] > 180)
                angles[PITCH] = angles[PITCH] - 360;
            angles[PITCH] /= 3;

            AngleVectors(angles, &pml.forward, &pml.right, &pml.up);

            PM_AirMove();
        }
    }

    // set groundEntity, waterType, and waterLevel for final spot
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
