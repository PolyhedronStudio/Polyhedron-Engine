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
static const Vector PM_ClipVelocity(const Vector &in,  const Vector &normal, float bounce) {
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
                //point.CopyToArray(trStart);
                //end.CopyToArray(trEnd);
                //mins.CopyToArray(trMins);
                //maxs.CopyToArray(trMaxs);
                trStart << point;
                trEnd << end;
                trMins << mins;
                trMaxs << maxs;
                trace_t trace = pm->Trace(trStart, trEnd, trMins, trMaxs);

//                trace_t trace = pm->Trace(start, end, mins, maxs);

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
    trStart << start;
    trEnd << end;
    trMins << mins;
    trMaxs << maxs;
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
            pm->state.velocity[2] = 0.0f;
            return true;
        }

        // If the trace succeeded, move some distance
        if (trace.fraction > 0.0f) {
            // Use the << Vector trick to copy the vec3_t.
            pm->state.origin << Vector(trace.endpos);

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
            pm->state.velocity << (Vector(pm->state.velocity) + Vector(trace.plane.normal));
            continue;
        }

        // And modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            // If velocity doesn't impact this plane, skip it
            if (Vector(pm->state.velocity) * planes[i] >= 0.0f) {
                continue;
            }

            // Slide along the plane
            Vector velocity = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

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
                    pm->state.velocity << Vector::Zero;
                    return true;
                }
            }

            // if we have fixed all interactions, try another move
            pm->state.velocity << velocity;
            break;
        }
    }

    return bump == 0;
}

//
//===============
// PM_CheckStep
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
    pm->state.origin << Vector(trace->endpos);
    
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
        pm->state.origin << Vector(stepUp.endpos);
        pm->state.velocity << Vector(vel0);

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

    // Copy the results back into the PM state.
    pm->state.origin << Vector(pm->state.origin);
    pm->state.velocity << Vector(pm->state.velocity);
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
    pm->state.velocity << Vector(pm->state.velocity).FMAF(accel_speed, dir);
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
    pm->state.velocity << Vector(pm->state.velocity).FMAF(PM_SPEED_CURRENT, current);
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
    //        Vector(Vector(pm->state.velocity) * pm->hook_pull_speed).CopyToArray(pm->state.velocity);
    //    }
    //    else {
    //        // Zero out PM State velocity.
    //        Vector::Zero.CopyToArray(pm->state.velocity);
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
            pos[2] = pm->state.origin[2] + pm->state.view_offset[2] + 1.0f;

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
// PM_CheckJump
//
// Handles ducking, adjusting both the player's bounding box and view
// offset accordingly.Players must be on the ground in order to duck.
//===============
//
static void PM_CheckDuck(void) {

    if (pm->state.type == PM_DEAD) {
        if (pm->state.flags & PMF_GIBLET) {
            pm->state.view_offset[2] = 0.0f;
        }
        else {
            pm->state.view_offset[2] = -16.0f;
        }
    }
    else {

        const bool is_ducking = pm->state.flags & PMF_DUCKED;
        const bool wants_ducking = (pm->cmd.upmove < 0) && !(pm->state.flags & PMF_ON_LADDER);

        if (!is_ducking && wants_ducking) {
            pm->state.flags |= PMF_DUCKED;
        }
        else if (is_ducking && !wants_ducking) {
            trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->state.origin, pm->mins, pm->maxs);

            if (!trace.allsolid && !trace.startsolid) {
                pm->state.flags &= ~PMF_DUCKED;
            }
        }

        const float height = pm->maxs[2] - pm->mins[2];

        if (pm->state.flags & PMF_DUCKED) { // ducked, reduce height
            const float target = pm->mins[2] + height * 0.5f;

            if (pm->state.view_offset[2] > target) { // go down
                pm->state.view_offset[2] -= pm_locals.time * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.view_offset[2] < target) {
                pm->state.view_offset[2] = target;
            }

            // change the bounding box to reflect ducking
            pm->maxs[2] = pm->maxs[2] + pm->mins[2] * 0.5f;
        }
        else {
            const float target = pm->mins[2] + height * 0.9f;

            if (pm->state.view_offset[2] < target) { // go up
                pm->state.view_offset[2] += pm_locals.time * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.view_offset[2] > target) {
                pm->state.view_offset[2] = target;
            }
        }
    }

    // ??
    //pm->s.view_offset = pm->s.view_offset;
}

