/***
*
*	License here.
*
*	@file
*
*	Implementations for the Client Game(CG) that wrap around the Client Game(CLG) 
*	functionalities required.
* 
***/
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
void SG_Error( int32_t errorType, const std::string &errorMessage ) {
	// Call and pass on the arguments to our imported Com_Error.
	clgi.Com_Error( errorType, "%s", errorMessage.c_str() );
}

/**
*	@brief	Wraps around Com_LPrintf
*	@param	printType		One of the print types in PrintType::
*	@param	printMessage	A string, possibly formatted using fmt::format.
**/
void SG_Print( int32_t printType, const std::string &printMessage ) {
	// Call and pass on the arguments to our imported Com_LPrintf.
	clgi.Com_LPrintf( printType, "%s", printMessage.c_str() );
}
