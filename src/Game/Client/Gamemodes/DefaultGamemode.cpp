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

// Game Mode.
#include "Game/Client/Gamemodes/DefaultGamemode.h"

// World.
#include "Game/Client/World/ClientGameWorld.h"

// Shared Game API
//#include "Game/Shared/GameBindings/ServerBinding.h"

//
// Constructor/Deconstructor.
//
DefaultGameMode::DefaultGameMode() : IGameMode() {
	CLG_Print(PrintType::DeveloperWarning, "ClientGame DefaultGameMode Constructed\n");
}
DefaultGameMode::~DefaultGameMode() {
	CLG_Print(PrintType::DeveloperWarning, "ClientGame DefaultGameMode Destructed\n");
}

/**
*   @brief  
**/
void DefaultGameMode::OnLevelExit() {

} 

/**
*   @brief  
**/
void DefaultGameMode::ClientBeginLocalFrame(CLGBasePlayer* player, ServerClient *client) {

}


/**
*   @brief  
**/
void DefaultGameMode::ClientEndLocalFrame(CLGBasePlayer* player, ServerClient* client) {

}

/**
*   @brief  
**/
void DefaultGameMode::ClientThink(CLGBasePlayer* player, ServerClient* client, ClientMoveCommand* moveCommand) {

}

/**
*   @brief  
**/
qboolean DefaultGameMode::ClientConnect(PODEntity *svEntity, char *userinfo) {
    return true;
}

/**
*   @brief  
**/
void DefaultGameMode::ClientBegin(PODEntity *svEntity) {
    // Call ClientEndServerFrame to update him through the beginning frame.
    //ClientEndLocalFrame(player, svEntity->client);
}

/**
*   @brief  
**/
void DefaultGameMode::ClientDisconnect(CLGBasePlayer* player, ServerClient *client) {
}

/**
*   @brief  
**/
void DefaultGameMode::InitializePlayerPersistentData(ServerClient* client) {
    
}

/**
*   @brief  
**/
void DefaultGameMode::InitializePlayerRespawnData(ServerClient* client) {
   
}

/**
*   @brief  
**/
void DefaultGameMode::ClientDeath(CLGBasePlayer *player) {

}
