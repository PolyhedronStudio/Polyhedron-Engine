/***
*
*	License here.
*
*	@file
*
*	ServerGame Exports Interface.
* 
***/
#pragma once

struct PlayerMove;

/**
*
*   Core ServerGame Exports Interface.
* 
**/
class IServerGameExportCore {
public:
    virtual ~IServerGameExportCore() = default;
    
    /**
	*   API Version.
	*   
	*   The version numbers will always be equal to those that were set in 
	*   CMake at the time of building the engine/game(dll/so) binaries.
	*   
	*   In an ideal world, we comply to proper version releasing rules.
	*   For Polyhedron FPS, the general following rules apply:
	*   --------------------------------------------------------------------
	*   MAJOR: Ground breaking new features, you can expect anything to be 
	*   incompatible at that.
	*   
	*   MINOR : Everytime we have added a new feature, or if the API between
	*   the Client / Server and belonging game counter-parts has actually 
	*   changed.
	*   
	*   POINT : Whenever changes have been made, and the above condition 
	*   is not met.
	**/
	struct APIVersion {
		int32_t major{ VERSION_MAJOR };
		int32_t minor{ VERSION_MINOR };
		int32_t point{ VERSION_POINT };
	} version;

	/**
	*   @brief  Initializes the ServerGame module.
	*/
	virtual void Initialize() = 0;

	/**
	*   @brief  Shuts down the ServerGame module.
	*/
	virtual void Shutdown() = 0;
};


/**
*
*   Client ServerGame Exports Interface.
* 
**/
class IServerGameExportClient {
public:
    //! Destructor.
    virtual ~IServerGameExportClient() = default;

    /**
    *   @brief  
    **/
    virtual qboolean Connect(Entity *ent, char *userinfo) = 0;
    /**
    *   @brief  
    **/
    virtual void Begin(Entity *ent) = 0;
    /**
    *   @brief  
    **/
    virtual void UserinfoChanged(Entity *ent, char *userinfo) = 0;
    /**
    *   @brief  
    **/
    virtual void Disconnect(Entity *ent) = 0;
    /**
    *   @brief  
    **/
    virtual void Command(Entity *ent) = 0;
    /**
    *   @brief  
    **/
    virtual void Think(Entity *ent, ClientMoveCommand *cmd) = 0;
};


/**
*
*   Entities ServerGame Exports Interface.
* 
**/
class IServerGameExportEntities {
public:
    virtual ~IServerGameExportEntities()  = default;
     
    /**
    *   @brief  Parses and spawns matching servergame entity objects for each
    *           classname found in the BSP Entity String.
    **/
    virtual void SpawnEntitiesFromBSPString(const char *mapName, const char *bspString, const char *spawnPoint) = 0;
};
7

/**
*
*   Save/Load ServerGame Exports Interface.
* 
**/
class IServerGameExportSaveLoad {
public:
    //! Destructor.
    virtual ~IServerGameExportSaveLoad() = default;

    /**
    *   @brief  Called every time a level is exited. Stores the persistent cross level 
    *           information about the world state and all clients.
    **/
    virtual void WriteGameState(const char *filename, qboolean autoSave) = 0;

    /**
    *   @brief  Called every loadgame. Reads the previously stored persistent cross level
    *           information of the world and its clients at that time.
    **/
    virtual void ReadGameState(const char *filename) = 0;

    /**
    *   @brief  Called every save game. Stores all entity level state related information.
    **/
    virtual void WriteLevelState(const char *filename) = 0;

    /**
    *   @brief  Called each time after the default map information has been loaded by
    *           SpawnEntitiesFromBSPSTring.
    **/
    virtual void ReadLevelState(const char *filename) = 0;
};


/**
*
*   Main ServerGame Exports Interface.
* 
**/
class IClientGameExports {
public:
    //! Default destructor.
    virtual ~IClientGameExports() = default;


    /***
    *
    *   Interface Accessors.
    *
    ***/
    /**
    *   @return A pointer to the client game's core interface.
    **/
    virtual IClientGameExportCore *GetCoreInterface() = 0;

    /**
    *   @return A pointer to the client game module's entities interface.
    **/
    virtual IClientGameExportEntities *GetEntityInterface() = 0;

    /**
    *   @return A pointer to the ServerGame module's save/load interface.
    **/
    virtual IServerGameExportSaveLoad *GetSaveLoadInterface() = 0;

    /**
    *   @return A pointer to the ServerGame module's client interface.
    **/
    virtual IServerGameExportClient *GetClientInterface() = 0;
    


    /***
    *
    *   General.
    *
    ***/
    /**
    *   @brief  Processes the ServerGame logic for a single frame.
    **/
    virtual void RunFrame() = 0;

    /**
    *   @brief  Called when an "sv <command>" command is issued throughout the server console.
    *           argc and argv can be used to get the rest of the parameters.
    **/
    virtual void ServerCommand() = 0;
};

