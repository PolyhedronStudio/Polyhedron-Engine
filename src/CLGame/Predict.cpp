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
void CLG_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, ClientEntity *skipEntity, const int32_t contentMask, trace_t *cmDstTrace) {
    int         i;
    // Destination cmTrace for 
    trace_t         cmSrcTrace;
    mnode_t*        headNode;
    ClientEntity*   clientEntity;
    mmodel_t*       cmodel;

    vec3_t traceStart = start;
    vec3_t traceAngles = vec3_zero();

    for (i = 0; i < cl->numSolidEntities; i++) {
        // Fetch client entity.
        clientEntity = cl->solidEntities[i];

        // This check is likely redundent but let's make sure it is there anyway for possible future changes.
        if (clientEntity == nullptr) {
            continue;
        }

        // Should we skip it?
        if (skipEntity != nullptr && skipEntity->current.number == clientEntity->current.number) {
            continue;
        }

        if (clientEntity->current.solid == PACKED_BSP) {
            // special value for bmodel
            cmodel = cl->clipModels[clientEntity->current.modelIndex];
            if (!cmodel)
                continue;
            headNode = cmodel->headNode;

            traceAngles = clientEntity->current.angles;
        } else {
            vec3_t entityMins = {0.f, 0.f, 0.f};
            vec3_t entityMaxs = {0.f, 0.f, 0.f};
            MSG_UnpackSolid32(clientEntity->current.solid, entityMins, entityMaxs);
            LerpVector(clientEntity->prev.origin, clientEntity->current.origin, cl->lerpFraction, traceStart);

            headNode = clgi.CM_HeadnodeForBox(entityMins, entityMaxs);
        }

        // TODO: probably need to add a skip entityt or so,
        //if (srcTrace->allSolid)
        //    return;

        clgi.CM_TransformedBoxTrace(&cmSrcTrace, start, end,
                                    mins, maxs, headNode, CONTENTS_MASK_PLAYERSOLID,
                                    traceStart, traceAngles);

        clgi.CM_ClipEntity(cmDstTrace, &cmSrcTrace, (struct entity_s*)clientEntity);
    }
}

/*
================
CLG_Trace
================
*/
CLGTrace CLG_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, ClientEntity *skipEntity, const int32_t contentMask = 0) {
    // Collision Model trace result.
    trace_t cmTrace;

    // Client Game Trace result, to stay uniform.
    CLGTrace clgTrace;

    //// Ensure we can pull of a proper trace.
    //if (!cl->bsp || !cl->bsp->nodes) {
    //    Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    //}

    //// Execute box trace.
    //clgi.CM_BoxTrace(&cmTrace, start, end, mins, maxs, cl->bsp->nodes, CONTENTS_MASK_PLAYERSOLID);
    //
    //if (cmTrace.fraction == 0) {
    //    
    //}
    //if (cmTrace.fraction < 1.0) {
    //    cmTrace.ent = (struct entity_s*)1;
    //}

    //// check all other solid models
    //CLG_ClipMoveToEntities(start, mins, maxs, end, skipEntity, contentMask, &cmTrace);
    cmTrace = clgi.Trace(start, mins, maxs, end, (struct entity_s*)skipEntity, contentMask);

    // Setup the client game trace.
    clgTrace.allSolid = cmTrace.allSolid;
    clgTrace.contents = cmTrace.contents;
    clgTrace.endPosition = cmTrace.endPosition;
    if (cmTrace.ent) {
        clgTrace.ent = (ClientEntity*)cmTrace.ent;
    } else {
        clgTrace.ent = nullptr;
    }
    clgTrace.fraction = cmTrace.fraction;
    clgTrace.offsets[0] = cmTrace.offsets[0];
    clgTrace.offsets[1] = cmTrace.offsets[1];
    clgTrace.offsets[2] = cmTrace.offsets[2];
    clgTrace.offsets[3] = cmTrace.offsets[3];
    clgTrace.offsets[4] = cmTrace.offsets[4];
    clgTrace.offsets[5] = cmTrace.offsets[5];
    clgTrace.offsets[6] = cmTrace.offsets[6];
    clgTrace.offsets[7] = cmTrace.offsets[7];
    clgTrace.plane = cmTrace.plane;
    clgTrace.startSolid = cmTrace.startSolid;
    clgTrace.surface = cmTrace.surface;

    return clgTrace;
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