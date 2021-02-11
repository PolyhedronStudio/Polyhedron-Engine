// LICENSE HERE.

//
// clg_predict.c
//
//
// Movement prediction implementation for the client side.
//
#include "clg_local.h"

//
//===============
// CLG_CheckPredictionError
// 
// Checks for prediction errors.
//================
//
void CLG_CheckPredictionError(int frame, unsigned int cmd) {
    int delta[3];
    int len;
    // compare what the server returned with what we had predicted it to be
    VectorSubtract(cl->frame.ps.pmove.origin, cl->predicted_origins[cmd & CMD_MASK], delta);

    // save the prediction error for interpolation
    len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
    if (len < 1 || len > 640) {
        // > 80 world units is a teleport or something
        VectorClear(cl->prediction_error);
        return;
    }

    // TODO: Add Debug functions to CG Module.
    //SHOWMISS("prediction miss on %i: %i (%d %d %d)\n",
    //    cl->frame.number, len, delta[0], delta[1], delta[2]);

    // don't predict steps against server returned data
    if (cl->predicted_step_frame <= cmd)
        cl->predicted_step_frame = cmd + 1;

    VectorCopy(cl->frame.ps.pmove.origin, cl->predicted_origins[cmd & CMD_MASK]);

    // save for error interpolation
    VectorScale(delta, 0.125f, cl->prediction_error);
}

//
//===============
// CLG_PredictAngles
// 
// Sets the predicted angles.
//================
//
void CLG_PredictAngles(void) {
    cl->predicted_angles[0] = cl->viewangles[0] + SHORT2ANGLE(cl->frame.ps.pmove.delta_angles[0]);
    cl->predicted_angles[1] = cl->viewangles[1] + SHORT2ANGLE(cl->frame.ps.pmove.delta_angles[1]);
    cl->predicted_angles[2] = cl->viewangles[2] + SHORT2ANGLE(cl->frame.ps.pmove.delta_angles[2]);
}

/*
====================
CL_ClipMoveToEntities

====================
*/
static void CLG_ClipMoveToEntities(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, trace_t* tr)
{
    int         i;
    trace_t     trace;
    mnode_t* headnode;
    centity_t* ent;
    mmodel_t* cmodel;

    for (i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl->model_clip[ent->current.modelindex];
            if (!cmodel)
                continue;
            headnode = cmodel->headnode;
        }
        else {
            headnode = clgi.CM_HeadnodeForBox(ent->mins, ent->maxs);
        }

        if (tr->allsolid)
            return;

        clgi.CM_TransformedBoxTrace(&trace, start, end,
            mins, maxs, headnode, MASK_PLAYERSOLID,
            ent->current.origin, ent->current.angles);

        clgi.CM_ClipEntity(tr, &trace, (struct edict_s*)ent);
    }
}

/*
================
CL_PMTrace
================
*/
static trace_t q_gameabi CLG_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
    trace_t    t;

    // check against world
    clgi.CM_BoxTrace(&t, start, end, mins, maxs, cl->bsp->nodes, MASK_PLAYERSOLID);
    if (t.fraction < 1.0)
        t.ent = (struct edict_s*)1;

    // check all other solid models
    CLG_ClipMoveToEntities(start, mins, maxs, end, &t);

    return t;
}

static int CLG_PointContents(vec3_t point)
{
    int         i;
    centity_t* ent;
    mmodel_t* cmodel;
    int         contents;

    contents = clgi.CM_PointContents(point, cl->bsp->nodes);

    for (i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid != PACKED_BSP) // special value for bmodel
            continue;

        cmodel = cl->model_clip[ent->current.modelindex];
        if (!cmodel)
            continue;

        contents |= clgi.CM_TransformedPointContents(
            point, cmodel->headnode,
            ent->current.origin,
            ent->current.angles);
    }

    return contents;
}

//
//===============
// CLG_PredictMovement
// 
// Predicts the actual client side movement.
//================
//
void CLG_PredictMovement(unsigned int ack, unsigned int current) {
    pmove_t     pm;
    int         step, oldz;
    int         frame;

    X86_PUSH_FPCW;
    X86_SINGLE_FPCW;

    // copy current state to pmove
    memset(&pm, 0, sizeof(pm));
    pm.trace = CLG_Trace;
    pm.pointcontents = CLG_PointContents;

    pm.s = cl->frame.ps.pmove;
#if USE_SMOOTH_DELTA_ANGLES
    VectorCopy(cl->delta_angles, pm.s.delta_angles);
#endif

    // run frames
    while (++ack <= current) {
        pm.cmd = cl->cmds[ack & CMD_MASK];
        Pmove(&pm, &cl->pmp);

        // save for debug checking
        VectorCopy(pm.s.origin, cl->predicted_origins[ack & CMD_MASK]);
    }

    // run pending cmd
    if (cl->cmd.msec) {
        pm.cmd = cl->cmd;
        pm.cmd.forwardmove = cl->localmove[0];
        pm.cmd.sidemove = cl->localmove[1];
        pm.cmd.upmove = cl->localmove[2];
        Pmove(&pm, &cl->pmp);
        frame = current;

        // save for debug checking
        VectorCopy(pm.s.origin, cl->predicted_origins[(current + 1) & CMD_MASK]);
    }
    else {
        frame = current - 1;
    }

    X86_POP_FPCW;

    if (pm.s.pm_type != PM_SPECTATOR && (pm.s.pm_flags & PMF_ON_GROUND)) {
        oldz = cl->predicted_origins[cl->predicted_step_frame & CMD_MASK][2];
        step = pm.s.origin[2] - oldz;
        if (step > 63 && step < 160) {
            cl->predicted_step = step * 0.125f;
            cl->predicted_step_time = clgi.GetRealTime();
            cl->predicted_step_frame = frame + 1;    // don't double step
        }
    }

    if (cl->predicted_step_frame < frame) {
        cl->predicted_step_frame = frame;
    }

    // copy results out for rendering
    VectorScale(pm.s.origin, 0.125f, cl->predicted_origin);
    VectorScale(pm.s.velocity, 0.125f, cl->predicted_velocity);
    VectorCopy(pm.viewangles, cl->predicted_angles);
}