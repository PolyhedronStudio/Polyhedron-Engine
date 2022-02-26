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

#include "Shared/Shared.h"
#include "Common/Msg.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"
#include "SharedGame/Protocol.h"



//===========================================================================//
// (Antique) Q2-Pro MSG Style.
//===========================================================================//
/*
==============================================================================

            MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

SizeBuffer   msg_write;
byte        msg_write_buffer[MAX_MSGLEN];

SizeBuffer   msg_read;
byte        msg_read_buffer[MAX_MSGLEN];

const EntityState       nullEntityState = {};
const PlayerState       nullPlayerState = {};
const ClientMoveCommand nullUserCmd = {};

/*
=============
MSG_Init

Initialize default buffers, clearing allow overflow/underflow flags.

This is the only place where writing buffer is initialized. Writing buffer is
never allowed to overflow.

Reading buffer is reinitialized in many other places. Reinitializing will set
the allow underflow flag as appropriate.
=============
*/
void MSG_Init(void)
{
    SZ_TagInit(&msg_read, msg_read_buffer, MAX_MSGLEN, SZ_MSG_READ);
    SZ_TagInit(&msg_write, msg_write_buffer, MAX_MSGLEN, SZ_MSG_WRITE);
}


/*
==============================================================================

            WRITING

==============================================================================
*/

/*
=============
MSG_BeginWriting
=============
*/
void MSG_BeginWriting(void)
{
    msg_write.currentSize = 0;
    msg_write.bitPosition = 0;
    msg_write.overflowed = false;
}

//
//===============
// MSG_WriteChar
// 
//===============
//
void MSG_WriteChar(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < -128 || c > 127)
        Com_Error(ERR_FATAL, "MSG_WriteChar: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 1); // CPP: Cast
    buf[0] = c;
}

//
//===============
// MSG_WriteByte
// 
//===============
//
void MSG_WriteByte(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < 0 || c > 255)
        Com_Error(ERR_FATAL, "MSG_WriteByte: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 1); // CPP: Cast
    buf[0] = c;
}

//
//===============
// MSG_WriteShort
// 
//===============
//
void MSG_WriteShort(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < ((short)0x8000) || c >(short)0x7fff)
        Com_Error(ERR_FATAL, "MSG_WriteShort: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 2); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = c >> 8;
}

//
//===============
// MSG_WriteLong
// 
//===============
//
void MSG_WriteLong(int c)
{
    byte* buf;

    buf = (byte*)SZ_GetSpace(&msg_write, 4); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = (c >> 8) & 0xff;
    buf[2] = (c >> 16) & 0xff;
    buf[3] = c >> 24;
}

//
//===============
// MSG_WriteFloat
// 
// The idea is smart and taken from Quetoo, use an union for memory mapping.
// Write the float as an int32_t, use it after reading as a float.
//================
//
void MSG_WriteFloat(float c) {
    msg_float vec;
    vec.f = c;
    MSG_WriteLong(vec.i);
}

//
//===============
// MSG_WriteString
// 
//===============
//
void MSG_WriteString(const char* string)
{
    size_t length;

    if (!string) {
        MSG_WriteByte(0);
        return;
    }

    length = strlen(string);
    if (length >= MAX_NET_STRING) {
        Com_WPrintf("%s: overflow: %" PRIz " chars", __func__, length);
        MSG_WriteByte(0);
        return;
    }

    MSG_WriteData(string, length + 1);
}

//
//===============
// MSG_WriteVector3
// 
//===============
//
void MSG_WriteVector3(const vec3_t& pos)
{
    MSG_WriteFloat(pos[0]);
    MSG_WriteFloat(pos[1]);
    MSG_WriteFloat(pos[2]);
}

#if USE_CLIENT

//
//===============
// MSG_WriteDeltaClientMoveCommand
// 
//===============
//
int MSG_WriteDeltaClientMoveCommand(const ClientMoveCommand* from, const ClientMoveCommand* cmd)
{
    // Send a null message in case we had none.
    if (!from) {
        from = &nullUserCmd;
    }

    //
    // send the movement message
    //
    int32_t bits = 0;

    if (cmd->input.viewAngles[0] != from->input.viewAngles[0])
        bits |= CM_ANGLE1;
    if (cmd->input.viewAngles[1] != from->input.viewAngles[1])
        bits |= CM_ANGLE2;
    if (cmd->input.viewAngles[2] != from->input.viewAngles[2])
        bits |= CM_ANGLE3;
    if (cmd->input.forwardMove != from->input.forwardMove)
        bits |= CM_FORWARD;
    if (cmd->input.rightMove != from->input.rightMove)
        bits |= CM_SIDE;
    if (cmd->input.upMove != from->input.upMove)
        bits |= CM_UP;
    if (cmd->input.buttons != from->input.buttons)
        bits |= CM_BUTTONS;
    if (cmd->input.impulse != from->input.impulse)
        bits |= CM_IMPULSE;

    // Write out the changed bits.
    MSG_WriteByte(bits);

    if (bits & CM_ANGLE1)
        MSG_WriteFloat(cmd->input.viewAngles[0]);
    if (bits & CM_ANGLE2)
        MSG_WriteFloat(cmd->input.viewAngles[1]);
    if (bits & CM_ANGLE3)
        MSG_WriteFloat(cmd->input.viewAngles[2]);

    if (bits & CM_FORWARD)
        MSG_WriteShort(cmd->input.forwardMove);
    if (bits & CM_SIDE)
        MSG_WriteShort(cmd->input.rightMove);
    if (bits & CM_UP)
        MSG_WriteShort(cmd->input.upMove);

    if (bits & CM_BUTTONS)
        MSG_WriteByte(cmd->input.buttons);

    if (bits & CM_IMPULSE)
        MSG_WriteByte(cmd->input.impulse);

    MSG_WriteByte(cmd->input.msec);
    MSG_WriteByte(cmd->input.lightLevel);

    // (Returned bits isn't used anywhere, but might as well keep it around.)
    return bits;
}

