/***
*
*	License here.
*
*	@file
* 
*   Client Game Locals.
*
***/
#pragma once

/**
*	Include ClientGame Main header for all Shared/Common functionalities.
**/
/**
*	Include Shared codebase with CGAME_INCLUDE defined.
**/
#define CGAME_INCLUDE 1

// Shared.
#include "Shared/Shared.h"
//#include "Game/Client/ClientGameMain.h"


/**
*	ConstExpr:
**/
//! Maximum amount of POD Entities.
static constexpr int32_t MAX_POD_ENTITIES = MAX_CLIENT_POD_ENTITIES;


/**
*	Interface/Base Entity Forward Declarations: For SharedGame inclusion.
**/
//! SharedGame Entity handles, read its header for its description.
class SGEntityHandle;
//! Interface containing the shared game entity functionalities that are shared between game modules.
class ISharedGameEntity;
//! Actual game entity type for the ClientGame module.
class IClientGameEntity;
//! Base entities.
class CLGBasePacketEntity;
class CLGBaseLocalEntity;
class CLGBaseMover;
class CLGBasePlayer;
class CLGBaseTrigger;
//! Worldspawn.
class WorldSpawn;

//! GameWorld.
class ClientGameWorld;
//! ClientGame TraceResult.
struct CLGTraceResult;
//! SharedGame TraceResult.
struct SGTRaceResult;


/**
*	Define the 'using' SharedGame types for ClientGame usage.
**/
//! Set GameEntity to IClientGameEntity;
using GameEntity	= IClientGameEntity;
//! Set SGGameEntity to ClientGameWOrld;
using SGGameWorld	= ClientGameWorld;
//! Commented out: Since the ServerGame has no awareness over local client entities
//using SGBaseLocalEntity = CLGBaseLocalEntity;
//! Set SGBaseEntity to CLGBasePacketEntity.
using SGBaseEntity = CLGBasePacketEntity;
//! Set SGBasePlayer to CLGBaseMover.
using SGBaseMover = CLGBaseMover;
//! Set SGBasePlayer to CLGBaseTrigger.
using SGBaseTrigger = CLGBaseTrigger;
//! Set SGBasePlayer to CLGBasePlayer.
using SGBasePlayer = CLGBasePlayer;

//! Set the parent class for the SharedGame base item behaviors class.
//! It is a local entity in the client game case as to perform possible
//! client side logic.
//! For the server side it is: using SGBaseItemParentClass = SVGBaseTrigger;
using SGBaseItemParentClass = CLGBaseLocalEntity;


/**
*	Include the SharedGame codebase.
**/
//! TypeInfo system.
#include "Game/Shared/Entities/TypeInfo.h"

//! ISharedGameEntity Interface.
#include "Game/Shared/Entities/ISharedGameEntity.h"
//! IClientGameEntity Interface.
#include "Game/Client/Entities/IClientGameEntity.h"

/**
*	Define our (Shared/Client)-Game container types using the SharedGame 'using' types.
**/
//! This is the actual GameWorld POD array with a size based on which GameModule we are building for.
using PODGameWorldArray = PODEntity[MAX_POD_ENTITIES];
//! std::span for PODEntity* objects.
using PODEntitySpan = std::span<PODEntity>;
//! std::vector for PODEntity* objects.
using PODEntityVector = std::vector<PODEntity*>;
//! std::span for GameEntity* derived objects.
using GameEntitySpan = std::span<GameEntity*>;
//! std::vector for GameEntity* derived objects.
using GameEntityVector = std::vector<GameEntity*>;


//! SharedGame: EntityHandle
#include "Game/Shared/Entities/SGEntityHandle.h"
//! SharedGame: Entity Filters.
#include "Game/Shared/Entities/EntityFilters.h"
//! ClientGame: Needed for SharedGame BaseItem.
#include "Game/Client/Entities/Base/CLGBaseLocalEntity.h"
//! Tracing.
#include "Game/Shared/Tracing.h"
//! Skeletal Animation.
#include "Game/Shared/SkeletalAnimation.h"
//! Player Move.
#include "Game/Shared/PlayerMove.h"

