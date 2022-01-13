#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Input.h"
#include "../Main.h"
#include "../Media.h"
#include "../Parse.h"
#include "../Predict.h"
#include "../Screen.h"
#include "../TemporaryEntities.h"
#include "../View.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Prediction.h"

// Distance that is allowed to be taken as a delta before we reset it.
static const float MAX_DELTA_ORIGIN = (2400.f * (1.0f / BASE_FRAMERATE));


//---------------
// ClientGamePrediction::CheckPredictionError
//
//---------------
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
        ClientMoveCommand* cmd = &cl->clientUserCommands[acknowledgedCommandIndex & CMD_MASK];

        // Execute a pmove with it.
        if (cmd->input.msec) {
            // Saved for prediction error checking.
            cmd->prediction.simulationTime = clgi.GetRealTime();

            pm.moveCommand = *cmd;
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
        pm.moveCommand.input.forwardMove = cl->localmove[0];
        pm.moveCommand.input.rightMove = cl->localmove[1];
        pm.moveCommand.input.upMove = cl->localmove[2];
        PMove(&pm);

        // Update player move client side audio effects.
        UpdateClientSoundSpecialEffects(&pm);

        // Save for error detection
        cl->moveCommand.prediction.origin = pm.state.origin;
    }

    // Copy results out for rendering
    cl->predictedState.viewOrigin = pm.state.origin;
    //cl->predictedState.velocity    = pm.state.velocity;
    cl->predictedState.viewOffset = pm.state.viewOffset;
    cl->predictedState.stepOffset = pm.state.stepOffset;
    cl->predictedState.viewAngles = pm.viewAngles;

    cl->predictedState.groundEntityPtr = pm.groundEntityPtr;
}

//
//================
// PM_UpdateClientSoundSpecialEffects
//
// Can be called by either the server or the client
//================
//
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