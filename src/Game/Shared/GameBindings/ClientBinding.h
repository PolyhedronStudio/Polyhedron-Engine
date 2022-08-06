/***
*
*	License here.
*
*	@file
*
*	Declarations for the Shared Game(SG) that wrap around the Client Game(CLG)
*	functionalities required.
* 
***/
#pragma once

//! The name of the module that is used for differentiating what module
//! printed text.
static constexpr const char *sharedModuleName = "SharedGame(CLG)";


/**
*
*
*	Predeclare classes/structs, include required headers for game module, and set 'using=' types 
*	for building SharedGame code for the ClientGame module.
*
*
**/
/**
*	ConstExpr:
**/
//! Maximum amount of POD Entities.
static constexpr int32_t MAX_POD_ENTITIES = MAX_CLIENT_POD_ENTITIES;


/**
*	Predeclarations:
**/
//! Predeclare for ClientGameLocals include.
class SGEntityHandle;
//! Interface containing the shared game entity functionalities that are shared between game modules.
class ISharedGameEntity;
//! Actual game entity type for the ClientGame module.
class IClientGameEntity;
//! GameWorld.
class ClientGameWorld;
//! TraceResult.
struct CLGTraceResult;
//! ClientGame TraceResult.
struct CLGTraceResult;
//! SharedGame TraceResult.
struct SGTRaceResult;


/**
*	Define the 'using' SharedGame types for ClientGame usage.
**/
//! Using: GameEntity
using GameEntity	= IClientGameEntity;
//! Using GameWorld.
using SGGameWorld	= ClientGameWorld;

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
// Needed includes.
// TODO: Look at these Common/ includes, nasty stuff, csame for CLTypes and CLGame, gotta deal with that asap.
#include "Common/CollisionModel.h"
#include "Common/Cmd.h"
#include "Common/Messaging.h"
#include "Common/Protocol.h"
// TODO: Look at these Common/ includes, nasty stuff, csame for CLTypes and CLGame, gotta deal with that asap.
#include "Shared/SVGame.h"
#include "Shared/CLTypes.h"
#include "Shared/CLGame.h"

//! When SHAREDGAME_UNIT is defined in a .cpp file it means we should
//! include the 'Module'-GameLocals.
#ifdef SHAREDGAME_UNIT
#include "Game/Client/ClientGameLocals.h"
#include "Game/Client/ClientGameImports.h"
#endif

/**
*	Includes:
**/
//! Type Info System.
#include "Game/Shared/Entities/TypeInfo.h"
//! Shared Entity Handle.
#include "Game/Shared/Entities/SGEntityHandle.h"
//! Shared Entity Interface.
#include "Game/Shared/Entities/ISharedGameEntity.h"
//! Client Game Entity Interface.
#include "Game/Client/Entities/IClientGameEntity.h"
//! Shared World Interface.
#include "Game/Shared/World/IGameWorld.h"



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