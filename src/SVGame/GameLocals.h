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

//
// TODO:    Inherit GameLocals from IGameExports. (IGameExports has yet to be created and serves as the exports class to a server.)
//
//          Add a SetMaxEntities, SetMaxClients, and Allocate functions that are friendly to several other objects.

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
    Gameworld* GetGameworld();

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    IGamemode* GetCurrentGamemode();

    /**
    *   @brief  Code shortcut for accessing gameworld's client array.
    * 
    *   @return A pointer to the gameworld's clients array.
    **/
    ServerClient* GetClients();
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxClients.
    * 
    *   @return The maximum allowed clients in this game.
    **/
    int32_t GetMaxClients();
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxEntities.
    * 
    *   @return The maximum allowed entities in this game.
    **/
    int32_t GetMaxEntities();



    /**
    *   
    **/
    
    /**
    *   
    **/
private:
    /**
    *   @brief Create the world member object and initialize it.
    **/
    void CreateWorld();
    /**
    *   @brief De-initialize the world and destroy it.
    **/
    void DestroyWorld();


    // TODO: Add Get methods and privatize the members below.
public:
    //! Gameworld.
    Gameworld* world = nullptr;

    //! needed for coop respawns
    //! Can't store spawnpoint32_t in level, because
    //! it would get overwritten by the savegame restore
    char spawnpoint[512];

    //! Will be set to latched cvar equivelants due to having to access them a lot.
    //int32_t maxClients = 0;
    //int32_t maxEntities = 0;

    //! Used to store Cross level triggers.
    int32_t serverflags = 0;

    //! Number of total items that exist in this game.
    int32_t numberOfItems = 0;

    //! Did we autosave?
    qboolean autoSaved = false;
};