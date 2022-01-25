// LICENSE HERE.

//
// clg_predict.c
//
//
// Movement prediction implementation for the client side.
//
#include "ClientGameLocal.h"

#include "Main.h"

//
//===============
// CLG_CheckPredictionError
// 
// Checks for prediction errors.
//================
//
//void CLG_CheckPredictionError(ClientMoveCommand *moveCommand) {
//    const PlayerMoveState* in = &cl->frame.playerState.pmove;
//    ClientPredictedState* out = &cl->predictedState;
//
//    // if prediction was not run (just spawned), don't sweat it
//    if (moveCommand->prediction.simulationTime == 0) {
//        out->viewOrigin = in->origin;
//        out->viewOffset = in->viewOffset;
//        out->viewAngles = in->viewAngles;
//        out->stepOffset = 0.f;
//
//        out->error = vec3_zero();
//        return;
//    }
//
//    // Subtract what the server returned from our predicted origin for that frame
//    out->error = moveCommand->prediction.error = (moveCommand->prediction.origin - in->origin);
//
//    // If the error is too large, it was likely a teleport or respawn, so ignore it
//    const float len = vec3_length(out->error);
//    if (len > .1f) {
//        if (len > MAX_DELTA_ORIGIN) {
//            Com_DPrint("CLG_PredictionError: if (len > MAX_DELTA_ORIGIN): %s\n", Vec3ToString(out->error));
//
//            out->viewOrigin = in->origin;
//            out->viewOffset = in->viewOffset;
//            out->viewAngles = in->viewAngles;
//            out->stepOffset = 0.f;
//
//            out->error = vec3_zero();
//        }
//        else {
//            Com_DPrint("CLG_PredictionError: %s\n", Vec3ToString(out->error));
//        }
//    }
//}

//
//===============
// CLG_PredictAngles
// 
// Sets the predicted angles.
//================
//
//void CLG_PredictAngles(void) {
//    cl->predictedState.viewAngles[0] = cl->viewAngles[0] + cl->frame.playerState.pmove.deltaAngles[0];
//    cl->predictedState.viewAngles[1] = cl->viewAngles[1] + cl->frame.playerState.pmove.deltaAngles[1];
//    cl->predictedState.viewAngles[2] = cl->viewAngles[2] + cl->frame.playerState.pmove.deltaAngles[2];
//}

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
    ClientEntity* ent;
    mmodel_t* cmodel;

    for (i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl->clipModels[ent->current.modelIndex];
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
trace_t q_gameabi CLG_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
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

int CLG_PointContents(const vec3_t &point)
{
    int         i;
    ClientEntity* ent;
    mmodel_t* cmodel;
    int         contents;

    contents = clgi.CM_PointContents(point, cl->bsp->nodes);

    for (i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid != PACKED_BSP) // special value for bmodel
            continue;

        cmodel = cl->clipModels[ent->current.modelIndex];
        if (!cmodel)
            continue;

        contents |= clgi.CM_TransformedPointContents(
            point, cmodel->headNode,
            ent->current.origin,
            ent->current.angles);
    }

    return contents;
}