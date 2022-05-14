/***
*
*	License here.
*
*	@file
*
*	Contains the code used for parsing received Player States.
*
***/
#include "Shared/Shared.h"
#include "../HalfFloat.h"
#include "../Messaging.h"
#include "../Protocol.h"
#include "../SizeBuffer.h"

// TODO: Make this not needed, let the Game Modules supply an API for these needs.
#include "Game/Shared/Protocol.h"

// Assertion.
#include <cassert>



/**
*   @brief  Parses the delta packets of player states.
**/
void MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t extraFlags) {

    // Sanity check. 
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: no state to delta to, 'to' == nullptr", __func__);
    }

    // Clear to old value before doing any delta parsing.
    if (!from) {
	    *to = {};
    } else if (to != from) {
        *to = *from;
    }

    // Read the flag bits.
    uint32_t flags = MSG_ReadUintBase128();

    //
    // Parse movement related state data.
    // 
    // PM Type
    if (flags & PS_PM_TYPE) { 
        to->pmove.type = MSG_ReadUint8();
    }

    // Origin X Y.
    if (flags & PS_PM_ORIGIN) {
        to->pmove.origin[0] = MSG_ReadFloat();
        to->pmove.origin[1] = MSG_ReadFloat();
    }

    // Origin Z.
    if (extraFlags & EPS_M_ORIGIN2) {
        to->pmove.origin[2] = MSG_ReadFloat();
    }

    // Velocity X Y.
    if (flags & PS_PM_VELOCITY) {
        to->pmove.velocity[0] = MSG_ReadHalfFloat();
        to->pmove.velocity[1] = MSG_ReadHalfFloat();
    }

    // Velocity Z.
    if (extraFlags & EPS_M_VELOCITY2) {
        to->pmove.velocity[2] = MSG_ReadHalfFloat();
    }

    // PM Time.
    if (flags & PS_PM_TIME) { 
        to->pmove.time = MSG_ReadUint16();
    }

    // PM Flags.
    if (flags & PS_PM_FLAGS) { 
        to->pmove.flags = MSG_ReadUint16();
    }

    // PM Gravity.
    if (flags & PS_PM_GRAVITY) {
        to->pmove.gravity = MSG_ReadUint16();
    }

    // PM Delta Angles.
    if (flags & PS_PM_DELTA_ANGLES) {
	    to->pmove.deltaAngles = MSG_ReadVector3(true);
    }

    // View Offset.
    if (flags & PS_PM_VIEW_OFFSET) {
	    to->pmove.viewOffset = MSG_ReadVector3(true);
    }

    // Step offset.
    if (flags & PS_PM_STEP_OFFSET) {
	    to->pmove.stepOffset = MSG_ReadHalfFloat();
    }

    // View Angles X Y Z.
    if (flags & PS_PM_VIEW_ANGLES) {
	    to->pmove.viewAngles = MSG_ReadVector3(true);
		//to->pmove.viewAngles = {
		//	Short2FloatAngle(MSG_ReadUint16()),
		//	Short2FloatAngle(MSG_ReadUint16()),
		//	Short2FloatAngle(MSG_ReadUint16()),
		//};
    }

    //
    // Parse the rest of the player state data.
    //
    // Kick Angles.
    if (flags & PS_KICKANGLES) {
        to->kickAngles = MSG_ReadVector3(true);
    }

    // Weapon Index.
    if (flags & PS_WEAPONINDEX) {
        to->gunIndex = MSG_ReadUint8();
    }

    if (flags & PS_GUNANIMATION_TIME_START) {
	    to->gunAnimationStartTime = MSG_ReadUintBase128();
    }
    if (flags & PS_GUNANIMATION_FRAME_START) {
	    to->gunAnimationStartFrame = MSG_ReadUint16();
    }
    if (flags & PS_GUNANIMATION_FRAME_END) {
	    to->gunAnimationEndFrame = MSG_ReadUint16();
    }
    if (flags & PS_GUNANIMATION_FRAME_TIME) {
	    to->gunAnimationFrametime = MSG_ReadHalfFloat();
    }
    if (flags & PS_GUNANIMATION_LOOP_COUNT) {
	    to->gunAnimationLoopCount = MSG_ReadUint8(); 
    }
    if (flags & PS_GUNANIMATION_LOOP_FORCE) {
    	to->gunAnimationForceLoop = MSG_ReadUint8();
    }

    // Gun Offset.
    if (extraFlags & EPS_GUNOFFSET) {
        to->gunOffset = MSG_ReadVector3(true);
    }

    // Gun Angles.
    if (extraFlags & EPS_GUNANGLES) {
        to->gunAngles = MSG_ReadVector3(true);
    }

    // Blend.
    if (flags & PS_BLEND) {
        //to->blend[0] = MSG_ReadHalfFloat();
        //to->blend[1] = MSG_ReadHalfFloat();
        //to->blend[2] = MSG_ReadHalfFloat();
        //to->blend[3] = MSG_ReadHalfFloat();
		to->blend[0] = (MSG_ReadUint8() * (1.0f / 255));
		to->blend[1] = (MSG_ReadUint8() * (1.0f / 255));
		to->blend[2] = (MSG_ReadUint8() * (1.0f / 255));
		to->blend[3] = (MSG_ReadUint8() * (1.0f / 255));
    }

    // FOV.
    if (flags & PS_FOV) {
        to->fov = MSG_ReadHalfFloat();
    }

    // RDFlags.
    if (flags & PS_RDFLAGS) {
        to->rdflags = MSG_ReadIntBase128();
    }

    // Parse Stats.
    if (extraFlags & EPS_STATS) {
        int32_t statbits = MSG_ReadIntBase128();
        for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
            if (statbits & (1 << i)) {
                to->stats[i] = MSG_ReadInt16();
            }
        }
    }
}