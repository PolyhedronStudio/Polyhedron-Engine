// LICENSE HERE.

//
// clg_predict.c
//
//
// Movement prediction implementation for the client side.
//
#include "clg_local.h"

#include "clg_main.h"

//
//===============
// CLG_CheckPredictionError
// 
// Checks for prediction errors.
//================
//
void CLG_CheckPredictionError(int frame, unsigned int cmd) {
    //vec3_t delta;
    //float len;

    //// First frame.
    //if (cl->frame.number == 0) {
    //    cl->predictedState.viewOrigin = cl->frame.playerState.pmove.origin;
    //    cl->predictedState.viewOffset = cl->frame.playerState.viewOffset;
    //    cl->predictedState.viewAngles = cl->frame.playerState.viewAngles;
    //    cl->predictedState.stepOffset = 0.f;

    //    cl->predictedState.error = vec3_zero();
    //}

    //// Compare what the server returned with what we had predicted it to be
    //cl->predictedState.error = cl->frame.playerState.pmove.origin - cl->predicted_origins[cmd & CMD_MASK];

    //// Length is 
    //len = vec3_length(cl->predictedState.error);
    //if (len > .1f) {
    //    if (len > 2400.f * (1.0f / BASE_FRAMERATE)) {
    //        Com_DPrint("MAX_DELTA_ORIGIN: %s\n", Vec3ToString(cl->predictedState.error));

    //        cl->predictedState.viewOrigin = cl->frame.playerState.pmove.origin;
    //        cl->predictedState.viewOffset = cl->frame.playerState.viewOffset;
    //        cl->predictedState.viewAngles = cl->frame.playerState.viewAngles;
    //        cl->predictedState.stepOffset = 0.f;

    //        cl->predictedState.error         = vec3_zero();
    //    }
    //    else {
    //        Com_DPrint("CLG_CheckPredictionError: %s len:%f\n", Vec3ToString(cl->predictedState.error), len);
    //    }
    //}

    //cl->predicted_origins[cmd & CMD_MASK] = cl->frame.playerState.pmove.origin;
}

//
//===============
// CLG_PredictAngles
// 
// Sets the predicted angles.
//================
//
void CLG_PredictAngles(void) {
    cl->predictedState.viewAngles[0] = cl->viewAngles[0] + cl->frame.playerState.pmove.deltaAngles[0];
    cl->predictedState.viewAngles[1] = cl->viewAngles[1] + cl->frame.playerState.pmove.deltaAngles[1];
    cl->predictedState.viewAngles[2] = cl->viewAngles[2] + cl->frame.playerState.pmove.deltaAngles[2];
}

//
//===============
// CLG_ClipMoveToEntities
// 
// 
//================
//
static void CLG_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, trace_t* tr)
{
    int         i;
    trace_t     trace;
    mnode_t* headNode;
    cl_entity_t* ent;
    mmodel_t* cmodel;

    for (i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl->model_clip[ent->current.modelindex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;
        }
        else {
            headNode = clgi.CM_HeadnodeForBox(ent->mins, ent->maxs);
        }

        if (tr->allSolid)
            return;

        clgi.CM_TransformedBoxTrace(&trace, start, end,
            mins, maxs, headNode, CONTENTS_MASK_PLAYERSOLID,
            ent->current.origin, ent->current.angles);

        clgi.CM_ClipEntity(tr, &trace, (struct entity_s*)ent);
    }
}

/*
================
CL_PMTrace
================
*/
static trace_t q_gameabi CLG_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
{
    trace_t    t;

    // check against world
    clgi.CM_BoxTrace(&t, start, end, mins, maxs, cl->bsp->nodes, CONTENTS_MASK_PLAYERSOLID);
    if (t.fraction < 1.0)
        t.ent = (struct entity_s*)1;

    // check all other solid models
    CLG_ClipMoveToEntities(start, mins, maxs, end, &t);

    return t;
}

static int CLG_PointContents(const vec3_t &point)
{
    int         i;
    cl_entity_t* ent;
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
            point, cmodel->headNode,
            ent->current.origin,
            ent->current.angles);
    }

    return contents;
}

//
//================
// PM_UpdateClientSoundSpecialEffects
//
// Can be called by either the server or the client
//================
//
static void CLG_UpdateClientSoundSpecialEffects(pm_move_t* pm)
{
    static int underwater;

    // Ensure that cl != NULL, it'd be odd but hey..
    if (cl == NULL) {
        return;
    }

    if ((pm->waterLevel == 3) && !underwater) {
        underwater = 1;
        cl->snd_is_underwater = 1; // OAL: snd_is_underwater moved to client struct.
// TODO: DO!
#ifdef USE_OPENAL
        if (cl->snd_is_underwater_enabled)
            clgi.SFX_Underwater_Enable();
#endif
    }

    if ((pm->waterLevel < 3) && underwater) {
        underwater = 0;
        cl->snd_is_underwater = 0; // OAL: snd_is_underwater moved to client struct.

// TODO: DO!
#ifdef USE_OPENAL
        if (cl->snd_is_underwater_enabled)
            clgi.SFX_Underwater_Disable();
#endif
    }
}

//
//===============
// CLG_PredictMovement
// 
// Predicts the actual client side movement.
//================
//
void CLG_PredictMovement(unsigned int ack, unsigned int currentFrame) {
    pm_move_t   pm = {};
    uint32_t    frameIndex;

    // copy current state to pmove
    memset(&pm, 0, sizeof(pm));
    pm.Trace = CLG_Trace;
    pm.PointContents = CLG_PointContents;
    pm.state = cl->frame.playerState.pmove;
#if USE_SMOOTH_DELTA_ANGLES
    VectorCopy(cl->deltaAngles, pm.state.deltaAngles);
#endif

    // run frames
    while (++ack <= currentFrame) {
        pm.cmd = cl->cmds[ack & CMD_MASK];
        PMove(&pm, &clg.pmoveParams);

        // Update player move client side audio effects.
        CLG_UpdateClientSoundSpecialEffects(&pm);

        // save for debug checking
        cl->predicted_origins[ack & CMD_MASK] = pm.state.origin;
    }

    // run pending cmd
    if (cl->cmd.cmd.msec) {
        pm.cmd = cl->cmd;
        pm.cmd.cmd.forwardmove = cl->localmove[0];
        pm.cmd.cmd.rightmove = cl->localmove[1];
        pm.cmd.cmd.upmove = cl->localmove[2];
        PMove(&pm, &clg.pmoveParams);
        frameIndex = currentFrame;


        // save for debug checking
        cl->predicted_origins[(currentFrame + 1) & CMD_MASK] = pm.state.origin;
    } else {
        frameIndex = currentFrame - 1;
    }

    // copy results out for rendering
    cl->predictedState.viewOrigin  = pm.state.origin;
    cl->predictedState.velocity    = pm.state.velocity;
    cl->predictedState.stepOffset  = pm.state.stepOffset;
    cl->predictedState.viewOffset  = pm.state.viewOffset;
    cl->predictedState.viewAngles  = pm.viewAngles;
}