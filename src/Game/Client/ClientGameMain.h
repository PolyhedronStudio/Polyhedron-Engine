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
#define CGAME_INCLUDE 1

// Shared.
#include "Shared/Shared.h"
#include "Shared/List.h"
#include "Shared/Refresh.h"
#include "Shared/SVGame.h"

/**
*	Include several parts of the Common codebase for acquiring data structures.
**/
#include "Common/CollisionModel.h"
#include "Common/Cmd.h"
#include "Common/Messaging.h"
#include "Common/Protocol.h"

/**
*	So now we can safely include these. 
*	
*	(TODO: Replace by simply having get/set functions
*	or an object to wrap this client stuff up with. No more need for sharing all these 
*	types that way.)
**/
#include "Shared/CLTypes.h"
#include "Shared/CLGame.h"


/**
*
*	Core Imports/Exports:
*	Used take and give access from game module to client.
*
**/
// ClientGame Exports Implementation.
#include "ClientGameExports.h"
//! Contains the function pointers being passed in from the engine.
extern ClientGameImport clgi;
//! Static export variable, lives as long as the client game dll lives.
extern ClientGameExports* clge;
//! Pointer to the actual client its state.
extern ClientState      *cl;
//! Pointer to the client shared data.
extern ClientShared     *cs;

/**
*   ClientGame Frame Time.		// WID: TODO: Make these part of the ClientGameImports instead.
**/
static constexpr double		CLG_FRAMETIME	= BASE_FRAMETIME;
static constexpr double		CLG_1_FRAMETIME	= BASE_1_FRAMETIME;
static constexpr int32_t	CLG_FRAMEDIV	= BASE_FRAMERATE / 10.0;



/***
*
*	General all-round functions/structures/const-expressions.
*
***/
/**
*	Custom load state enumerator.
*	
*	Rename LOAD_CUSTOM_# or add your own.
*	Once the load stage is set, the client will inquire the
*	CLG_GetMediaLoadStateName function for a matching display string.
**/
typedef enum {
	//! Do not adjust this LOAD_CUSTOM_START value.
    LOAD_CUSTOM_START = LOAD_SOUNDS + 1,
    LOAD_RESERVED_0,
    LOAD_RESERVED_1,
    LOAD_RESERVED_2
    // One can add more here if wished for.
} clg_load_state_t;


/**
*	Client player model settings.
**/
static constexpr int32_t CL_PLAYER_MODEL_DISABLED       = 0;
static constexpr int32_t CL_PLAYER_MODEL_ONLY_GUN       = 1;
static constexpr int32_t CL_PLAYER_MODEL_FIRST_PERSON   = 2;
static constexpr int32_t CL_PLAYER_MODEL_THIRD_PERSON   = 3;



/***
*
*
*	CVars.
*
*
***/
/**
*	General/View/Gameplay.
**/
extern cvar_t* cl_disable_explosions;
extern cvar_t* cl_explosion_sprites;
extern cvar_t* cl_explosion_frametime;
extern cvar_t* cl_disable_particles;
extern cvar_t* cl_footsteps;
extern cvar_t* cl_kickangles;
extern cvar_t* cl_monsterfootsteps;
extern cvar_t* cl_noglow;
extern cvar_t* cl_noskins;
extern cvar_t* cl_player_model;
extern cvar_t* cl_predict;
extern cvar_t* cl_rollhack;
extern cvar_t* cl_thirdperson_angle;
extern cvar_t* cl_thirdperson_range;
extern cvar_t* cl_thirdperson_traceshape;
extern cvar_t* cl_vwep;

/**
*	User Info.
**/
extern cvar_t* info_fov;
extern cvar_t* info_hand;
extern cvar_t* info_msg;
extern cvar_t* info_name;
extern cvar_t* info_password;
extern cvar_t* info_skin;
extern cvar_t* info_spectator;
extern cvar_t* info_uf;
//! Is set to 1  at boot time when loading mainmenu.bsp, and is set 
//! to 1 when disconnecting from a server hence, once again, loading mainmenu.bsp
extern cvar_t* info_in_bspmenu;     


/**
*	Chat.
**/
extern cvar_t* cl_chat_notify;
extern cvar_t* cl_chat_sound;
extern cvar_t* cl_chat_filter;

/**
*	Refresh. TODO: Move.
**/
extern cvar_t* cvar_pt_beam_lights;


/**
*	Server.
**/
extern cvar_t *sv_paused;
extern cvar_t *sv_maxvelocity;
extern cvar_t *sv_gravity;


/**
*	Video.
**/
extern cvar_t* vid_rtx;				// 1 if we're in RTX mode, 0 if not.


/**
*	Developer cvars.
**/
//! Developer view weapon offset cvars.
extern cvar_t *cl_vwep_x;
extern cvar_t *cl_vwep_y;
extern cvar_t *cl_vwep_z;