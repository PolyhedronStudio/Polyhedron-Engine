/***
*
*	License here.
*
*	@file
*
*	Declarations for the Shared Game(SG) that wrap around the Server Game(SVG) 
*	functionalities required.
* 
***/
#pragma once

//! The name of the module that is used for differentiating what module
//! printed text.
static constexpr const char *sharedModuleName = "SharedGame(SVG)";



/**
*
*
*	Predeclare classes/structs, include required headers for game module, and set 'using=' types 
*	for building SharedGame code for the ServerGame module.
*
*
**/
/**
*	ConstExpr:
**/
//! Maximum amount of POD Entities.
static constexpr int32_t MAX_POD_ENTITIES = MAX_SERVER_POD_ENTITIES;

/**
*	ServerGame Predeclarations:
**/
//! Predeclare for ServerGameLocals include.
class SGEntityHandle;
//! Interface containing the shared game entity functionalities that are shared between game modules.
class ISharedGameEntity;
//! Actual game entity type for the ServerGame module.
class IServerGameEntity;
//! GameWorld.
class ServerGameWorld;
//! ServerGame TraceResult.
struct SVGTraceResult;
//! SharedGame TraceResult.
struct SGTRaceResult;


/**
*	Define the 'using' SharedGame types for ServerGame usage.
**/
//! Using: GameEntity.
using GameEntity = IServerGameEntity;
//! Using: GameWorld.
using SGGameWorld = ServerGameWorld;


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





/**
*	Includes:
**/
//! Type Info System.
#include "../Entities/TypeInfo.h"
//! Shared Entity Handle.
#include "../Entities/SGEntityHandle.h"
//! Shared Entity Interface.
#include "../Entities/ISharedGameEntity.h"
//! Server Game Entity Interface.
#include "../../Server/Entities/IServerGameEntity.h"
//! Shared World Interface.
#include "../World/IGameWorld.h"




/***
*
*
*	Error and Print Functions.
*
*
***/
/**
*	@brief	Wraps around Com_Error.
*	@param	errorType		One of the error types in ErrorType::
*	@param	errorMessage	A string, possibly formatted using fmt::format.
**/
void SG_Error( int32_t errorType, const std::string &errorMessage );

/**
*	@brief	Wraps around Com_LPrintf
*	@param	printType		One of the print types in PrintType::
*	@param	printMessage	A string, possibly formatted using fmt::format.
**/
void SG_Print( int32_t printType, const std::string &printMessage );



/***
*
*
*	Entity Functions.
*
*
***/
/**
*	@brief	An easy way to acquire the proper entity number from a POD Entity.
*	@return	-1 if the entity was a (nullptr).
**/
const int32_t SG_GetEntityNumber( const PODEntity *podEntity );
/**
*	@brief	An easy way to acquire the proper entity number from a Game Entity.
*	@return	On success: POD Entity Number. On failure: -1 if the Game Entity or its belonging POD Entity was a (nullptr).
**/
const int32_t SG_GetEntityNumber( GameEntity *gameEntity );