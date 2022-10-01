//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! ClientGame Local headers.
#include "Game/Client/ClientGameLocals.h"

// Temporary Entities.
#include "Game/Client/TemporaryEntities.h"

// Export Implementations.
#include "Game/Client/Exports/Core.h"
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/Media.h"
#include "Game/Client/Exports/Movement.h"
#include "Game/Client/Exports/Prediction.h"
#include "Game/Client/Exports/Screen.h"
#include "Game/Client/Exports/ServerMessage.h"
#include "Game/Client/Exports/View.h"

// Effects.
#include "Game/Client/Effects/DynamicLights.h"
#include "Game/Client/Effects/LightStyles.h"
#include "Game/Client/Effects/Particles.h"

// Core entities (Need access to some for view attributes.)
#include "Game/Client/Entities/Base/CLGBaseMover.h"
#include "Game/Client/Entities/Func/FuncPlat.h"

// Gamemodes.
#include "Game/Client/Gamemodes/IGamemode.h"

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
        CLG_Error( ErrorType::Drop, fmt::format( "{}: bad fov: {}", __func__, fieldOfViewX ) );

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
    // Clear Particle.
    Particles::Clear();
    // Clear Dynamic Light Effects.
    DynamicLights::Clear();

    // Clear Temp Entities.
    CLG_ClearTempEntities();
}

#ifdef _DEBUG
/**
*   @brief  For debugging problems when out-of-date entity origin is referenced.
**/
void ClientGameExports::CheckEntityPresent(int32_t entityNumber, const std::string &what) {
    // We're good if the player entity == current.
    if (entityNumber == cl->frame.clientNumber + 1) {
        return; // Player entity = current.
    }

    // We're good if the entity serverFrame == current.
    PODEntity *clEntity = &cs->entities[entityNumber];
    if (clEntity->serverFrame == cl->frame.number) {
        return; // current
    }

    // If we got to this point, something is fishy.
    if (clEntity->serverFrame) {
        CLG_Print( PrintType::DeveloperWarning, fmt::format( "Malicious Server Entity: %s on entity %d last seen %d frames ago\n",
                    what.c_str(), entityNumber, cl->frame.number - clEntity->serverFrame ) );
    } else {
        CLG_Print( PrintType::DeveloperWarning, fmt::format( "Malicious Server Entity:: %s on entity %d never seen before\n",
                    what.c_str(), entityNumber ) );
    }
}
#endif

/**
*   @brief  Called right after connecting to a (loopback-)server and succesfully 
*			loaded up the BSP map data. This gives it a chance to initialize game objects.
**/
void ClientGameExports::ClientConnect() {
	// First fetch ourselves the sv_ cvars we need.
	sv_gravity = clgi.Cvar_Get("sv_gravity", NULL, 0);
	sv_maxvelocity = clgi.Cvar_Get("sv_maxvelocity", NULL, 0);

	// Setup a fresh game locals object.
	game = ClientGameLocals{};
	game.Initialize();

    // Reset level locals.
    level = LevelLocals{};

	// Notify Gamemode about ClientConnect:
	IGamemode *gameMode = GetGameMode();
	gameMode->ClientConnect( nullptr, nullptr );

}

/**
*   @brief  Called after all downloads are done. (Aka, a map has started.)
*           Not used for demos.
**/
void ClientGameExports::ClientBegin() {

	// Notify Gamemode about ClientBegin:
	IGamemode *gameMode = GetGameMode();
	gameMode->ClientBegin( nullptr );
}

/**
*   @brief  Called upon whenever a client changes maps or disconnects for whichever reason:
*           Could be him quiting, or pinging out etc. 
**/
void ClientGameExports::ClientClearState() {
	// Notify Gamemode about ClientDisconnect:
	IGamemode *gameMode = GetGameMode();
	// There is a chance there is no game mode, however, clearstate is called, so make sure we got a gamemode.
	if (gameMode) {
		gameMode->ClientDisconnect( nullptr, nullptr );
	}

    // Clear Particle.
    Particles::Clear();
    // Clear Dynamic Light Effects.
    DynamicLights::Clear();

    // WID: TODO: I think this #ifdef can go lol.
#if USE_LIGHTSTYLES
    LightStyles::Clear();
#endif
    CLG_ClearTempEntities();

	// Notify our game locals about shutting down.
	game.Shutdown();
}


/**
*   @brief  Called each client frame. Handle per frame basis things here.
**/
void ClientGameExports::ClientFrame() {


    // Advance local effects.
    DynamicLights::RunFrame();
#if USE_LIGHTSTYLES
    LightStyles::RunFrame();
#endif
}