#endif // USE_CLIENT

void MSG_WriteDeltaEntity(const EntityState* from, const EntityState* to, EntityStateMessageFlags flags) {
    uint32_t bits = 0;
    uint32_t mask = 0;

    if (!to) {
        if (!from) {
            Com_Error(ERR_DROP, "%s: NULL", __func__);
        }

        if (from->number < 1 || from->number >= MAX_EDICTS) {
            Com_Error(ERR_DROP, "%s: bad number: %d", __func__, from->number);
        }
        
        // Write the bits.
	    MSG_WriteLong(bits | EntityMessageBits::MoreBitsA);

        // Set remove bit.
        int16_t number = from->number;

        // Since we won't be having more than 8192 entities either way, it is save
        // to use the actual remaining bits(14, and 15) for other means.
	    number |= (1 << 15); // Set bit 15 to notify removal.
        
        // Write it out.
	    MSG_WriteShort(number);
        //uint32_t bits = EntityMessageBits::Remove;

        //// If number is higher than 255, add U_NUMBER16 to the bit list.
        //if (from->number & 0xff00)
        //    bits |= U_NUMBER16 | EntityMessageBits::MoreBitsA;

        //// Write out first byte of bits.
        //MSG_WriteByte(bits & 255);

        //// Write out second byte of bits if necessary.
        //if (bits & 0x0000ff00)
        //    MSG_WriteByte((bits >> 8) & 255);

        //// Write a short in case entity number > 255 of course.
        //if (bits & U_NUMBER16)
        //    MSG_WriteShort(from->number);
        //else
        //    MSG_WriteByte(from->number);

        return; // remove entity
    }

    if (to->number < 1 || to->number >= MAX_EDICTS)
        Com_Error(ERR_DROP, "%s: bad number: %d", __func__, to->number);

    if (!from)
        from = &nullEntityState;

    // send an update
    bits = 0;

    if (!(flags & MSG_ES_FIRSTPERSON)) {
	    if (!EqualEpsilonf(to->origin[0], from->origin[0])) {
	        bits |= EntityMessageBits::OriginX;
	    }
	    if (!EqualEpsilonf(to->origin[1], from->origin[1])) {
		    bits |= EntityMessageBits::OriginY;
        }
	    if (!EqualEpsilonf(to->origin[2], from->origin[2])) { 
            bits |= EntityMessageBits::OriginZ;
        }

        // N&C: Full float precision.
        if (!EqualEpsilonf(to->angles[0], from->angles[0])) {
            bits |= EntityMessageBits::AngleX;
        }
	    if (!EqualEpsilonf(to->angles[1], from->angles[1])) {
	        bits |= EntityMessageBits::AngleY;
	    }
	    if (!EqualEpsilonf(to->angles[2], from->angles[2])) {
		    bits |= EntityMessageBits::AngleZ;
	    }

        if (flags & MSG_ES_NEWENTITY) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->origin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->origin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->origin[2]))
            {
                bits |= EntityMessageBits::OldOrigin;
            }
        }
    }
    
    mask = 0xffff8000;  // don't confuse old clients

    if (to->skinNumber != from->skinNumber) {
        bits |= EntityMessageBits::Skin;
    }

    if (!EqualEpsilonf(to->animationFrame, from->animationFrame)) {
        bits |= EntityMessageBits::AnimationFrame;
    }

    if (to->effects != from->effects) {
	    bits |= EntityMessageBits::EntityEffects;
    }

    if (to->renderEffects != from->renderEffects) {
	    bits |= EntityMessageBits::RenderEffects;
    }

    if (to->solid != from->solid) {
	    bits |= EntityMessageBits::Solid;
    }

    // Event is not delta compressed, it's bit is set when eventID is non 0.
    if (to->eventID) {
	    bits |= EntityMessageBits::EventID;
    }

    if (to->modelIndex != from->modelIndex) {
	    bits |= EntityMessageBits::ModelIndex;
    }
    if (to->modelIndex2 != from->modelIndex2) {
	    bits |= EntityMessageBits::ModelIndex2;
    }
    if (to->modelIndex3 != from->modelIndex3) {
	    bits |= EntityMessageBits::ModelIndex3;
    }
    if (to->modelIndex4 != from->modelIndex4) {
	    bits |= EntityMessageBits::ModelIndex4;
    }

    if (to->sound != from->sound) {
	    bits |= EntityMessageBits::Sound;
    }

    if (to->renderEffects & RenderEffects::FrameLerp) {
        bits |= EntityMessageBits::OldOrigin;
    }
    else if (to->renderEffects & RenderEffects::Beam) {
        if (flags & MSG_ES_BEAMORIGIN) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->oldOrigin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->oldOrigin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->oldOrigin[2])) 
            {
                bits |= EntityMessageBits::OldOrigin;
            }
        } else {
            bits |= EntityMessageBits::OldOrigin;
        }
    }

    if (to->animationStartTime != from->animationStartTime) { 
        bits |= EntityMessageBits::AnimationTimeStart;
    }
    if (to->animationStartFrame != from->animationStartFrame) {
	    bits |= EntityMessageBits::AnimationFrameStart;
    }
    if (to->animationEndFrame != from->animationEndFrame) {
	    bits |= EntityMessageBits::AnimationFrameEnd;
    }
    if (to->animationFramerate != from->animationFramerate) {
	    bits |= EntityMessageBits::AnimationFrameTime;
    }
    
    //
    // write the message
    //
    if (!bits && !(flags & MSG_ES_FORCE))
        return;     // nothing to send!

    //----------

    // Set the "more bits" flags based on how deep this state update is.
    if (bits & 0xff000000)
        bits |= EntityMessageBits::MoreBitsC | EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    else if (bits & 0x00ff0000)
        bits |= EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    else if (bits & 0x0000ff00)
        bits |= EntityMessageBits::MoreBitsA;

    // Write out the first 8 bits.
    MSG_WriteByte(bits & 255);

    // Write out the next 24 bits if this is an update reaching to MoreBitsC
    if (bits & 0xff000000) {
        MSG_WriteByte((bits >> 8) & 255);
        MSG_WriteByte((bits >> 16) & 255);
        MSG_WriteByte((bits >> 24) & 255);
    }
    // Write out the next 16 bits if this is an update reaching to MoreBitsB
    else if (bits & 0x00ff0000) {
        MSG_WriteByte((bits >> 8) & 255);
        MSG_WriteByte((bits >> 16) & 255);
    }
    // Write out the next 8 bits if this is an update reaching to MoreBitsA
    else if (bits & 0x0000ff00) {
        MSG_WriteByte((bits >> 8) & 255);
    }

    //----------
    // Make sure the "REMOVE" bit is unset before writing out the Entity State Number.
    int32_t entityNumber = to->number & ~(1U << 15);

    // Write out the Entity State number.
    MSG_WriteShort(to->number);

    // Write out the ModelIndex.
    if (bits & EntityMessageBits::ModelIndex) { 
        MSG_WriteByte(to->modelIndex);
    }
    // Write out the ModelIndex2.
    if (bits & EntityMessageBits::ModelIndex2) {
	    MSG_WriteByte(to->modelIndex2);
    }
    // Write out the ModelIndex3.
    if (bits & EntityMessageBits::ModelIndex3) {
	    MSG_WriteByte(to->modelIndex3);
    }
    // Write out the ModelIndex4.
    if (bits & EntityMessageBits::ModelIndex4) {
	    MSG_WriteByte(to->modelIndex4);
    }

    // Write out the AnimationFrame.
    if (bits & EntityMessageBits::AnimationFrame) {
	    MSG_WriteFloat(to->animationFrame);
    }
    
    // Write out the Skin Number.
    if (bits & EntityMessageBits::Skin) {
	    MSG_WriteShort(to->skinNumber);
    }

    // Write out the Entity Effects.
    if (bits & EntityMessageBits::EntityEffects) {
	    MSG_WriteLong(to->effects);
    }

    // Write out the Render Effects.
    if (bits & EntityMessageBits::RenderEffects) {
	    MSG_WriteLong(to->renderEffects);
    }

    // Write out the Origin X.
    if (bits & EntityMessageBits::OriginX) {
	    MSG_WriteFloat(to->origin[0]);
    }
    // Write out the Origin Y.
    if (bits & EntityMessageBits::OriginY) {
	    MSG_WriteFloat(to->origin[1]);
    }
    // Write out the Origin Z.
    if (bits & EntityMessageBits::OriginZ) {
	    MSG_WriteFloat(to->origin[2]);
    }

    // Write out the Angle X.
    if (bits & EntityMessageBits::AngleX) {
	    MSG_WriteFloat(to->angles[0]);
    }
    // Write out the Angle Y.
    if (bits & EntityMessageBits::AngleY) {
	    MSG_WriteFloat(to->angles[1]);
    }
    // Write out the Angle Z.
    if (bits & EntityMessageBits::AngleZ) {
	    MSG_WriteFloat(to->angles[2]);
    }

    // Write out the Old Origin.
    if (bits & EntityMessageBits::OldOrigin) {
        MSG_WriteFloat(to->oldOrigin[0]);
        MSG_WriteFloat(to->oldOrigin[1]);
        MSG_WriteFloat(to->oldOrigin[2]);
    }

    // Write out the Sound.
    if (bits & EntityMessageBits::Sound) {
	    MSG_WriteByte(to->sound);
    }

    // Write out the Event ID.
    if (bits & EntityMessageBits::EventID) {
	    MSG_WriteByte(to->eventID);
    }

    // Write out the Solid.
    if (bits & EntityMessageBits::Solid) {
        MSG_WriteLong(to->solid);
    }

    // Write out the Animation Start Time.
    if (bits & EntityMessageBits::AnimationTimeStart) {
	    MSG_WriteLong(to->animationStartTime);
    }
    // Write out the Animation Start Frame.
    if (bits & EntityMessageBits::AnimationFrameStart) {
	    MSG_WriteShort(to->animationStartFrame);
    }
    // Write out the Animation End Frame.
    if (bits & EntityMessageBits::AnimationFrameEnd) {
	    MSG_WriteShort(to->animationEndFrame);
    }
    // Write out the Animation Frame Time.
    if (bits & EntityMessageBits::AnimationFrameTime) {
    	MSG_WriteFloat(to->animationFramerate);
    }
}

int MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, PlayerStateMessageFlags flags)
{
    int     i;
    int     pflags, eflags;
    int     statbits;

    if (!to)
        Com_Error(ERR_DROP, "%s: NULL", __func__);

    if (!from)
        from = &nullPlayerState;

    //
    // Determine what needs to be sent
    //
    pflags = 0;
    eflags = 0;

    if (to->pmove.type != from->pmove.type)
        pflags |= PS_PM_TYPE;

    if (!EqualEpsilonf(to->pmove.origin[0], from->pmove.origin[0]) ||
        !EqualEpsilonf(to->pmove.origin[1], from->pmove.origin[1]))
        pflags |= PS_PM_ORIGIN;

    if (!EqualEpsilonf(to->pmove.origin[2], from->pmove.origin[2]))
        eflags |= EPS_M_ORIGIN2;

    if (!(flags & MSG_PS_IGNORE_PREDICTION)) {
        if (!EqualEpsilonf(to->pmove.velocity[0], from->pmove.velocity[0]) ||
            !EqualEpsilonf(to->pmove.velocity[1], from->pmove.velocity[1]))
            pflags |= PS_PM_VELOCITY;

        if (!EqualEpsilonf(to->pmove.velocity[2], from->pmove.velocity[2]))
            eflags |= EPS_M_VELOCITY2;

        if (to->pmove.time != from->pmove.time)
            pflags |= PS_PM_TIME;

        if (to->pmove.flags != from->pmove.flags)
            pflags |= PS_PM_FLAGS;

        if (to->pmove.gravity != from->pmove.gravity)
            pflags |= PS_PM_GRAVITY;
    }
    else {
        // save previous state
        VectorCopy(from->pmove.velocity, to->pmove.velocity);
        to->pmove.time = from->pmove.time;
        to->pmove.flags = from->pmove.flags;
        to->pmove.gravity = from->pmove.gravity;
    }

    if (!(flags & MSG_PS_IGNORE_DELTAANGLES)) {
        if (!EqualEpsilonf(to->pmove.deltaAngles.x, from->pmove.deltaAngles.x) ||
            !EqualEpsilonf(to->pmove.deltaAngles.y, from->pmove.deltaAngles.y) ||
            !EqualEpsilonf(to->pmove.deltaAngles.z, from->pmove.deltaAngles.z))
            pflags |= PS_PM_DELTA_ANGLES;
    }
    else {
        // save previous state
        VectorCopy(from->pmove.deltaAngles, to->pmove.deltaAngles);
    }

    if (!EqualEpsilonf(to->pmove.viewOffset.x, from->pmove.viewOffset.x) ||
        !EqualEpsilonf(to->pmove.viewOffset.y, from->pmove.viewOffset.y) ||
        !EqualEpsilonf(to->pmove.viewOffset.z, from->pmove.viewOffset.z))
        pflags |= PS_PM_VIEW_OFFSET;

    if (!(flags & MSG_PS_IGNORE_VIEWANGLES)) {
        if (!EqualEpsilonf(to->pmove.viewAngles.x, from->pmove.viewAngles.x) ||
            !EqualEpsilonf(to->pmove.viewAngles.y, from->pmove.viewAngles.y) ||
            !EqualEpsilonf(to->pmove.viewAngles.z, from->pmove.viewAngles.z))
            pflags |= PS_PM_VIEW_ANGLES;
    }
    else {
        // save previous state
        to->pmove.viewAngles[0] = from->pmove.viewAngles[0];
        to->pmove.viewAngles[1] = from->pmove.viewAngles[1];
        to->pmove.viewAngles[2] = from->pmove.viewAngles[2];
    }

    if (from->kickAngles[0] != to->kickAngles[0] ||
        from->kickAngles[1] != to->kickAngles[1] ||
        from->kickAngles[2] != to->kickAngles[2])
        pflags |= PS_KICKANGLES;

    if (from->blend[0] != to->blend[0] ||
        from->blend[1] != to->blend[1] ||
        from->blend[2] != to->blend[2] ||
        from->blend[3] != to->blend[3])
        pflags |= PS_BLEND;

    if (from->pmove.stepOffset != to->pmove.stepOffset)
        pflags |= PS_PM_STEP_OFFSET;

    if (from->fov != to->fov)
        pflags |= PS_FOV;

    if (to->rdflags != from->rdflags)
        pflags |= PS_RDFLAGS;

    if (to->gunIndex != from->gunIndex)
        pflags |= PS_WEAPONINDEX;

    if (to->gunAnimationStartTime != from->gunAnimationStartTime) {
	    pflags |= PS_GUNANIMATION_TIME_START;
    }
    if (to->gunAnimationStartFrame != from->gunAnimationStartFrame) {
        pflags |= PS_GUNANIMATION_FRAME_START;
    }
    if (to->gunAnimationEndFrame != from->gunAnimationEndFrame) {
	    pflags |= PS_GUNANIMATION_FRAME_END;
    }
    if (!EqualEpsilonf(to->gunAnimationFrametime, from->gunAnimationFrametime)) {
        pflags |= PS_GUNANIMATION_FRAME_TIME;
    }
    if (to->gunAnimationLoopCount != from->gunAnimationLoopCount) {
	    pflags |= PS_GUNANIMATION_LOOP_COUNT;
    }
    if (to->gunAnimationForceLoop != from->gunAnimationForceLoop) {
	    pflags |= PS_GUNANIMATION_LOOP_FORCE;
    }


    if (from->gunOffset[0] != to->gunOffset[0] ||
        from->gunOffset[1] != to->gunOffset[1] ||
        from->gunOffset[2] != to->gunOffset[2])
        eflags |= EPS_GUNOFFSET;

    if (from->gunAngles[0] != to->gunAngles[0] ||
        from->gunAngles[1] != to->gunAngles[1] ||
        from->gunAngles[2] != to->gunAngles[2])
        eflags |= EPS_GUNANGLES;

    statbits = 0;
    for (i = 0; i < MAX_STATS; i++)
        if (to->stats[i] != from->stats[i])
            statbits |= 1 << i;

    if (statbits)
        eflags |= EPS_STATS;

    //
    // write it
    //
    MSG_WriteLong(pflags);

    //
    // write the PlayerMoveState
    //
    if (pflags & PS_PM_TYPE)
        MSG_WriteByte(to->pmove.type);

    if (pflags & PS_PM_ORIGIN) {
        MSG_WriteFloat(to->pmove.origin[0]);
        MSG_WriteFloat(to->pmove.origin[1]);
    }

    if (eflags & EPS_M_ORIGIN2)
        MSG_WriteFloat(to->pmove.origin[2]);

    if (pflags & PS_PM_VELOCITY) {
        MSG_WriteFloat(to->pmove.velocity[0]);
        MSG_WriteFloat(to->pmove.velocity[1]);
    }

    if (eflags & EPS_M_VELOCITY2)
        MSG_WriteFloat(to->pmove.velocity[2]);

    if (pflags & PS_PM_TIME)
        MSG_WriteShort(to->pmove.time);

    if (pflags & PS_PM_FLAGS)
        MSG_WriteShort(to->pmove.flags);

    if (pflags & PS_PM_GRAVITY)
        MSG_WriteShort(to->pmove.gravity);

    if (pflags & PS_PM_DELTA_ANGLES) {
        MSG_WriteFloat(to->pmove.deltaAngles.x);
        MSG_WriteFloat(to->pmove.deltaAngles.y);
        MSG_WriteFloat(to->pmove.deltaAngles.z);
    }

    //
    // write the rest of the PlayerState
    //
    if (pflags & PS_PM_VIEW_OFFSET) {
        MSG_WriteFloat(to->pmove.viewOffset[0]);
        MSG_WriteFloat(to->pmove.viewOffset[1]);
        MSG_WriteFloat(to->pmove.viewOffset[2]);
    }

    if (pflags & PS_PM_STEP_OFFSET) {
        MSG_WriteFloat(to->pmove.stepOffset);
    }

    if (pflags & PS_PM_VIEW_ANGLES) {
        MSG_WriteFloat(to->pmove.viewAngles[0]);
        MSG_WriteFloat(to->pmove.viewAngles[1]);
        MSG_WriteFloat(to->pmove.viewAngles[2]);
    }

    if (pflags & PS_KICKANGLES) {
        MSG_WriteFloat(to->kickAngles[0]);
        MSG_WriteFloat(to->kickAngles[1]);
        MSG_WriteFloat(to->kickAngles[2]);
    }

    if (pflags & PS_WEAPONINDEX)
        MSG_WriteByte(to->gunIndex);

    if (pflags & PS_GUNANIMATION_TIME_START) {
	    MSG_WriteLong(to->gunAnimationStartTime);
    }
    if (pflags & PS_GUNANIMATION_FRAME_START) {
	    MSG_WriteShort(to->gunAnimationStartFrame);
    }
    if (pflags & PS_GUNANIMATION_FRAME_END) {
	    MSG_WriteShort(to->gunAnimationEndFrame);
    }
    if (pflags & PS_GUNANIMATION_FRAME_TIME) {
	    MSG_WriteFloat(to->gunAnimationFrametime);
    }
    if (pflags & PS_GUNANIMATION_LOOP_COUNT) {
	    MSG_WriteByte(to->gunAnimationLoopCount);
    }
    if (pflags & PS_GUNANIMATION_LOOP_FORCE) {
	    MSG_WriteByte(to->gunAnimationForceLoop);
    }

    if (eflags & EPS_GUNOFFSET) {
        MSG_WriteFloat(to->gunOffset[0]);
        MSG_WriteFloat(to->gunOffset[1]);
        MSG_WriteFloat(to->gunOffset[2]);
    }

    if (eflags & EPS_GUNANGLES) {
        MSG_WriteFloat(to->gunAngles[0]);
        MSG_WriteFloat(to->gunAngles[1]);
        MSG_WriteFloat(to->gunAngles[2]);
    }

    if (pflags & PS_BLEND) {
        MSG_WriteFloat(to->blend[0]);
        MSG_WriteFloat(to->blend[1]);
        MSG_WriteFloat(to->blend[2]);
        MSG_WriteFloat(to->blend[3]);
    }
  
    if (pflags & PS_FOV)
        MSG_WriteFloat(to->fov);

    if (pflags & PS_RDFLAGS)
        MSG_WriteLong(to->rdflags);

    // send stats
    if (eflags & EPS_STATS) {
        MSG_WriteLong(statbits);
        for (i = 0; i < MAX_STATS; i++)
            if (statbits & (1 << i))
                MSG_WriteShort(to->stats[i]);
    }

    return eflags;
}

