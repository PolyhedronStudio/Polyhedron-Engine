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
#include "shared/math/vector3.hpp"
#include "sharedgame/pmove.h"
#include "client/client.h"

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

// PM Move pointer to the current (client-)pmove being processed.
static pm_move_t* pm;
// PM Move pointer to the current (client-)pmove parameter settings.
static pmoveParams_t* pmp;

//-------------------
// @brief A structure containing full floating point precision copies of all
// movement variables. This is initialized with the player's last movement
// at each call to Pm_Move (this is obviously not thread-safe).
//-------------------
static struct {
    // Previous (incoming) origin, in case movement fails and must be reverted.
    vec3_t previousOrigin;
    // Previous (incoming) velocity, used for detecting landings.
    vec3_t previousVelocity;

    // Directional vectors based on command angles, with Z component.
    vec3_t forwardXYZ, rightXYZ, upXYZ;
    // Directional vectors without Z component, for air and ground movement.
    vec3_t forwardXY, rightXY;

    // The current movement command duration, in seconds.
    float time;

    // The player's ground interaction, copied from a trace.
    struct {
        // The ground plane.
        cplane_t plane;

        // The ground surface, used for texinfo.
        csurface_t* surface;

        // The ground contents.
        int32_t contents;
    } ground;
} pm_locals;


/**
 * @brief Water level
 */
typedef enum {
    WATER_UNKNOWN = -1,
    WATER_NONE,
    WATER_FEET,
    WATER_WAIST,
    WATER_UNDER
} pm_water_level_t;


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
// Slide off of the impacted plane.
//===============
//
static const Vector &PM_ClipVelocity(const Vector &in,  const Vector &normal, float bounce) {
    // Calculate the dot product for in and normal.
    float backoff = in * normal;

    // Multiply/Divide backoff by bounce, based on negative or positive value.
    if (backoff < 0.0f) {
        backoff *= bounce;
    }
    else {
        backoff /= bounce;
    }

    // Return the input minus the normal scaled by backoff.
    return in - normal * backoff;
}

//
//===============
// PM_TouchEntity
// 
// Mark the specified entity as touched. This enables the game module to
// detect player -> entity interactions.
//===============
//
static void PM_TouchEntity(struct edict_s* ent) {
    // Ensure it is valid.
    if (ent == NULL) {
        return;
    }

    // Return in case of crossing the max touch entities limit.
    if (pm->numTouchedEntities == PM_MAX_TOUCH_ENTS) {
        //Com_DPrintf("MAX_TOUCH_ENTS\n");
        return;
    }

    // Return in case of a duplicate entity.
    for (int32_t i = 0; i < pm->numTouchedEntities; i++) {
        if (pm->touchedEntities[i] == ent) {
            return;
        }
    }

    // Increment counter, and store pointer to entity.
    pm->touchedEntities[pm->numTouchedEntities++] = ent;
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
static trace_t PM_TraceCorrectAllSolid(const Vector &start, const Vector& end, const Vector& mins, const Vector& maxs) {
    // Offsets testing array.
    const int32_t offsets[] = { 0, 1, -1 };

    // Trace vec3_t
    vec3_t trStart;
    vec3_t trEnd;
    vec3_t trMins;
    vec3_t trMaxs;

    // Jitter around, 3x3x3 = 18 traces... TODO: Improve this? Seems expensive.
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            for (uint32_t k = 0; k < 3; k++) {
                // Calculate point to start tracing from.
                Vector point = start + Vector(offsets[i], offsets[j], offsets[k]);

                // Copy Vectors into vec3_t and excute trace. 
                // (Point, not start, unlike below in the return.)
                point.CopyToArray(trStart);
                end.CopyToArray(trEnd);
                mins.CopyToArray(trMins);
                maxs.CopyToArray(trMaxs);
                trace_t trace = pm->Trace(trStart, trEnd, trMins, trMaxs);

                // No solid had to be fixed.
                if (!trace.allsolid) {
                    // In case solids had to be fixed, debug print.
                    if (i != 0 || j != 0 || k != 0) {
                        Com_DPrintf("Fixed all-solid\n");
                    }

                    return trace;
                }
            }
        }
    }

    Com_DPrintf("No good position\n");
    // Copy Vectors into vec3_t and excute trace.
    start.CopyToArray(trStart);
    end.CopyToArray(trEnd);
    mins.CopyToArray(trMins);
    maxs.CopyToArray(trMaxs);
    return pm->Trace(trStart, trEnd, trMins, trMaxs);
}


