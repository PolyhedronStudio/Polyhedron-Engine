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


const ClientMoveCommand nullUserCmd = {};

#if USE_CLIENT
/**
*   @brief Write a client's delta move command.
**/
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

    if (cmd->input.viewAngles[0] != from->input.viewAngles[0]) {
        bits |= UserCommandBits::AngleX;
    }
    if (cmd->input.viewAngles[1] != from->input.viewAngles[1]) {
        bits |= UserCommandBits::AngleY;
    }
    if (cmd->input.viewAngles[2] != from->input.viewAngles[2]) {
        bits |= UserCommandBits::AngleZ;
    }
    if (cmd->input.forwardMove != from->input.forwardMove) {
        bits |= UserCommandBits::Forward;
    }
    if (cmd->input.rightMove != from->input.rightMove) {
        bits |= UserCommandBits::Side;
    }
    if (cmd->input.upMove != from->input.upMove) {
        bits |= UserCommandBits::Up;
    }
    if (cmd->input.buttons != from->input.buttons) {
        bits |= UserCommandBits::Buttons;
    }
    if (cmd->input.impulse != from->input.impulse) {
        bits |= UserCommandBits::Impulse;
    }

    // Write out the changed bits.
    MSG_WriteUint8(bits);

    if (bits & UserCommandBits::AngleX) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[0]);
    }
    if (bits & UserCommandBits::AngleY) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[1]);
    }
    if (bits & UserCommandBits::AngleZ) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[2]);
    }

    if (bits & UserCommandBits::Forward) {
        MSG_WriteInt16(cmd->input.forwardMove);
    }
    if (bits & UserCommandBits::Side) {
        MSG_WriteInt16(cmd->input.rightMove);
    }
    if (bits & UserCommandBits::Up) {
        MSG_WriteInt16(cmd->input.upMove);
    }

    if (bits & UserCommandBits::Buttons) {
        MSG_WriteUint8(cmd->input.buttons);
    }

    if (bits & UserCommandBits::Impulse) {
        MSG_WriteUint8(cmd->input.impulse);
    }

    MSG_WriteUint8(cmd->input.msec);
    MSG_WriteUint8(cmd->input.lightLevel);

    // (Returned bits isn't used anywhere, but might as well keep it around.)
    return bits;
}

#endif // USE_CLIENT