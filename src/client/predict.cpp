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

#include "client.h"
#include "client/gamemodule.h"


//
//===============
// CLG_CheckPredictionError
// 
// Checks for prediction errors. Calls into the CG Module.
//================
//
void CL_CheckPredictionError(void)
{
    if (!cls.netChannel) {
        return;
    }

    if (sv_paused->integer) {
        cl.predictedState.error = vec3_zero();
        return;
    }

    if (!cl_predict->integer || (cl.frame.playerState.pmove.flags & PMF_NO_PREDICTION))
        return;

    // If we are too far out of date, just freeze in place
    const uint32_t last = cls.netChannel->outgoingSequence;
    uint32_t ack = cls.netChannel->incomingAcknowledged;

    if (last - ack >= CMD_BACKUP) {
        Com_DPrintf("Exceeded CMD_BACKUP\n");
        return;
    }

    // Calculate the last ClientMoveCommand we sent that the server has processed
    uint32_t frame = cls.netChannel->incomingAcknowledged & CMD_MASK;
    uint32_t commandNumber = cl.clientCommandHistory[frame].commandNumber;

    ClientMoveCommand* cmd = &cl.clientUserCommands[commandNumber & CMD_MASK];

    // Call into the CG Module to let it handle this.
    CL_GM_CheckPredictionError(cmd);
}


/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictMovement(void)
{
    if (cls.connectionState != ClientConnectionState::Active) {
        return;
    }

    if (cls.demo.playback) {
        return;
    }

    if (sv_paused->integer) {
        return;
    }

    if (!cl_predict->integer || (cl.frame.playerState.pmove.flags & PMF_NO_PREDICTION)) {
        // N&C: Call into the CG Module to let it handle this.
        // just set angles
        CL_GM_PredictAngles();
        return;
    }

    // Fetch acknowledged command and frame.
    uint32_t ack = cl.clientCommandHistory[cls.netChannel->incomingAcknowledged & CMD_MASK].commandNumber;
    uint32_t currentFrameIndex = cl.currentClientCommandNumber;

    // If we are too far out of date, just freeze
    if (currentFrameIndex - ack > CMD_BACKUP - 1) {
        SHOWMISS("%i: exceeded CMD_BACKUP\n", cl.frame.number);
        return;
    }

    // Ensure we had moved.
    if (!cl.moveCommand.input.msec && currentFrameIndex == ack) {
        SHOWMISS("%i: not moved\n", cl.frame.number);
        return;
    }

    // Call into the CG Module to let it handle this.
    CL_GM_PredictMovement(ack, currentFrameIndex);
}

