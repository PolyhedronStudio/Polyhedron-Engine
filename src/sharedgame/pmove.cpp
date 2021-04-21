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
// a temporary structure. The prev origin and velocity are copied in there for storage.
// 
// At the start of pmove we initialize certain pm-> variables, and the playerMoveLocals.
// 
// After that, we check for whether we are on a ladder, ducking, on-ground, in-water,
// and/or in air. We execute the right movement according to that.
//
#include "shared/shared.h"
#include "sharedgame/sharedgame.h"
#include "sharedgame/pmove.h"
#include "client/client.h"

//--------------------------------------------------
// Player Movement configuration.
//
// Most settings can be easily tweaked here to fine tune movement to custom
// desires.
//--------------------------------------------------
//-----------------
// Acceleration Constants.
//-----------------
constexpr float PM_ACCEL_AIR = 2.125f;
constexpr float PM_ACCEL_AIR_MOD_DUCKED = 0.125f;
constexpr float PM_ACCEL_GROUND = 10.f;
constexpr float PM_ACCEL_GROUND_SLICK = 4.375f;
constexpr float PM_ACCEL_LADDER = 16.f;
constexpr float PM_ACCEL_SPECTATOR = 2.5f;
constexpr float PM_ACCEL_WATER = 2.8f;

//-----------------
// Bounce constant when clipping against solids.
//-----------------
constexpr float PM_CLIP_BOUNCE = 1.01f;

//-----------------
// Friction constants.
//-----------------
constexpr float PM_FRICT_AIR = 0.075f; // N&C: Tweaked - old value: 0.1
constexpr float PM_FRICT_GROUND = 6.f;
constexpr float PM_FRICT_GROUND_SLICK = 2.f;
constexpr float PM_FRICT_LADDER = 5.f;
constexpr float PM_FRICT_SPECTATOR = 2.5f;
constexpr float PM_FRICT_WATER = 2.f;

//-----------------
// Water gravity constant.
//-----------------
constexpr float PM_GRAVITY_WATER = 0.33f;

//-----------------
// Distances traced when seeking ground.
//-----------------
constexpr float PM_GROUND_DIST = .25f;
constexpr float PM_GROUND_DIST_TRICK = 16.f;

//-----------------
// Speed constants; intended velocities are clipped to these.
//-----------------
constexpr float PM_SPEED_AIR = 285.f; // N&C: Tweaked - old value: 350
constexpr float PM_SPEED_CURRENT = 100.f;
constexpr float PM_SPEED_DUCK_STAND = 200.f;
constexpr float PM_SPEED_DUCKED = 140.f;
constexpr float PM_SPEED_FALL = -700.f;
constexpr float PM_SPEED_FALL_FAR = -900.f;
constexpr float PM_SPEED_JUMP = 270.f;
constexpr float PM_SPEED_LADDER = 125.f;
constexpr float PM_SPEED_LAND = -280.f;
constexpr float PM_SPEED_RUN = 300.f; // This is the wished for running speed. Changing it, also impacts walking speed.
constexpr float PM_SPEED_SPECTATOR = 500.f;
constexpr float PM_SPEED_STOP = 100.f;
constexpr float PM_SPEED_UP = 0.1f;
constexpr float PM_SPEED_TRICK_JUMP = 0.f;
constexpr float PM_SPEED_WATER = 118.f;
constexpr float PM_SPEED_WATER_JUMP = 420.f;
constexpr float PM_SPEED_WATER_SINK = -16.f;

//-----------------
// General.
//-----------------
constexpr float PM_SPEED_MOD_WALK = 0.48f;// The walk modifier slows all user-controlled speeds.
constexpr float PM_SPEED_JUMP_MOD_WATER = 0.66;// Water reduces jumping ability.
constexpr float PM_STOP_EPSILON = 0.1f; // Velocity is cleared when less than this.
constexpr float PM_NUDGE_DIST = 1.f;  // Invalid player positions are nudged to find a valid position.
constexpr float PM_SNAP_DISTANCE = PM_GROUND_DIST; // Valid player positions are snapped a small distance away from planes.

//-----------------
// Step Climbing.
//-----------------
//constexpr float PM_STEP_HEIGHT = OMG!? - Moved to pmove.h, because clg_predict wants to know about it also :)
constexpr float PM_STEP_HEIGHT_MIN = 4.f;  // The smallest step that will be interpolated by the client.
constexpr float PM_STEP_NORMAL = 0.7f; // The minimum Z plane normal component required for standing.

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
    vec3_t      previousVelocity;
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

//
// PM_MINS and PM_MAXS are the default bounding box, scaled by PM_SCALE
// in Pm_Init. They are referenced in a few other places e.g. to create effects
// at a certain body position on the player model.
//
const vec3_t PM_MINS = { -16.f, -16.f, -24.f };
const vec3_t PM_MAXS = { 16.f,  16.f,  32.f };

static const vec3_t PM_DEAD_MINS = { -16.f, -16.f, -24.f };
static const vec3_t PM_DEAD_MAXS = { 16.f,  16.f,  -4.f };

