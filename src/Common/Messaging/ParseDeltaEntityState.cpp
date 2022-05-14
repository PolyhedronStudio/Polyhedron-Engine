/***
*
*	License here.
*
*	@file
*
*	Contains the code used for parsing received Entity States.
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
*   @brief Reads the delta entity state, can go from either a baseline or a previous packet Entity State.
**/
void MSG_ParseDeltaEntityState(const EntityState* from, EntityState* to, int32_t number, uint32_t byteMask, uint32_t entityStateFlags) {
    // Sanity checks.
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: NULL", __func__);
    }

    // Ensure the number is valid.
    if (number < 1 || number >= MAX_PACKET_ENTITIES) {
        Com_Error(ErrorType::Drop, "%s: bad entity number: %d", __func__, number);
    }

    // Ensure our 'to' state is cleared out in case of not having a 'from'.
    // Otherwise, assign 'from' to 'to' so we already got the unchanged values.
    if (!from) {
	    *to = {};
    } else if (to != from) {
        *to = *from;
    }

    // Store number.
    to->number = number;

    // Reset eventID.
    to->eventID = 0;

    // If no byteMask were received, we got no business here, return.
    if (!byteMask) {
        return;
    }

    // Model Indexes.
    if (byteMask & EntityMessageBits::ModelIndex) {
        to->modelIndex = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex2) {
        to->modelIndex2 = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex3) {
        to->modelIndex3 = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex4) {
        to->modelIndex4 = MSG_ReadUint8();
    }

    // Frame.
    if (byteMask & EntityMessageBits::AnimationFrame)
        to->animationFrame = MSG_ReadHalfFloat();

    // Skinnum.
    if (byteMask & EntityMessageBits::Skin) { 
        to->skinNumber = MSG_ReadInt16();
    }

    // Effects.
    if (byteMask & EntityMessageBits::EntityEffects) {
	    to->effects = MSG_ReadUintBase128();
    }

    // RenderFX.
    if (byteMask & EntityMessageBits::RenderEffects) {
	    to->renderEffects = MSG_ReadIntBase128();
    }

    // HashedClassname.
    if (byteMask & EntityMessageBits::HashedClassname) {
	    to->hashedClassname = MSG_ReadUint32();
    }

    // Origin.
    if (byteMask & EntityMessageBits::OriginX) {
        to->origin[0] = MSG_ReadFloat();
    }
    if (byteMask & EntityMessageBits::OriginY) {
        to->origin[1] = MSG_ReadFloat();
    }
    if (byteMask & EntityMessageBits::OriginZ) {
        to->origin[2] = MSG_ReadFloat();
    }

    // Angle.
    if (byteMask & EntityMessageBits::AngleX) {
		to->angles[0] = MSG_ReadHalfFloat();
    }
    if (byteMask & EntityMessageBits::AngleY) {
		to->angles[1] = MSG_ReadHalfFloat();
    }
    if (byteMask & EntityMessageBits::AngleZ) {
		to->angles[2] = MSG_ReadHalfFloat();
    }

    // Old Origin.
    if (byteMask & EntityMessageBits::OldOrigin) {
        to->oldOrigin = MSG_ReadVector3();
    }

    // Sound.
    if (byteMask & EntityMessageBits::Sound) {
        to->sound = MSG_ReadUint8();
    }

    // Event.
    if (byteMask & EntityMessageBits::EventID) {
        to->eventID = MSG_ReadUint8();
    }

    // Solid.
    if (byteMask & EntityMessageBits::Solid) {
        to->solid = MSG_ReadInt32();
    }

    // Read in the Bounding Box.
    if (byteMask & EntityMessageBits::Bounds) {

        //MSG_WriteIntBase128(to->solid);
		to->mins.x = -MSG_ReadUint8();
		to->mins.y = -MSG_ReadUint8();
		to->mins.z = -MSG_ReadUint8();

		to->maxs.x = MSG_ReadUint8();
		to->maxs.y = MSG_ReadUint8();
		to->maxs.z = MSG_ReadUint8();
    }

    if (byteMask & EntityMessageBits::AnimationTimeStart) {
	    to->animationStartTime = MSG_ReadUintBase128();
    }
    if (byteMask & EntityMessageBits::AnimationFrameStart) {
	    to->animationStartFrame = MSG_ReadUint16();
    }
    if (byteMask & EntityMessageBits::AnimationFrameEnd) {
	    to->animationEndFrame = MSG_ReadUint16();
    }
    if (byteMask & EntityMessageBits::AnimationFrameTime) {
    	to->animationFramerate = MSG_ReadHalfFloat();
    }
}