/***
*
*	License here.
*
*	@file
*
*	GameLocal class contains all of the game. Its world, entities, clients, items, etc.
*   It stays persistently intact until the end of the game, when the dll is unloaded.
* 
*   Its current state at time of load/save is also read/written to the server.ssv file 
*   for savegames
*
***/
#pragma once

// Pre-define.
class Gameworld;
class IGamemode;
#include "World/GameWorld.h"
//
// TODO: Inherit GameLocals from IGameExports. (IGameExports has yet to be created and serves as the exports class to a server.)
//

/**
*	@brief GameLocal is the main server game class.
* 
*	@details 
**/
class GameLocals {
public:
    /**
	*	@brief Default constructor.
	**/
    GameLocals() = default;

    /**
	*	@brief Default destructor
	**/
    ~GameLocals() = default;

public:
    /**
	*	@brief Initializes the gameworld and its member objects.
	**/
    void Initialize();
    
    /**
	*	@brief Shutsdown the gamelocal.
	**/
    void Shutdown();

    /**
    *   @return A pointer to the gameworld object.
    **/
    inline Gameworld* GetGameworld() { return world; }

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    inline IGamemode* GetCurrentGamemode() { return world->GetCurrentGamemode(); }

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    inline int32_t GetMaxClients() { return maxClients; }

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    inline int32_t GetMaxEntities() { return maxEntities; }

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    
    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
private:
    /**
    *   @brief Counts the length of our items array so the game is aware of the total of items.
    **/
    void PrepareItems();
    /**
    *   @brief Sets up the pointers to entities, and ensures all entities are a nullptr at start.
    **/
    void PrepareEntities();
    /**
    *   @brief Prepares the game's clients array for use.
    **/
    void PrepareClients();

    /**
    *   @brief Create the world member object and initialize it.
    **/
    void CreateWorld();


    // TODO: Add Get methods and privatize the members below.
public:
    //! Gameworld.
    Gameworld* world = nullptr;

    //! Clients pointer array.
    ServerClient* clients = nullptr;

    //! needed for coop respawns
    //! Can't store spawnpoint32_t in level, because
    //! it would get overwritten by the savegame restore
    char spawnpoint[512];

    //! Will be set to latched cvar equivelants due to having to access them a lot.
    int32_t maxClients = 0;
    int32_t maxEntities = 0;

    //! Used to store Cross level triggers.
    int32_t serverflags = 0;

    //! Number of total items that exist in this game.
    int32_t numberOfItems = 0;

    //! Did we autosave?
    qboolean autoSaved = false;
};