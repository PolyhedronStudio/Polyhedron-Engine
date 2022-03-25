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

// Define CGAME_INCLUDE so that files such as:
// Common/cmodel.h
// Common/cmd.h
//
#define CGAME_INCLUDE 1

// Shared.
#include "Shared/Shared.h"
#include "Shared/List.h"
#include "Shared/refresh.h"

// Common.
#include "Common/CModel.h"
#include "Common/Cmd.h"
#include "Common/Msg.h"
#include "Common/Protocol.h"

// Shared Client Game Headers.
#include "Shared/CLTypes.h"
#include "Shared/CLGame.h"

// Shared Game "Framework".
#include "../Shared/SharedGame.h"

//// Temporary.
#include "LevelLocals.h"
extern LevelLocals level;
// END OF TEMPORARY.




/**
*
*
*   ClientGame Frame Time.
*
*
**/
// WID: TODO: Make these part of the ClientGameImports instead.
static constexpr double CLG_FRAMETIME   = BASE_FRAMETIME;
static constexpr double CLG_1_FRAMETIME = BASE_1_FRAMETIME;
static constexpr int32_t CLG_FRAMEDIV   = BASE_FRAMERATE / 10.0;


/**
*
*
*   Client Game structures and definitions.
*
*
**/
/**
*   Custom client game trace struct, stores ClientEntity* instead.
**/
struct CLGTrace {
    // If true, the trace startedand ended within the same solid.
    qboolean allSolid = false;
    // If true, the trace started within a solid, but exited it.
    qboolean startSolid = false;
    // The fraction of the desired distance traveled(0.0 - 1.0).If
    // 1.0, no plane was impacted.
    float fraction = 0.f;
    // The destination position.
    vec3_t endPosition = vec3_zero();
    // [signBits][x] = either size[0][x] or size[1][x]
    vec3_t offsets[8] = {
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
	    vec3_zero(),
    };
        
    // The impacted plane, or empty. Note that a copy of the plane is returned, 
    // rather than a pointer.This is because the plane may belong to an inline 
    // BSP model or the box hull of a solid entity.
    // 
    // If it is an inline BSP Model or a box hull of a solid entity the plane 
    // must be transformed by the entity's current position.
    CollisionPlane plane = {};
    // The impacted surface, or nullptr.
    CollisionSurface* surface = nullptr;
    // The contents mask of the impacted brush, or 0.
    int32_t contents = 0;
    // The impacted entity, or nullptr.
    ClientEntity *ent = nullptr;
};


// Custom load state enumerator.
//
// Rename LOAD_CUSTOM_# or add your own.
// Once the load stage is set, the client will inquire the
// CLG_GetMediaLoadStateName function for a matching display string.
typedef enum {
    LOAD_CUSTOM_START = LOAD_SOUNDS + 1,    // DO NOT TOUCH.
    LOAD_CUSTOM_0,  // Let thy will be known, rename to your hearts content.
    LOAD_CUSTOM_1,  // Let thy will be known, rename to your hearts content.
    LOAD_CUSTOM_2   // Let thy will be known, rename to your hearts content.
    // You can add more here if you desire so.
} clg_load_state_t;

//-------------------
// Client player model settings.
//-------------------
static constexpr int32_t CL_PLAYER_MODEL_DISABLED       = 0;
static constexpr int32_t CL_PLAYER_MODEL_ONLY_GUN       = 1;
static constexpr int32_t CL_PLAYER_MODEL_FIRST_PERSON   = 2;
static constexpr int32_t CL_PLAYER_MODEL_THIRD_PERSON   = 3;

//-------------------
// Core - Used to access the client's internals.
//-------------------
extern ClientGameImport clgi;
extern ClientState      *cl;
extern ClientShared     *cs;

//-------------------
// Game - Specific to the game itself.
//-------------------
// Stores parameters parsed from a temporary entity message.s
extern tent_params_t   teParameters;
// Stores parameters parsed from a muzzleflash message.
extern mz_params_t     mzParameters;

//-------------------
// CVars - Externed so they can be accessed all over the CG Module.
//-------------------
// Client.
extern cvar_t* cl_chat_notify;
extern cvar_t* cl_chat_sound;
extern cvar_t* cl_chat_filter;

extern cvar_t* cl_disable_explosions;
extern cvar_t* cl_explosion_sprites;
extern cvar_t* cl_explosion_frametime;
extern cvar_t* cl_disable_particles;
extern cvar_t* cl_footsteps;
extern cvar_t* cl_gunalpha;
extern cvar_t* cl_kickangles;
extern cvar_t* cl_monsterfootsteps;
extern cvar_t* cl_noglow;
extern cvar_t* cl_noskins;
extern cvar_t* cl_player_model;
extern cvar_t* cl_predict;
extern cvar_t* cl_rollhack;
extern cvar_t* cl_thirdperson_angle;
extern cvar_t* cl_thirdperson_range;
extern cvar_t* cl_vwep;

// Refresh... TODO: Move.
extern cvar_t* cvar_pt_beam_lights;

// Server.
extern cvar_t* sv_paused;

// User Info.
extern cvar_t* info_fov;
extern cvar_t* info_hand;
extern cvar_t* info_msg;
extern cvar_t* info_name;
extern cvar_t* info_password;
extern cvar_t* info_skin;
extern cvar_t* info_spectator;
extern cvar_t* info_uf;
extern cvar_t* info_in_bspmenu;     // Is set to 1  at boot time when loading mainmenu.bsp, and is set 
                                    // to 1 when disconnecting from a server hence, once again, loading mainmenu.bsp
// Video.
extern cvar_t* vid_rtx;     // 1 if we're in RTX mode, 0 if not.