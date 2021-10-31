#include "clg_local.h"

#include "clg_effects.h"
#include "clg_entities.h"
#include "clg_input.h"
#include "clg_main.h"
#include "clg_media.h"
#include "clg_parse.h"
#include "clg_predict.h"
#include "clg_screen.h"
#include "clg_tents.h"
#include "clg_view.h"

#include "shared/interfaces/IClientGameExports.h"
#include "ClientGameExports.h"

//---------------
// ClientGameExports::ClientCalculateFieldOfView
//
//---------------
float ClientGameExports::ClientCalculateFieldOfView(float fieldOfViewX, float width, float height) {
    float    a;
    float    x;

    if (fieldOfViewX < 1.f || fieldOfViewX > 179.f)
        Com_Error(ERR_DROP, "%s: bad fov: %f", __func__, fieldOfViewX);

    x = width / tan(fieldOfViewX / 360.f * M_PI);

    a = atan(height / x);
    a = a * 360.f / M_PI;

    return a;
}

//---------------
// ClientGameExports::ClientClearState
//
//---------------
void ClientGameExports::ClientClearState() {
    // Clear Effects.
    CLG_ClearEffects();

    // WID: TODO: I think this #ifdef can go lol.
#if USE_LIGHTSTYLES
    CLG_ClearLightStyles();
#endif
    CLG_ClearTempEntities();
}

//---------------
// ClientGameExports::ClientUpdateOrigin
//
//---------------
void ClientGameExports::ClientUpdateOrigin() {
    PlayerState* currentPlayerState = NULL;
    PlayerState* previousPlayerState = NULL;

    float lerpFraction = cl->lerpFraction;

    // Only do this if we had a valid frame.
    if (!cl->frame.valid) {
        return;
    }

    // Find states to interpolate between
    currentPlayerState = &cl->frame.playerState;
    previousPlayerState = &cl->oldframe.playerState;

    //
    // Origin
    //
    if (!clgi.IsDemoPlayback()
        && cl_predict->integer
        && !(currentPlayerState->pmove.flags & PMF_NO_PREDICTION)) {
        // Add view offset to view org.
        ClientPredictedState* predictedState = &cl->predictedState;
        cl->refdef.vieworg = predictedState->viewOrigin + predictedState->viewOffset;

        const vec3_t error = vec3_scale(predictedState->error, 1.f - lerpFraction);
        cl->refdef.vieworg += error;

        cl->refdef.vieworg.z -= predictedState->stepOffset;
    }
    else {
        // Just use interpolated values
        // Adjust origins to keep stepOffset in mind.
        vec3_t oldOrigin = previousPlayerState->pmove.origin + previousPlayerState->pmove.viewOffset;
        oldOrigin.z -= cl->predictedState.stepOffset;
        vec3_t newOrigin = currentPlayerState->pmove.origin + currentPlayerState->pmove.viewOffset;
        newOrigin.z -= cl->predictedState.stepOffset;

        // Calculate final origin.
        cl->refdef.vieworg = vec3_mix(oldOrigin, newOrigin, lerpFraction);
    }

    //
    // View Angles.
    //
    // if not running a demo or on a locked frame, add the local angle movement
    if (clgi.IsDemoPlayback()) {
        LerpAngles(previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction, cl->refdef.viewAngles);
    }
    else if (currentPlayerState->pmove.type < EnginePlayerMoveType::Dead) {
        // use predicted values
        cl->refdef.viewAngles = cl->predictedState.viewAngles;
    }
    else {
        // just use interpolated values
        LerpAngles(previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction, cl->refdef.viewAngles);
    }

#if USE_SMOOTH_DELTA_ANGLES
    cl->deltaAngles[0] = LerpAngle(previousPlayerState->pmove.deltaAngles[0], currentPlayerState->pmove.deltaAngles[0], lerpFraction);
    cl->deltaAngles[1] = LerpAngle(previousPlayerState->pmove.deltaAngles[1], currentPlayerState->pmove.deltaAngles[1], lerpFraction);
    cl->deltaAngles[2] = LerpAngle(previousPlayerState->pmove.deltaAngles[2], currentPlayerState->pmove.deltaAngles[2], lerpFraction);
#endif

    // don't interpolate blend color
    Vector4Copy(currentPlayerState->blend, cl->refdef.blend);

    // Interpolate field of view
    cl->fov_x = LerpFieldOfView(previousPlayerState->fov, currentPlayerState->fov, lerpFraction);
    cl->fov_y = CLG_CalculateFOV(cl->fov_x, 4, 3);

    // Calculate new client forward, right, and up vectors.
    vec3_vectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    // Setup player entity origin and angles accordingly to update the client's listener origins with.
    cl->playerEntityOrigin = cl->refdef.vieworg;
    cl->playerEntityAngles = cl->refdef.viewAngles;

    if (cl->playerEntityAngles[vec3_t::Pitch] > 180) {
        cl->playerEntityAngles[vec3_t::Pitch] -= 360;
    }

    cl->playerEntityAngles[vec3_t::Pitch] = cl->playerEntityAngles[vec3_t::Pitch] / 3;


    // Update the client's listener origin values.
    clgi.UpdateListenerOrigin();
}

//---------------
// ClientGameExports::DemoSeek
//
//---------------
void ClientGameExports::DemoSeek() {
    // Clear Effects.
    CLG_ClearEffects();
    // Clear Temp Entities.
    CLG_ClearTempEntities();
}

//---------------
// ClientGameExports::ClientBegin
//
//---------------
void ClientGameExports::ClientBegin() {

}

//---------------
// ClientGameExports::ClientDeltaFrame
//
//---------------
void ClientGameExports::ClientDeltaFrame() {
    // Called each time a valid client frame has been 
    SCR_SetCrosshairColor();
}

//---------------
// ClientGameExports::ClientFrame
//
//---------------
void ClientGameExports::ClientFrame() {
    // Advance local effects.
#if USE_DLIGHTS
    CLG_RunDLights();
#endif
#if USE_LIGHTSTYLES
    CLG_RunLightStyles();
#endif
}

//---------------
// ClientGameExports::ClientDisconnect
//
//---------------
void ClientGameExports::ClientDisconnect() {
    // Clear the chat hud.
    SCR_ClearChatHUD_f();
}


//---------------
// ClientGameExports::ClientUpdateUserinfo
//
//---------------
void ClientGameExports::ClientUpdateUserinfo(cvar_t* var, from_t from) {
    // If there is a skin change, let's go for it.
    if (var == info_skin && from > FROM_CONSOLE) {
        char sk[MAX_QPATH];
        Q_strlcpy(sk, info_skin->string, sizeof(sk));
    }
}


//---------------
// ClientGameExports::LerpFieldOfView
//
//---------------
float ClientGameExports::LerpFieldOfView(float oldFieldOfView, float newFieldOfView, float lerp) {
    if (clgi.IsDemoPlayback()) {
        float fov = info_fov->value;

        if (fov < 1)
            fov = 90;
        else if (fov > 160)
            fov = 160;

        if (info_uf->integer & UserFields::LocalFieldOfView)
            return fov;

        if (!(info_uf->integer & UserFields::PlayerFieldOfView)) {
            if (oldFieldOfView >= 90)
                oldFieldOfView = fov;
            if (newFieldOfView >= 90)
                newFieldOfView = fov;
        }
    }

    return oldFieldOfView + lerp * (newFieldOfView - oldFieldOfView);
}