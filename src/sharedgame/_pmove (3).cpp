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
// The playerMoveLocals_t structure is used locally when processing the movement. Consider it
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
// Pointer to the actual (client-/npc-)entity PlayerMove(PM) structure.
//--------------------------------------------------
static pm_move_t* pm;

//--------------------------------------------------
// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
//--------------------------------------------------
static struct {
    vec3_t      origin;
    vec3_t      velocity;

    vec3_t      forward, right, up;
    vec3_t      forwardXY, rightXY;

    float       frameTime;

    vec3_t      previousOrigin;
    qboolean    ladder;

    // Ground trace results.
    struct {
        csurface_t* surface;
        cplane_t plane;
        int contents;
    } ground;
} playerMoveLocals;


// Static locals.
static pmoveParams_t* pmp;      // Pointer to the player movement parameter settings.


// Player Movement Parameters
static const float  pm_stopspeed = 100;
static const float  pm_duckspeed = 100;
static const float  pm_accelerate = 10;
static const float  pm_wateraccelerate = 10;
static const float  pm_waterspeed = 400;

//
// PM_MINS and PM_MAXS are the default bounding box, scaled by PM_SCALE
// in Pm_Init. They are referenced in a few other places e.g. to create effects
// at a certain body position on the player model.
//
const vec3_t PM_MINS = { -16.f, -16.f, -24.f };
const vec3_t PM_MAXS = {  16.f,  16.f,  32.f };

static const vec3_t PM_DEAD_MINS = { -16.f, -16.f, -24.f };
static const vec3_t PM_DEAD_MAXS = {  16.f,  16.f,  -4.f };

static const vec3_t PM_GIBLET_MINS = { -8.f, -8.f, -8.f };
static const vec3_t PM_GIBLET_MAXS = {  8.f,  8.f,  8.f };


