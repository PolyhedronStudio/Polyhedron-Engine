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
#include "common/x86/fpu.h"

/*
===================
CL_CheckPredictionError
===================
*/
void CL_CheckPredictionError(void)
{
    int         frame;
    //int         delta[3];
    unsigned    cmd;
    //int         len;

    if (!cls.netchan) {
        return;
    }

    if (sv_paused->integer) {
        VectorClear(cl.prediction_error);
        return;
    }

    if (!cl_predict->integer || (cl.frame.playerState.pmove.flags & PMF_NO_PREDICTION))
        return;

    // calculate the last usercmd_t we sent that the server has processed
    frame = cls.netchan->incomingAcknowledged & CMD_MASK;
    cmd = cl.history[frame].cmdNumber;

    // N&C: Call into the CG Module to let it handle this.
    CL_GM_CheckPredictionError(frame, cmd);
}

/*
====================
CL_ClipMoveToEntities

====================
*/
static void CL_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, trace_t *tr)
{
    int         i;
    trace_t     trace;
    mnode_t     *headNode;
    cl_entity_t   *ent;
    mmodel_t    *cmodel;

    for (i = 0; i < cl.numSolidEntities; i++) {
        ent = cl.solidEntities[i];

        if (ent->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl.model_clip[ent->current.modelindex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;
        } else {
            headNode = CM_HeadnodeForBox(ent->mins, ent->maxs);
        }

        if (tr->allSolid)
            return;

        CM_TransformedBoxTrace(&trace, start, end,
                               mins, maxs, headNode,  CONTENTS_MASK_PLAYERSOLID,
                               ent->current.origin, ent->current.angles);

        CM_ClipEntity(tr, &trace, (struct entity_s *)ent);
    }
}


/*
================
CL_PMTrace
================
*/
static trace_t q_gameabi CL_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end)
{
    trace_t    t;

    // check against world
    CM_BoxTrace(&t, start, end, mins, maxs, cl.bsp->nodes, CONTENTS_MASK_PLAYERSOLID);
    if (t.fraction < 1.0)
        t.ent = (struct entity_s *)1;

    // check all other solid models
    CL_ClipMoveToEntities(start, mins, maxs, end, &t);

    return t;
}

static int CL_PointContents(const vec3_t &point)
{
    int         i;
    cl_entity_t   *ent;
    mmodel_t    *cmodel;
    int         contents;

    contents = CM_PointContents(point, cl.bsp->nodes);

    for (i = 0; i < cl.numSolidEntities; i++) {
        ent = cl.solidEntities[i];

        if (ent->current.solid != PACKED_BSP) // special value for bmodel
            continue;

        cmodel = cl.model_clip[ent->current.modelindex];
        if (!cmodel)
            continue;

        contents |= CM_TransformedPointContents(
                        point, cmodel->headNode,
                        ent->current.origin,
                        ent->current.angles);
    }

    return contents;
}

/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictAngles(void)
{
    //cl.predicted_angles[0] = cl.viewAngles[0] + SHORT2ANGLE(cl.frame.playerState.pmove.delta_angles[0]);
    //cl.predicted_angles[1] = cl.viewAngles[1] + SHORT2ANGLE(cl.frame.playerState.pmove.delta_angles[1]);
    //cl.predicted_angles[2] = cl.viewAngles[2] + SHORT2ANGLE(cl.frame.playerState.pmove.delta_angles[2]);
}

void CL_PredictMovement(void)
{
    unsigned    ack, current;

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

    ack = cl.history[cls.netchan->incomingAcknowledged & CMD_MASK].cmdNumber;
    current = cl.cmdNumber;

    // if we are too far out of date, just freeze
    if (current - ack > CMD_BACKUP - 1) {
        SHOWMISS("%i: exceeded CMD_BACKUP\n", cl.frame.number);
        return;
    }

    if (!cl.cmd.msec && current == ack) {
        SHOWMISS("%i: not moved\n", cl.frame.number);
        return;
    }

    // N&C: Call into the CG Module to let it handle this.
    CL_GM_PredictMovement(ack, current);
}

