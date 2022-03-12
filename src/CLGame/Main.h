/*
// LICENSE HERE.

//
// clgame/clg_main.h
//
*/

#pragma once

class ClientGameExports;

//! Contains the function pointers being passed in from the engine.
extern ClientGameImport clgi;
//! Static export variable, lives as long as the client game dll lives.
extern ClientGameExports* clge;

/**
*	@brief	Common print text to screen function.
**/
void Com_Print(const char* fmt, ...);

/**
*	@brief	Common print debug text to screen function.
**/
void Com_DPrint(const char* fmt, ...);

/**
*	@brief	Common print warning text to screen function.
**/
void Com_WPrint(const char* fmt, ...);

/**
*	@brief	Common print error text to screen function.
**/
void Com_EPrint(const char* fmt, ...);

/**
*	@brief	Common print specific error type and text to screen function.
**/
void Com_Error(ErrorType code, const char* fmt, ...);

/**
*	@brief	Common print text type of your choice to screen function.
**/
void Com_LPrintf(PrintType type, const char* fmt, ...);