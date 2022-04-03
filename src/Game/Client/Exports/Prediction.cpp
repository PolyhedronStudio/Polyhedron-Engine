#include "../ClientGameLocals.h"

#include "../Entities.h"
#include "../TemporaryEntities.h"

// Exports.
#include "../ClientGameExports.h"
#include "Prediction.h"
#include "View.h"

// Distance that is allowed to be taken as a delta before we reset it.
static const double MAX_DELTA_ORIGIN = (2400.0 * (1.00 / BASE_FRAMERATE));


/**
*   @brief  Checks for prediction incorectness. If found, corrects it.
**/
void ClientGamePrediction::CheckPredictionError(ClientMoveCommand* moveCommand) {
    const PlayerMoveState* in = &cl->frame.playerState.pmove;
    ClientPredictedState* out = &cl->predictedState;

    // if prediction was not run (just spawned), don't sweat it
    if (moveCommand->prediction.simulationTime == 0) {
        out->viewOrigin = in->origin;
        out->viewOffset = in->viewOffset;
        out->viewAngles = in->viewAngles;
        out->stepOffset = 0.f;

        out->error = vec3_zero();
        return;
    }

    // Subtract what the server returned from our predicted origin for that frame
    out->error = moveCommand->prediction.error = (moveCommand->prediction.origin - in->origin);

    // If the error is too large, it was likely a teleport or respawn, so ignore it
    const float len = vec3_length(out->error);
    if (len > .1) {
        if (len > MAX_DELTA_ORIGIN) {
            Com_DPrint("CLG_PredictionError: if (len > MAX_DELTA_ORIGIN): %s\n", Vec3ToString(out->error));

            out->viewOrigin = in->origin;
            out->viewOffset = in->viewOffset;
            out->viewAngles = in->viewAngles;
            out->stepOffset = 0.f;

            out->error = vec3_zero();
        }
        else {
            Com_DPrint("CLG_PredictionError: %s\n", Vec3ToString(out->error));
        }
    }
}

/**
*   @brief  Adds the delta angles to the view angles. Required for other
*           (especially rotating) objects to be able to push the player around properly.
**/
void ClientGamePrediction::PredictAngles() {
    // Add delta predicted angles to our view angles. (Allows for doors and other objects to push the player properly.)
    cl->predictedState.viewAngles[0] = cl->viewAngles[0] + cl->frame.playerState.pmove.deltaAngles[0];
    cl->predictedState.viewAngles[1] = cl->viewAngles[1] + cl->frame.playerState.pmove.deltaAngles[1];
    cl->predictedState.viewAngles[2] = cl->viewAngles[2] + cl->frame.playerState.pmove.deltaAngles[2];
}

/**
*   @brief  Process the actual predict movement simulation.
**/
void ClientGamePrediction::PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex) {
    // Player Move object.
    PlayerMove pm = {};

    // Only continue if there is an acknowledged command index, or a current command index.
    if (!acknowledgedCommandIndex || !currentCommandIndex)
        return;

    // Setup base trace calls.
    pm.Trace = PM_Trace;
    pm.PointContents = PM_PointContents;

    // Restore ground entity for this frame.
    pm.groundEntityPtr = cl->predictedState.groundEntityPtr;

    // Copy current state to pmove
    pm.state = cl->frame.playerState.pmove;
#if USE_SMOOTH_DELTA_ANGLES
    pm.state.deltaAngles = clge->view->GetViewCamera()->GetViewDeltaAngles(); //cl->deltaAngles;
#endif

    // Run frames in order.
    while (++acknowledgedCommandIndex <= currentCommandIndex) {
        // Fetch the command.
        ClientMoveCommand* cmd = &cl->clientUserCommands[acknowledgedCommandIndex & CMD_MASK];

        // If the command has an msec value it means movement has taken place and we prepare for 
        // processing another simulation.
        if (cmd->input.msec) {
            // Saved for prediction error checking.
            cmd->prediction.simulationTime = clgi.GetRealTime();

            // Assign the move command.
            pm.moveCommand = *cmd;
            // Simulate the move command.
            PMove(&pm);

            // Update player move client side audio effects.
            UpdateClientSoundSpecialEffects(&pm);
        }

        // Save for error detection
        cmd->prediction.origin = pm.state.origin;
    }

    // Run pending cmd
    if (cl->moveCommand.input.msec) {
        // Saved for prediction error checking.
        cl->moveCommand.prediction.simulationTime = clgi.GetRealTime();

        pm.moveCommand = cl->moveCommand;
        pm.moveCommand.input.forwardMove = cl->localMove[0];
        pm.moveCommand.input.rightMove = cl->localMove[1];
        pm.moveCommand.input.upMove = cl->localMove[2];
        PMove(&pm);

        // Update player move client side audio effects.
        UpdateClientSoundSpecialEffects(&pm);

        // Save for error detection
        cl->moveCommand.prediction.origin = pm.state.origin;
    }

    // Copy results out for rendering
    // TODO: This isn't really the nicest way of preventing these "do not get stuck" budges.
    // Perhaps take another look at pmove to correct this in the future.
    if (vec3_distance(cl->predictedState.viewOrigin, pm.state.origin) > 0.03125f) {
        cl->predictedState.viewOrigin = pm.state.origin;
    }

    //cl->predictedState.velocity    = pm.state.velocity;
    cl->predictedState.viewOffset = pm.state.viewOffset;
    cl->predictedState.stepOffset = pm.state.stepOffset;
    cl->predictedState.viewAngles = pm.viewAngles;

    cl->predictedState.groundEntityPtr = pm.groundEntityPtr;
}

/**
*   @brief  Update the client side audio state.
**/
void ClientGamePrediction::UpdateClientSoundSpecialEffects(PlayerMove* pm)
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
//#ifdef USE_OPENAL
        if (cl->snd_is_underwater_enabled)
            clgi.SFX_Underwater_Enable();
//#endif
    }

    if ((pm->waterLevel < 3) && underwater) {
        underwater = 0;
        cl->snd_is_underwater = 0; // OAL: snd_is_underwater moved to client struct.

                                   // TODO: DO!
//#ifdef USE_OPENAL
        if (cl->snd_is_underwater_enabled)
            clgi.SFX_Underwater_Disable();
//#endif
    }
}

/**
*   @brief  Player Move Simulation Trace Wrapper.
**/
TraceResult ClientGamePrediction::PM_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end) {
    TraceResult cmTrace;
    
    cmTrace = clgi.Trace(start, mins, maxs, end, 0, BrushContentsMask::PlayerSolid);

    return cmTrace;
}

/**
*   @brief  Player Move Simulation PointContents Wrapper.
**/
int32_t ClientGamePrediction::PM_PointContents(const vec3_t &point) {
    ClientEntity* ent = nullptr;
    mmodel_t* cmodel = nullptr;

    int32_t contents = clgi.CM_PointContents(point, cl->bsp->nodes);

    for (int32_t i = 0; i < cl->numSolidEntities; i++) {
        ent = cl->solidEntities[i];

        if (ent->current.solid != PACKED_BBOX) // special value for bmodel
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