static const vec3_t PM_GIBLET_MINS = { -8.f, -8.f, -8.f };
static const vec3_t PM_GIBLET_MAXS = { 8.f,  8.f,  8.f };


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
#define DEBUG_CLIENT_PMOVE 0
#if DEBUG_CLIENT_PMOVE == 1

// Client debug output.
static void CLGPM_Debug(const char* func, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);
    std::string str = "[CLG PM_Debug:";
    str += func;
    str += "] ";
    str += buffer;
    str += "\n";
    Com_DPrintf(str.c_str());

    va_end(args);
}
#define PM_Debug(...) CLGPM_Debug(__func__, __VA_ARGS__);
#else
static void CLGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    va_end(args);
    CLGPM_Debug
#define PM_Debug(...) CLGPM_Debug(__func__, __VA_ARGS__);
        //#define PM_Debug () void(0)
#endif // PMOVE_DEBUG
#else
#define DEBUG_SERVER_PMOVE 0
#if DEBUG_SERVER_PMOVE == 1

static void SVGPM_Debug(const char* func, const char* fmt, ...) {
    if (!developer->value)
        return;

    char buffer[MAX_STRING_CHARS];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);
    std::string str = "[SVGPM_Debug:{";
    str += func;
    str += "}] ";
    str += buffer;
    str += "\n";
    Com_DPrintf(str.c_str());

    va_end(args);
}
#define PM_Debug(...) SVGPM_Debug(__func__, __VA_ARGS__);
#else
// Server debug output.
static void SVGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    va_end(args);
}
#define PM_Debug(...) SVGPM_Debug(__func__, __VA_ARGS__);
//#define PM_Debug (...) void(0)
#endif // PMOVE_DEBUG
#endif // CGAME_INCLUDE