//
//===============
// PM_ImpactPlane
// 
// Returns true if `plane` is unique to `planes` and should be impacted, false otherwise.
//===============
//
static bool PM_ImpactPlane(Vector *planes, int32_t num_planes, const Vector &plane) {

    for (int32_t i = 0; i < num_planes; i++) {
        if (plane * planes[i] > 1.0f - PM_STOP_EPSILON) {
            return false;
        }
    }

    return true;
}

//
//===============
// PM_SlideMove
// 
// Calculates a new origin, velocity, and contact entities based on the
// movement command and world state. Returns true if not blocked.
//===============
//
#define MAX_CLIP_PLANES	6
static bool PM_SlideMove(void) {
    const int32_t numBumps = MAX_CLIP_PLANES - 2;
    Vector planes[MAX_CLIP_PLANES];
    int32_t bump;

    float timeRemaining = pm_locals.time;
    int32_t numPlanes = 0;

    // Never turn against our ground plane
    if (pm->state.flags & PMF_ON_GROUND) {
        planes[numPlanes] = pm_locals.ground.plane.normal;
        numPlanes++;
    }

    // or our original velocity
    planes[numPlanes] = Vector(pm->state.velocity).Normalized();
    numPlanes++;

    for (bump = 0; bump < numBumps; bump++) {
        Vector position;

        if (timeRemaining <= 0.0f) { // out of time
            break;
        }

        // Project desired destination
        position = Vector(pm->state.origin).FMAF(timeRemaining, pm->state.velocity);

        // Execute trace.
        trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, position, pm->mins, pm->maxs);

        // If the player is trapped in a solid, don't build up Z
        if (trace.allsolid) {
            pm->state.velocity[0] = 0.0f;
            return true;
        }

        // If the trace succeeded, move some distance
        if (trace.fraction > 0.0f) {
            // Do a Vec3_Copy into the pm->state.origin vec3_t. 
            Vec3_Copy(trace.endpos, pm->state.origin);

            // If the trace didn't hit anything, we're done
            if (trace.fraction == 1.0f) {
                break;
            }

            // Update the movement time remaining
            timeRemaining -= (timeRemaining * trace.fraction);
        }

        // Store a reference to the entity for firing game events
        PM_TouchEntity(trace.ent);

        // Record the impacted plane, or nudge velocity out along it
        if (PM_ImpactPlane(planes, numPlanes, trace.plane.normal)) {
            planes[numPlanes] = trace.plane.normal;
            numPlanes++;
        }
        else {
            // If we've seen this plane before, nudge our velocity out along it
            Vector velocity = Vector(pm->state.velocity) + Vector(trace.plane.normal);
            velocity.CopyToArray(pm->state.velocity);
            continue;
        }

        // And modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            Vector velocity = pm->state.velocity;

            // If velocity doesn't impact this plane, skip it
            if (velocity * planes[i] >= 0.0f) {
                continue;
            }

            // Slide along the plane
            velocity = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

            // See if there is a second plane that the new move enters
            for (int32_t j = 0; j < numPlanes; j++) {
                Vector cross;

                // Skip in case we are testing the same plane.
                if (j == i) {
                    continue;
                }

                // If the clipped velocity doesn't impact this plane, skip it
                if (velocity * planes[j] >= 0.0f) {
                    continue;
                }

                // We are now intersecting a second plane
                velocity = PM_ClipVelocity(velocity, planes[j], PM_CLIP_BOUNCE);

                // But if we clip against it without being deflected back, we're okay
                if (velocity * planes[i] >= 0.0f) {
                    continue;
                }

                // We must now slide along the crease (cross product of the planes)
                cross = Vector(planes[i]).CrossProduct(planes[j]);
                cross.Normalize();

                // Calculate new velocity.
                const float scale = cross * pm->state.velocity;
                velocity = cross * scale;

                // see if there is a third plane the the new move enters
                for (int32_t k = 0; k < numPlanes; k++) {
                    // Once again, make sure we aren't testing the same plane.
                    if (k == i || k == j) {
                        continue;
                    }

                    if (velocity * planes[k] >= 0.0f) {
                        continue;
                    }

                    // stop dead at a triple plane interaction
                    Vector::Zero.CopyToArray(pm->state.velocity);
                    return true;
                }
            }

            // if we have fixed all interactions, try another move
            velocity.CopyToArray(pm->state.velocity);
            break;
        }
    }

    return bump == 0;
}

