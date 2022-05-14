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

#if USE_SERVER
/**
*   @brief Read a client's delta move command.
**/
void MSG_ReadDeltaClientMoveCommand(const ClientMoveCommand* from, ClientMoveCommand* to)
{
    int bits;

    if (from) {
        *to = *from;
    } else {
        *to = {};
    }

    bits = MSG_ReadUint8();

    // Read current angles.
    if (bits & UserCommandBits::AngleX) {
        to->input.viewAngles[0] = MSG_ReadHalfFloat();
    }
    if (bits & UserCommandBits::AngleY) {
        to->input.viewAngles[1] = MSG_ReadHalfFloat();
    }
    if (bits & UserCommandBits::AngleZ) {
        to->input.viewAngles[2] = MSG_ReadHalfFloat();
    }

    // Read movement.
    if (bits & UserCommandBits::Forward) {
        to->input.forwardMove = MSG_ReadInt16();
    }
    if (bits & UserCommandBits::Side) {
        to->input.rightMove = MSG_ReadInt16();
    }
    if (bits & UserCommandBits::Up) {
        to->input.upMove = MSG_ReadInt16();
    }

    // Read buttons.
    if (bits & UserCommandBits::Buttons) {
        to->input.buttons = MSG_ReadUint8();
    }

    if (bits & UserCommandBits::Impulse) {
        to->input.impulse = MSG_ReadUint8();
    }

    // Read time to run command.
    to->input.msec = MSG_ReadUint8();

    // Read the light level.
    to->input.lightLevel = MSG_ReadUint8();
}
#endif