//
//===============
// PM_ClipVelocity
//
// Walking up a step should kill some velocity.
//  
// Slide off of the impacting object
// returns the Blocked flags(1 = floor, 2 = step / wall)
//===============
//
#define STOP_EPSILON    0.1
static vec3_t PM_ClipVelocity(vec3_t & in, vec3_t & normal, float overbounce)
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
static void PM_TouchEntity(struct entity_s* ent) {
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
static bool PM_CheckStep(const trace_t * trace) {

    if (!trace->allSolid) {
        if (trace->ent && trace->plane.normal.z >= PM_STEP_NORMAL) {
            if (trace->ent != pm->groundEntityPtr || trace->plane.dist != playerMoveLocals.ground.plane.dist) {
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
static void PM_StepDown(const trace_t * trace) {
    // Calculate step height.
    pm->state.origin = trace->endPosition;
    pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;

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
const trace_t PM_TraceCorrectAllSolid(const vec3_t & start, const vec3_t & mins, const vec3_t & maxs, const vec3_t & end) {
    // Disabled, for this we have no need. It seems to work fine at this moment without it.
    // Not getting stuck into rotating objects or what have we....
    //
    // If we do end up running into trouble, we may first want to look at the old
    // M_CheckGround function instead. (Seems related to a -0.25f).
    // 
    // And otherwise, we got this solution below, which... is seemingly slow in certain cases...
    return pm->Trace(start, mins, maxs, end);
#if 0
    const vec3_t offsets = { 0.f, 1.f, -1.f };

    // Jitter around
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            for (uint32_t k = 0; k < 3; k++) {
                // Calculate start.
                const vec3_t point = start + vec3_t{
                    offsets[i],
                    offsets[j],
                    offsets[k]
                };

                // Execute trace.
                const trace_t trace = pm->Trace(point, mins, maxs, end);

                if (!trace.allSolid) {

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
#endif
}

//
//===============
// PM_ImpactPlane
// 
// Return True if `plane` is unique to `planes` and should be impacted, 
// return false otherwise.
//===============
//
static bool PM_ImpactPlane(vec3_t * planes, int32_t num_planes, const vec3_t & plane) {

    for (int32_t i = 0; i < num_planes; i++) {
        if (vec3_dot(plane, planes[i]) > 1.0f - PM_STOP_EPSILON) {
            return false;
        }
    }

    return true;
}

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

    vec3_t primal_velocity = pm->state.velocity;

    // or our original velocity
    planes[numPlanes] = vec3_normalize(pm->state.velocity);
    numPlanes++;

    for (bump = 0; bump < numBumps; bump++) {
        if (timeRemaining < (FLT_EPSILON - 1.0f)) { // out of time
            break;
        }

        // project desired destination
        vec3_t pos = vec3_fmaf(pm->state.origin, timeRemaining, pm->state.velocity);

        // trace to it
        const trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

        // if the player is trapped in a solid, don't build up Z
        if (trace.allSolid) {
            pm->state.velocity.z = 0.0f;
            return true;
        }

        // if the trace succeeded, move some distance
        if (trace.fraction > (FLT_EPSILON - 1.0f)) {
            pm->state.origin = trace.endPosition;

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
            pm->state.velocity += trace.plane.normal;
            continue;
        }

        // and modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            vec3_t vel;

            // if velocity doesn't impact this plane, skip it
            if (vec3_dot(pm->state.velocity, planes[i]) > (FLT_EPSILON - 1.0f)) {
                continue;
            }

            // slide along the plane
            vel = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

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

                const float scale = vec3_dot(cross, pm->state.velocity);
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
                    pm->state.velocity = vec3_zero();
                    return true;
                }
            }

            // if we have fixed all interactions, try another move
            pm->state.velocity = vel;
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
    vec3_t start_o = pm->state.origin;
    vec3_t start_v = pm->state.velocity;

    // Attempt to move; if nothing blocks us, we're done
    if (PM_StepSlideMove_()) {
        // attempt to step down to remain on ground
        if ((pm->state.flags & PMF_ON_GROUND) && pm->cmd.upmove <= 0) {

            const vec3_t down = vec3_fmaf(pm->state.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
            const trace_t step_down = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);

            if (PM_CheckStep(&step_down)) {
                PM_StepDown(&step_down);
            }
        }

        return;
    }

    vec3_t down_o = pm->state.origin;
    vec3_t down_v = pm->state.velocity;

    vec3_t up = start_o;
    up.z += PM_STEP_HEIGHT;

    trace_t trace = PM_TraceCorrectAllSolid(up, pm->mins, pm->maxs, up);
    if (trace.allSolid)
        return;     // Can't step up

    // Try sliding above
    pm->state.origin = up;
    pm->state.velocity = start_v;

    PM_StepSlideMove_();

    // Push down the final amount
    vec3_t down = pm->state.origin;
    down.z -= PM_STEP_HEIGHT;
    trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);
    if (!trace.allSolid) {
        pm->state.origin = trace.endPosition;
    }
    up = pm->state.origin;

    // decide which one went farther
    float down_dist = (down_o.x - start_o.x) * (down_o.x - start_o.x)
        + (down_o.y - start_o.y) * (down_o.y - start_o.y);
    float up_dist = (up.x - start_o.x) * (up.x - start_o.x)
        + (up.y - start_o.y) * (up.y - start_o.y);

    if (down_dist > up_dist || trace.plane.normal.z < PM_STEP_NORMAL) {
        pm->state.origin = down_o;
        pm->state.velocity = down_v;
        return;
    }
    //!! Special case
    // if we were walking along a plane, then we need to copy the Z over
    pm->state.velocity.z = down_v.z;
}

//
//=============================================================================
//
//	MOVEMENT CONDITION CHECKS/HANDLING
//
//=============================================================================
//
//
//===============
// PM_CheckTrickJump
//
// Returns true if the player will be eligible for trick jumping should they
// impact the ground on this frame, false otherwise.
//===============
//
static qboolean PM_CheckTrickJump(void) {
    // False in the following conditions.
    if (pm->groundEntityPtr) { return false; }
    if (playerMoveLocals.previousVelocity.z < PM_SPEED_UP) { return false; }
    if (pm->cmd.upmove < 1) { return false; }
    if (pm->state.flags & PMF_JUMP_HELD) { return false; }
    if (pm->state.flags & PMF_TIME_MASK) { return false; }

    // True otherwise :)
    return true;
}

//
//===============
// PM_CheckJump
//
// Check for jumpingand trick jumping.
//
// Returns true if a jump occurs, false otherwise.
//===============
//
static qboolean PM_CheckJump(void) {
    // Before allowing a new jump:
    // 1. Wait for landing damage to subside.
    if (pm->state.flags & PMF_TIME_LAND) {
        return false;
    }

    // 2. Wait for jump key to be released
    if (pm->state.flags & PMF_JUMP_HELD) {
        return false;
    }

    // 3. Check if, they didn't ask to jump.
    if (pm->cmd.upmove < 1) {
        return false;
    }

    // We're ok to go, prepare for jumping.
    float jump = PM_SPEED_JUMP;

    // Factor in water level, modulate jump force based on that.
    if (pm->waterLevel > WATER_FEET) {
        jump *= PM_SPEED_JUMP_MOD_WATER;
    }

    // Add in the trick jump if eligible
    if (pm->state.flags & PMF_TIME_TRICK_JUMP) {
        jump += PM_SPEED_TRICK_JUMP;

        pm->state.flags &= ~PMF_TIME_TRICK_JUMP;
        pm->state.time = 0;

        PM_Debug("Trick jump: %i", pm->cmd.upmove);
    } else {
        PM_Debug("Jump: %i", pm->cmd.upmove);
    }

    if (pm->state.velocity.z < 0.0f) {
        pm->state.velocity.z = jump;
    } else {
        pm->state.velocity.z += jump;
    }

    // indicate that jump is currently held
    pm->state.flags |= (PMF_JUMPED | PMF_JUMP_HELD);

    // clear the ground indicators
    pm->state.flags &= ~PMF_ON_GROUND;
    pm->groundEntityPtr = NULL;

    // we can trick jump soon
    pm->state.flags |= PMF_TIME_TRICK_START;
    pm->state.time = 100;

    return true;
}

//
//===============
// PM_CheckDuck
//
// Sets the wished for values to crouch:
// pm->mins, pm->maxs, and pm->viewHeight
//===============
//
static void PM_CheckDuck(void) {
    // Any state after dead, can be checked for here.
    if (pm->state.type >= PM_DEAD) {
        if (pm->state.type == PM_GIB) {
            pm->state.view_offset.z = 0.0f;
        } else {
            pm->state.view_offset.z = -16.0f;
        }
        // Other states go here :)
    } else {

        const qboolean is_ducking = pm->state.flags & PMF_DUCKED;
        const qboolean wants_ducking = (pm->cmd.upmove < 0) && !(playerMoveLocals.ladder);

        if (!is_ducking && wants_ducking) {
            pm->state.flags |= PMF_DUCKED;
        } else if (is_ducking && !wants_ducking) {
            const trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pm->state.origin);

            if (!trace.allSolid && !trace.startSolid) {
                pm->state.flags &= ~PMF_DUCKED;
            }
        }

        // For either case, ducked, or not ducked. We first change the target 
        // view height.
        // 
        // This doesn't reflect the actual bbox, since in real life your eyes 
        // aren't at the top of your skull either.
        // 
        // Considering games == faking effects to get a nice real feel... Here we go.
        const float height = pm->maxs.z - pm->mins.z;

        if (pm->state.flags & PMF_DUCKED) {
            // A nice view height value.
            const float targetViewHeight = pm->mins.z + height * 0.5f;

            // LERP it.
            if (pm->state.view_offset.z > targetViewHeight) { // go down
                pm->state.view_offset.z -= playerMoveLocals.frameTime * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.view_offset.z < targetViewHeight) {
                pm->state.view_offset.z = targetViewHeight;
            }

            // Change the actual bounding box to reflect ducking
            pm->maxs.z = pm->maxs.z + pm->mins.z * 0.66f;
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
}


//
//===============
// PM_CheckLadder
//
// Check for ladder interaction.
// Returns true if the player is on a ladder.
//===============
//
static qboolean PM_CheckLadder(void) {
    // If any time mask flag is set, return.
    if (pm->state.flags & PMF_TIME_MASK) {
        return false;
    }

    // Calculate a trace for determining whether there is a ladder in front of us.
    const vec3_t pos = vec3_fmaf(pm->state.origin, 1, playerMoveLocals.forwardXY);
    const trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

    // Found one, engage ladder state.
    if ((trace.fraction < 1.0f) && (trace.contents & CONTENTS_LADDER)) {
        // Add ladder flag.
        pm->state.flags |= PMF_ON_LADDER;

        // No ground entity, obviously.
        pm->groundEntityPtr = NULL;

        // Remove ducked and possible ON_GROUND flags.
        pm->state.flags &= ~(PMF_ON_GROUND | PMF_DUCKED);

        return true;
    }

    return false;
}

//
//===============
// PM_CheckWater
//
// Checks for water exit.The player may exit the water when they can
// see a usable step out of the water.
//
// Returns true if a water jump has occurred, false otherwise.
//===============
//
static qboolean PM_CheckWaterJump(void) {

    if (pm->state.type == PM_HOOK_PULL || pm->state.type == PM_HOOK_SWING) {
        return false;
    }

    if (pm->state.flags & PMF_TIME_WATER_JUMP) {
        return false;
    }

    if (pm->waterLevel != WATER_WAIST) {
        return false;
    }

    if (pm->cmd.upmove < 1 && pm->cmd.forwardmove < 1) {
        return false;
    }

    vec3_t pos = vec3_fmaf(pm->state.origin, 16.f, playerMoveLocals.forward);
    trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

    if ((trace.fraction < 1.0f) && (trace.contents & CONTENTS_MASK_SOLID)) {

        pos.z += PM_STEP_HEIGHT + pm->maxs.z - pm->mins.z;

        trace = PM_TraceCorrectAllSolid(pos, pm->mins, pm->maxs, pos);

        if (trace.startSolid) {
            PM_Debug("Can't exit water: Blocked");
            return false;
        }

        vec3_t pos2 = {
            pos.x,
            pos.y,
            pm->state.origin.z
        };

        trace = PM_TraceCorrectAllSolid(pos, pm->mins, pm->maxs, pos2);

        if (!(trace.ent && trace.plane.normal.z >= PM_STEP_NORMAL)) {
            PM_Debug("Can't exit water: not a step\n");
            return false;
        }

        // Set up water velocity.
        pm->state.velocity.z = PM_SPEED_WATER_JUMP;

        // Set up the time state, JUMP == HELD, WATER JUMPING == ACTIVE
        pm->state.flags |= PMF_TIME_WATER_JUMP | PMF_JUMP_HELD;
        pm->state.time = 2000; // 2 seconds.

        return true;
    }

    return false;
}

//
//===============
// PM_CheckWater
//
// Checks for water interaction, accounting for player ducking, etc.
//===============
//
static void PM_CheckWater(void) {
    // When checking for water level we first reset all to defaults for this frame.
    pm->waterLevel = WATER_NONE;
    pm->waterType = 0;

    // Create the position for testing.
    vec3_t contentPosition = {
        pm->state.origin.x,
        pm->state.origin.y,
        // Pick the mins bounding box Z, PM_GROUND_DIST and add it to our current Z to use for testing.
        // (This should give us about the feet pos)
        pm->state.origin.z + pm->mins.z + PM_GROUND_DIST
    };

    // Perform the actual test.
    int32_t contents = pm->PointContents(contentPosition);

    // Are we in liquid? Hallelujah!
    if (contents & CONTENTS_MASK_LIQUID) {
        // Watertype is whichever ocntents type we are in with at least our feet.
        pm->waterType = contents;
        pm->waterLevel = WATER_FEET;

        contentPosition.z = pm->state.origin.z;

        contents = pm->PointContents(contentPosition);

        if (contents & CONTENTS_MASK_LIQUID) {

            pm->waterType |= contents;
            pm->waterLevel = WATER_WAIST;

            contentPosition.z = pm->state.origin.z + pm->state.view_offset.z + 1.0f;

            contents = pm->PointContents(contentPosition);

            if (contents & CONTENTS_MASK_LIQUID) {
                pm->waterType |= contents;
                pm->waterLevel = WATER_UNDER;

                pm->state.flags |= PMF_UNDER_WATER;
            }
        }
    }
}

//
//===============
// PM_CheckGround
//
// Checks for ground interaction, enabling trick jumpingand dealing with landings.
//===============
//
static void PM_CheckGround(void) {
    // If we jumped, or been pushed, do not attempt to seek ground
    if (pm->state.flags & (PMF_JUMPED | PMF_TIME_PUSHED | PMF_ON_LADDER)) {
        return;
    }

    // Seek ground eagerly in case the player wishes to trick jump
    const qboolean trick_jump = PM_CheckTrickJump();
    vec3_t pos;

    if (trick_jump) {
        pos = vec3_fmaf(pm->state.origin, playerMoveLocals.frameTime, pm->state.velocity);
        pos.z -= PM_GROUND_DIST_TRICK;
    } else {
        pos = pm->state.origin;
        pos.z -= PM_GROUND_DIST;
    }

    // Seek the ground
    trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

    playerMoveLocals.ground.plane = trace.plane;
    playerMoveLocals.ground.surface = trace.surface;
    playerMoveLocals.ground.contents = trace.contents;

    // If we hit an upward facing plane, make it our ground
    if (trace.ent && trace.plane.normal.z >= PM_STEP_NORMAL) {

        // If we had no ground, then handle landing events
        if (!pm->groundEntityPtr) {

            // Any landing terminates the water jump
            if (pm->state.flags & PMF_TIME_WATER_JUMP) {
                pm->state.flags &= ~PMF_TIME_WATER_JUMP;
                pm->state.time = 0;
            }

            // Hard landings disable jumping briefly
            if (playerMoveLocals.previousVelocity.z <= PM_SPEED_LAND) {
                pm->state.flags |= PMF_TIME_LAND;
                pm->state.time = 1;

                if (playerMoveLocals.previousVelocity.z <= PM_SPEED_FALL) {
                    pm->state.time = 16;

                    if (playerMoveLocals.previousVelocity.z <= PM_SPEED_FALL_FAR) {
                        pm->state.time = 256;
                    }
                }
            } else {
                // Soft landings with upward momentum grant trick jumps
                if (trick_jump) {
                    pm->state.flags |= PMF_TIME_TRICK_JUMP;
                    pm->state.time = 32;
                }
            }
        }

        // Save a reference to the ground
        pm->state.flags |= PMF_ON_GROUND;
        pm->groundEntityPtr = trace.ent;

        // Sink down to it if not trick jumping
        if (!(pm->state.flags & PMF_TIME_TRICK_JUMP)) {
            pm->state.origin = trace.endPosition;

            pm->state.velocity = PM_ClipVelocity(pm->state.velocity, trace.plane.normal, PM_CLIP_BOUNCE);
        }
    } else {
        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntityPtr = NULL;
    }

    // Always touch the entity, even if we couldn't stand on it
    PM_TouchEntity(trace.ent);
}


//
//=============================================================================
//
//	PHYSICS - ACCELERATION/FRICTION/GRAVITY
//
//=============================================================================
//

//
//===============
// PM_Friction
// 
// Handles friction against user intentions, and based on contents.
//===============
//
static void PM_Friction(void) {
    vec3_t vel = pm->state.velocity;

    if (pm->state.flags & PMF_ON_GROUND) {
        vel.z = 0.0;
    }

    const float speed = vec3_length(vel);

    if (speed < 1.0f) {
        pm->state.velocity.x = pm->state.velocity.y = 0.0f;
        return;
    }

    const float control = Maxf(PM_SPEED_STOP, speed);

    float friction = 0.0;

    // SPECTATOR friction
    if (pm->state.type == PM_SPECTATOR) {
        friction = PM_FRICT_SPECTATOR;
        // LADDER friction
    } else if (pm->state.flags & PMF_ON_LADDER) {
        friction = PM_FRICT_LADDER;
        // WATER friction.
    } else if (pm->waterLevel > WATER_FEET) {
        friction = PM_FRICT_WATER;
        // GROUND friction.
    } else if (pm->state.flags & PMF_ON_GROUND) {
        if (playerMoveLocals.ground.surface && (playerMoveLocals.ground.surface->flags & SURF_SLICK)) {
            friction = PM_FRICT_GROUND_SLICK;
        } else {
            friction = PM_FRICT_GROUND;
        }
        // OTHER friction
    } else {
        friction = PM_FRICT_AIR;
    }

    // scale the velocity, taking care to not reverse direction
    const float scale = Maxf(0.0, speed - (friction * control * playerMoveLocals.frameTime)) / speed;

    pm->state.velocity = vec3_scale(pm->state.velocity, scale);
}

//
//===============
// PM_Accelerate
// 
// Returns the newly user intended velocity
//===============
//
static void PM_Accelerate(const vec3_t & dir, float speed, float accel) {
    const float current_speed = vec3_dot(pm->state.velocity, dir);
    const float add_speed = speed - current_speed;

    if (add_speed <= 0.0f) {
        return;
    }

    float accel_speed = accel * playerMoveLocals.frameTime * speed;

    if (accel_speed > add_speed) {
        accel_speed = add_speed;
    }

    pm->state.velocity = vec3_fmaf(pm->state.velocity, accel_speed, dir);
}

//
//===============
// PM_Gravity
// 
// Applies gravity to the current movement.
//===============
//
static void PM_Gravity(void) {
    float gravity = pm->state.gravity;

    if (pm->waterLevel > WATER_WAIST) {
        gravity *= PM_GRAVITY_WATER;
    }

    pm->state.velocity.z -= gravity * playerMoveLocals.frameTime;
}


//
//===============
// PM_ApplyCurrents
// 
// Applies external force currents, such as water currents or
// conveyor belts.
//===============
//
static void PM_ApplyCurrents(void) {
    // Start off with 0 currents.
    vec3_t current = vec3_zero();

    // add water currents
    if (pm->waterLevel) {
        if (pm->waterType & CONTENTS_CURRENT_0) {
            current.x += 1.0;
        }
        if (pm->waterType & CONTENTS_CURRENT_90) {
            current.y += 1.0;
        }
        if (pm->waterType & CONTENTS_CURRENT_180) {
            current.x -= 1.0;
        }
        if (pm->waterType & CONTENTS_CURRENT_270) {
            current.y -= 1.0;
        }
        if (pm->waterType & CONTENTS_CURRENT_UP) {
            current.z += 1.0;
        }
        if (pm->waterType & CONTENTS_CURRENT_DOWN) {
            current.z -= 1.0;
        }
    }

    // add conveyer belt velocities
    if (pm->groundEntityPtr) {
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_0) {
            current.x += 1.0;
        }
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_90) {
            current.y += 1.0;
        }
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_180) {
            current.x -= 1.0;
        }
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_270) {
            current.y -= 1.0;
        }
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_UP) {
            current.z += 1.0;
        }
        if (playerMoveLocals.ground.contents & CONTENTS_CURRENT_DOWN) {
            current.z -= 1.0;
        }
    }

    if (!vec3_equal(current, vec3_zero())) {
        current = vec3_normalize(current);
    }

    pm->state.velocity = vec3_fmaf(pm->state.velocity, PM_SPEED_CURRENT, current);
}


//
//=============================================================================
//
//	PLAYE MOVEMENT STYLE IMPLEMENTATIONS
//
//=============================================================================
//
//
//===============
// PM_LadderMove
// 
// Called when the player is climbing a ladder.
//===============
//
static void PM_LadderMove(void) {
    PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_ApplyCurrents();

    // user intentions in X/Y
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->cmd.forwardmove, playerMoveLocals.forwardXY);
    vel = vec3_fmaf(vel, pm->cmd.sidemove, playerMoveLocals.rightXY);

    const float s = PM_SPEED_LADDER * 0.125f;

    // limit horizontal speed when on a ladder
    vel.x = Clampf(vel.x, -s, s);
    vel.y = Clampf(vel.y, -s, s);
    vel.z = 0.f;

    // handle Z intentions differently
    if (std::fabsf(pm->state.velocity.z) < PM_SPEED_LADDER) {

        if ((pm->viewAngles.x <= -15.0f) && (pm->cmd.forwardmove > 0)) {
            vel.z = PM_SPEED_LADDER;
        }
        else if ((pm->viewAngles.x >= 15.0f) && (pm->cmd.forwardmove > 0)) {
            vel.z = -PM_SPEED_LADDER;
        }
        else if (pm->cmd.upmove > 0) {
            vel.z = PM_SPEED_LADDER;
        }
        else if (pm->cmd.upmove < 0) {
            vel.z = -PM_SPEED_LADDER;
        }
        else {
            vel.z = 0.0;
        }
    }

    if (pm->cmd.upmove > 0) { // avoid jumps when exiting ladders
        pm->state.flags |= PMF_JUMP_HELD;
    }

    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0, PM_SPEED_LADDER);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    PM_Accelerate(dir, speed, PM_ACCEL_LADDER);

    PM_StepSlideMove();
}