//
//===============
// PM_SlideMove
// 
// Returns True if the downward trace yielded a step, false otherwise.
//===============
//
static bool PM_CheckStep(const trace_t* trace) {
    if (!trace->allsolid) {
        if (trace->ent && trace->plane.normal[2] >= PM_STEP_NORMAL) {
            if (trace->ent != pm->groundEntity || trace->plane.dist != pm_locals.ground.plane.dist) {
                return true;
            }
        }
    }

    return false;
}

//
//===============
// PM_StepDown
// 
// Calculates the step value, and removes stair flag if needed.
//===============
//
static void PM_StepDown(const trace_t* trace) {
    // Hard copy the trace end into the pm state origin.
    Vec3_Copy(trace->endpos, pm->state.origin);
    
    // Calculate step value.
    pm->step = pm->state.origin[2] - pm_locals.previousOrigin[2];

    if (pm->step >= PM_STEP_HEIGHT_MIN) {
        Com_DPrintf("step up %3.2f\n", pm->step);
        pm->state.flags |= PMF_ON_STAIRS;
    }
    else if (pm->step <= -PM_STEP_HEIGHT_MIN && (pm->state.flags & PMF_ON_GROUND)) {
        Com_DPrintf("step down %3.2f\n", pm->step);
        pm->state.flags |= PMF_ON_STAIRS;
    }
    else {
        pm->step = 0.0;
    }
}

//
//===============
// PM_StepDown
// 
// 
//===============
//
static void PM_StepSlideMove(void) {

    // Store pre-move parameters
    const Vector org0 = pm->state.origin;
    const Vector vel0 = pm->state.velocity;

    // Attempt to move; if nothing blocks us, we're done
    if (PM_SlideMove()) {
        // Attempt to step down to remain on ground
        if ((pm->state.flags & PMF_ON_GROUND) && pm->cmd.upmove <= 0) {
            // Calculate down vector.
            Vector down = Vector(pm->state.origin).FMAF(PM_STEP_HEIGHT + PM_GROUND_DIST, Vector::Down);

            // Execute trace.
            trace_t stepDown = PM_TraceCorrectAllSolid(pm->state.origin, down, pm->mins, pm->maxs);

            // Check for valid step down, if so, step it.
            if (PM_CheckStep(&stepDown)) {
                PM_StepDown(&stepDown);
            }
        }

        return;
    }

    // we were blocked, so try to step over the obstacle
    const Vector org1 = pm->state.origin;
    const Vector vel1 = pm->state.velocity;

    Vector up = Vector(org0).FMAF(PM_STEP_HEIGHT, Vector::Up);
    trace_t stepUp = PM_TraceCorrectAllSolid(org0, up, pm->mins, pm->maxs);

    if (!stepUp.allsolid) {

        // Step from the higher position, with the original velocity
        Vec3_Copy(stepUp.endpos, pm->state.origin);
        vel0.CopyToArray(pm->state.velocity);

        PM_SlideMove();

        // Settle to the new ground, keeping the step if and only if it was successful
        const Vector down = Vector(pm->state.origin).FMAF(PM_STEP_HEIGHT + PM_GROUND_DIST, Vector::Down);
        trace_t stepDown = PM_TraceCorrectAllSolid(pm->state.origin, down, pm->mins, pm->maxs);

        if (PM_CheckStep(&stepDown)) {
            // Quake2 trick jump secret sauce
            if ((pm->state.flags & PMF_ON_GROUND) || vel0.z < PM_SPEED_UP) {
                PM_StepDown(&stepDown);
            }
            else {
                pm->step = pm->state.origin[2] - pm_locals.previousOrigin[2];
                pm->state.flags |= PMF_ON_STAIRS;
            }

            return;
        }
    }

    // Copy into original vec3_t vectors.
    org1.CopyToArray(pm->state.origin);
    vel1.CopyToArray(pm->state.velocity);
}