/*
==============================================================================

            READING

==============================================================================
*/

void MSG_BeginReading(void)
{
    msg_read.readCount = 0;
    msg_read.bitPosition = 0;
}

byte* MSG_ReadData(size_t len)
{
    byte* buf = msg_read.data + msg_read.readCount;

    msg_read.readCount += len;
    msg_read.bitPosition = msg_read.readCount << 3;

    if (msg_read.readCount > msg_read.currentSize) {
        if (!msg_read.allowUnderflow) {
            Com_Error(ERR_DROP, "%s: read past end of message", __func__);
        }
        return NULL;
    }

    return buf;
}

// returns -1 if no more characters are available
int MSG_ReadChar(void)
{
    byte* buf = MSG_ReadData(1);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (signed char)buf[0];
    }

    return c;
}

int MSG_ReadByte(void)
{
    byte* buf = MSG_ReadData(1);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (unsigned char)buf[0];
    }

    return c;
}

int MSG_ReadShort(void)
{
    byte* buf = MSG_ReadData(2);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (signed short)LittleShortMem(buf);
    }

    return c;
}

int MSG_ReadWord(void)
{
    byte* buf = MSG_ReadData(2);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (unsigned short)LittleShortMem(buf);
    }

    return c;
}

int MSG_ReadLong(void)
{
    byte* buf = MSG_ReadData(4);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = LittleLongMem(buf);
    }

    return c;
}

