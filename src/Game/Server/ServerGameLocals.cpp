/***
*
*	License here.
*
*	@file
*
*	ServerGameLocals class contains all of the game. Its world, entities, clients, items, etc.
*   It stays persistently intact until the end of the game, when the dll is unloaded.
* 
*
***/
// Core.
#include "ServerGameLocals.h"	 // Include SVGame header.

// Entities.
#include "Entities.h"
//#include "Entities/Base/SVGEntityHandle.h"
#include "Entities/Base/SVGBasePlayer.h"

// GameModes.
#include "Gamemodes/IGamemode.h"
#include "Gamemodes/DefaultGamemode.h"
#include "Gamemodes/CoopGamemode.h"
#include "Gamemodes/DeathMatchGamemode.h"

// GameWorld.
#include "World/ServerGameWorld.h"

//-----------------
// Global Game Variables.
//
// These are used all throughout the code. To store game state related
// information, and callbacks to the engine server game API.
//-----------------
GameLocals game;
LevelLocals level;

// Extern cvars.
extern cvar_t* gamemode;

/**
*   @return A pointer to the gameworld object. The big man in charge.
**/
ServerGameWorld *GetGameWorld() {
    return game.world;
}


/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
IGameMode *GetGameMode() {
    if (game.world) {
        return game.world->GetGameMode();
    } else {
        return nullptr;
    }
}

/**
*	@brief Initializes the gameworld and its member objects.
**/
void GameLocals::Initialize() {
    // Create and initialize the world object.
    // 
    // Since it manages entities and clients it does the following things:
    // Allocate and reserve the clients array based on maxclients cvar.
    // Parse the BSP entity string to create, precache and spawn each game entity instances.
    CreateWorld();
}

/**
*	@brief Shutsdown the gamelocal.
**/
void GameLocals::Shutdown() {
    // Uninitialize world and destroy its object.
    DestroyWorld();
}



/**
*   @brief Create the world member object and initialize it.
**/
void GameLocals::CreateWorld() {
    // Create game world object.
    world = new ServerGameWorld();

    // Initialize it.
    world->Initialize();
}

/**
*   @brief De-initialize the world and destroy it.
**/
void GameLocals::DestroyWorld() {
    // Give the gameworld a chance to finalize anything.
    if (world) { 
        world->Shutdown();

        // Delete game world from memory.
        delete world;
        world = nullptr;
    }
}



/**
*   @return A pointer to the gameworld object.
**/
ServerGameWorld* GameLocals::GetGameWorld() { 
    return world; 
}

/**
*   @return A pointer to the gameworld its current gamemode object.
**/
IGameMode* GameLocals::GetGameMode() { 
    return world->GetGameMode(); 
}



/**
*   @brief  Code shortcut for accessing gameworld's client array.
* 
*   @return A pointer to the gameworld's clients array.
**/
ServerClient* GameLocals::GetClients() { return world->GetClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxClients.
* 
*   @return The maximum allowed clients in this game.
**/
int32_t GameLocals::GetMaxClients() { return world->GetMaxClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxEntities.
* 
*   @return The maximum allowed entities in this game.
**/
int32_t GameLocals::GetMaxEntities() { return world->GetMaxEntities(); }