//
//===============
// PM_CheckJump
//
// Check for jumping and trick jumping.
// 
// Returns True if a jump occurs, false otherwise.
//===============
//
static bool PM_CheckJump(void) {
    if (PM_CheckHookJump()) {
        return true;
    }

    // Must wait for landing damage to subside
    if (pm->state.flags & PMF_TIME_LAND) {
        return false;
    }

    // Must wait for jump key to be released
    if (pm->state.flags & PMF_JUMP_HELD) {
        return false;
    }

    // Didn't ask to jump
    if (pm->cmd.upmove < 1) {
        return false;
    }

    // Finally, do the jump
    float jump = PM_SPEED_JUMP;

    // Factoring in water level
    if (pm->waterLevel > WATER_FEET) {
        jump *= PM_SPEED_JUMP_MOD_WATER;
    }

    // Adding the trick jump if eligible
    if (pm->state.flags & PMF_TIME_TRICK_JUMP) {
        jump += PM_SPEED_TRICK_JUMP;

        pm->state.flags &= ~PMF_TIME_TRICK_JUMP;
        pm->state.time = 0;

        Com_DPrintf("Trick jump: %d\n", pm->cmd.upmove);
    }
    else {
        Com_DPrintf("Jump: %d\n", pm->cmd.upmove);
    }

    // If velocity is below zero, make it JUMP.
    if (pm->state.velocity[2] < 0.0f) {
        pm->state.velocity[2] = jump;
    }
    else {
        pm->state.velocity[2] += jump;
    }

    // Indicate that jump is currently held
    pm->state.flags |= (PMF_JUMPED | PMF_JUMP_HELD);

    // clear the ground indicators
    pm->state.flags &= ~PMF_ON_GROUND;
    pm->groundEntity = NULL;

    // we can trick jump soon
    pm->state.flags |= PMF_TIME_TRICK_START;
    pm->state.time = 100;

    return true;
}

//
//===============
// PM_CheckLadder
//
// Check for ladder interaction.
//
// Returns True if the player is on a ladder, false otherwise.
//===============
//
static bool PM_CheckLadder(void) {
    // Don't check for ladders if time mask is on.
    if (pm->state.flags & PMF_TIME_MASK) {
        return false;
    }

    // Don't check for ladders if in a hook pull.
    if (pm->state.type == PM_HOOK_PULL) {
        return false;
    }

    // Project desired destination
    Vector position = Vector(pm->state.origin).FMAF(4.f, pm_locals.forwardXY);

    // Execute trace.
    trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, position, pm->mins, pm->maxs);

    // See if we found a CONTENST_LADDER.
    if ((trace.fraction < 1.0f) && (trace.contents & CONTENTS_LADDER)) {
        pm->state.flags |= PMF_ON_LADDER;

        pm->groundEntity = NULL;
        pm->state.flags &= ~(PMF_ON_GROUND | PMF_DUCKED);

        return true;
    }

    // No ladder found.
    return false;
}