//
//===============
// PM_WaterJumpMove
// 
// Called when the player is jumping out of the water to a solid.
//===============
//
static void PM_WaterJumpMove(void) {

    PM_Debug("%s\n", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_Gravity();

    // check for a usable spot directly in front of us
    const vec3_t pos = vec3_fmaf(pm->state.origin, 30.f, playerMoveLocals.forwardXY);

    // if we've reached a usable spot, clamp the jump to avoid launching
    if (PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos).fraction == 1.0f) {
        pm->state.velocity.z = Clampf(pm->state.velocity.z, 0.f, PM_SPEED_JUMP);
    }

    // if we're falling back down, clear the timer to regain control
    if (pm->state.velocity.z <= 0.0f) {
        pm->state.flags &= ~PMF_TIME_MASK;
        pm->state.time = 0;
    }

    PM_StepSlideMove();
}

//
//===============
// PM_WaterMove
// 
// Called for movements where player is in the water
//===============
//
static void PM_WaterMove(void) {

    if (PM_CheckWaterJump()) {
        PM_WaterJumpMove();
        return;
    }

    PM_Debug("%s\n", Vec3ToString(pm->state.origin));

    // apply friction, slowing rapidly when first entering the water
    PM_Friction();

    // and sink if idle
    if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove) {
        if (pm->state.velocity.z > PM_SPEED_WATER_SINK) {
            PM_Gravity();
        }
    }

    PM_ApplyCurrents();

    // user intentions on X/Y/Z
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->cmd.forwardmove, playerMoveLocals.forward);
    vel = vec3_fmaf(vel, pm->cmd.sidemove, playerMoveLocals.right);

    // add explicit Z
    vel.z += pm->cmd.upmove;

    // disable water skiing
    if (pm->waterLevel == WATER_WAIST) {
        vec3_t view = pm->state.origin + pm->state.view_offset;
        view.z -= 4.0;

        if (!(pm->PointContents(view) & CONTENTS_MASK_LIQUID)) {
            pm->state.velocity.z = Minf(pm->state.velocity.z, 0.0);
            vel.z = Minf(vel.z, 0.0);
        }
    }

    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0, PM_SPEED_WATER);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    PM_Accelerate(dir, speed, PM_ACCEL_WATER);

    if (pm->cmd.upmove > 0) {
        PM_StepSlideMove_();
    }
    else {
        PM_StepSlideMove();
    }
}

