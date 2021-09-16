#include "../clg_local.h"

#include "../clg_effects.h"
#include "../clg_entities.h"
#include "../clg_input.h"
#include "../clg_main.h"
#include "../clg_media.h"
#include "../clg_parse.h"
#include "../clg_predict.h"
#include "../clg_screen.h"
#include "../clg_tents.h"
#include "../clg_view.h"

#include "shared/interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Prediction.h"

// Distance that is allowed to be taken as a delta before we reset it.
static const float MAX_DELTA_ORIGIN = (2400.f * (1.0f / BASE_FRAMERATE));


//---------------
// ClientGamePrediction::CheckPredictionError
//
//---------------
void ClientGamePrediction::CheckPredictionError(ClientUserCommand* clientUserCommand) {
    const PlayerMoveState* in = &cl->frame.playerState.pmove;
    ClientPredictedState* out = &cl->predictedState;

    // if prediction was not run (just spawned), don't sweat it
    if (clientUserCommand->prediction.simulationTime == 0) {
        out->viewOrigin = in->origin;
        out->viewOffset = in->viewOffset;
        out->viewAngles = in->viewAngles;
        out->stepOffset = 0.f;

        out->error = vec3_zero();
        return;
    }

    // Subtract what the server returned from our predicted origin for that frame
    out->error = clientUserCommand->prediction.error = (clientUserCommand->prediction.origin - in->origin);

    // If the error is too large, it was likely a teleport or respawn, so ignore it
    const float len = vec3_length(out->error);
    if (len > .1f) {
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

//---------------
// ClientGamePrediction::PredictAngles
//
//---------------
void ClientGamePrediction::PredictAngles() {
    cl->predictedState.viewAngles[0] = cl->viewAngles[0] + cl->frame.playerState.pmove.deltaAngles[0];
    cl->predictedState.viewAngles[1] = cl->viewAngles[1] + cl->frame.playerState.pmove.deltaAngles[1];
    cl->predictedState.viewAngles[2] = cl->viewAngles[2] + cl->frame.playerState.pmove.deltaAngles[2];
}

//---------------
// ClientGamePrediction::CheckPredictionError
//
//---------------
void ClientGamePrediction::PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex) {
    PlayerMove   pm = {};

    if (!acknowledgedCommandIndex || !currentCommandIndex)
        return;

    // Setup base trace calls.
    pm.Trace = CLG_Trace;
    pm.PointContents = CLG_PointContents;

    // Restore ground entity for this frame.
    pm.groundEntityPtr = cl->predictedState.groundEntityPtr;

    // Copy current state to pmove
    pm.state = cl->frame.playerState.pmove;
#if USE_SMOOTH_DELTA_ANGLES
    pm.state.deltaAngles = cl->deltaAngles;
#endif

    // Run frames in order.
    while (++acknowledgedCommandIndex <= currentCommandIndex) {
        // Fetch the command.
        ClientUserCommand* cmd = &cl->clientUserCommands[acknowledgedCommandIndex & CMD_MASK];

        // Execute a pmove with it.
        if (cmd->moveCommand.msec) {
            // Saved for prediction error checking.
            cmd->prediction.simulationTime = clgi.GetRealTime();

            pm.clientUserCommand = *cmd;
            PMove(&pm);

            // Update player move client side audio effects.
            CLG_UpdateClientSoundSpecialEffects(&pm);
        }

        // Save for error detection
        cmd->prediction.origin = pm.state.origin;
    }

    // Run pending cmd
    if (cl->clientUserCommand.moveCommand.msec) {
        // Saved for prediction error checking.
        cl->clientUserCommand.prediction.simulationTime = clgi.GetRealTime();

        pm.clientUserCommand = cl->clientUserCommand;
        pm.clientUserCommand.moveCommand.forwardMove = cl->localmove[0];
        pm.clientUserCommand.moveCommand.rightMove = cl->localmove[1];
        pm.clientUserCommand.moveCommand.upMove = cl->localmove[2];
        PMove(&pm);

        // Update player move client side audio effects.
        CLG_UpdateClientSoundSpecialEffects(&pm);

        // Save for error detection
        cl->clientUserCommand.prediction.origin = pm.state.origin;
    }

    // Copy results out for rendering
    cl->predictedState.viewOrigin = pm.state.origin;
    //cl->predictedState.velocity    = pm.state.velocity;
    cl->predictedState.viewOffset = pm.state.viewOffset;
    cl->predictedState.stepOffset = pm.state.stepOffset;
    cl->predictedState.viewAngles = pm.viewAngles;

    cl->predictedState.groundEntityPtr = pm.groundEntityPtr;
}