//
//===============
// PM_CheckWaterJump
//
// Checks for water exit. The player may exit the water when they can
// see a usable step out of the water.
//
// Returns True if a water jump has occurred, false otherwise.
//===============
//
static bool PM_CheckWaterJump(void) {

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

    // Project desired destination
    Vector position = Vector(pm->state.origin).FMAF(16.f, pm_locals.forwardXYZ);
    trace_t trace = PM_TraceCorrectAllSolid(pm->state.origin, position, pm->mins, pm->maxs);

    if ((trace.fraction < 1.0f) && (trace.contents & CONTENTS_MASK_SOLID)) {
        // Trace the step height + bounding box height.
        position.z += PM_STEP_HEIGHT + pm->maxs[2] - pm->mins[2];
        trace = PM_TraceCorrectAllSolid(position, position, pm->mins, pm->maxs);

        if (trace.startsolid) {
            Com_DPrintf("Can't exit water: blocked\n");
            return false;
        }

        // Trace.
        Vector position2 = Vector(position.x, position.y, pm->state.origin[2]);
        trace = PM_TraceCorrectAllSolid(position, position2, pm->mins, pm->maxs);

        // Couldn't exist water if the result is higher than PM_STEP_NORMAL
        if (!(trace.ent && trace.plane.normal[2] >= PM_STEP_NORMAL)) {
            Com_DPrintf("Can't exit water: not a step\n");
            return false;
        }

        // Jump out of water
        pm->state.velocity[2] = PM_SPEED_WATER_JUMP;

        pm->state.flags |= PMF_TIME_WATER_JUMP | PMF_JUMP_HELD;
        pm->state.time = 2000;

        return true;
    }

    return false;
}

//
//===============
// PM_LadderMove
//
//===============
//
static void PM_LadderMove(void) {

    //Com_DPrintf("%s\n", vtos(pm->state.origin));

    //PM_Friction();

    //PM_Currents();

    //// user intentions in X/Y
    //vec3_t vel = Vec3_Zero();
    //vel = Vec3_Fmaf(vel, pm->cmd.forward, pm_locals.forward_xy);
    //vel = Vec3_Fmaf(vel, pm->cmd.right, pm_locals.right_xy);

    //const float s = PM_SPEED_LADDER * 0.125f;

    //// limit horizontal speed when on a ladder
    //vel.x = Clampf(vel.x, -s, s);
    //vel.y = Clampf(vel.y, -s, s);
    //vel.z = 0.f;

    //// handle Z intentions differently
    //if (fabsf(pm->state.velocity.z) < PM_SPEED_LADDER) {

    //    if ((pm->viewAngles[0] <= -15.0f) && (pm->cmd.forward > 0)) {
    //        vel.z = PM_SPEED_LADDER;
    //    }
    //    else if ((pm->viewAngles[0] >= 15.0f) && (pm->cmd.forward > 0)) {
    //        vel.z = -PM_SPEED_LADDER;
    //    }
    //    else if (pm->cmd.up > 0) {
    //        vel.z = PM_SPEED_LADDER;
    //    }
    //    else if (pm->cmd.up < 0) {
    //        vel.z = -PM_SPEED_LADDER;
    //    }
    //    else {
    //        vel.z = 0.0;
    //    }
    //}

    //if (pm->cmd.up > 0) { // avoid jumps when exiting ladders
    //    pm->state.flags |= PMF_JUMP_HELD;
    //}

    //float speed;
    //const vec3_t dir = Vec3_NormalizeLength(vel, &speed);
    //speed = Clampf(speed, 0.0, PM_SPEED_LADDER);

    //if (speed < PM_STOP_EPSILON) {
    //    speed = 0.0;
    //}

    //PM_Accelerate(dir, speed, PM_ACCEL_LADDER);

    //PM_StepSlideMove();
}

//
//===============
// PM_WaterJumpMove
//
//===============
//
static void PM_WaterJumpMove(void) {
    //Com_DPrintf("%s\n", vtos(pm->state.origin));

    //PM_Friction();
    //PM_Gravity();

    //// check for a usable spot directly in front of us
    //const vec3_t pos = Vec3_Fmaf(pm->state.origin, 30.f, pm_locals.forward_xy);

    //// if we've reached a usable spot, clamp the jump to avoid launching
    //if (PM_TraceCorrectAllSolid(pm->state.origin, pos, pm->mins, pm->maxs).fraction == 1.0f) {
    //    pm->state.velocity.z = Clampf(pm->state.velocity.z, 0.f, PM_SPEED_JUMP);
    //}

    //// if we're falling back down, clear the timer to regain control
    //if (pm->state.velocity.z <= 0.0f) {
    //    pm->state.flags &= ~PMF_TIME_MASK;
    //    pm->state.time = 0;
    //}

    //PM_StepSlideMove();
}

