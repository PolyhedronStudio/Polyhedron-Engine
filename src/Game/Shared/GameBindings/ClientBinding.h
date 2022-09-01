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

#include "Shared/Shared.h"

//! The name of the module that is used for differentiating what module
//! printed text.
static constexpr const char *sharedModuleName = "SharedGame(CLG)";



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


/**
*	@brief	An easy way to acquire the proper POD Entity by its number.
*	@return	(nullptr) in case of failure. Entity might be nonexistent.
**/
PODEntity *SG_GetPODEntityByNumber( const int32_t entityNumber );
/**
*	@brief	An easy way to acquire the proper POD Entity by its number.
*	@return	(nullptr) in case of failure. Entity might be nonexistent.
**/
GameEntity *SG_GetGameEntityByNumber( const int32_t entityNumber );



/***
*
*
*	CVar Access.
*
*
***/
extern cvar_t *sv_maxvelocity;
extern cvar_t *sv_gravity;
extern cvar_t *sv_friction;
extern cvar_t *sv_stopspeed;