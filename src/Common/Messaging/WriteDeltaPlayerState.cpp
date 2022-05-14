/***
*
*	License here.
*
*	@file
*
*	Contains the code used for writing Delta Player States.
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

// PlayerState BaseLine.
static const PlayerState       nullPlayerState = {};


/**
*   @brief Writes the delta player state.
**/
int MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t playerStateMessageFlags)
{
    // Sanity check.
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: NULL", __func__);
    }

    // Derive 'from' from nullPlayerState if we ain't got one.
    if (!from) {
        from = &nullPlayerState;
    }

    //
    // Determine what needs to be sent
    //
    uint32_t playerStateFlags = 0;
    uint32_t entityStateFlags = 0;

    if (to->pmove.type != from->pmove.type) {
        playerStateFlags |= PS_PM_TYPE;
    }

    if (!EqualEpsilonf(to->pmove.origin[0], from->pmove.origin[0]) ||
        !EqualEpsilonf(to->pmove.origin[1], from->pmove.origin[1])) {
        playerStateFlags |= PS_PM_ORIGIN;
    }

    if (!EqualEpsilonf(to->pmove.origin[2], from->pmove.origin[2])) {
        entityStateFlags |= EPS_M_ORIGIN2;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_PREDICTION)) {
        if (!EqualEpsilonf(to->pmove.velocity[0], from->pmove.velocity[0]) ||
            !EqualEpsilonf(to->pmove.velocity[1], from->pmove.velocity[1])) {
            playerStateFlags |= PS_PM_VELOCITY;
        }

        if (!EqualEpsilonf(to->pmove.velocity[2], from->pmove.velocity[2])) { 
            entityStateFlags |= EPS_M_VELOCITY2;
        }

        if (to->pmove.time != from->pmove.time) {
            playerStateFlags |= PS_PM_TIME;
        }

        if (to->pmove.flags != from->pmove.flags) {
            playerStateFlags |= PS_PM_FLAGS;
        }

        if (to->pmove.gravity != from->pmove.gravity) {
            playerStateFlags |= PS_PM_GRAVITY;
        }
    } else {
        // Save previous state.
	    to->pmove.velocity = from->pmove.velocity;
        to->pmove.time = from->pmove.time;
        to->pmove.flags = from->pmove.flags;
        to->pmove.gravity = from->pmove.gravity;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_DELTAANGLES)) {
        if (!EqualEpsilonf(to->pmove.deltaAngles.x, from->pmove.deltaAngles.x) ||
            !EqualEpsilonf(to->pmove.deltaAngles.y, from->pmove.deltaAngles.y) ||
            !EqualEpsilonf(to->pmove.deltaAngles.z, from->pmove.deltaAngles.z))
            playerStateFlags |= PS_PM_DELTA_ANGLES;
    } else {
        // Save previous state.
	    to->pmove.deltaAngles = from->pmove.deltaAngles;
    }

    if (!EqualEpsilonf(to->pmove.viewOffset.x, from->pmove.viewOffset.x) ||
        !EqualEpsilonf(to->pmove.viewOffset.y, from->pmove.viewOffset.y) ||
        !EqualEpsilonf(to->pmove.viewOffset.z, from->pmove.viewOffset.z)) {
        playerStateFlags |= PS_PM_VIEW_OFFSET;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_VIEWANGLES)) {
        if (!EqualEpsilonf(to->pmove.viewAngles.x, from->pmove.viewAngles.x) ||
            !EqualEpsilonf(to->pmove.viewAngles.y, from->pmove.viewAngles.y) ||
            !EqualEpsilonf(to->pmove.viewAngles.z, from->pmove.viewAngles.z)) {
            playerStateFlags |= PS_PM_VIEW_ANGLES;
        }
    } else {
        // save previous state
        to->pmove.viewAngles[0] = from->pmove.viewAngles[0];
        to->pmove.viewAngles[1] = from->pmove.viewAngles[1];
        to->pmove.viewAngles[2] = from->pmove.viewAngles[2];
    }

    if (from->kickAngles[0] != to->kickAngles[0] ||
        from->kickAngles[1] != to->kickAngles[1] ||
        from->kickAngles[2] != to->kickAngles[2]) {
        playerStateFlags |= PS_KICKANGLES;
    }

    if (from->blend[0] != to->blend[0] ||
        from->blend[1] != to->blend[1] ||
        from->blend[2] != to->blend[2] ||
        from->blend[3] != to->blend[3]) {
        playerStateFlags |= PS_BLEND;
    }

    if (from->pmove.stepOffset != to->pmove.stepOffset) {
	    playerStateFlags |= PS_PM_STEP_OFFSET;
    }

    if (from->fov != to->fov) {
	    playerStateFlags |= PS_FOV;
    }

    if (to->rdflags != from->rdflags) {
	    playerStateFlags |= PS_RDFLAGS;
    }

    if (to->gunIndex != from->gunIndex) {
	    playerStateFlags |= PS_WEAPONINDEX;
    }

    if (to->gunAnimationStartTime != from->gunAnimationStartTime) {
	    playerStateFlags |= PS_GUNANIMATION_TIME_START;
    }
    if (to->gunAnimationStartFrame != from->gunAnimationStartFrame) {
        playerStateFlags |= PS_GUNANIMATION_FRAME_START;
    }
    if (to->gunAnimationEndFrame != from->gunAnimationEndFrame) {
	    playerStateFlags |= PS_GUNANIMATION_FRAME_END;
    }
    if (!EqualEpsilonf(to->gunAnimationFrametime, from->gunAnimationFrametime)) {
        playerStateFlags |= PS_GUNANIMATION_FRAME_TIME;
    }
    if (to->gunAnimationLoopCount != from->gunAnimationLoopCount) {
	    playerStateFlags |= PS_GUNANIMATION_LOOP_COUNT;
    }
    if (to->gunAnimationForceLoop != from->gunAnimationForceLoop) {
	    playerStateFlags |= PS_GUNANIMATION_LOOP_FORCE;
    }


    if (from->gunOffset[0] != to->gunOffset[0] ||
        from->gunOffset[1] != to->gunOffset[1] ||
        from->gunOffset[2] != to->gunOffset[2]) {
        entityStateFlags |= EPS_GUNOFFSET;
    }

    if (from->gunAngles[0] != to->gunAngles[0] ||
        from->gunAngles[1] != to->gunAngles[1] ||
        from->gunAngles[2] != to->gunAngles[2]) {
        entityStateFlags |= EPS_GUNANGLES;
    }

    int32_t statbits = 0;
    for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
	    if (to->stats[i] != from->stats[i]) {
	        statbits |= 1 << i;
        }
    }

    if (statbits) {
	    entityStateFlags |= EPS_STATS;
    }

    //
    // write it
    //
    MSG_WriteUintBase128(playerStateFlags);

    //
    // write the PlayerMoveState
    //
    if (playerStateFlags & PS_PM_TYPE) {
        MSG_WriteUint8(to->pmove.type);
    }

    if (playerStateFlags & PS_PM_ORIGIN) {
        MSG_WriteFloat(to->pmove.origin[0]);
        MSG_WriteFloat(to->pmove.origin[1]);
    }

    if (entityStateFlags & EPS_M_ORIGIN2) {
	    MSG_WriteFloat(to->pmove.origin[2]);
    }

    if (playerStateFlags & PS_PM_VELOCITY) {
        MSG_WriteHalfFloat(to->pmove.velocity[0]);
        MSG_WriteHalfFloat(to->pmove.velocity[1]);
    }

    if (entityStateFlags & EPS_M_VELOCITY2) {
	    MSG_WriteHalfFloat(to->pmove.velocity[2]);
    }

    if (playerStateFlags & PS_PM_TIME) {
	    MSG_WriteUint16(to->pmove.time);
    }

    if (playerStateFlags & PS_PM_FLAGS) {
	    MSG_WriteUint16(to->pmove.flags);
    }

    if (playerStateFlags & PS_PM_GRAVITY) {
	    MSG_WriteUint16(to->pmove.gravity);
    }

    if (playerStateFlags & PS_PM_DELTA_ANGLES) {
        MSG_WriteVector3(to->pmove.deltaAngles, true);
    }

    //
    // write the rest of the PlayerState
    //
    if (playerStateFlags & PS_PM_VIEW_OFFSET) {
        MSG_WriteVector3(to->pmove.viewOffset, true);
    }

    if (playerStateFlags & PS_PM_STEP_OFFSET) {
        MSG_WriteHalfFloat(to->pmove.stepOffset);
    }

    if (playerStateFlags & PS_PM_VIEW_ANGLES) {
		//MSG_WriteUint16(FloatAngle2Short(to->pmove.viewAngles.x));
		//MSG_WriteUint16(FloatAngle2Short(to->pmove.viewAngles.y));
		//MSG_WriteUint16(FloatAngle2Short(to->pmove.viewAngles.z));
        MSG_WriteVector3(to->pmove.viewAngles, true);
    }

    if (playerStateFlags & PS_KICKANGLES) {
        MSG_WriteVector3(to->kickAngles, true);
    }

    if (playerStateFlags & PS_WEAPONINDEX) {
	    MSG_WriteUint8(to->gunIndex);
    }

    if (playerStateFlags & PS_GUNANIMATION_TIME_START) {
	    MSG_WriteUintBase128(to->gunAnimationStartTime);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_START) {
	    MSG_WriteUint16(to->gunAnimationStartFrame);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_END) {
	    MSG_WriteUint16(to->gunAnimationEndFrame);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_TIME) {
	    MSG_WriteHalfFloat(to->gunAnimationFrametime);
    }
    if (playerStateFlags & PS_GUNANIMATION_LOOP_COUNT) {
	    MSG_WriteUint8(to->gunAnimationLoopCount);
    }
    if (playerStateFlags & PS_GUNANIMATION_LOOP_FORCE) {
	    MSG_WriteUint8(to->gunAnimationForceLoop);
    }

    if (entityStateFlags & EPS_GUNOFFSET) {
        MSG_WriteVector3(to->gunOffset, true);
    }

    if (entityStateFlags & EPS_GUNANGLES) {
        MSG_WriteVector3(to->gunAngles, true);
    }

    if (playerStateFlags & PS_BLEND) {
        //MSG_WriteVector4(to->blend, true);
		MSG_WriteUint8(to->blend[0] * 255);
		MSG_WriteUint8(to->blend[1] * 255);
		MSG_WriteUint8(to->blend[2] * 255);
		MSG_WriteUint8(to->blend[3] * 255);
    }
  
    if (playerStateFlags & PS_FOV) {
	    MSG_WriteHalfFloat(to->fov);
    }

    if (playerStateFlags & PS_RDFLAGS) {
        MSG_WriteIntBase128(to->rdflags);
    }

    // Send stats
    if (entityStateFlags & EPS_STATS) {
        MSG_WriteIntBase128(statbits);
	    for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
	        if (statbits & (1 << i)) {
    		    MSG_WriteInt16(to->stats[i]);
	        }
	    }
    }

    return entityStateFlags;
}