//
//===============
// PM_WaterJumpMove
//
//===============
//
static void PM_WaterMove(void) {

    //if (PM_CheckWaterJump()) {
    //    PM_WaterJumpMove();
    //    return;
    //}

    //Com_DPrintf("%s\n", vtos(pm->state.origin));

    //// apply friction, slowing rapidly when first entering the water
    //PM_Friction();

    //// and sink if idle
    //if (!pm->cmd.forward && !pm->cmd.right && !pm->cmd.up && pm->state.type != PM_HOOK_PULL && pm->state.type != PM_HOOK_SWING) {
    //    if (pm->state.velocity.z > PM_SPEED_WATER_SINK) {
    //        PM_Gravity();
    //    }
    //}

    //PM_Currents();

    //// user intentions on X/Y/Z
    //vec3_t vel = Vec3_Zero();
    //vel = Vec3_Fmaf(vel, pm->cmd.forward, pm_locals.forward);
    //vel = Vec3_Fmaf(vel, pm->cmd.right, pm_locals.right);

    //// add explicit Z
    //vel.z += pm->cmd.up;

    //// disable water skiing
    //if (pm->state.type != PM_HOOK_PULL && pm->state.type != PM_HOOK_SWING) {
    //    if (pm->waterLevel == WATER_WAIST) {
    //        vec3_t view = Vec3_Add(pm->state.origin, pm->state.view_offset);
    //        view.z -= 4.0;

    //        if (!(pm->PointContents(view) & CONTENTS_MASK_LIQUID)) {
    //            pm->state.velocity.z = Minf(pm->state.velocity.z, 0.0);
    //            vel.z = Minf(vel.z, 0.0);
    //        }
    //    }
    //}

    //float speed;
    //const vec3_t dir = Vec3_NormalizeLength(vel, &speed);
    //speed = Clampf(speed, 0, PM_SPEED_WATER);

    //if (speed < PM_STOP_EPSILON) {
    //    speed = 0.0;
    //}

    //PM_Accelerate(dir, speed, PM_ACCEL_WATER);

    //if (pm->cmd.up > 0) {
    //    PM_SlideMove();
    //}
    //else {
    //    PM_StepSlideMove();
    //}
}

//
//===============
// PM_AirMove
//
//===============
//
static void PM_AirMove(void) {
//    Com_DPrintf("%s\n", vtos(pm->state.origin));

    // Execute friction forces.
    PM_Friction();

    // Execute gravity forces.
    PM_Gravity();

    // Calculate velocity
    Vector velocity = Vector::Zero;
    velocity = velocity.FMAF(pm->cmd.forwardmove, pm_locals.forwardXY);
    velocity = velocity.FMAF(pm->cmd.sidemove, pm_locals.rightXY);

    // Set Z explicitly to 0.
    velocity.z = 0.f;

    // Air speed.
    float maxSpeed = PM_SPEED_AIR;

    // accounting for walk modulus
    if (pm->cmd.buttons & BUTTON_WALK) {
        maxSpeed *= PM_SPEED_MOD_WALK;
    }
 
    // Calculate speed.
    Vector direction = velocity.Normalized();
    float speed = direction.Length();
    speed = clamp(speed, 0.0, maxSpeed);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    // Check if ducking has to slow down our acceleration.
    float accel = PM_ACCEL_AIR;

    if (pm->state.flags & PMF_DUCKED) {
        accel *= PM_ACCEL_AIR_MOD_DUCKED;
    }

    // Accelerate.
    PM_Accelerate(direction, speed, accel);

    // Step slide move.
    PM_StepSlideMove();
}

