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
static pmove_t* pm;         // Pointer to the actual player move structure.

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
    AngleVectors(pm->viewAngles, pml.forward, pml.right, pml.up);
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
    int     i;

    // This check is not needed anymore. Whether to test for a position or not
    // can now be decided by calling PM_FinalizePosition with true as its arg. 
    //if (pm->s.type == PM_SPECTATOR)
    //    return qtrue;

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
    // Copy over velocity and origin.
    VectorCopy(pml.velocity, pm->state.velocity);
    VectorCopy(pml.origin, pm->state.origin);

    // Don't test for a valid position if not wished for.
    if (!testForValid)
        return;

    // Check to see if the position is valid.
    if (PM_TestPosition())
        return;

    // Revert back to the previous origin.
    VectorCopy(pml.previous_origin, pm->state.origin);
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
//===============
// PMove
// 
// Can be called by either the server or the client
//===============
//
void PMove(pmove_t* pmove, pmoveParams_t* params)
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