//
//===============
// PM_Friction
// 
// Handles friction against user intentions, and based on contents.
//===============
//
static void PM_Friction(void) {
    Vector vel1 = pm->state.velocity;

    // Set Z to 0 if on ground, so we can do a proper speed value calculation.
    if (pm->state.flags & PMF_ON_GROUND) {
        vel1.z = 0.0;
    }

    // If the speed is considerably lower than 1.0f, set X and Y velocity to 0.
    const float speed = vel1.Length();

    if (speed < 1.0f) {
        pm->state.velocity[0] = pm->state.velocity[1] = 0.0f;
        return;
    }

    // Make sure that the "control" speed does not exceed PM_SPEED_STOP
    const float control = std::fmaxf(PM_SPEED_STOP, speed);

    // The actual friction value we'll be using, set it based on several
    // conditions.
    float friction = 0.0;

    if (pm->state.type == PM_SPECTATOR) { // spectator friction
        friction = PM_FRICT_SPECTATOR;
    }
    else if (pm->state.flags & PMF_ON_LADDER) { // ladder friction
        friction = PM_FRICT_LADDER;
    }
    else if (pm->waterLevel > WATER_FEET) { // water friction
        friction = PM_FRICT_WATER;
    }
    else if (pm->state.flags & PMF_ON_GROUND) { // ground friction
        if (pm_locals.ground.surface && (pm_locals.ground.surface->flags & SURF_SLICK)) {
            friction = PM_FRICT_GROUND_SLICK;
        }
        else {
            friction = PM_FRICT_GROUND;
        }
    }
    else { // everything else friction
        friction = PM_FRICT_AIR;
    }

    // Scale the velocity, taking care to not reverse direction
    const float scale = std::fmaxf(0.0, speed - (friction * control * pm_locals.time)) / speed;

    // Calculate new velocity.
    Vector vel2 = pm->state.velocity;
    vel2 *= scale;

    // Copy the new velocity back into player move state.
    vel2.CopyToArray(pm->state.velocity);
}

//
//===============
// PM_Accelerate
// 
// Handles user intended acceleration.
//===============
//
static void PM_Accelerate(const Vector &dir, float speed, float accel) {
    // Calculate the dot product between velocity and direction.
    // This gives us the current speed.
    // 
    // We then subtract it from the intended speed value to give us
    // the added speed left to add.
    const float current_speed = Vector(pm->state.velocity) * dir;
    const float add_speed = speed - current_speed;

    if (add_speed <= 0.0f) {
        return;
    }

    // Calculate the actual acceleration speed based on time.
    float accel_speed = accel * pm_locals.time * speed;

    // Make sure we do not exceed the speed we wished to add.
    if (accel_speed > add_speed) {
        accel_speed = add_speed;
    }

    // Set the new player move state velocity.
    // (add multiplied accel_speed * dir to pm->state.velocity).
    Vector(pm->state.velocity).FMAF(accel_speed, dir).CopyToArray(pm->state.velocity);
}

//
//===============
// PM_Gravity
// 
// Applies gravity to the current movement.
//===============
//
static void PM_Gravity(void) {

    if (pm->state.type == PM_HOOK_PULL) {
        return;
    }

    // Fetch the current state gravity.
    float gravity = pm->state.gravity;

    // In case we are with our waist under water, double the gravity force.
    if (pm->waterLevel > WATER_WAIST) {
        gravity *= PM_GRAVITY_WATER;
    }

    // Update the velocity with gravity.
    pm->state.velocity[2] -= gravity * pm_locals.time;
}