//
//===============
// PM_AirMove
// 
// Called for movements where player is in air.
//===============
//
static void PM_AirMove(void) {

    PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_Gravity();

    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->cmd.forwardmove, playerMoveLocals.forwardXY);
    vel = vec3_fmaf(vel, pm->cmd.sidemove, playerMoveLocals.rightXY);
    vel.z = 0.f;

    float max_speed = PM_SPEED_AIR;

    // Accounting for walk modulus
    if (pm->cmd.buttons & BUTTON_WALK) {
        max_speed *= PM_SPEED_MOD_WALK;
    }

    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.f, max_speed);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.f;
    }

    float accel = PM_ACCEL_AIR;

    if (pm->state.flags & PMF_DUCKED) {
        accel *= PM_ACCEL_AIR_MOD_DUCKED;
    }

    PM_Accelerate(dir, speed, accel);

    PM_StepSlideMove();
}

//
//===============
// PM_WalkMove
// 
// Called for movements where player is on ground, regardless of water level.
//===============
//
static void PM_WalkMove(void) {

    // Check for beginning of a jump
    if (PM_CheckJump()) {
        PM_AirMove();
        return;
    }

    PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_ApplyCurrents();

    // Project the desired movement into the X/Y plane
    const vec3_t forward = vec3_normalize(PM_ClipVelocity(playerMoveLocals.forwardXY, playerMoveLocals.ground.plane.normal, PM_CLIP_BOUNCE));
    const vec3_t right = vec3_normalize(PM_ClipVelocity(playerMoveLocals.rightXY, playerMoveLocals.ground.plane.normal, PM_CLIP_BOUNCE));

    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->cmd.forwardmove, forward);
    vel = vec3_fmaf(vel, pm->cmd.sidemove, right);

    float max_speed;

    // Clamp to max speed
    if (pm->waterLevel > WATER_FEET) {
        max_speed = PM_SPEED_WATER;
    }
    else if (pm->state.flags & PMF_DUCKED) {
        max_speed = PM_SPEED_DUCKED;
    }
    else {
        max_speed = PM_SPEED_RUN;
    }

    // Accounting for walk modulus
    if (pm->cmd.buttons & BUTTON_WALK) {
        max_speed *= PM_SPEED_MOD_WALK;
        PM_Debug("BUTTON WALK HA");
    }
    else {
        PM_Debug("PMOVE_NOBUUTTONWALK");
    }

    // Clamp the speed to min/max speed
    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0, max_speed);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    // Accelerate based on slickness of ground surface
    const float accel = (playerMoveLocals.ground.surface->flags & SURF_SLICK) ? PM_ACCEL_GROUND_SLICK : PM_ACCEL_GROUND;

    PM_Accelerate(dir, speed, accel);

    // Determine the speed after acceleration
    speed = vec3_length(pm->state.velocity);

    // Clip to the ground
    pm->state.velocity = PM_ClipVelocity(pm->state.velocity, playerMoveLocals.ground.plane.normal, PM_CLIP_BOUNCE);

    // And now scale by the speed to avoid slowing down on slopes
    pm->state.velocity = vec3_normalize(pm->state.velocity);
    pm->state.velocity = vec3_scale(pm->state.velocity, speed);

    // And finally, step if moving in X/Y
    if (pm->state.velocity.x || pm->state.velocity.y) {
        PM_StepSlideMove();
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
    PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    // User intentions on X/Y/Z
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->cmd.forwardmove, playerMoveLocals.forward);
    vel = vec3_fmaf(vel, pm->cmd.sidemove, playerMoveLocals.right);

    // Add explicit Z
    vel.z += pm->cmd.upmove;

    float speed;
    vel = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0, PM_SPEED_SPECTATOR);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    // Accelerate
    PM_Accelerate(vel, speed, PM_ACCEL_SPECTATOR);

    // Do the move
    PM_StepSlideMove();
}