//
//===============
// PM_WalkMove
// Called for movements where player is on ground, regardless of water level.
//===============
//
static void PM_WalkMove(void) {

    //// check for beginning of a jump
    //if (PM_CheckJump()) {
    //    PM_AirMove();
    //    return;
    //}

    //Com_DPrintf("%s\n", vtos(pm->state.origin));

    //PM_Friction();

    //PM_Currents();

    //// project the desired movement into the X/Y plane

    //const vec3_t forward = Vec3_Normalize(Pm_ClipVelocity(pm_locals.forward_xy, pm_locals.ground.plane.normal, PM_CLIP_BOUNCE));
    //const vec3_t right = Vec3_Normalize(Pm_ClipVelocity(pm_locals.right_xy, pm_locals.ground.plane.normal, PM_CLIP_BOUNCE));

    //vec3_t vel = Vec3_Zero();
    //vel = Vec3_Fmaf(vel, pm->cmd.forward, forward);
    //vel = Vec3_Fmaf(vel, pm->cmd.right, right);

    //float max_speed;

    //// clamp to max speed
    //if (pm->water_level > WATER_FEET) {
    //    max_speed = PM_SPEED_WATER;
    //}
    //else if (pm->state.flags & PMF_DUCKED) {
    //    max_speed = PM_SPEED_DUCKED;
    //}
    //else {
    //    max_speed = PM_SPEED_RUN;
    //}

    //// accounting for walk modulus
    //if (pm->cmd.buttons & BUTTON_WALK) {
    //    max_speed *= PM_SPEED_MOD_WALK;
    //}

    //// clamp the speed to min/max speed
    //float speed;
    //const vec3_t dir = Vec3_NormalizeLength(vel, &speed);
    //speed = Clampf(speed, 0.0, max_speed);

    //if (speed < PM_STOP_EPSILON) {
    //    speed = 0.0;
    //}

    //// accelerate based on slickness of ground surface
    //const float accel = (pm_locals.ground.texinfo->flags & SURF_SLICK) ? PM_ACCEL_GROUND_SLICK : PM_ACCEL_GROUND;

    //PM_Accelerate(dir, speed, accel);

    //// determine the speed after acceleration
    //speed = Vec3_Length(pm->state.velocity);

    //// clip to the ground
    //pm->state.velocity = PM_ClipVelocity(pm->state.velocity, pm_locals.ground.plane.normal, PM_CLIP_BOUNCE);

    //// and now scale by the speed to avoid slowing down on slopes
    //pm->state.velocity = Vec3_Normalize(pm->state.velocity);
    //pm->state.velocity = Vec3_Scale(pm->state.velocity, speed);

    //// and finally, step if moving in X/Y
    //if (pm->state.velocity.x || pm->state.velocity.y) {
    //    PM_StepSlideMove();
    //}
}