//
//===============
// PM_Currents
// 
// Handles user intended acceleration.
//===============
//
static void PM_Currents(void) {
    Vector current = Vector::Zero;

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
    if (pm->groundEntity) {
        if (pm_locals.ground.contents & CONTENTS_CURRENT_0) {
            current.x += 1.0;
        }
        if (pm_locals.ground.contents & CONTENTS_CURRENT_90) {
            current.y += 1.0;
        }
        if (pm_locals.ground.contents & CONTENTS_CURRENT_180) {
            current.x -= 1.0;
        }
        if (pm_locals.ground.contents & CONTENTS_CURRENT_270) {
            current.y -= 1.0;
        }
        if (pm_locals.ground.contents & CONTENTS_CURRENT_UP) {
            current.z += 1.0;
        }
        if (pm_locals.ground.contents & CONTENTS_CURRENT_DOWN) {
            current.z -= 1.0;
        }
    }

    // Make sure it isn't equal to our Zero vector.
    if (current != Vector::Zero) {
        current.Normalize();
    }

    // Set the new player move state velocity.
    // (add multiplied PM_SPEED_CURRENT * current to pm->state.velocity).
    Vector(pm->state.velocity).FMAF(PM_SPEED_CURRENT, current).CopyToArray(pm->state.velocity);
}

//
//===============
// PM_CheckTrickJump
// 
// True if the player will be eligible for trick jumping should they
// impact the ground on this frame, false otherwise.
//===============
//
static bool PM_CheckTrickJump(void) {

    if (pm->groundEntity) {
        return false;
    }

    if (pm_locals.previousVelocity[2] < PM_SPEED_UP) {
        return false;
    }

    if (pm->cmd.upmove < 1) {
        return false;
    }

    if (pm->state.flags & PMF_JUMP_HELD) {
        return false;
    }

    if (pm->state.flags & PMF_TIME_MASK) {
        return false;
    }

    return true;
}

//
//===============
// PM_CheckTrickJump
// 
// Returns True if the player is attempting to leave the ground via grappling hook.
//===============
//
static bool PM_CheckHookJump(void) {

    if ((pm->state.type == PM_HOOK_PULL || pm->state.type == PM_HOOK_SWING) 
        && pm->state.velocity[2] > 4.0f) 
    {

        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntity = NULL;

        return true;
    }

    return false;
}