//
//===============
// PM_FreezeMove
// 
// Freeze Player movement/
//===============
//
static void PM_FreezeMove(void) {
    PM_Debug("%s", Vec3ToString(pm->state.origin));
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
static void PM_Init(pm_move_t * pmove) {
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
    pm->groundEntityPtr = NULL;

    pm->waterType = 0;
    pm->waterLevel = 0;

    // Reset the flags, and step, values. These are set on a per frame basis.
    pm->state.flags &= ~(PMF_ON_GROUND | PMF_ON_STAIRS | PMF_ON_LADDER);
    pm->state.flags &= ~(PMF_JUMPED | PMF_UNDER_WATER);
    pm->step = 0.0f;

    // Jump "held" also, in case its key was released.
    if (pm->cmd.upmove < 1) {
        pm->state.flags &= ~PMF_JUMP_HELD;
    }

    // Decrement the movement timer, used for "dropping" the player, landing after jumps, 
    // or falling off a ledge/slope, by the duration of the command.
    if (pm->state.time) {
        if (pm->cmd.msec >= pm->state.time) { // clear the timer and timed flags
            pm->state.flags &= ~PMF_TIME_MASK;
            pm->state.time = 0;
        }
        else { // or just decrement the timer
            pm->state.time -= pm->cmd.msec;
        }
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
    // Copy the command angles into the outgoing state
    for (int i = 0; i < 3; i++) {
        float temp = pm->cmd.angles[i] + pm->state.delta_angles[i];
        pm->viewAngles[i] = SHORT2ANGLE(temp);
    }

    // Clamp pitch to prevent the player from looking up or down more than 90º
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
    // Clear all PM local vars
    playerMoveLocals = {};

    // Increase frame time based on seconds.
    playerMoveLocals.frameTime = pm->cmd.msec * 0.001f;

    // Save in case we get stuck and wish to undo this move.
    playerMoveLocals.previousOrigin = pm->state.origin;
    playerMoveLocals.previousVelocity = pm->state.velocity;

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
void PMove(pm_move_t * pmove, pmoveParams_t * params)
{
    // TODO: When PMOVE is finished, remove this useless pmoveParams thing.
    pmp = params;

    // Initialize the PMove.
    PM_Init(pmove);

    // Ensure angles are clamped.
    PM_ClampAngles();

    // Initialize the locals.
    PM_InitLocal();

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

    // Check for Ladders.
    PM_CheckLadder();

    // Set mins, maxs, and viewHeight
    PM_CheckDuck();

    // Check for water.
    PM_CheckWater();

    // Check for ground.
    PM_CheckGround();

    if (pm->state.flags & PMF_TIME_TELEPORT) {
        // pause in place briefly
    }
    else if (pm->state.flags & PMF_TIME_WATER_JUMP) {
        PM_WaterJumpMove();
    }
    else if (pm->state.flags & PMF_ON_LADDER) {
        PM_LadderMove();
    }
    else if (pm->state.flags & PMF_ON_GROUND) {
        PM_WalkMove();
    }
    else if (pm->waterLevel > WATER_FEET) {
        PM_WaterMove();
    }
    else {
        PM_AirMove();
    }

    // Check for ground at new spot.
    PM_CheckGround();

    // Check for water at new spot.
    PM_CheckWater();
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
void PMoveInit(pmoveParams_t * pmp)
{
    // Set up default pmove parameters
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
void PMoveEnableQW(pmoveParams_t * pmp)
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
