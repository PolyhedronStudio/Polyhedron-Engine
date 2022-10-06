/*
// LICENSE HERE.

//
// DefaultGameMode.cpp
//
//
*/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Server Game Local headers.
#include "Game/Client/ClientGameLocals.h"

// Server Game Base Entity.
//#include "../Entities/Base/BodyCorpse.h"
#include "Game/Client/Entities/Base/CLGBasePlayer.h"

// ClientGameExports implementations.
#include "Game/Client/Exports/Core.h"
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/Media.h"
#include "Game/Client/Exports/Movement.h"
#include "Game/Client/Exports/Prediction.h"
#include "Game/Client/Exports/Screen.h"
#include "Game/Client/Exports/ServerMessage.h"
#include "Game/Client/Exports/View.h"

// Game Mode.
#include "Game/Client/Gamemodes/DefaultGamemode.h"

// World.
#include "Game/Client/World/ClientGameWorld.h"

// Shared Game API
#include "Game/Shared/GameBindings/ClientBinding.h"

//
// Constructor/Deconstructor.
//
DefaultGameMode::DefaultGameMode() : IGamemode() {
	CLG_Print(PrintType::DeveloperWarning, "ClientGame DefaultGameMode Constructed\n");
}
DefaultGameMode::~DefaultGameMode() {
	CLG_Print(PrintType::DeveloperWarning, "ClientGame DefaultGameMode Destructed\n");
}

/**
*   @brief  
**/
void DefaultGameMode::OnLevelExit() {
	CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::OnLevelExit\n" );
} 

/**
*   @brief  
**/
void DefaultGameMode::ClientBeginLocalFrame( CLGBasePlayer* player, ServerClient *client ) {
	ViewCamera *viewCamera = clge->view->GetViewCamera();

	if (player) {
		// Adjust the actual origin and velocity to that which we are predicting.	
		//player->GetClient()->playerState.pmove.origin = cl->predictedState.viewOrigin;
		//player->GetClient()->playerState.pmove.velocity = cl->predictedState.velocity;
		//player->SetOrigin( cl->predictedState.viewOrigin );
		//player->SetVelocity( cl->predictedState.velocity );
	//	player->LinkEntity();
	}
}


/**
*   @brief  
**/
void DefaultGameMode::ClientEndLocalFrame( CLGBasePlayer* player, ServerClient* client ) {
	ViewCamera *viewCamera = clge->view->GetViewCamera();

	viewCamera->CalculateBobMoveCycle( &cl->oldframe.playerState, &cl->frame.playerState );
}

/**
*   @brief  
**/
void DefaultGameMode::ClientThink( CLGBasePlayer* player, ServerClient* client, ClientMoveCommand* moveCommand ) {
	CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::ClientThink\n" );
}

/**
*   @brief  
**/
qboolean DefaultGameMode::ClientConnect( PODEntity *svEntity, char *userinfo ) {
	CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::ClientConnect\n" );

    return true;
}

/**
*   @brief  
**/
void DefaultGameMode::ClientBegin( PODEntity *svEntity ) {
    // Call ClientEndServerFrame to update him through the beginning frame.
    //ClientEndLocalFrame(player, svEntity->client);
	CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::ClientBegin\n" );
}

/**
*   @brief  
**/
void DefaultGameMode::ClientDisconnect( CLGBasePlayer* player, ServerClient *client ) {
	CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::ClientDisconnect\n" );
}

/**
*   @brief  
**/
void DefaultGameMode::InitializePlayerPersistentData( ServerClient* client ) {
    CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::InitializePlayerPersistentData\n" );
}

/**
*   @brief  
**/
void DefaultGameMode::InitializePlayerRespawnData( ServerClient* client ) {
    CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::InitializePlayerRespawnData\n" );   
}

/**
*   @brief  
**/
void DefaultGameMode::ClientDeath( CLGBasePlayer *player ) {
    CLG_Print( PrintType::DeveloperWarning, "DefaultGameMode::ClientDeath\n" );
}