//
//===============
// MSG_ReadFloat
// 
// The idea is smart and taken from Quetoo, use an union for memory mapping.
// Read the float as an int32_t, use the union struct trick to convert it to a float.
//================
//
float MSG_ReadFloat(void) {
    msg_float vec;
    vec.i = MSG_ReadLong();
    return vec.f;
}

size_t MSG_ReadString(char* dest, size_t size)
{
    int     c;
    size_t  len = 0;

    while (1) {
        c = MSG_ReadByte();
        if (c == -1 || c == 0) {
            break;
        }
        if (len + 1 < size) {
            *dest++ = c;
        }
        len++;
    }
    if (size) {
        *dest = 0;
    }

    return len;
}

size_t MSG_ReadStringLine(char* dest, size_t size)
{
    int     c;
    size_t  len = 0;

    while (1) {
        c = MSG_ReadByte();
        if (c == -1 || c == 0 || c == '\n') {
            break;
        }
        if (len + 1 < size) {
            *dest++ = c;
        }
        len++;
    }
    if (size) {
        *dest = 0;
    }

    return len;
}

vec3_t MSG_ReadVector3(void) {
    return vec3_t{
        MSG_ReadFloat(),
        MSG_ReadFloat(),
        MSG_ReadFloat()
    };
}

void MSG_ReadDeltaClientMoveCommand(const ClientMoveCommand* from, ClientMoveCommand* to)
{
    int bits;

    if (from) {
        memcpy(to, from, sizeof(*to));
    }
    else {
        memset(to, 0, sizeof(*to));
    }

    bits = MSG_ReadByte();

    // read current angles
    if (bits & CM_ANGLE1)
        to->input.viewAngles[0] = MSG_ReadFloat();
    if (bits & CM_ANGLE2)
        to->input.viewAngles[1] = MSG_ReadFloat();
    if (bits & CM_ANGLE3)
        to->input.viewAngles[2] = MSG_ReadFloat();

    // read movement
    if (bits & CM_FORWARD)
        to->input.forwardMove = MSG_ReadShort();
    if (bits & CM_SIDE)
        to->input.rightMove = MSG_ReadShort();
    if (bits & CM_UP)
        to->input.upMove = MSG_ReadShort();

    // read buttons
    if (bits & CM_BUTTONS)
        to->input.buttons = MSG_ReadByte();

    if (bits & CM_IMPULSE)
        to->input.impulse = MSG_ReadByte();

    // read time to run command
    to->input.msec = MSG_ReadByte();

    // read the light level
    to->input.lightLevel = MSG_ReadByte();
}