//
//===============
// PM_CheckHook
// 
//===============
//
static void PM_CheckHook(void) {
    ////// hookers only
    //if (pm->state.type != PM_HOOK_PULL && pm->state.type != PM_HOOK_SWING) {
    //    pm->state.flags &= ~PMF_HOOK_RELEASED;
    //    return;
    //}

    //// get chain length
    //if (pm->state.type == PM_HOOK_PULL) {

    //    // if we let go of hook, just go back to normal
    //    if (!(pm->cmd.buttons & BUTTON_HOOK)) {
    //        pm->state.type = PM_NORMAL;
    //        return;
    //    }

    //    pm->cmd.forwardmove = pm->cmd.sidemove = 0;

    //    // Pull physics
    //    // Calculate the distance vector.
    //    const float dist = Vector(pm->state.hook_position - pm->state.origin).Length();
    //    // Normalize the pm->state.velocity vector to make it directional.
    //    Vector(pm->state.velocity).Normalized().CopyToArray(pm->state.velocity);

    //    if (dist > 8.0f && !PM_CheckHookJump()) {
    //        // Scale PM State velocity.
    //        Vector(Vector(pm->s.velocity) * pm->hook_pull_speed).CopyToArray(pm->state.velocity);
    //    }
    //    else {
    //        // Zero out PM State velocity.
    //        Vector::Zero.CopyToArray(pm->s.velocity);
    //    }
    //}
    //else {

    //    // Check for disable
    //    if (!(pm->state.flags & PMF_HOOK_RELEASED)) {
    //        if (!(pm->cmd.buttons & BUTTON_HOOK)) {
    //            pm->state.flags |= PMF_HOOK_RELEASED;
    //        }
    //    }
    //    else {
    //        // If we let go of hook, just go back to normal.
    //        if (pm->cmd.buttons & BUTTON_HOOK) {
    //            pm->state.type = PM_NORMAL;
    //            pm->state.flags &= ~PMF_HOOK_RELEASED;
    //            return;
    //        }
    //    }

    //    const float hook_rate = (pm->hook_pull_speed / 1.5f) * pm_locals.time;

    //    // Chain physics
    //    // Grow/shrink chain based on input
    //    if ((pm->cmd.up > 0 || !(pm->state.flags & PMF_HOOK_RELEASED)) && (pm->state.hook_length > PM_HOOK_MIN_DIST)) {
    //        pm->state.hook_length = std::fmaxf(pm->state.hook_length - hook_rate, PM_HOOK_MIN_DIST);
    //    }
    //    else if ((pm->cmd.up < 0) && (pm->state.hook_length < PM_HOOK_MAX_DIST)) {
    //        pm->state.hook_length = std::fminf(pm->state.hook_length + hook_rate, PM_HOOK_MAX_DIST);
    //    }

    //    Vector chainVec     = Vector(pm->state.hook_position) - Vector(pm->state.origin);
    //    float chainLength   = chainVec.Length();

    //    // If player's location is already within the chain's reach
    //    if (chainLength <= pm->state.hook_length) {
    //        return;
    //    }

    //    // Reel us in!
    //    Vector velocityPart;

    //    // Determine player's velocity component of chain vector
    //    velocityPart = chainVec * ((Vector(pm->state.velocity ) * chainVec) / (chainVec * chainVec));

    //    // Restrainment default force
    //    float force = (chainLength - pm->state.hook_length) * 5.0f;

    //    // If player's velocity heading is away from the hook
    //    if (Vector(pm->state.velocity) * chainVec) < 0.0f) {

    //        // If chain has streched for 24 units
    //        if (chainLength > pm->state.hook_length + 24.0f) {

    //            // Remove player's velocity component moving away from hook
    //            Vector(Vector(pm->state.velocity) - velocityPart)).CopyToArray(pm->state.velocity);
    //        }
    //    }
    //    else { // If player's velocity heading is towards the hook
    //        float length = velocityPart.Length();
    //        if (length < force) {
    //            force -= length;
    //        }
    //        else {
    //            force = 0.0f;
    //        }
    //    }

    //    if (force) {
    //        // Applies chain restrainment
    //        chainVec.Normalize();
    //        Vector(pm->state.velocity).FMAF(force, chainVec).CopyToArray(pm->state.velocity);
    //    }
    //}
}

/**
 * @brief Checks for ground interaction, enabling trick jumping and dealing with landings.
 */
//
//===============
// PM_CheckGround
// 
// Checks for ground interaction, enabling trick jumping and dealing with landings.
//===============
//
static void PM_CheckGround(void) {
    if (PM_CheckHookJump()) {
        return;
    }

    // If we jumped, or been pushed, do not attempt to seek ground
    if (pm->state.flags & (PMF_JUMPED | PMF_TIME_PUSHED | PMF_ON_LADDER)) {
        return;
    }

    // Seek ground eagerly if the player wishes to trick jump
    const bool trick_jump = PM_CheckTrickJump();
    Vector position;

    if (trick_jump) {
        position = Vector(pm->state.origin).FMAF(pm_locals.time, pm->state.velocity);
        position.z -= PM_GROUND_DIST_TRICK;
    }
    else {
        position = pm->state.origin;
        position.z -= PM_GROUND_DIST;
    }

    // Seek the ground
    trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, position, pm->mins, pm->maxs);

    // Store ground information.
    pm_locals.ground.plane = trace.plane;
    pm_locals.ground.surface = trace.surface;
    pm_locals.ground.contents = trace.contents;

    // If we hit an upward facing plane, make it our ground
    if (trace.ent && trace.plane.normal[2] >= PM_STEP_NORMAL) {
        // If we had no ground, then handle landing events
        if (!pm->groundEntity) {
            // Any landing terminates the water jump
            if (pm->state.flags & PMF_TIME_WATER_JUMP) {
                pm->state.flags &= ~PMF_TIME_WATER_JUMP;
                pm->state.time = 0;
            }

            // Hard landings disable jumping briefly
            if (pm_locals.previousVelocity[2] <= PM_SPEED_LAND) {
                pm->state.flags |= PMF_TIME_LAND;
                pm->state.time = 1;

                if (pm_locals.previousVelocity[2] <= PM_SPEED_FALL) {
                    pm->state.time = 16;

                    if (pm_locals.previousVelocity[2] <= PM_SPEED_FALL_FAR) {
                        pm->state.time = 256;
                    }
                }
            }
            else { // Soft landings with upward momentum grant trick jumps
                if (trick_jump) {
                    pm->state.flags |= PMF_TIME_TRICK_JUMP;
                    pm->state.time = 32;
                }
            }
        }

        // Save a reference to the ground
        pm->state.flags |= PMF_ON_GROUND;
        pm->groundEntity = trace.ent;

        // And sink down to it if not trick jumping
        if (!(pm->state.flags & PMF_TIME_TRICK_JUMP)) {
            // Copy trace result into the PM State origin.
            Vector(trace.endpos).CopyToArray(pm->state.origin);

            // Clip velocity, and copy results back into PM State velocity.
            PM_ClipVelocity(pm->state.velocity, trace.plane.normal, PM_CLIP_BOUNCE).CopyToArray(pm->state.velocity);
        }
    }
    else {
        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntity = NULL;
    }

    // always touch the entity, even if we couldn't stand on it
    PM_TouchEntity(trace.ent);
}

 //
 //===============
 // PM_CheckWater
 // 
 // Checks for water interaction, accounting for player ducking, etc.
 //===============
 //
