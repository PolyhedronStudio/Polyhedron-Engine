/***
*
*	License here.
*
*	@file
* 
*   Client Game Main.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"

// ClientGame.
//#include "Game/Client/TemporaryEntities.h"
#include "Game/Client/World/ClientGameWorld.h"

// ClientGameExports implementations.
#include "Exports/Core.h"
#include "Exports/Entities.h"
#include "Exports/Media.h"
#include "Exports/Movement.h"
#include "Exports/Prediction.h"
#include "Exports/Screen.h"
#include "Exports/ServerMessage.h"
#include "Exports/View.h"



/**
*	Local ClientGame objects.
**/
ClientGameLocals game;
LevelLocals level;

/**
*   @return A pointer to the gameworld object. The big man in charge.
**/
ClientGameWorld *GetGameWorld() {
    return game.world;
}

/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
IGamemode *GetGameMode() {
    if (game.world) {
        return game.world->GetGameMode();
    } else {
        return nullptr;
    }
}

/**
*	@return	An std::vector containing the found boxed entities.Will not exceed listCount.
**/
GameEntityVector CLG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType) {
    // Boxed server entities set by gi.BoxEntities.
    Entity* boxedPODEntities[MAX_CLIENT_POD_ENTITIES];

    // Vector of the boxed game entities to return.
    GameEntityVector boxedGameEntities;

	// Acquire gameworld.
	ClientGameWorld *gameWorld = GetGameWorld();

    // Ensure the listCount can't exceed the max edicts.
    if (listCount > MAX_CLIENT_POD_ENTITIES) {
        listCount = MAX_CLIENT_POD_ENTITIES;
    }

    // Box the entities.
    int32_t numEntities = clgi.BoxEntities(mins, maxs, boxedPODEntities, listCount, areaType);

    // Go through the boxed entities list, and store there classEntities (SVGBaseEntity aka baseEntities).
    for (int32_t i = 0; i < numEntities; i++) {
		// Acquire entity number.
		const int32_t entityNumber = boxedPODEntities[i]->clientEntityNumber;
		
		// Get GameEntity.
		GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(entityNumber);

        if (gameEntity != nullptr) {
            boxedGameEntities.push_back(gameEntity);
        }
    }

    // Return our boxed base entities vector.
    return boxedGameEntities;
}

/**
*
*
*   Client Game Locals.
*
*
**/
/**
*   @return A pointer to the gameworld object. The big man in charge.
**/
ClientGameWorld *ClientGameLocals::GetGameWorld() {
    return world;
}


/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
IGamemode *ClientGameLocals::GetGameMode() {
    if (game.world) {
        return game.world->GetGameMode();
    } else {
        return nullptr;
    }
}

/**
*	@brief Initializes the gameworld and its member objects.
**/
void ClientGameLocals::Initialize() {
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
void ClientGameLocals::Shutdown() {
    // Uninitialize world and destroy its object.
    DestroyWorld();
}



/**
*   @brief Create the world member object and initialize it.
**/
void ClientGameLocals::CreateWorld() {
    // Create game world object.
    world = new ClientGameWorld();

    // Initialize it.
    world->Initialize();
}

/**
*   @brief De-initialize the world and destroy it.
**/
void ClientGameLocals::DestroyWorld() {
    // Give the gameworld a chance to finalize anything.
    if (world) { 
        world->Shutdown();

        // Delete game world from memory.
        delete world;
        world = nullptr;
    }
}


/**
*   @return A pointer to the gameworld its current gamemode object.
**/
//IGamemode* ClientGameLocals::GetGameMode() { 
//    return world->GetGameMode(); 
//}



/**
*   @brief  Code shortcut for accessing gameworld's client array.
* 
*   @return A pointer to the gameworld's clients array.
**/
ServerClient* ClientGameLocals::GetClients() { return nullptr; } // { return world->GetClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxClients.
* 
*   @return The maximum allowed clients in this game.
**/
int32_t ClientGameLocals::GetMaxClients() { return 4; /* ?? */ } // { return world->GetMaxClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxEntities.
* 
*   @return The maximum allowed entities in this game.
**/
int32_t ClientGameLocals::GetMaxEntities() { return world->GetMaxEntities(); }



