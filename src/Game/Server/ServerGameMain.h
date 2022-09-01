/***
*
*	License here.
*
*	@file
* 
*   General functionality things of the ClientGame module goes here.
*
***/
#pragma once



/***
*
*	Include required codebases.
*
***/
/**
*	Include Shared codebase with CGAME_INCLUDE defined.
**/
#define GAME_INCLUDE 1

// Shared.
#include "Shared/Shared.h"
#include "Shared/List.h"
#include "Shared/SVGame.h"
#include "Shared/EntitySkeleton.h"


/**
*
*	Core Imports/Exports:
*	Used take and give access from game module to client.
*
**/
// ClientGame Exports Implementation.
#include "ServerGameImports.h"
#include "ServerGameExports.h"
//! Contains the function pointers being passed in from the engine.
extern ServerGameImports gi;
//! Static export variable, lives as long as the client game dll lives.
extern ServerGameExports globals;


/**
*   ClientGame Frame Time.		// WID: TODO: Make these part of the ClientGameImports instead.
**/
static constexpr double		CLG_FRAMETIME	= BASE_FRAMETIME;
static constexpr double		CLG_1_FRAMETIME	= BASE_1_FRAMETIME;
static constexpr int32_t	CLG_FRAMEDIV	= BASE_FRAMERATE / 10.0;



/**
*	Memory tags to allow dynamic memory to be cleaned up
**/
static constexpr int32_t TAG_GAME = 765;   // clear when unloading the dll
static constexpr int32_t TAG_LEVEL = 766;  // clear when loading a new level


/***
*
*	General all-round functions/structures/const-expressions.
*
***/




/***
*
*
*	CVars.
*
*
***/
/**
*	Gamemode.
**/
extern  cvar_t  *deathmatch;
extern  cvar_t  *coop;
extern  cvar_t  *gamemodeflags;
extern  cvar_t  *skill;
extern  cvar_t  *fraglimit;
extern  cvar_t  *timelimit;
extern  cvar_t  *password;
extern  cvar_t  *spectator_password;
extern  cvar_t  *needpass;
extern  cvar_t  *g_select_empty;
extern  cvar_t  *dedicated;

extern  cvar_t  *filterban;

/**
*	Physics
**/
extern  cvar_t  *sv_gravity;
extern  cvar_t  *sv_maxvelocity;


/**
*	View.
**/
extern  cvar_t  *sv_rollspeed;
extern  cvar_t  *sv_rollangle;

extern  cvar_t  *run_pitch;
extern  cvar_t  *run_roll;
extern  cvar_t  *bob_up;
extern  cvar_t  *bob_pitch;
extern  cvar_t  *bob_roll;

/**
*	General.
**/
extern  cvar_t  *sv_cheats;
extern  cvar_t  *maximumclients;
extern  cvar_t  *maxspectators;

/**
*	Anti-Message Flooding
**/
extern  cvar_t  *flood_msgs;
extern  cvar_t  *flood_persecond;
extern  cvar_t  *flood_waitdelay;

extern  cvar_t  *sv_maplist;

extern  cvar_t  *sv_flaregun;

extern  cvar_t  *cl_monsterfootsteps;