/**
*	@brief	Called each VALID client frame. Sets the proper level.time for the current received delta frame.
**/
void ClientGameExports::SetDeltaFrameLevelTime() {
	entities->SetDeltaFrameLevelTime();
};

/**
*   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
**/
void ClientGameExports::ClientPacketEntityDeltaFrame() {
	entities->RunPacketEntitiesDeltaFrame();
}

/**
*   @brief  Gives Local Entities a chance to think. Called synchroniously to the server frames.
**/
void ClientGameExports::ClientLocalEntitiesFrame() {
	entities->RunLocalEntitiesFrame();
}

/**
*   @brief  Gives local entities a chance to think. These are called "synchroniously" to the server frames.
*	@return	The GameEntity's hashed classname value, 0 if it has no GameEntity.
**/
uint32_t ClientGameExports::GetHashedGameEntityClassname(PODEntity *podEntity) {
	return entities->GetHashedGameEntityClassname(podEntity);
}

/**
*   @brief  Called for each prediction frame, so all entities can try and predict like the player does.
**/
void ClientGameExports::ClientPredictEntitiesFrame(int64_t msec) {
    // Low and behold, time to run the ClientGame Entity logic for another single frame.
    entities->RunPackEntitiesPredictionFrame(msec);
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
    // Only do this if we had a valid frame.
    if (!cl->frame.valid) {
        return;
    }



    // Find states to interpolate between
    const PlayerState* currentPlayerState  = &cl->frame.playerState;
    const PlayerState* previousPlayerState = &cl->oldframe.playerState;

    // These are applied to the view camera at the very end.
    vec3_t newViewOrigin = vec3_zero();
    vec3_t oldViewOrigin = vec3_zero();

    vec3_t newViewAngles = vec3_zero();
    vec3_t oldViewAngles = vec3_zero();

    // Get Lerp Fraction.
    const double lerpFraction = cl->lerpFraction;


    /*
    *	View Origin.
    **/
    if ( !clgi.IsDemoPlayback() && cl_predict->integer && !( currentPlayerState->pmove.flags & PMF_NO_PREDICTION ) ) {
        // Set the view camera's origin to that of the predicted state's view origin + view offset.
        ClientPredictedState* predictedState = &cl->predictedState;
        newViewOrigin = predictedState->viewOrigin + predictedState->viewOffset;

        // Scale prediction error to frame lerp fraction and add it to the camera view origin.
		ClientGameWorld *gameWorld = GetGameWorld();
		GameEntity *geGround = gameWorld->GetGameEntityByIndex( cl->predictedState.groundEntityNumber );

		// Is the ground a valid pointer?
		if ( geGround && geGround->GetPODEntity()->linearMovement ) {//geGround->IsExtrapolating() ) {
			//if ( cl->xerpFraction ) {
			//	const vec3_t error = vec3_scale( predictedState->error, cl->xerpFraction );
			//	newViewOrigin += error;
			//}
	        oldViewOrigin = currentPlayerState->pmove.origin + currentPlayerState->pmove.viewOffset;
		    oldViewOrigin.z -= cl->predictedState.stepOffset;

	        //newViewOrigin = cl->predictedState.viewOrigin + cl->predictedState.viewOffset;
			GameEntity *gePlayer = gameWorld->GetClientGameEntity();
			PlayerState gePlayerState = gePlayer->GetClient()->playerState;
			newViewOrigin = cl->predictedState.viewOrigin + cl->predictedState.viewOffset; //gePlayerState.pmove.origin + gePlayerState.pmove.viewOffset;
		    newViewOrigin.z -= cl->predictedState.stepOffset;

			if ( cl->xerpFraction ) {
		        // Interpolate new view origin based on the frame's lerpfraction.
			    newViewOrigin = vec3_mix( oldViewOrigin, newViewOrigin, cl->xerpFraction );
				const vec3_t error = vec3_scale( predictedState->error, 1.f - lerpFraction );//->xerpFraction );
				newViewOrigin += error;
			} else {
				newViewOrigin = vec3_mix( oldViewOrigin, newViewOrigin, lerpFraction );
				const vec3_t error = vec3_scale( predictedState->error, 1.f - lerpFraction );
				newViewOrigin += error;
			}

		} else {
			const vec3_t error = vec3_scale( predictedState->error, 1.f - lerpFraction );
			newViewOrigin += error;
		}

        // Last but not least, subtract the stepOffset from the Z axis.
        newViewOrigin.z -= predictedState->stepOffset;
	} else {
        // Use the interpolated values, and substract stepOffset.
        oldViewOrigin = previousPlayerState->pmove.origin + previousPlayerState->pmove.viewOffset;
        oldViewOrigin.z -= cl->predictedState.stepOffset;
        newViewOrigin = currentPlayerState->pmove.origin + currentPlayerState->pmove.viewOffset;
        newViewOrigin.z -= cl->predictedState.stepOffset;

        // Interpolate new view origin based on the frame's lerpfraction.
        newViewOrigin = vec3_mix( oldViewOrigin, newViewOrigin, lerpFraction );
    }

    /*
    *	View Angles.
    **/
    // Interpolate between previous and current player state.
    if ( clgi.IsDemoPlayback() ) {
        // Interpolate view angles.
        newViewAngles = vec3_mix_euler( previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction );
	// Use predicted state view angles.
	} else if ( currentPlayerState->pmove.type < EnginePlayerMoveType::Dead ) {
        newViewAngles = cl->predictedState.viewAngles;
    // Interpolate from our predicted angles.
	} else if ( previousPlayerState->pmove.type < EnginePlayerMoveType::Dead ) {
		newViewAngles = vec3_mix_euler( cl->predictedState.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction );
	// Interpolate between previous and current player state.
	} else {
		newViewAngles = vec3_mix_euler( previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction );
	}
    
    //// if not running a demo or on a locked frame, add the local angle movement
    //if (cls.demo.playback) {
    //    LerpAngles(ops->viewangles, ps->viewangles, lerp, cl.refdef.viewangles);
    //} else if (ps->pmove.pm_type < PM_DEAD) {
    //    // use predicted values
    //    VectorCopy(cl.predicted_angles, cl.refdef.viewangles);
    //} else if (ops->pmove.pm_type < PM_DEAD && cls.serverProtocol > PROTOCOL_VERSION_DEFAULT) {
    //    // lerp from predicted angles, since enhanced servers
    //    // do not send viewangles each frame
    //    LerpAngles(cl.predicted_angles, ps->viewangles, lerp, cl.refdef.viewangles);
    //} else {
    //    // just use interpolated values
    //    LerpAngles(ops->viewangles, ps->viewangles, lerp, cl.refdef.viewangles);
    //}

    /*
    *	Delta Angles, Blend, and FOV.
    **/
    // Lerp between previous and current frame delta angles.
    const vec3_t newDeltaAngles = vec3_mix_euler(previousPlayerState->pmove.deltaAngles, currentPlayerState->pmove.deltaAngles, lerpFraction);

    // Don't interpolate blend color
    cl->refdef.blend = currentPlayerState->blend;

    // Interpolate field of view
    cl->fov_x = LerpFieldOfView(previousPlayerState->fov, currentPlayerState->fov, lerpFraction);
    cl->fov_y = ClientCalculateFieldOfView(cl->fov_x, 4, 3);

    /*
    *	Properly update our view camera and final player entity origin and angles.
    **/
    // Acquire the view camera.
    ViewCamera *viewCamera = clge->view->GetViewCamera();

    // Set new view origin, angles and delta angles.
    viewCamera->SetViewOrigin(newViewOrigin);
    viewCamera->SetViewAngles(newViewAngles);
    viewCamera->SetViewDeltaAngles(newDeltaAngles);

    // Calculate new client forward, right, and up vectors.
    viewCamera->UpdateViewVectors();

    // Setup player entity origin and angles accordingly to update the client's listener origins with.
    cl->playerEntityOrigin = newViewOrigin;
    cl->playerEntityAngles = newViewAngles;

    // Keep it properly within range.
    if (cl->playerEntityAngles[vec3_t::Pitch] > 180) {
        cl->playerEntityAngles[vec3_t::Pitch] -= 360;
    }
    cl->playerEntityAngles[vec3_t::Pitch] = cl->playerEntityAngles[vec3_t::Pitch] / 3;


    // Update the client's 3D Sound Spatialization Origin values. This is a nescessity for the game 
    // in order to properly play sound effects.
    clgi.UpdateSoundSpatializationOrigin(cl->refdef.vieworg, viewCamera->GetForwardViewVector(), viewCamera->GetRightViewVector(), viewCamera->GetUpViewVector());
}

/**
*   @brief  Called when there is a needed retransmit of user info variables.
**/
void ClientGameExports::ClientUpdateUserinfo(cvar_t* var, int32_t from) {
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