static void PM_CheckWater(void) {
    // Reset water levels first.
    pm->waterLevel = WATER_NONE;
    pm->waterType = 0;

    // Calculate view height aka GROUND_DIST origin position.
    vec3_t pos;// = pm->state.origin;
    // Copy the vec3_t state origin into the vec3_t pos.
    Vec3_Copy(pm->state.origin, pos);
    pos[2] = pm->state.origin[2] + pm->mins[2] + PM_GROUND_DIST;

    // Test contents.
    int32_t contents = pm->PointContents(pos);
    if (contents & CONTENTS_MASK_LIQUID) {
        // Set water FEET level.
        pm->waterType = contents;
        pm->waterLevel = WATER_FEET;

        // Reset Z position for testing.
        pos[2] = pm->state.origin[2];

        // Test contents.
        contents = pm->PointContents(pos);
        if (contents & CONTENTS_MASK_LIQUID) {
            // Set water WAIST level.
            pm->waterType |= contents;
            pm->waterLevel = WATER_WAIST;

            // Calculate the new Z.
            pos[2] = pm->state.origin[2] + pm->viewHeight + 1.0f;

            // Test contents.
            contents = pm->PointContents(pos);
            if (contents & CONTENTS_MASK_LIQUID) {
                // Set water UNDER level.
                pm->waterType |= contents;
                pm->waterLevel = WATER_UNDER;

                pm->state.flags |= PMF_UNDER_WATER;
            }
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
static void PM_ClampAngles(void)
{
    short   temp;
    int     i;

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
    AngleVectors(pm->viewAngles, pm_locals.forwardXYZ, pm_locals.rightXYZ, pm_locals.upXYZ);
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
    //if (pm->s.type == PM_SPECTATOR)
    //    return qtrue;

    // Copy over the s.origin to end and origin for trace testing.
    Vec3_Copy(pm->state.origin, origin);
    Vec3_Copy(pm->state.origin, end);

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
    // Don't test for a valid position if not wished for.
    if (!testForValid)
        return;

    // Check to see if the position is valid.
    if (PM_TestPosition())
        return;

    // Revert back to the previous origin.
    Vec3_Copy(pm_locals.previousOrigin, pm->state.origin);
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
        Vec3_Copy(pm->state.origin, pm_locals.previousOrigin);
        return;
    }
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
    pmp->qwmode = qtrue;
    pmp->watermult = 0.7f;
    pmp->maxspeed = 320;
    //pmp->upspeed = (sv_qwmod->integer > 1) ? 310 : 350;
    pmp->friction = 4;
    pmp->waterfriction = 4;
    pmp->airaccelerate = qtrue;
}