//
//=============================================================================
//
//	UTILITY FUNCTIONS.
//
//=============================================================================
//

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
static bool PM_CheckStep(const trace_t* trace) {

    if (!trace->allsolid) {
        if (trace->ent && trace->plane.normal.z >= PM_STEP_NORMAL) {
            if (trace->ent != pm->groundEntity || trace->plane.dist != playerMoveLocals.ground.plane.dist) {
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
static void PM_StepDown(const trace_t* trace) {
    // Calculate step height.
    playerMoveLocals.origin = trace->endpos;
    pm->step = playerMoveLocals.origin.z - playerMoveLocals.previousOrigin.z;

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

//
//===============
// PM_TraceCorrectAllSolid
// 
// Adapted from Quake III, this function adjusts a trace so that if it starts inside of a wall,
// it is adjusted so that the trace begins outside of the solid it impacts.
// Returns the actual trace.
//===============
//
const trace_t PM_TraceCorrectAllSolid(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end) {
    const vec3_t offsets = { 0.f, 1.f, -1.f };

    // Jitter around
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            for (uint32_t k = 0; k < 3; k++) {
                // Calculate start.
                const vec3_t point = start + vec3_t {
                    offsets[i],
                    offsets[j],
                    offsets[k]
                };

                // Execute trace.
                const trace_t trace = pm->Trace(point, mins, maxs, end);

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
    return pm->Trace(start, mins, maxs, end);
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
        if (vec3_dot(plane, planes[i]) > 1.0f - PM_STOP_EPSILON) {
            return false;
        }
    }

    return true;
}

#define OLD_SLIDE_MOVE 0
#if OLD_SLIDE_MOVE == 0
//
//===============
// PM_StepSlideMove_
// 
// Each intersection will try to step over the obstruction instead of
// sliding along it.
//
// Modifies the pml velocity and origin if succesful.
//===============
//
#define MIN_STEP_NORMAL 0.7     // can't step up onto very steep slopes
#define MAX_CLIP_PLANES 6
static qboolean PM_StepSlideMove_(void)
{
    const int32_t numBumps = MAX_CLIP_PLANES - 2;
    vec3_t planes[MAX_CLIP_PLANES];
    int32_t bump;

    float timeRemaining = playerMoveLocals.frameTime;
    int32_t numPlanes = 0;

    // never turn against our ground plane
    if (pm->state.flags & PMF_ON_GROUND) {
        planes[numPlanes] = playerMoveLocals.ground.plane.normal;
        numPlanes++;
    }

    vec3_t primal_velocity = playerMoveLocals.velocity;

    // or our original velocity
    planes[numPlanes] = vec3_normalize(playerMoveLocals.velocity);
    numPlanes++;

    for (bump = 0; bump < numBumps; bump++) {
        if (timeRemaining < (FLT_EPSILON - 1.0f)) { // out of time
            break;
        }

        // project desired destination
        vec3_t pos = vec3_fmaf(playerMoveLocals.origin, timeRemaining, playerMoveLocals.velocity);

        // trace to it
        const trace_t trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, pos);

        // if the player is trapped in a solid, don't build up Z
        if (trace.allsolid) {
            playerMoveLocals.velocity.z = 0.0f;
            return true;
        }

        // if the trace succeeded, move some distance
        if (trace.fraction > (FLT_EPSILON - 1.0f)) {
            playerMoveLocals.origin = trace.endpos;

            // if the trace didn't hit anything, we're done
            if (trace.fraction == 1.0f) {
                break;
            }

            // update the movement time remaining
            timeRemaining -= (timeRemaining * trace.fraction);
        }

        // store a reference to the entity for firing game events
        PM_TouchEntity(trace.ent);

        // record the impacted plane, or nudge velocity out along it
        if (PM_ImpactPlane(planes, numPlanes, trace.plane.normal)) {
            planes[numPlanes] = trace.plane.normal;
            numPlanes++;
        }
        else {
            // if we've seen this plane before, nudge our velocity out along it
            playerMoveLocals.velocity += trace.plane.normal;
            continue;
        }

        // and modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            vec3_t vel;

            // if velocity doesn't impact this plane, skip it
            if (vec3_dot(playerMoveLocals.velocity, planes[i]) > (FLT_EPSILON - 1.0f)) {
                continue;
            }

            // slide along the plane
            vel = PM_ClipVelocity(playerMoveLocals.velocity, planes[i], PM_CLIP_BOUNCE);

            // see if there is a second plane that the new move enters
            for (int32_t j = 0; j < numPlanes; j++) {
                vec3_t cross;

                if (j == i) {
                    continue;
                }

                // if the clipped velocity doesn't impact this plane, skip it
                if (vec3_dot(vel, planes[j]) > (FLT_EPSILON - 1.0f)) {
                    continue;
                }

                // we are now intersecting a second plane
                vel = PM_ClipVelocity(vel, planes[j], PM_CLIP_BOUNCE);

                // but if we clip against it without being deflected back, we're okay
                if (vec3_dot(vel, planes[i]) > (FLT_EPSILON - 1.0f)) {
                    continue;
                }

                // we must now slide along the crease (cross product of the planes)
                cross = vec3_cross(planes[i], planes[j]);
                cross = vec3_normalize(cross);

                const float scale = vec3_dot(cross, playerMoveLocals.velocity);
                vel = vec3_scale(cross, scale);

                // see if there is a third plane the the new move enters
                for (int32_t k = 0; k < numPlanes; k++) {

                    if (k == i || k == j) {
                        continue;
                    }

                    if (vec3_dot(vel, planes[k]) > (FLT_EPSILON - 1.0f)) {
                        continue;
                    }

                    // stop dead at a triple plane interaction
                    playerMoveLocals.velocity = vec3_zero();
                    return true;
                }
            }

            // if we have fixed all interactions, try another move
            playerMoveLocals.velocity = vel;
            break;
        }
    }

    return bump == 0;
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
    vec3_t start_o = playerMoveLocals.origin;
    vec3_t start_v = playerMoveLocals.velocity;

    //PM_StepSlideMove_();
    // Attempt to move; if nothing blocks us, we're done
    if (PM_StepSlideMove_()) {

        // attempt to step down to remain on ground
        if ((pm->state.flags & PMF_ON_GROUND) && pm->cmd.upmove <= 0) {

            const vec3_t down = vec3_fmaf(playerMoveLocals.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
            const trace_t step_down = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, down);

            if (PM_CheckStep(&step_down)) {
                PM_StepDown(&step_down);
            }
        }

        return;
    }

    vec3_t down_o = playerMoveLocals.origin;
    vec3_t down_v = playerMoveLocals.velocity;

    vec3_t up = start_o;
    up.z += PM_STEP_HEIGHT_MAX;

    trace_t trace = PM_TraceCorrectAllSolid(up, pm->mins, pm->maxs, up);
    if (trace.allsolid)
        return;     // Can't step up

    // Try sliding above
    playerMoveLocals.origin = up;
    playerMoveLocals.velocity = start_v;

    PM_StepSlideMove_();

    // Push down the final amount
    vec3_t down = playerMoveLocals.origin;
    down.z -= PM_STEP_HEIGHT_MAX;
    trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, down);
    if (!trace.allsolid) {
        playerMoveLocals.origin = trace.endpos;
    }
    up = playerMoveLocals.origin;

    // decide which one went farther
    float down_dist = (down_o.x - start_o.x) * (down_o.x - start_o.x)
        + (down_o.y - start_o.y) * (down_o.y - start_o.y);
    float up_dist = (up.x - start_o.x) * (up.x - start_o.x)
        + (up.y - start_o.y) * (up.y - start_o.y);

    if (down_dist > up_dist || trace.plane.normal.z < PM_STEP_NORMAL) {
        playerMoveLocals.origin = down_o;
        playerMoveLocals.velocity = down_v;
        return;
    }
    //!! Special case
    // if we were walking along a plane, then we need to copy the Z over
    playerMoveLocals.velocity.z = down_v.z;
}
#elif OLD_SLIDE_MOVE == 1
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
    int         bumpCount, numBumps;
    vec3_t      dir;
    float       d;
    int         numPlanes;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      primal_velocity;
    int         i, j;
    trace_t trace;
    vec3_t      end;
    float       timeLeft;

    primal_velocity = playerMoveLocals.velocity;

    numBumps = 4;
    numPlanes = 0;

    timeLeft = playerMoveLocals.frameTime;

    for (bumpCount = 0; bumpCount < numBumps; bumpCount++) {
        for (i = 0; i < 3; i++)
            end[i] = playerMoveLocals.origin[i] + timeLeft * playerMoveLocals.velocity[i];

        trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, end);

        if (trace.allsolid) {
            // entity is trapped in another solid
            playerMoveLocals.velocity.z = 0;    // don't build up falling damage
            return;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            playerMoveLocals.origin = trace.endpos;
            numPlanes = 0;
        }

        if (trace.fraction == 1)
            break;     // moved the entire distance

        // Save entity for contact (touch) callbacks.
        PM_TouchEntity(trace.ent);

        timeLeft -= timeLeft * trace.fraction;

        // slide along this plane
        if (numPlanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            playerMoveLocals.velocity = vec3_zero();
            break;
        }

        planes[numPlanes] = trace.plane.normal;
        numPlanes++;

        //
        // modify original_velocity so it parallels all of the clip planes
        //
        for (i = 0; i < numPlanes; i++) {
            playerMoveLocals.velocity = PM_ClipVelocity(playerMoveLocals.velocity, planes[i], 1.01f);
            for (j = 0; j < numPlanes; j++)
                if (j != i) {
                    if (vec3_dot(playerMoveLocals.velocity, planes[j]) < 0)
                        break;  // not ok
                }
            if (j == numPlanes)
                break;
        }

        if (i != numPlanes) {
            // go along this plane
        }
        else {
            // go along the crease
            if (numPlanes != 2) {
                PM_Debug("clip velocity, numPlanes == %i", numPlanes);
                playerMoveLocals.velocity = vec3_zero();
                break;
            }
            dir = vec3_cross(planes[0], planes[1]);
            d = vec3_dot(dir, playerMoveLocals.velocity);
            playerMoveLocals.velocity = vec3_scale(dir, d);
        }

        //
        // if velocity is against the original velocity, stop dead
        // to avoid tiny occilations in sloping corners
        //
        if (vec3_dot(playerMoveLocals.velocity, primal_velocity) <= 0) {
            playerMoveLocals.velocity = vec3_zero();
            break;
        }
    }

    if (pm->state.time) {
        playerMoveLocals.velocity = primal_velocity;
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
    vec3_t start_o = playerMoveLocals.origin;
    vec3_t start_v = playerMoveLocals.velocity;

    PM_StepSlideMove_();

    vec3_t down_o = playerMoveLocals.origin;
    vec3_t down_v = playerMoveLocals.velocity;

    vec3_t up = start_o;
    up.z += PM_STEP_HEIGHT_MAX;

    trace_t trace = PM_TraceCorrectAllSolid(up, pm->mins, pm->maxs, up);
    if (trace.allsolid)
        return;     // Can't step up

    // Try sliding above
    playerMoveLocals.origin = up;
    playerMoveLocals.velocity = start_v;

    PM_StepSlideMove_();

    // New stepdown code. (Works somewhat if you disable the check below duh)
    //down = playerMoveLocals.origin; 
    //down.z -= PM_STEP_HEIGHT_MAX;
    //trace_t step_down = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, down);
    //if (PM_CheckStep(&step_down)) {
    //    PM_StepDown(&step_down);
    //}

    // Push down the final amount
    vec3_t down = playerMoveLocals.origin;
    down.z -= PM_STEP_HEIGHT_MAX;
    trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, down);
    if (!trace.allsolid) {
        playerMoveLocals.origin = trace.endpos;
    }
    up = playerMoveLocals.origin;

    // decide which one went farther
    float down_dist = (down_o.x - start_o.x) * (down_o.x - start_o.x)
        + (down_o.y - start_o.y) * (down_o.y - start_o.y);
    float up_dist = (up.x - start_o.x) * (up.x - start_o.x)
        + (up.y - start_o.y) * (up.y - start_o.y);

    if (down_dist > up_dist || trace.plane.normal.z < PM_STEP_NORMAL) {
        playerMoveLocals.origin = down_o;
        playerMoveLocals.velocity = down_v;
        return;
    }
    //!! Special case
    // if we were walking along a plane, then we need to copy the Z over
    playerMoveLocals.velocity.z = down_v.z;
}
#else
//
//===============
// PM_SlideMove
// 
// Calculates a new origin, velocity, and contact entities based on the
// movement command and world state. Returns true if not blocked.
//===============
//
#define MIN_STEP_NORMAL 0.7     // can't step up onto very steep slopes
#define MAX_CLIP_PLANES 5
static bool PM_SlideMove(void)
{
    trace_t     trace;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      end;

    int         i, j;
    vec3_t      dir;
    float       d;


    // Setup defaults.
    int bumpCount = 0;
    int numBumps = 4;
    int numPlanes = 0;
    float timeLeft = playerMoveLocals.frameTime;

    // Store primal velocity.
    vec3_t primal_velocity = playerMoveLocals.velocity;

    for (int bumpCount = 0; bumpCount < numBumps; bumpCount++) {
        // Break in case we have run out of time for this frame.
        if (timeLeft <= 0.0f)
            break;

        // Project desired destination
        end = vec3_fmaf(playerMoveLocals.origin, timeLeft, playerMoveLocals.velocity);

        // Trace to it.
        trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, end);

        // If the player is trapped in a solid, don't build up Z
        if (trace.allsolid) {
            // entity is trapped in another solid
            playerMoveLocals.velocity.z = 0;    // don't build up falling damage
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
            timeLeft -= (timeLeft * trace.fraction);
        }

        // OLD FRACTION CODE.
        //--------------------------------------------
        //if (trace.fraction > 0) {
        //    // actually covered some distance
        //    VectorCopy(trace.endpos, playerMoveLocals.origin);
        //    numPlanes = 0;
        //}

        //if (trace.fraction == 1)
        //    break;     // moved the entire distance
        //--------------------------------------------

        // Save entity for contact (touch) callbacks.
        PM_TouchEntity(trace.ent);

        timeLeft -= timeLeft * trace.fraction;

        // slide along this plane
        if (numPlanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            VectorCopy(vec3_origin, playerMoveLocals.velocity);
            break;
        }

        // Record the impacted plane, or nudge velocity out along it
        if (PM_ImpactPlane(planes, numPlanes, trace.plane.normal)) {
            VectorCopy(trace.plane.normal, planes[numPlanes]);
            numPlanes++;
        }
        else {
            // if we've seen this plane before, nudge our velocity out along it
            //pm->state.velocity = VectorAdd(pm->s.velocity, trace.plane.normal);
            VectorAdd(pm->state.velocity, trace.plane.normal, pm->state.velocity);
            continue;
        }

        // and modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            vec3_t vel;

            // If velocity doesn't impact this plane, skip it
            if (DotProduct(pm->state.velocity, planes[i]) >= 0.0f) {
                continue;
            }

            // Slide along the plane
            vel = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

            // See if there is a second plane that the new move enters
            for (int32_t j = 0; j < numPlanes; j++) {
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
                for (int32_t k = 0; k < numPlanes; k++) {

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

    return bumpCount == 0;
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
            trace_t step_down = PM_TraceCorrectAllSolid(pm->state.origin, down, pm->mins, pm->maxs);

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
    trace_t step_up = PM_TraceCorrectAllSolid(org0, up, pm->mins, pm->maxs);

    if (!step_up.allsolid) {

        // Step from the higher position, with the original velocity
        VectorCopy(step_up.endpos, pm->state.origin);
        VectorCopy(vel0, pm->state.velocity);

        PM_SlideMove();

        // Settle to the new ground, keeping the step if and only if it was successful
        vec3_t down;            // Vec3_FMAF
        for (int i = 0; i < 3; i++) down[i] = org0[i] + PM_STEP_HEIGHT * upV[i];

        trace_t step_down = PM_TraceCorrectAllSolid(pm->state.origin, down, pm->mins, pm->maxs);

        if (PM_CheckStep(&step_down)) {
            // Quake2 trick jump secret sauce
            if ((pm->state.flags & PMF_ON_GROUND) || vel0.z < PM_SPEED_UP) {
                PM_StepDown(&step_down);
            }
            else {
                pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
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

    vel = playerMoveLocals.velocity;

    speed = vec3_length(vel);
    if (speed < 1) {
        vel.x = 0;
        vel.y = 0;
        return;
    }

    drop = 0;

    // apply ground friction
    if ((pm->groundEntity && playerMoveLocals.ground.surface && !(playerMoveLocals.ground.surface->flags & SURF_SLICK)) || (playerMoveLocals.ladder)) {
        friction = pmp->friction;
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * friction * playerMoveLocals.frameTime;
    }

    // apply water friction
    if (pm->waterLevel && !playerMoveLocals.ladder)
        drop += speed * pmp->waterfriction * pm->waterLevel * playerMoveLocals.frameTime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }
    newspeed /= speed;

    vel.x = vel.x * newspeed;
    vel.y = vel.y * newspeed;
    vel.z = vel.z * newspeed;

    // Apply new velocity to playerMoveLocals.
    playerMoveLocals.velocity = vel;
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

    currentspeed = DotProduct(playerMoveLocals.velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0)
        return;
    accelspeed = accel * playerMoveLocals.frameTime * wishspeed;
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    for (i = 0; i < 3; i++)
        playerMoveLocals.velocity[i] += accelspeed * wishdir[i];
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
    currentspeed = DotProduct(playerMoveLocals.velocity, wishdir);
    addspeed = wishspd - currentspeed;
    if (addspeed <= 0)
        return;
    accelspeed = accel * wishspeed * playerMoveLocals.frameTime;
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    for (i = 0; i < 3; i++)
        playerMoveLocals.velocity[i] += accelspeed * wishdir[i];
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
    if (playerMoveLocals.ladder && fabs(playerMoveLocals.velocity.z) <= 200) {
        if ((pm->viewAngles[vec3_t::Pitch] <= -15) && (pm->cmd.forwardmove > 0))
            wishvel.z = 200;
        else if ((pm->viewAngles[vec3_t::Pitch] >= 15) && (pm->cmd.forwardmove > 0))
            wishvel.z = -200;
        else if (pm->cmd.upmove > 0)
            wishvel.z = 200;
        else if (pm->cmd.upmove < 0)
            wishvel.z = -200;
        else
            wishvel.z = 0;

        // Limit horizontal speed when on a ladder
        if (wishvel.x < -25)
            wishvel.x = -25;
        else if (wishvel.x > 25)
            wishvel.x = 25;

        if (wishvel.y < -25)
            wishvel.y = -25;
        else if (wishvel.y > 25)
            wishvel.y = 25;
    }


    // Water Current Velocities.
    if (pm->waterType & CONTENTS_MASK_CURRENT) {
        v = vec3_zero();

        if (pm->waterType & CONTENTS_CURRENT_0)
            v.x += 1;
        if (pm->waterType & CONTENTS_CURRENT_90)
            v.y += 1;
        if (pm->waterType & CONTENTS_CURRENT_180)
            v.x -= 1;
        if (pm->waterType & CONTENTS_CURRENT_270)
            v.y -= 1;
        if (pm->waterType & CONTENTS_CURRENT_UP)
            v.z += 1;
        if (pm->waterType & CONTENTS_CURRENT_DOWN)
            v.z -= 1;

        s = pm_waterspeed;
        if ((pm->waterLevel == 1) && (pm->groundEntity))
            s /= 2;

        wishvel = vec3_fmaf(wishvel, s, v);
    }

    // Conveyor Belt Velocities.
    if (pm->groundEntity) {
        VectorClear(v);

        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_0)
            v.x += 1;
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_90)
            v.y += 1;
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_180)
            v.x -= 1;
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_270)
            v.y -= 1;
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_UP)
            v.z += 1;
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_DOWN)
            v.z -= 1;

        wishvel = vec3_fmaf(wishvel, 100 /* pm->groundEntity->speed */, v);
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
        wishvel[i] = playerMoveLocals.forward[i] * pm->cmd.forwardmove + playerMoveLocals.right[i] * pm->cmd.sidemove;

    if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
        wishvel.z -= 60;       // drift towards bottom
    else
        wishvel.z += pm->cmd.upmove;

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
    playerMoveLocals.forward.z = 0;
    playerMoveLocals.right.z = 0;
    VectorNormalize(playerMoveLocals.forward);
    VectorNormalize(playerMoveLocals.right);
