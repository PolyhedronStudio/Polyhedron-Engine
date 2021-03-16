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

#ifndef SHAREDGAME_PMOVE_H
#define SHAREDGAME_PMOVE_H

// Shared include, we need it :)
#include "shared/shared.h"

//
//==============================================================
//
// Player Movement(PM) Code.
//
// Common between server and client so prediction matches
//
//==============================================================
//

//
// The server has a copy of this struct.
//
typedef struct {
    qboolean    qwmode;
    qboolean    airaccelerate;
    qboolean    strafehack;
    qboolean    flyhack;
    qboolean    waterhack;
    float       speedmult;
    float       watermult;
    float       maxspeed;
    float       friction;
    float       waterfriction;
    float       flyfriction;
} pmoveParams_t;

// PMOVE: Old PMove code.
#define MAXTOUCH    32
typedef struct {
    // IN/OUT variables.
    pm_state_t   s;  // Player Move State.

    // IN variables.
    usercmd_t       cmd;            // User input command.
    qboolean        testInitial;    // If .s has changed outside of pmove, testInitial is true.

    // OUT(results) variables.
    int             numtouch;               // Number of touched entities.
    struct edict_s* touchents[MAXTOUCH];   // Pointers to touched entities.

    struct edict_s* groundentity; // Pointer to the entity that is below the player.

    float       step; // Traversed step height.

    vec3_t      viewangles; // Clamped View Angles
    float       viewheight; // Viewheight.

    vec3_t      mins, maxs; // Bounding box size

    int         watertype;  // Water Type.
    int         waterlevel; // Water Level (1 - 3)

    // Callback function pointers for testing the world with.
    trace_t(*q_gameabi trace)(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);
    int         (*pointcontents)(vec3_t point);
} pmove_t;

void PMove(pmove_t* pmove, pmoveParams_t* params);

void PMoveInit(pmoveParams_t* pmp);
void PMoveEnableQW(pmoveParams_t* pmp);

#endif // PMOVE_H