//! SharedGame: BaseItem Entity.
#include "Game/Shared/Entities/SGBaseItem.h"



/**
*
*
*   Client Game structures and definitions.
*
*
**/
/**
*	ClientGame Trace Results.
**/
#include "Utilities/CLGTraceResult.h"



/**
*	@brief GameLocal is the main server game class.
* 
*	@details 
**/
class ClientGameLocals {
public:
    /**
	*	@brief Default constructor.
	**/
    ClientGameLocals() = default;

    /**
	*	@brief Default destructor
	**/
    ~ClientGameLocals() = default;

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
    ClientGameWorld* GetGameWorld();

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    //IGameMode* GetGameMode();

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
    //! GameWorld.
    ClientGameWorld* world = nullptr;

    //! needed for coop respawns
    //! Can't store spawnpoint32_t in level, because
    //! it would get overwritten by the savegame restore
	char spawnpoint[512] = {};

    //! Will be set to latched cvar equivelants due to having to access them a lot.
    //int32_t maxClients = 0;
    //int32_t maxEntities = 0;

    //! Used to store Cross level triggers.
    int32_t serverflags = 0;

    //! Did we autosave?
    qboolean autoSaved = false;
};


/**
*   @brief  This one is here temporarily, it's currently the way how things still operate in the ServerGame
*           module. Eventually we got to aim for a more streamlined design. So for now it resides here as an
*           ugly darn copy of..
**/
struct LevelLocals  {
	uint64_t frameNumber = 0;
	//! Current sum of total frame time taken.
    GameTime time = GameTime::zero();
	
	////! The current serverTime.
 //   GameTime curServerTime = GameTime::zero();
	////! The previous serverTime.
	//GameTime prevServerTime = GameTime::zero();

	//! The next serverTime.
	GameTime nextServerTime = GameTime::zero();
	//! The current serverTime.
    GameTime curServerTime = GameTime::zero();
	//! The previous serverTime.
	GameTime prevServerTime = GameTime::zero();

    //std::string levelName;  //! The descriptive name (Outer Base, etc)
    std::string mapName;    //! The server name (base1, etc)
    //char nextMap[MAX_QPATH];    //! Go here when fraglimit is hit

    // The current entity that is actively being ran from SVG_RunFrame.
    IClientGameEntity *currentEntity = nullptr;

    // Index for the que pile of dead bodies.
    int32_t bodyQue = 0;
};



/**
*	Game Externals.
**/
//! Global game object.
extern ClientGameLocals game;
//! Global level locals.
extern LevelLocals level;

/**
*   @return A pointer to the game's world object. The man that runs the show.
**/
ClientGameWorld* GetGameWorld();

/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
//IGameMode* GetGameMode();



/**
*
*
*   Client Game Specific Functions.
*
*
**/
/**
*	@brief	Wraps around Com_Error.
*	@param	errorType		One of the error types in ErrorType::
*	@param	errorMessage	A string, possibly formatted using fmt::format.
**/
void CLG_Error( int32_t errorType, const std::string &errorMessage );

/**
*	@brief	Wraps around Com_LPrintf
*	@param	printType		One of the print types in PrintType::
*	@param	printMessage	A string, possibly formatted using fmt::format.
**/
void CLG_Print( int32_t printType, const std::string &printMessage );

/**
*	@brief	... Sound.
**/
void CLG_Sound(GameEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset);

/**
*	@brief	Precaches the model and returns the model index qhandle_t.
**/
qhandle_t CLG_PrecacheModel(const std::string& filename);

/**
*	@brief	Precaches the image and returns the image index qhandle_t.
**/
qhandle_t CLG_PrecacheImage(const std::string& filename);

/**
*	@brief	Precaches the sound and returns the sound index qhandle_t.
**/
qhandle_t CLG_PrecacheSound(const std::string& filename);