#if USE_CLIENT

/*
=================
MSG_ParseEntityBits

Returns the entity number and the header bits
=================
*/
int MSG_ParseEntityBits(int* bits)
{
    int         b = 0, total = 0;
    int         number = 0;

    total = MSG_ReadByte();
    if (total & EntityMessageBits::MoreBitsA) {
        b = MSG_ReadByte();
        total |= b << 8;
    }
    if (total & EntityMessageBits::MoreBitsB) {
        b = MSG_ReadByte();
        total |= b << 16;
    }
    if (total & EntityMessageBits::MoreBitsC) {
        b = MSG_ReadByte();
        total |= b << 24;
    }

	number = MSG_ReadShort();


    *bits = total;

    return number;
}

/*
==================
MSG_ParseDeltaEntity

Can go from either a baseline or a previous packet_entity
==================
*/
void MSG_ParseDeltaEntity(const EntityState* from, EntityState* to, int number, int bits, EntityStateMessageFlags flags) {
    // Sanity checks.
    if (!to) {
        Com_Error(ERR_DROP, "%s: NULL", __func__);
    }

    if (number < 1 || number >= MAX_EDICTS) {
        Com_Error(ERR_DROP, "%s: bad entity number: %d", __func__, number);
    }

    // Set everything to the state we are delta'ing from
    if (!from) {
        //memset(to, 0, sizeof(*to));
	    *to = {};
    } else if (to != from) {
        *to = *from;
        //memcpy(to, from, sizeof(*to));
    }

    // Store number.
    to->number = number;

    // Reset eventID.
    to->eventID = 0;

    // If no bits were received, we got no business here, return.
    if (!bits) {
        return;
    }

    // Model Indexes.
    if (bits & EntityMessageBits::ModelIndex) {
        to->modelIndex = MSG_ReadByte();
    }
    if (bits & EntityMessageBits::ModelIndex2) {
        to->modelIndex2 = MSG_ReadByte();
    }
    if (bits & EntityMessageBits::ModelIndex3) {
        to->modelIndex3 = MSG_ReadByte();
    }
    if (bits & EntityMessageBits::ModelIndex4) {
        to->modelIndex4 = MSG_ReadByte();
    }

    // Frame.
    if (bits & EntityMessageBits::AnimationFrame)
        to->animationFrame = MSG_ReadFloat();

    // Skinnum.
    if (bits & EntityMessageBits::Skin) { 
        to->skinNumber = MSG_ReadWord();
    }

    // Effects.
    if (bits & EntityMessageBits::EntityEffects) {
	    to->effects = MSG_ReadLong();
    }

    // RenderFX.
    if (bits & EntityMessageBits::RenderEffects) {
	    to->renderEffects = MSG_ReadLong();
    }

    // Origin.
    if (bits & EntityMessageBits::OriginX) {
        to->origin[0] = MSG_ReadFloat();
    }
    if (bits & EntityMessageBits::OriginY) {
        to->origin[1] = MSG_ReadFloat();
    }
    if (bits & EntityMessageBits::OriginZ) {
        to->origin[2] = MSG_ReadFloat();
    }

    // Angle.
    if (bits & EntityMessageBits::AngleX) {
	    to->angles[0] = MSG_ReadFloat();
    }
    if (bits & EntityMessageBits::AngleY) {
    	to->angles[1] = MSG_ReadFloat();
    }
    if (bits & EntityMessageBits::AngleZ) {
	    to->angles[2] = MSG_ReadFloat();
    }

    // Old Origin.
    if (bits & EntityMessageBits::OldOrigin) {
        to->oldOrigin = MSG_ReadVector3();
    }

    // Sound.
    if (bits & EntityMessageBits::Sound) {
        to->sound = MSG_ReadByte();
    }

    // Event.
    if (bits & EntityMessageBits::EventID) {
        to->eventID = MSG_ReadByte();
    }

    // Solid.
    if (bits & EntityMessageBits::Solid) {
        to->solid = MSG_ReadLong();
    }

    if (bits & EntityMessageBits::AnimationTimeStart) {
	    to->animationStartTime = MSG_ReadLong();
    }
    if (bits & EntityMessageBits::AnimationFrameStart) {
	    to->animationStartFrame = MSG_ReadShort();
    }
    if (bits & EntityMessageBits::AnimationFrameEnd) {
	    to->animationEndFrame = MSG_ReadShort();
    }
    if (bits & EntityMessageBits::AnimationFrameTime) {
    	to->animationFramerate = MSG_ReadFloat();
    }

}

