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
// Common between server and client so prediction matches.
//
//==============================================================
//

//-----------------
// Step Climbing.
//----------------- 
constexpr float PM_STEP_HEIGHT			= 18.f; // The vertical distance afforded in step climbing.

//-----------------
// Player bounding box scaling. mins = VectorScale(PM_MINS, PM_SCALE)..
//-----------------
constexpr float PM_SCALE = 1.f;

extern const vec3_t PM_MINS;
extern const vec3_t PM_MAXS;

//
// Game-specific button hits.
//
//#define BUTTON_ATTACK		(1 << 0)
//#define BUTTON_WALK			(1 << 1)
//#define BUTTON_HOOK			(1 << 2)
//#define BUTTON_SCORE		(1 << 3)

//
// Game-specific flags for PlayerMoveState.flags.
//
// Player Move Flags.
constexpr int32_t PMF_DUCKED            = (PMF_GAME << 0);  // Player is ducked
constexpr int32_t PMF_JUMPED			= (PMF_GAME << 1);  // Player jumped
constexpr int32_t PMF_JUMP_HELD         = (PMF_GAME << 2);  // Player's jump key is down
constexpr int32_t PMF_ON_GROUND         = (PMF_GAME << 3);  // Player is on ground
constexpr int32_t PMF_ON_LADDER         = (PMF_GAME << 4);  // Player is on ladder
constexpr int32_t PMF_UNDER_WATER       = (PMF_GAME << 5);  // Player is under water
constexpr int32_t PMF_TIME_PUSHED       = (PMF_GAME << 6);  // Time before can seek ground
constexpr int32_t PMF_TIME_TRICK_JUMP   = (PMF_GAME << 7);  // Time eligible for trick jump
constexpr int32_t PMF_TIME_WATER_JUMP   = (PMF_GAME << 8);  // Time before control
constexpr int32_t PMF_TIME_LAND         = (PMF_GAME << 9); // Time before jump eligible
constexpr int32_t PMF_GIBLET            = (PMF_GAME << 10); // Player is a giblet
constexpr int32_t PMF_TIME_TRICK_START  = (PMF_GAME << 11); // Time until we can initiate a trick jump

//
// The mask of PlayerMoveState.flags affecting PlayerMoveState.time.
//
constexpr int32_t PMF_TIME_MASK = (
    PMF_TIME_PUSHED |
    PMF_TIME_TRICK_START |
    PMF_TIME_TRICK_JUMP | PMF_TIME_WATER_JUMP | PMF_TIME_LAND |
    PMF_TIME_TELEPORT
);

//
// The maximum number of entities any single player movement can impact.
//
constexpr int32_t PM_MAX_TOUCH_ENTS = 32;


//-------------------
// The player movement structure provides context management between the
// game modules and the player movement code.
// 
// (in), (out), (in/out) mark which way a variable goes. Copied in to the
// state befor processing, or copied back out of the state after processing.
//-------------------
typedef struct {
    // Movement command (in)
    ClientMoveCommand moveCommand;
    
    // Movement state (in/out)
    PlayerMoveState state;

    // Hook pull speed (in)
    float hookPullSpeed;

    // Entities touched (out)
    struct entity_s* touchedEntities[PM_MAX_TOUCH_ENTS];
    int32_t numTouchedEntities;

    // Pointer to the entity that is below the player. (out)
    struct entity_s* groundEntityPtr; 

    // Clamped, and including kick and delta (out)
    vec3_t viewAngles;

    // Bounding box size (out)
    vec3_t mins, maxs;
    
    float       step; // Traversed step height. (out)

    // Water Type (lava, slime, water), and waterLevel.
    int32_t waterType;  
    int32_t waterLevel; // Water Level (1 - 3)

    // Callback functions for collision with the world and solid entities
    trace_t (*q_gameabi Trace)(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end);
    int     (*PointContents)(const vec3_t &point);
} PlayerMove;

//
// PMove functions.
//
void PMove(PlayerMove* pmove);

#endif // PMOVE_H