//
//===============
// PM_SpectatorMove
// 
//===============
//
static void PM_SpectatorMove(void) {
    // Apply standard friction.
    PM_Friction();

    // User intentions on X/Y/Z
    Vector velocity = Vector::Zero;
    velocity = velocity.FMAF(pm->cmd.forwardmove, pm_locals.forwardXYZ);
    velocity = velocity.FMAF(pm->cmd.sidemove, pm_locals.rightXYZ);

    // Add explicit Z
    velocity.z += pm->cmd.upmove;

    // Calculate speed.
    velocity.Normalize();
    float speed = velocity.Length();
    velocity *= 1.0 / speed;
//    velocity = Vec3_NormalizeLength(velocity, &speed);
    speed = clamp(speed, 0.0, PM_SPEED_SPECTATOR);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    // Accelerate
    PM_Accelerate(velocity, speed, PM_ACCEL_SPECTATOR);

    // Do the move
    pm->state.origin << Vector(pm->state.origin).FMAF(pm_locals.time, pm->state.velocity);
}
/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
static char* vtos(vec3_t v)
{
    static  int     index;
    static  char    str[8][32];
    char* s;

    // use an array so that multiple vtos won't collide
    s = str[index];
    index = (index + 1) & 7;

    Q_snprintf(s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

    return s;
}
//
//===============
// PM_SpectatorMove
// 
//===============
//
static void PM_FreezeMove(void) {

    Com_DPrintf("%s\n", vtos(pm->state.origin));
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
    //AngleVectors(pm->viewAngles, pm_locals.forwardXYZ, pm_locals.rightXYZ, pm_locals.upXYZ);
}

//
//===============
// PM_Init
// 
//===============
//
static void PM_Init(void) {
    // Set the default bounding box
    if (pm->state.type == PM_DEAD) {
        if (pm->state.flags & PMF_GIBLET) {
            // Use Vector << to  copy PM_GIBLET_MINS, PM_GIBLET_MAXS into
            // the pm->mins/maxs vec3_t.
            pm->mins << Vector(PM_GIBLET_MINS);
            pm->maxs << Vector(PM_GIBLET_MAXS);
        } else {
            // Use Vector << to  copy PM_DEAD_MINS, PM_DEAD_MAXS into
            // the pm->mins/maxs vec3_t.
            pm->mins << Vector(PM_DEAD_MINS) * PM_SCALE;
            pm->maxs << Vector(PM_DEAD_MAXS) * PM_SCALE;
        }
    } else {
        // Use Vector << to  copy PM_MINS, PM_MAXS into
        // the pm->mins/maxs vec3_t.
        pm->mins << Vector(PM_MINS) * PM_SCALE;
        pm->maxs << Vector(PM_MAXS) * PM_SCALE;
    }

    
    // Initialize pm values to 0.
    pm->viewAngles << Vector::Zero; // Vector << trick to initialize angles to 0.

    pm->numTouchedEntities = 0;
    pm->waterLevel = WATER_NONE;
    pm->waterType = 0;

    pm->step = 0.0;

    // Reset flags that we test each move
    pm->state.flags &= ~(PMF_ON_GROUND | PMF_ON_STAIRS | PMF_ON_LADDER);
    pm->state.flags &= ~(PMF_JUMPED | PMF_UNDER_WATER);

    if (pm->cmd.upmove < 1) { // jump key released
        pm->state.flags &= ~PMF_JUMP_HELD;
    }

    // Decrement the movement timer by the duration of the command
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
static void PM_InitLocal(void) {

    memset(&pm_locals, 0, sizeof(pm_locals));

    // save previous values in case move fails, and to detect landings
    pm_locals.previousOrigin << Vector(pm->state.origin);
    pm_locals.previousVelocity << Vector(pm->state.velocity);

    // convert from milliseconds to seconds
    pm_locals.time = pm->cmd.msec * 0.001;

    // calculate the directional vectors for this move
    AngleVectors(pm->viewAngles, pm_locals.forwardXYZ, pm_locals.rightXYZ, pm_locals.upXYZ);

    // and calculate the directional vectors in the XY plane
    vec3_t angle = { 0, pm->viewAngles[1], 0 };

    AngleVectors(angle, pm_locals.forwardXY, pm_locals.rightXY, NULL);
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
    pm = pmove;
    pmp = params;

    // Initialize PMove for this iteration.
    PM_Init();

    // Clamp Angles.
    PM_ClampAngles();

    // Init PM locals.
    PM_InitLocal();

    // No movement if FROZEN.
    if (pm->state.type == PM_FREEZE) {
        PM_FreezeMove();
        return;
    }

    // No interaction if spectating.
    //if (pm->state.type == PM_SPECTATOR) { 
        PM_SpectatorMove();
        return;
    //}

    // No user input control if dead.
    if (pm->state.type == PM_DEAD) { 
        pm->cmd.forwardmove = pm->cmd.sidemove = pm->cmd.upmove = 0;
    }

    // Check for ladders
    PM_CheckLadder();

    // Check for grapple hook
    PM_CheckHook();

    // Check for ducking
    PM_CheckDuck();

    // Check for water level, water type
    PM_CheckWater();

    // Check for ground
    PM_CheckGround();

    //if (pm->state.flags & PMF_TIME_TELEPORT) {
    //    // pause in place briefly
    //}
    //else if (pm->state.flags & PMF_TIME_WATER_JUMP) {
    //    PM_WaterJumpMove();
    //}
    //else if (pm->state.flags & PMF_ON_LADDER) {
    //    PM_LadderMove();
    //}
    //else if (pm->state.flags & PMF_ON_GROUND) {
     //   PM_WalkMove();
    //}
    //else if (pm->waterLevel > WATER_FEET) {
    //    PM_WaterMove();
    //}
    //else {
    //    PM_AirMove();
    //}

    // Check for ground at new spot
    PM_CheckGround();

    // Check for water level, water type at new spot
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