/**
*   @brief  Parses the delta packets of player states.
**/
void MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, int extraflags) {

    // Sanity check. 
    if (!to) {
        Com_Error(ERR_DROP, "%s: no state to delta to, 'to' == nullptr", __func__);
    }

    // Clear to old value before doing any delta parsing.
    if (!from) {
	    *to = {};
    } else if (to != from) {
        *to = *from;
    }

    // Read the flag bits.
    int32_t flags = MSG_ReadLong();

    //
    // Parse movement related state data.
    // 
    // PM Type
    if (flags & PS_PM_TYPE)
        to->pmove.type = MSG_ReadByte();

    // Origin X Y.
    if (flags & PS_PM_ORIGIN) {
        to->pmove.origin[0] = MSG_ReadFloat();
        to->pmove.origin[1] = MSG_ReadFloat();
    }

    // Origin Z.
    if (extraflags & EPS_M_ORIGIN2) {
        to->pmove.origin[2] = MSG_ReadFloat();
    }

    // Velocity X Y.
    if (flags & PS_PM_VELOCITY) {
        to->pmove.velocity[0] = MSG_ReadFloat();
        to->pmove.velocity[1] = MSG_ReadFloat();
    }

    // Velocity Z.
    if (extraflags & EPS_M_VELOCITY2) {
        to->pmove.velocity[2] = MSG_ReadFloat();
    }

    // PM Time.
    if (flags & PS_PM_TIME)
        to->pmove.time = MSG_ReadShort();

    // PM Flags.
    if (flags & PS_PM_FLAGS)
        to->pmove.flags = MSG_ReadShort();

    // PM Gravity.
    if (flags & PS_PM_GRAVITY)
        to->pmove.gravity = MSG_ReadShort();

    // PM Delta Angles.
    if (flags & PS_PM_DELTA_ANGLES) {
        to->pmove.deltaAngles.x = MSG_ReadFloat();
        to->pmove.deltaAngles.y = MSG_ReadFloat();
        to->pmove.deltaAngles.z = MSG_ReadFloat();
    }

    // View Offset.
    if (flags & PS_PM_VIEW_OFFSET) {
	    to->pmove.viewOffset.x = MSG_ReadFloat();
	    to->pmove.viewOffset.y = MSG_ReadFloat();
	    to->pmove.viewOffset.z = MSG_ReadFloat();
    }

    // Step offset.
    if (flags & PS_PM_STEP_OFFSET) {
	    to->pmove.stepOffset = MSG_ReadFloat();
    }

    // View Angles X Y Z.
    if (flags & PS_PM_VIEW_ANGLES) {
	to->pmove.viewAngles.x = MSG_ReadFloat();
	to->pmove.viewAngles.y = MSG_ReadFloat();
	to->pmove.viewAngles.z = MSG_ReadFloat();
    }

    //
    // Parse the rest of the player state data.
    //
    // Kick Angles.
    if (flags & PS_KICKANGLES) {
        to->kickAngles[0] = MSG_ReadFloat();
        to->kickAngles[1] = MSG_ReadFloat();
        to->kickAngles[2] = MSG_ReadFloat();
    }

    // Weapon Index.
    if (flags & PS_WEAPONINDEX) {
        to->gunIndex = MSG_ReadByte();
    }

    if (flags & PS_GUNANIMATION_TIME_START) {
	    to->gunAnimationStartTime = MSG_ReadLong();
    }
    if (flags & PS_GUNANIMATION_FRAME_START) {
	    to->gunAnimationStartFrame = MSG_ReadShort();
    }
    if (flags & PS_GUNANIMATION_FRAME_END) {
	    to->gunAnimationEndFrame = MSG_ReadShort();
    }
    if (flags & PS_GUNANIMATION_FRAME_TIME) {
	    to->gunAnimationFrametime = MSG_ReadFloat();
    }
    if (flags & PS_GUNANIMATION_LOOP_COUNT) {
	    to->gunAnimationLoopCount = MSG_ReadByte(); 
    }
    if (flags & PS_GUNANIMATION_LOOP_FORCE) {
    	to->gunAnimationForceLoop = MSG_ReadByte();
    }

    // Gun Offset.
    if (extraflags & EPS_GUNOFFSET) {
        to->gunOffset[0] = MSG_ReadFloat();
        to->gunOffset[1] = MSG_ReadFloat();
        to->gunOffset[2] = MSG_ReadFloat();
    }

    // Gun Angles.
    if (extraflags & EPS_GUNANGLES) {
        to->gunAngles[0] = MSG_ReadFloat();
        to->gunAngles[1] = MSG_ReadFloat();
        to->gunAngles[2] = MSG_ReadFloat();
    }

    // Blend.
    if (flags & PS_BLEND) {
        to->blend[0] = MSG_ReadFloat();
        to->blend[1] = MSG_ReadFloat();
        to->blend[2] = MSG_ReadFloat();
        to->blend[3] = MSG_ReadFloat();
    }

    // FOV.
    if (flags & PS_FOV)
        to->fov = MSG_ReadFloat();

    // RDFlags.
    if (flags & PS_RDFLAGS)
        to->rdflags = MSG_ReadLong();

    // Parse Stats.
    if (extraflags & EPS_STATS) {
        int32_t statbits = MSG_ReadLong();
        for (int32_t i = 0; i < MAX_STATS; i++) {
            if (statbits & (1 << i)) {
                to->stats[i] = MSG_ReadShort();
            }
        }
    }

}

