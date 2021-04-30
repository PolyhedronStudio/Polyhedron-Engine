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
    if (!cls.netchan) {
        return;
    }

    if (sv_paused->integer) {
        cl.predictedState.error = vec3_zero();
        return;
    }

    if (!cl_predict->integer || (cl.frame.playerState.pmove.flags & PMF_NO_PREDICTION))
        return;

    // Calculate the last cl_cmd_t we sent that the server has processed
    cl_cmd_t* cmd = &cl.cmds[cls.netchan->incomingAcknowledged & CMD_MASK];

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
    if (cls.state != ca_active) {
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
    uint32_t ack = cl.history[cls.netchan->incomingAcknowledged & CMD_MASK].commandNumber;
    uint32_t currentFrameIndex = cl.commandNumber;

    // If we are too far out of date, just freeze
    if (currentFrameIndex - ack > CMD_BACKUP - 1) {
        SHOWMISS("%i: exceeded CMD_BACKUP\n", cl.frame.number);
        return;
    }

    // Ensure we had moved.
    if (!cl.cmd.cmd.msec && currentFrameIndex == ack) {
        SHOWMISS("%i: not moved\n", cl.frame.number);
        return;
    }

    // Call into the CG Module to let it handle this.
    CL_GM_PredictMovement(ack, currentFrameIndex);
}

