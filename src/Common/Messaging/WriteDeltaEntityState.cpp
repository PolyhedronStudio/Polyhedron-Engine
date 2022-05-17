/***
*
*	License here.
*
*	@file
*
*	Contains the code used for writing Delta Entity States.
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

// Entity BaseLine State.
static const EntityState       nullEntityState = {};


/**
*   @brief Writes the delta values of the entity state.
**/
void MSG_WriteDeltaEntityState(const EntityState* from, const EntityState* to, uint32_t entityStateMessageFlags) {
    // Check whether to remove this entity if it isn't going anywhere.
    if (!to) {
        // If it never came from anywhere, we got no data to work with, error out.
        if (!from) {
            Com_Error(ErrorType::Drop, "%s: NULL", __func__);
        }

        // Invalid number? Error out.
        if (from->number < 1 || from->number >= MAX_PACKET_ENTITIES) {
            Com_Error(ErrorType::Drop, "%s: bad number: %d", __func__, from->number);
        }

        // Write out entity number with a remove bit set.
        MSG_WriteEntityNumber(from->number, true, 0);

        // We're done, remove entity.
        return;
    }

    // Entity number sanity check.
    if (to->number < 1 || to->number >= MAX_PACKET_ENTITIES) {
        Com_Error(ErrorType::Drop, "%s: bad number: %d", __func__, to->number);
    }

    // Update from null entity state in case there is nothing to delta from.
    if (!from) {
        from = &nullEntityState;
    }

    // Set byteMask to 0.
    uint32_t byteMask = 0;

    if (!(entityStateMessageFlags & MSG_ES_FIRSTPERSON)) {
	    if (!EqualEpsilonf(to->origin[0], from->origin[0])) {
	        byteMask |= EntityMessageBits::OriginX;
	    }
	    if (!EqualEpsilonf(to->origin[1], from->origin[1])) {
		    byteMask |= EntityMessageBits::OriginY;
        }
	    if (!EqualEpsilonf(to->origin[2], from->origin[2])) { 
            byteMask |= EntityMessageBits::OriginZ;
        }

        if (!EqualEpsilonf(to->angles[0], from->angles[0])) {
            byteMask |= EntityMessageBits::AngleX;
        }
	    if (!EqualEpsilonf(to->angles[1], from->angles[1])) {
	        byteMask |= EntityMessageBits::AngleY;
	    }
	    if (!EqualEpsilonf(to->angles[2], from->angles[2])) {
		    byteMask |= EntityMessageBits::AngleZ;
	    }

        if (entityStateMessageFlags & MSG_ES_NEWENTITY) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->origin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->origin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->origin[2]))
            {
                byteMask |= EntityMessageBits::OldOrigin;
            }
        }
    }

    if (to->skinNumber != from->skinNumber) {
        byteMask |= EntityMessageBits::Skin;
    }

    if (!EqualEpsilonf(to->animationFrame, from->animationFrame)) {
        byteMask |= EntityMessageBits::AnimationFrame;
    }

    if (to->effects != from->effects) {
	    byteMask |= EntityMessageBits::EntityEffects;
    }

    if (to->renderEffects != from->renderEffects) {
	    byteMask |= EntityMessageBits::RenderEffects;
    }

    if (to->hashedClassname != from->hashedClassname) {
        byteMask |= EntityMessageBits::HashedClassname;
    }

    if (to->solid != from->solid) {
	    byteMask |= EntityMessageBits::Solid;
    }

    if (to->solid != from->solid) {
	    byteMask |= EntityMessageBits::Solid;
    }
    
	if (to->solid != PACKED_BBOX && (!vec3_equal(to->mins, from->mins) || !vec3_equal(to->maxs, from->maxs))) {
        byteMask |= EntityMessageBits::Bounds;
    }

    // Event is not delta compressed, it's bit is set when eventID is non 0.
    if (to->eventID) {
	    byteMask |= EntityMessageBits::EventID;
    }

    if (to->modelIndex != from->modelIndex) {
	    byteMask |= EntityMessageBits::ModelIndex;
    }
    if (to->modelIndex2 != from->modelIndex2) {
	    byteMask |= EntityMessageBits::ModelIndex2;
    }
    if (to->modelIndex3 != from->modelIndex3) {
	    byteMask |= EntityMessageBits::ModelIndex3;
    }
    if (to->modelIndex4 != from->modelIndex4) {
	    byteMask |= EntityMessageBits::ModelIndex4;
    }

    if (to->sound != from->sound) {
	    byteMask |= EntityMessageBits::Sound;
    }

    if (to->renderEffects & RenderEffects::FrameLerp) {
        byteMask |= EntityMessageBits::OldOrigin;
    }
    else if (to->renderEffects & RenderEffects::Beam) {
        if (entityStateMessageFlags & MSG_ES_BEAMORIGIN) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->oldOrigin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->oldOrigin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->oldOrigin[2])) 
            {
                byteMask |= EntityMessageBits::OldOrigin;
            }
        } else {
            byteMask |= EntityMessageBits::OldOrigin;
        }
    }

    if (to->currentAnimation.startTime != from->currentAnimation.startTime) { 
        byteMask |= EntityMessageBits::AnimationTimeStart;
    }
    if (to->currentAnimation.animationIndex != from->currentAnimation.animationIndex) {
	    byteMask |= EntityMessageBits::AnimationIndex;
    }
    //if (to->animationEndFrame != from->animationEndFrame) {
	   // byteMask |= EntityMessageBits::AnimationFrameEnd;
    //}
    //if (to->animationFramerate != from->animationFramerate) {
	   // byteMask |= EntityMessageBits::AnimationFrameTime;
    //}
    
    //
    // write the message
    //
    if (!byteMask && !(entityStateMessageFlags & MSG_ES_FORCE))
        return;     // nothing to send!

    //----------

    // Set the "more byteMask" flags based on how deep this state update is.
    //if (byteMask & 0xff000000)
    //    byteMask |= EntityMessageBits::MoreBitsC | EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    //else if (byteMask & 0x00ff0000)
    //    byteMask |= EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    //else if (byteMask & 0x0000ff00)
    //    byteMask |= EntityMessageBits::MoreBitsA;

    // Write out the first 8 byteMask.
    //MSG_WriteUint8(byteMask & 255);

    //// Write out the next 24 byteMask if this is an update reaching to MoreBitsC
    //if (byteMask & 0xff000000) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //    MSG_WriteUint8((byteMask >> 16) & 255);
    //    MSG_WriteUint8((byteMask >> 24) & 255);
    //}
    //// Write out the next 16 byteMask if this is an update reaching to MoreBitsB
    //else if (byteMask & 0x00ff0000) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //    MSG_WriteUint8((byteMask >> 16) & 255);
    //}
    //// Write out the next 8 byteMask if this is an update reaching to MoreBitsA
    //else if (byteMask & 0x0000ff00) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //}
    //MSG_WriteInt32(byteMask);


    ////----------
    //// Make sure the "REMOVE" bit is unset before writing out the Entity State Number.
    //int32_t entityNumber = to->number & ~(1U << 15);

    //// Write out the Entity State number.
    //MSG_WriteInt16(to->number);
    MSG_WriteEntityNumber(to->number, false, byteMask);

    // Write out the ModelIndex.
    if (byteMask & EntityMessageBits::ModelIndex) { 
        MSG_WriteUint8(to->modelIndex);
    }
    // Write out the ModelIndex2.
    if (byteMask & EntityMessageBits::ModelIndex2) {
	    MSG_WriteUint8(to->modelIndex2);
    }
    // Write out the ModelIndex3.
    if (byteMask & EntityMessageBits::ModelIndex3) {
	    MSG_WriteUint8(to->modelIndex3);
    }
    // Write out the ModelIndex4.
    if (byteMask & EntityMessageBits::ModelIndex4) {
	    MSG_WriteUint8(to->modelIndex4);
    }

    // Write out the AnimationFrame.
    if (byteMask & EntityMessageBits::AnimationFrame) {
	    MSG_WriteHalfFloat(to->animationFrame);
    }
    
    // Write out the Skin Number.
    if (byteMask & EntityMessageBits::Skin) {
	    MSG_WriteInt16(to->skinNumber);
    }

    // Write out the Entity Effects.
    if (byteMask & EntityMessageBits::EntityEffects) {
	    MSG_WriteUintBase128(to->effects);
    }

    // Write out the Render Effects.
    if (byteMask & EntityMessageBits::RenderEffects) {
	    MSG_WriteIntBase128(to->renderEffects);
    }

    // Write out the Render Effects.
    if (byteMask & EntityMessageBits::HashedClassname) {
	    MSG_WriteUint32(to->hashedClassname);
    }

    // Write out the Origin X.
    if (byteMask & EntityMessageBits::OriginX) {
	    MSG_WriteFloat(to->origin[0]);
    }
    // Write out the Origin Y.
    if (byteMask & EntityMessageBits::OriginY) {
	    MSG_WriteFloat(to->origin[1]);
    }
    // Write out the Origin Z.
    if (byteMask & EntityMessageBits::OriginZ) {
	    MSG_WriteFloat(to->origin[2]);
    }

    // Write out the Angle X.
    if (byteMask & EntityMessageBits::AngleX) {
	    //MSG_WriteFloat(to->angles.x);
		//MSG_WriteUint16(FloatAngle2Short(to->angles[0]));
		MSG_WriteHalfFloat(to->angles[0]);
    }
    // Write out the Angle Y.
    if (byteMask & EntityMessageBits::AngleY) {
		//MSG_WriteFloat(to->angles.y);
		MSG_WriteHalfFloat(to->angles[1]);
    }
    // Write out the Angle Z.
    if (byteMask & EntityMessageBits::AngleZ) {
	    //MSG_WriteFloat(to->angles.z);
		MSG_WriteHalfFloat(to->angles[2]);
    }

    // Write out the Old Origin.
    if (byteMask & EntityMessageBits::OldOrigin) {
        MSG_WriteVector3(to->oldOrigin);
    }

    // Write out the Sound.
    if (byteMask & EntityMessageBits::Sound) {
	    MSG_WriteUint8(to->sound);
    }

    // Write out the Event ID.
    if (byteMask & EntityMessageBits::EventID) {
	    MSG_WriteUint8(to->eventID);
    }

    // Write out the Solid.
    if (byteMask & EntityMessageBits::Solid) {
        MSG_WriteInt32(to->solid);
    }

    // Write out the Bounding Box.
    if (byteMask & EntityMessageBits::Bounds) {
        //MSG_WriteIntBase128(to->solid);
		MSG_WriteUint8(-to->mins.x);
		MSG_WriteUint8(-to->mins.y);
		MSG_WriteUint8(-to->mins.z);

		MSG_WriteUint8(to->maxs.x);
		MSG_WriteUint8(to->maxs.y);
		MSG_WriteUint8(to->maxs.z);
    }

    // Write out the Animation Start Time.
    if (byteMask & EntityMessageBits::AnimationTimeStart) {
	    MSG_WriteUintBase128(to->currentAnimation.startTime);
    }
    // Write out the Animation Start Frame.
    if (byteMask & EntityMessageBits::AnimationIndex) {
	    MSG_WriteUint8(to->currentAnimation.animationIndex);
    }
    //// Write out the Animation End Frame.
    //if (byteMask & EntityMessageBits::AnimationFrameEnd) {
	   // MSG_WriteUint16(to->animationEndFrame);
    //}
    //// Write out the Animation Frame Time.
    //if (byteMask & EntityMessageBits::AnimationFrameTime) {
    //	MSG_WriteHalfFloat(to->animationFramerate);
    //}
}