#endif // USE_CLIENT

/*
==============================================================================

            DEBUGGING STUFF

==============================================================================
*/

#ifdef _DEBUG

#define SHOWBITS(x) Com_LPrintf(PRINT_DEVELOPER, x " ")

#if USE_CLIENT

void MSG_ShowDeltaPlayerstateBits(int flags, int extraflags)
{
#define SP(b,s) if(flags&PS_##b) SHOWBITS(s)
#define SE(b,s) if(extraflags&EPS_##b) SHOWBITS(s)
    SP(PM_TYPE, "pmove.type");
    SP(PM_ORIGIN, "pmove.origin[x,y]");
    SE(M_ORIGIN2, "pmove.origin[z]");
    SP(PM_VELOCITY, "pmove.velocity[x,y]");
    SE(M_VELOCITY2, "pmove.velocity[z]");
    SP(PM_TIME, "pmove.time");
    SP(PM_FLAGS, "pmove.flags");
    SP(PM_GRAVITY, "pmove.gravity");
    SP(PM_DELTA_ANGLES, "pmove.deltaAngles");
    SP(PM_VIEW_OFFSET, "pmove.viewOffset");
    SP(PM_VIEW_ANGLES, "pmove.viewAngles");
    SP(KICKANGLES, "kickAngles");
    SP(WEAPONINDEX, "gunIndex");
    SP(GUNANIMATION_TIME_START, "gunAnimationStartTime");
    SP(GUNANIMATION_FRAME_START, "gunAnimationFrameStart");
    SP(GUNANIMATION_FRAME_END, "gunAnimationFrameEnd");
    SP(GUNANIMATION_FRAME_TIME, "gunAnimationFrameTime");
    SP(GUNANIMATION_LOOP_COUNT, "gunAnimationLoopCount");
    SP(GUNANIMATION_LOOP_FORCE, "gunAnimationForceLoop");
    SE(GUNOFFSET, "gunOffset");
    SE(GUNANGLES, "gunAngles");
    SP(BLEND, "blend");
    SP(FOV, "fov");
    SP(RDFLAGS, "rdflags");
    SE(STATS, "stats");
#undef SP
#undef SE
}

void MSG_ShowDeltaUsercmdBits(int bits)
{
    if (!bits) {
        SHOWBITS("<none>");
        return;
    }

#define S(b,s) if(bits&CM_##b) SHOWBITS(s)
    S(ANGLE1, "angle1");
    S(ANGLE2, "angle2");
    S(ANGLE3, "angle3");
    S(FORWARD, "forward");
    S(SIDE, "side");
    S(UP, "up");
    S(BUTTONS, "buttons");
    S(IMPULSE, "msec");
#undef S
}

#endif // USE_CLIENT

#if USE_CLIENT

void MSG_ShowDeltaEntityBits(int bits)
{
//#define S(b,s) if(bits&BIT_##b) SHOWBITS(s)
//    S(MODEL, "modelIndex");
//    S(MODEL2, "modelIndex2");
//    S(MODEL3, "modelIndex3");
//    S(MODEL4, "modelIndex4");
//
//    if (bits & EntityMessageBits::AnimationFrame)
//        SHOWBITS("frame8");
//    if (bits & U_FRAME16)
//        SHOWBITS("frame16");
//
//    if ((bits & (U_SKIN8 | U_SKIN16)) == (U_SKIN8 | U_SKIN16))
//        SHOWBITS("skinnum32");
//    else if (bits & U_SKIN8)
//        SHOWBITS("skinnum8");
//    else if (bits & U_SKIN16)
//        SHOWBITS("skinnum16");
//
//    if ((bits & (U_EFFECTS8 | U_EFFECTS16)) == (U_EFFECTS8 | U_EFFECTS16))
//        SHOWBITS("effects32");
//    else if (bits & U_EFFECTS8)
//        SHOWBITS("effects8");
//    else if (bits & U_EFFECTS16)
//        SHOWBITS("effects16");
//
//    if ((bits & (U_RENDERFX8 | U_RENDERFX16)) == (U_RENDERFX8 | U_RENDERFX16))
//        SHOWBITS("renderfx32");
//    else if (bits & U_RENDERFX8)
//        SHOWBITS("renderfx8");
//    else if (bits & U_RENDERFX16)
//        SHOWBITS("renderfx16");
//
//    S(ORIGIN_X, "origin.x");
//    S(ORIGIN_Y, "origin.y");
//    S(ORIGIN_Z, "origin.z");
//    S(ANGLE_X, "angles.x");
//    S(ANGLE_Y, "angles.y");
//    S(ANGLE_Z, "angles.z");
//    S(OLD_ORIGIN, "oldOrigin");
//    S(SOUND, "sound");
//    S(EVENT_ID, "event");
//    S(SOLID, "solid");
//#undef S
}

const char* MSG_ServerCommandString(int cmd)
{
    switch (cmd) {
    case -1: return "END OF MESSAGE";
    default: return "UNKNOWN COMMAND";
#define S(x) case ServerCommand::##x: return "ServerCommand::" #x;
            S(Bad)
            // N&C: Protocol todo: add a game callback for this...?
            //S(muzzleflash)
            //S(muzzleflash2)
            //S(temp_entity)
            //S(layout)
            //S(inventory)
            S(Padding)
            S(Disconnect)
            S(Reconnect)
            S(Sound)
            S(Print)
            S(StuffText)
            S(ServerData)
            S(ConfigString)
            S(SpawnBaseline)
            S(CenterPrint)
            S(Download)
            S(PlayerInfo)
            S(PacketEntities)
	        S(DeltaPacketEntities)
            S(Frame)
            S(ZPacket)
            S(ZDownload)
            S(GameState)
#undef S
    }
}

#endif // USE_CLIENT

#endif // _DEBUG