#endif

    for (i = 0; i < 2; i++)
        wishvel[i] = playerMoveLocals.forward[i] * fmove + playerMoveLocals.right[i] * smove;
    wishvel.z = 0;

    wishdir = wishvel = PM_AddCurrents(wishvel);
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    maxspeed = (pm->state.flags & PMF_DUCKED) ? pm_duckspeed : pmp->maxspeed;

    if (wishspeed > maxspeed) {
        wishvel = vec3_scale(wishvel, maxspeed / wishspeed);
        wishspeed = maxspeed;
    }

    if (playerMoveLocals.ladder) {
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);
        if (!wishvel.z) {
            if (playerMoveLocals.velocity.z > 0) {
                playerMoveLocals.velocity.z -= pm->state.gravity * playerMoveLocals.frameTime;
                if (playerMoveLocals.velocity.z < 0)
                    playerMoveLocals.velocity.z = 0;
            }
            else {
                playerMoveLocals.velocity.z += pm->state.gravity * playerMoveLocals.frameTime;
                if (playerMoveLocals.velocity.z > 0)
                    playerMoveLocals.velocity.z = 0;
            }
        }
        PM_StepSlideMove();
    }
    else if (pm->groundEntity) {
        // walking on ground
        playerMoveLocals.velocity.z = 0; //!!! this is before the accel
        PM_Accelerate(wishdir, wishspeed, pm_accelerate);


        // PGM  -- fix for negative trigger_gravity fields
        //      playerMoveLocals.velocity.z = 0;
        if (pm->state.gravity > 0)
            playerMoveLocals.velocity.z = 0;
        else
            playerMoveLocals.velocity.z -= pm->state.gravity * playerMoveLocals.frameTime;
        // PGM

        if (!playerMoveLocals.velocity.x && !playerMoveLocals.velocity.y)
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
        playerMoveLocals.velocity.z -= pm->state.gravity * playerMoveLocals.frameTime;
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

        if (playerMoveLocals.velocity.z <= -300)
            return;

        // FIXME: makes velocity dependent on client FPS,
        // even causes prediction misses
        if (pm->waterType == CONTENTS_WATER)
            playerMoveLocals.velocity.z = 100;
        else if (pm->waterType == CONTENTS_SLIME)
            playerMoveLocals.velocity.z = 80;
        else
            playerMoveLocals.velocity.z = 50;
        return;
    }

    if (pm->groundEntity == NULL)
        return;     // in air, so no effect

    pm->state.flags |= PMF_JUMP_HELD;

    pm->groundEntity = NULL;
    pm->state.flags &= ~PMF_ON_GROUND;
    playerMoveLocals.velocity.z += 270;
    if (playerMoveLocals.velocity.z < 270)
        playerMoveLocals.velocity.z = 270;
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
    if (pm->state.time)
        return;

    playerMoveLocals.ladder = false;

    // Check for ladder
    vec3_t flatforward = {
        playerMoveLocals.forward.x,
        playerMoveLocals.forward.y,
        0.f
    };
    flatforward = vec3_normalize(flatforward);

    vec3_t spot = vec3_fmaf(playerMoveLocals.origin, 1, flatforward);
    trace_t trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, spot);
    if ((trace.fraction < 1) && (trace.contents & CONTENTS_LADDER))
        playerMoveLocals.ladder = true;

    // Check for water jump
    if (pm->waterLevel != 2)
        return;

    spot = vec3_fmaf(playerMoveLocals.origin, 30, flatforward);
    spot.z += 4;

    int cont = pm->PointContents(spot);
    if (!(cont & CONTENTS_SOLID))
        return;

    spot.z += 16;
    cont = pm->PointContents(spot);
    if (cont)
        return;

    // Jump out of water
    playerMoveLocals.velocity = vec3_scale(flatforward, 50);
    playerMoveLocals.velocity.z = 350;

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
    // Any state after dead, can be checked for here.
    if (pm->state.type >= PM_DEAD) {
        if (pm->state.type == PM_GIB) {
            pm->state.view_offset.z = 0.0f;
        }
        else {
            pm->state.view_offset.z = -16.0f;
        }
    // Other states go here :)
    } else {

        const qboolean is_ducking = pm->state.flags & PMF_DUCKED;
        const qboolean wants_ducking = (pm->cmd.upmove < 0) && !(playerMoveLocals.ladder);

        if (!is_ducking && wants_ducking) {
            pm->state.flags |= PMF_DUCKED;
        }
        else if (is_ducking && !wants_ducking) {
            const trace_t trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, playerMoveLocals.origin);

            if (!trace.allsolid && !trace.startsolid) {
                pm->state.flags &= ~PMF_DUCKED;
            }
        }

        const float height = pm->maxs.z - pm->mins.z;

        // For either case, ducked, or not ducked. We first change the target 
        // view height.
        // 
        // This doesn't reflect the actual bbox, since in real life your eyes 
        // aren't at the top of your skull either.
        // 
        // Considering games == faking effects to get a nice real feel... Here we go.
        if (pm->state.flags & PMF_DUCKED) {
            // A nice view height value.
            const float targetViewHeight = pm->mins.z + height * 0.45f;

            // LERP it.
            if (pm->state.view_offset.z > targetViewHeight) { // go down
                pm->state.view_offset.z -= playerMoveLocals.frameTime * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.view_offset.z < targetViewHeight) {
                pm->state.view_offset.z = targetViewHeight;
            }

            // Change the actual bounding box to reflect ducking
            pm->maxs.z = pm->maxs.z + pm->mins.z * 0.35f;
        }
        else {
            // A nice view height value.
            const float targetViewHeight = pm->mins.z + (height - 6) * 0.9f;

            // LERP it.
            if (pm->state.view_offset.z < targetViewHeight) { // go up
                pm->state.view_offset.z += playerMoveLocals.frameTime * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.view_offset.z > targetViewHeight) {
                pm->state.view_offset.z = targetViewHeight;
            }

            // No need to change the bounding box, it has already been initialized at the start of the frame.
        }
    }

    pm->state.view_offset = pm->state.view_offset;
    //trace_t trace;

    //pm->mins.x = -16;
    //pm->mins.y = -16;

    //pm->maxs.x = 16;
    //pm->maxs.y = 16;

    //if (pm->state.type == PM_GIB) {
    //    pm->mins.z = 0;
    //    pm->maxs.z = 16;
    //    pm->state.view_offset.z = 8;
    //    return;
    //}

    //pm->mins.z = -24;

    //if (pm->state.type == PM_DEAD) {
    //    pm->state.flags |= PMF_DUCKED;
    //}
    //else if (pm->cmd.upmove < 0 && (pm->state.flags & PMF_ON_GROUND)) {
    //    // duck
    //    pm->state.flags |= PMF_DUCKED;
    //}
    //else {
    //    // stand up if possible
    //    if (pm->state.flags & PMF_DUCKED) {
    //        // try to stand up
    //        pm->maxs.z = 32;
    //        trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, playerMoveLocals.origin);
    //        if (!trace.allsolid)
    //            pm->state.flags &= ~PMF_DUCKED;
    //    }
    //}

    //if (pm->state.flags & PMF_DUCKED) {
    //    pm->maxs.z = 4;
    //    pm->state.view_offset.z = -2;
    //}
    //else {
    //    pm->maxs.z = 32;
    //    pm->state.view_offset.z = 22;
    //}
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
    // if the player hull point one unit down is solid, the player
    // is on ground

    // See if standing on something solid
    vec3_t point = {
        playerMoveLocals.origin.x,
        playerMoveLocals.origin.y,
        playerMoveLocals.origin.z - 0.25f
    };

    if (playerMoveLocals.velocity.z > 180) { //!!ZOID changed from 100 to 180 (ramp accel)
        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntity = NULL;
    }
    else {
        // Execute trace.
        trace_t trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, point);

        // Set results in playerMoveLocals.
        playerMoveLocals.ground.plane = trace.plane;
        playerMoveLocals.ground.surface = trace.surface;
        playerMoveLocals.ground.contents = trace.contents;

        // No ent, or place normal is under PM_STEP_NORMAL.
        if (!trace.ent || (trace.plane.normal.z < PM_STEP_NORMAL && !trace.startsolid)) {
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
                if (playerMoveLocals.velocity.z < -200 && !pmp->strafehack) {
                    pm->state.flags |= PMF_TIME_LAND;
                    // don't allow another jump for a little while
                    if (playerMoveLocals.velocity.z < -400)
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
    point.z = playerMoveLocals.origin.z + pm->mins.z + 1;
    int cont = pm->PointContents(point);

    int sample2 = pm->state.view_offset.z - pm->mins.z;
    int sample1 = sample2 / 2;

    pm->waterLevel = 0;
    pm->waterType = 0;

    if (cont & CONTENTS_MASK_LIQUID) {
        pm->waterType = cont;
        pm->waterLevel = 1;

        point.z = playerMoveLocals.origin.z + pm->mins.z + sample1;
        cont = pm->PointContents(point);

        if (cont & CONTENTS_MASK_LIQUID) {
            pm->waterLevel = 2;
            point.z = playerMoveLocals.origin.z + pm->mins.z + sample2;
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
    // This check is not needed anymore. Whether to test for a position or not
    // can now be decided by calling PM_FinalizePosition with true as its arg. 
    //if (pm->state.type == PM_SPECTATOR)
    //    return true;

    // Copy over the s.origin to end and origin for trace testing.
    vec3_t origin = pm->state.origin;
    vec3_t end = pm->state.origin;

    // Do a trace test.
    trace_t trace = PM_TraceCorrectAllSolid(origin, pm->mins, pm->maxs, end);

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
    pm->state.origin = playerMoveLocals.origin;
    pm->state.velocity = playerMoveLocals.velocity;

    // Don't test for a valid position if not wished for.
    if (!testForValid)
        return;

    // Check to see if the position is valid.
    if (PM_TestPosition())
        return;

    // Revert back to the previous origin.
    pm->state.origin = playerMoveLocals.previousOrigin;
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
        playerMoveLocals.origin = pm->state.origin;
        playerMoveLocals.previousOrigin = pm->state.origin;
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
    speed = vec3_length(playerMoveLocals.velocity);
    if (speed < 1) {
        // Reset velocity.
        playerMoveLocals.velocity = vec3_zero();
    }
    else {
        drop = 0;

        friction = pmp->flyfriction;
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * friction * playerMoveLocals.frameTime;

        // scale the velocity
        newspeed = speed - drop;
        if (newspeed < 0)
            newspeed = 0;
        newspeed /= speed;

        playerMoveLocals.velocity = vec3_scale(playerMoveLocals.velocity, newspeed);
    }

    // accelerate
    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.sidemove;

    playerMoveLocals.forward = vec3_normalize(playerMoveLocals.forward);
    playerMoveLocals.right = vec3_normalize(playerMoveLocals.right);

    for (i = 0; i < 3; i++)
        wishvel[i] = playerMoveLocals.forward[i] * fmove + playerMoveLocals.right[i] * smove;
    wishvel.z += pm->cmd.upmove;

    wishdir = wishvel;
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    if (wishspeed > pmp->maxspeed) {
        wishvel = vec3_scale(wishvel, pmp->maxspeed / wishspeed);
        wishspeed = pmp->maxspeed;
    }

    currentspeed = DotProduct(playerMoveLocals.velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0) {
        if (!pmp->flyhack) {
            return; // original buggy behaviour
        }
    }
    else {
        // Calculate new acceleration speed.
        accelspeed = pm_accelerate * playerMoveLocals.frameTime * wishspeed;
        if (accelspeed > addspeed)
            accelspeed = addspeed;

        // Apply to velocity.
        playerMoveLocals.velocity.x += accelspeed * wishdir.x;
        playerMoveLocals.velocity.y += accelspeed * wishdir.y;
        playerMoveLocals.velocity.z += accelspeed * wishdir.z;
    }

#if 0
    if (doclip) {
        //for (i = 0; i < 3; i++)
        //    end[i] = playerMoveLocals.origin[i] + playerMoveLocals.frameTime * playerMoveLocals.velocity[i];
        end = vec3_fmaf(playerMoveLocals.origin, playerMoveLocals.frameTime, playerMoveLocals.velocity);
        trace = PM_TraceCorrectAllSolid(playerMoveLocals.origin, pm->mins, pm->maxs, end);
        playerMoveLocals.origin = trace.endpos;
    }
    else
#endif
    {
        // move
        playerMoveLocals.origin = vec3_fmaf(playerMoveLocals.origin, playerMoveLocals.frameTime, playerMoveLocals.velocity);
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

    // Return if not on the ground.
    if (!pm->groundEntity)
        return;

    // extra friction
    forward = vec3_length(playerMoveLocals.velocity);
    forward -= 20;
    if (forward <= 0) {
        // Clear  velocity.
        playerMoveLocals.velocity = vec3_zero();
    }
    else {
        // Normalize and scale towards direction.
        playerMoveLocals.velocity = vec3_normalize(playerMoveLocals.velocity);
        playerMoveLocals.velocity = vec3_scale(playerMoveLocals.velocity, forward);
    }
}

//
//===============
// PM_SpectatorMove
// 
// Handles special spectator movement.
//===============
//
static void PM_SpectatorMove(void) {
    //// Setup a different frameTime for movement.
    //playerMoveLocals.frameTime = pmp->speedmult * pm->cmd.msec * 0.001f;

    //// Execute typical fly movement.
    //PM_FlyMove();

    // Finalize the position. Do no position testing, a spectator is free to
    // roam where he pleases.
    PM_FinalizePosition(false);
}

//
//===============
// PM_FreezeMove
// 
// Freeze Player movement/
//===============
//
static void PM_FreezeMove(void) {
    PM_Debug("%s", vec3_to_str(pm->state.origin));
    // Other than that call.... It's empty.
    // Or, what else did you expect to find here?
    //
    // The miracles of the universe? Try the vkpt folder for that.
}

//
//===============
// PM_Init
// 
// Initializes the current set PMove pointer for another frame iteration.
//===============
//
static void PM_Init(pm_move_t* pmove) {
    // Store pmove ptr.
    pm = pmove;

    // Set the default bounding box
    if (pm->state.type >= PM_DEAD) {
        if (pm->state.type == PM_GIB) {
            pm->mins = PM_GIBLET_MINS;
            pm->maxs = PM_GIBLET_MAXS;
        }
        else {
            pm->mins = vec3_scale(PM_DEAD_MINS, PM_SCALE);
            pm->maxs = vec3_scale(PM_DEAD_MAXS, PM_SCALE);
        }
    }
    else {
        pm->mins = vec3_scale(PM_MINS, PM_SCALE);
        pm->maxs = vec3_scale(PM_MAXS, PM_SCALE);
    }

    // Clear out previous PM iteration results
    pm->viewAngles = vec3_zero();

    pm->numTouchedEntities = 0;
    pm->groundEntity = NULL;

    pm->waterType = 0;
    pm->waterLevel = 0;

    // Reset the flags, and step, values. These are set on a per frame basis.
    pm->state.flags &= ~(PMF_ON_GROUND | PMF_ON_STAIRS | PMF_DUCKED);
    pm->step = 0.0f;

    // Jump "held" also, in case its key was released.
    if (pm->cmd.upmove < 1) {
        pm->state.flags &= ~PMF_JUMP_HELD;
    }

    // Decrement the movement timer, used for "dropping" the player, landing after jumps, 
    // or falling off a ledge/slope, by the duration of the command.
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
}

//
//===============
// PM_ClampAngles
// 
// Clamp angles with deltas. Ensure they pitch doesn't exceed 90 or 270
//===============
//
static void PM_ClampAngles(void) {
    // TODO: Store viewangles in state, for prediction usages.
    // pm->state.viewAngles = pm->cmd.angles; // 
    // copy the command angles into the outgoing state
    for (int i = 0; i < 3; i++) {
        float temp = pm->cmd.angles[i] + pm->state.delta_angles[i];
        pm->viewAngles[i] = SHORT2ANGLE(temp);
    }

    // clamp pitch to prevent the player from looking up or down more than 90º
    if (pm->viewAngles.x > 90.0f && pm->viewAngles.x < 270.0f) {
        pm->viewAngles.x = 90.0f;
    }
    else if (pm->viewAngles.x <= 360.0f && pm->viewAngles.x >= 270.0f) {
        pm->viewAngles.x -= 360.0f;
    }
}

//
//===============
// PM_InitLocal
// 
// Resets the current local pmove values, and stores the required data for
// reverting an invalid pmove command.
//===============
//
static void PM_InitLocal() {
    // clear all PM local vars
    playerMoveLocals = {};

    // Copy over the actual player state data we need into the
    // local player move data. This is where we'll be working with.
    playerMoveLocals.origin = pm->state.origin;
    playerMoveLocals.velocity = pm->state.velocity;

    // Save in case we get stuck and wish to undo this move.
    playerMoveLocals.previousOrigin = pm->state.origin;

    // Calculate the directional vectors for this move, and in the XY Plane.
    vec3_vectors(pm->viewAngles, &playerMoveLocals.forward, &playerMoveLocals.right, &playerMoveLocals.up);
    vec3_vectors(vec3_t{ 0.f, pm->viewAngles.y, 0.f }, &playerMoveLocals.forwardXY, &playerMoveLocals.rightXY, NULL);
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
    // TODO: When PMOVE is finished, remove this useless pmoveParams thing.
    pmp = params;

    // Initialize the PMove.
    PM_Init(pmove);
    
    // Ensure angles are clamped.
    PM_ClampAngles();

    // Initialize the locals.
    PM_InitLocal();

    // Increase frame time based on seconds.
    playerMoveLocals.frameTime = pm->cmd.msec * 0.001f;

    // Special PM_FREEZE(Player Movement is frozen, idle, not happening!) treatment.
    if (pm->state.type == PM_FREEZE) {
        PM_FreezeMove();
        return;
    }

    // Special PM_SPECTATOR(Spectator Movement, viewing a match, etc) treatment.
    if (pm->state.type == PM_SPECTATOR) {
        PM_SpectatorMove();
        return;
    }
    
    // Erase input direction values in case we are dead, or something alike.
    if (pm->state.type >= PM_DEAD) {
        pm->cmd.forwardmove = 0;
        pm->cmd.sidemove = 0;
        pm->cmd.upmove = 0;
    }

    // set mins, maxs, and viewheight
    PM_CheckDuck();

    // Check whether we need to test the initial position, in case it has been modified outside of
    // pmove.cpp
    if (pm->testInitial)
        PM_TestInitialPosition();

    // Set groundEntity, waterType, and waterLevel
    PM_CategorizePosition();

    // Check for whether we're dead, if so, call PM_DeadMove. It will stop
    // the player from keeping on moving forward.
    if (pm->state.type == PM_DEAD)
        PM_DeadMove();

    // Check for special movements to execute.
    PM_CheckSpecialMovements();

    // Teleport pause stays exactly in place
    if (pm->state.flags & PMF_TIME_TELEPORT) {

    }
    // waterjump has no control, but falls
    else if (pm->state.flags & PMF_TIME_WATERJUMP) {
        // Apply gravity.
        playerMoveLocals.velocity.z -= pm->state.gravity * playerMoveLocals.frameTime;

        // Cancel as soon as we are falling down again
        if (playerMoveLocals.velocity.z < 0) {
            pm->state.flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
            pm->state.time = 0;
        }

        // Slide move with our new velocity.
        PM_StepSlideMove();
    }
    else {
        // Check for jump.
        PM_CheckJump();

        // Apply friction.
        PM_Friction();

        // Watermove.
        if (pm->waterLevel >= 2)
            PM_WaterMove();
        else {
            // Fetch angles and create specific view vectors for air move.
            vec3_t angles = pm->viewAngles;
            if (angles[vec3_t::Pitch] > 180)
                angles[vec3_t::Pitch] = angles[vec3_t::Pitch] - 360;
            angles[vec3_t::Pitch] /= 3;

            // Calculate view vectors to move into.
            vec3_vectors(angles, &playerMoveLocals.forward, &playerMoveLocals.right, &playerMoveLocals.up);

            // Airmove.
            PM_AirMove();
        }
    }

    // Set groundEntity, waterType, and waterLevel for final spot
    PM_CategorizePosition();

    // Finalize position, do testing with the pml results, and apply if valid.
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

    pmp->qwmode = true;
    pmp->watermult = PM_GRAVITY_WATER;
    pmp->maxspeed = PM_SPEED_RUN;
    //pmp->upspeed = (sv_qwmod->integer > 1) ? 310 : 350;
    pmp->friction = PM_FRICT_GROUND;
    pmp->waterfriction = PM_FRICT_WATER;
    pmp->airaccelerate = true;
    pmp->flyfriction = 9;
    //pmp->speedmult = 1;
    //pmp->watermult = 0.5f;
    //pmp->maxspeed = 300;
    //pmp->friction = 6;
    //pmp->waterfriction = 1;
    //pmp->flyfriction = 9;
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
    pmp->watermult = PM_GRAVITY_WATER;
    pmp->maxspeed = PM_SPEED_RUN;
    //pmp->upspeed = (sv_qwmod->integer > 1) ? 310 : 350;
    pmp->friction = PM_FRICT_GROUND;
    pmp->waterfriction = PM_FRICT_WATER;
    pmp->airaccelerate = true;
    //pmp->qwmode = true;
    //pmp->watermult = 0.7f;
    //pmp->maxspeed = 320;
    ////pmp->upspeed = (sv_qwmod->integer > 1) ? 310 : 350;
    //pmp->friction = 4;
    //pmp->waterfriction = 4;
    //pmp->airaccelerate = true;
}
