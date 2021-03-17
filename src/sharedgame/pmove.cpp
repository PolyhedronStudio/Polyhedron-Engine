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

        // The ground texinfo.
        csurface_t* texInfo;

        // The ground contents.
        int32_t contents;
    } ground;
} pm_locals;

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
