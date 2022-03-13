#include "ClientGameLocal.h"

#include "Effects.h"
#include "Entities.h"
#include "Input.h"
#include "Main.h"
#include "Media.h"
#include "Parse.h"
#include "Predict.h"
#include "Screen.h"
#include "TemporaryEntities.h"
#include "View.h"

// Export Interfaces.
#include "Shared/Interfaces/IClientGameExports.h"

// Export Implementations.
#include "ClientGameExports.h"
#include "Exports/Core.h"
#include "Exports/Entities.h"
#include "Exports/Media.h"
#include "Exports/Movement.h"
#include "Exports/Prediction.h"
#include "Exports/Screen.h"
#include "Exports/ServerMessage.h"
#include "Exports/View.h"

//! Static 
ClientGameExports *clge = nullptr;


//! Constructor/Destructor.
ClientGameExports::ClientGameExports() {
    core = new ClientGameCore();
    entities = new ClientGameEntities();
    media = new ClientGameMedia();
    movement = new ClientGameMovement();
    prediction = new ClientGamePrediction();
    screen = new ClientGameScreen();
    serverMessage = new ClientGameServerMessage();
    view = new ClientGameView();
}

ClientGameExports::~ClientGameExports()  {
    delete core;
    core = nullptr;
    delete entities;
    entities = nullptr;
    delete media;
    media = nullptr;
    delete prediction;
    prediction = nullptr;
    delete screen;
    screen = nullptr;
    delete serverMessage;
    serverMessage = nullptr;
    delete view;
    view = nullptr;
}

/**
*   @brief  Calculates the FOV the client is running. (Important to have in order.)
**/
float ClientGameExports::ClientCalculateFieldOfView(float fieldOfViewX, float width, float height) {
    // Ensure field of view is within valid ranges.
    if (fieldOfViewX <= 0 || fieldOfViewX > 179)
        Com_Error(ERR_DROP, "%s: bad fov: %f", __func__, fieldOfViewX);

    // Calculate proper fov value.
    float x = width / tan(fieldOfViewX / 360.f * M_PI);
    float a = atan(height / x);
    a = a * 360.f / M_PI;

    // Return fov value.
    return a;
}

/**
*   @brief  Called when a demo is being seeked through.
**/
void ClientGameExports::DemoSeek() {
    // Clear Effects.
    CLG_ClearEffects();
    // Clear Temp Entities.
    CLG_ClearTempEntities();
}

/**
*   @brief  Called after all downloads are done. (Aka, a map has started.)
*           Not used for demos.
**/
void ClientGameExports::ClientBegin() {

}

/**
*   @brief  Called upon whenever a client disconnects, for whichever reason.
*           Could be him quiting, or pinging out etc.
**/
void ClientGameExports::ClientClearState() {
    // Clear Effects.
    CLG_ClearEffects();

    // WID: TODO: I think this #ifdef can go lol.
#if USE_LIGHTSTYLES
    CLG_ClearLightStyles();
#endif
    CLG_ClearTempEntities();
}

/**
*   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
**/
void ClientGameExports::ClientDeltaFrame() {
    // SCR_SetCrosshairColor used to be here. It's a shitty name for a function
    // that sets its color based on your health.
    //
    // Either way, this function can be used for such things. 
}

/**
*   @brief  Called each client frame. Handle per frame basis things here.
**/
void ClientGameExports::ClientFrame() {
    // Advance local effects.
    CLG_RunDLights();
#if USE_LIGHTSTYLES
    CLG_RunLightStyles();
#endif
}

/**
*   @brief  Called when a disconnect even occures. Including those for Com_Error
**/
void ClientGameExports::ClientDisconnect() {
    // Clear the chat hud.
    ClientGameScreen::Cmd_ClearChatHUD_f();
}

/**
*   @brief  Updates the origin. (Used by the engine for determining current audio position too.)
**/
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

    // Lerp between previous and current frame delta angles.
    cl->deltaAngles[0] = LerpAngle(previousPlayerState->pmove.deltaAngles[0], currentPlayerState->pmove.deltaAngles[0], lerpFraction);
    cl->deltaAngles[1] = LerpAngle(previousPlayerState->pmove.deltaAngles[1], currentPlayerState->pmove.deltaAngles[1], lerpFraction);
    cl->deltaAngles[2] = LerpAngle(previousPlayerState->pmove.deltaAngles[2], currentPlayerState->pmove.deltaAngles[2], lerpFraction);

    // don't interpolate blend color
    cl->refdef.blend = currentPlayerState->blend;

    // Interpolate field of view
    cl->fov_x = LerpFieldOfView(previousPlayerState->fov, currentPlayerState->fov, lerpFraction);
    cl->fov_y = ClientCalculateFieldOfView(cl->fov_x, 4, 3);

    // Calculate new client forward, right, and up vectors.
    vec3_vectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    // Setup player entity origin and angles accordingly to update the client's listener origins with.
    cl->playerEntityOrigin = cl->refdef.vieworg;
    cl->playerEntityAngles = cl->refdef.viewAngles;

    // Keep it properly within range.
    if (cl->playerEntityAngles[vec3_t::Pitch] > 180) {
        cl->playerEntityAngles[vec3_t::Pitch] -= 360;
    }
    cl->playerEntityAngles[vec3_t::Pitch] = cl->playerEntityAngles[vec3_t::Pitch] / 3;

    // Update the client's listener origin values. This is a nescessity for the game in order
    // to properly play sound effects.
    clgi.UpdateListenerOrigin();
}

/**
*   @brief  Called when there is a needed retransmit of user info variables.
**/
void ClientGameExports::ClientUpdateUserinfo(cvar_t* var, from_t from) {
    // If there is a skin change, let's go for it.
    if (var == info_skin && from > FROM_CONSOLE) {
        char sk[MAX_QPATH];
        Q_strlcpy(sk, info_skin->string, sizeof(sk));
    }
}


/****
* 
*   Interface Accessors.
* 
****/
/**
*   @return A pointer to the client game's core interface.
**/
IClientGameExportCore* ClientGameExports::GetCoreInterface() {
    return core;
}

/**
*   @return A pointer to the client game module's entities interface.
**/
IClientGameExportEntities* ClientGameExports::GetEntityInterface() {
    return entities;
}

/**
*   @return A pointer to the client game module's media interface.
**/
IClientGameExportMedia* ClientGameExports::GetMediaInterface() {
    return media;
}

/**
*   @return A pointer to the client game module's movement interface.
**/
IClientGameExportMovement* ClientGameExports::GetMovementInterface() {
    return movement;
}

/**
*   @return A pointer to the client game module's prediction interface.
**/
IClientGameExportPrediction* ClientGameExports::GetPredictionInterface() {
    return prediction;
}

/**
*   @return A pointer to the client game module's screen interface.
**/
IClientGameExportScreen* ClientGameExports::GetScreenInterface() {
    return screen;
}

/**
*   @return A pointer to the client game module's servermessage interface.
**/
IClientGameExportServerMessage* ClientGameExports::GetServerMessageInterface() {
    return serverMessage;
}

/**
*   @return A pointer to the client game module's view interface.
**/
IClientGameExportView* ClientGameExports::GetViewInterface() {
    return view;
}

/**
*   @brief  Utility function for ClientUpdateOrigin
**/
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