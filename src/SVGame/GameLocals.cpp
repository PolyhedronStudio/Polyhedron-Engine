// Core.
#include "ServerGameLocal.h"	 // Include SVGame header.

// Entities.
#include "Entities.h"
//#include "Entities/Base/SVGEntityHandle.h"
#include "Entities/Base/PlayerClient.h"

// Gamemodes.
#include "Gamemodes/IGamemode.h"
#include "Gamemodes/DefaultGamemode.h"
#include "Gamemodes/CoopGamemode.h"
#include "Gamemodes/DeathmatchGamemode.h"

// Gameworld.
#include "World/GameWorld.h"

// GameLocals.
#include "GameLocals.h"

// Extern cvars.
extern cvar_t* gamemode;
extern gitem_s itemlist[];

/**
*	@brief Initializes the gameworld and its member objects.
**/
void GameLocals::Initialize() {
    // Prepare items.
    PrepareItems();

    // Prepare entities.
    PrepareEntities();

    // Prepare clients.
    PrepareClients();

    // Create the world.
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
*   @brief Sets up the pointers to entities, and ensures all entities are a nullptr at start.
**/
void GameLocals::PrepareEntities() {
    // Clamp it just in case.
    maxEntities = Clampi(MAX_EDICTS, (int)maximumclients->value + 1, MAX_EDICTS);

    // Setup our game globals for entities.
    globals.entities = g_entities;
    globals.maxEntities = maxEntities;

    // Ensure, all base entities are nullptrs. Just to be save.
    for (int32_t i = 0; i < MAX_EDICTS; i++) {
    	g_baseEntities[i] = nullptr;
    }
}

/**
*   @brief Prepares the game clients array for use.
**/
void GameLocals::PrepareClients() {
    // Allocate our clients array.
    maxClients = maximumclients->value;
    clients = (ServerClient*)gi.TagMalloc(maxClients * sizeof(clients[0]), TAG_GAME);  // CPP: Cast

    // Current total number of entities in our game = world + maximum clients.
    globals.numberOfEntities = maxClients + 1;
}


/**
*   @brief Create the world member object and initialize it.
**/
void GameLocals::CreateWorld() {
    // Create game world object.
    world = new Gameworld();

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