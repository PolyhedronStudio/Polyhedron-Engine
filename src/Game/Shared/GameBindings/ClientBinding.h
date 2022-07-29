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

// Shared Game.
#include "../SharedGame.h"



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
