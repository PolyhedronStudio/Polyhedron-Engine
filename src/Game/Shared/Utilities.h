/***
*
*	License here.
*
*	@file
*
*	SharedGame Utilities.
* 
***/
#pragma once


/**
*	ClientGame Entity required includes for building SharedGame code.
**/
#ifdef SHAREDGAME_CLIENTGAME
// Include IClientGameEntity.
#include "../Client/Utilities/CLGTraceResult.h"
#endif
/**
*	ServerGame Entity required includes for building SharedGame code.
**/
#ifdef SHAREDGAME_CLIENTGAME
// Include IClientGameEntity.
#include "../Server/Utilities/SVGTraceResult.h"
#endif