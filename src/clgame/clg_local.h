/*
// LICENSE HERE.

//
// clg_local.h
//
//
// Contains the definitions of the import, and export API structs.
//
*/

#ifndef __CLGAME_LOCAL_H__
#define __CLGAME_LOCAL_H__

// Define CGAME_INCLUDE so that files such as:
// common/cmodel.h
// common/cmd.h
//
#define CGAME_INCLUDE 1

// Shared.
#include "shared/shared.h"
#include "shared/list.h"
#include "shared/refresh.h"

// // Common.
#include "common/x86/fpu.h"
#include "common/msg.h"
#include "common/protocol.h"

// Shared Game Headers.
#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/pmove.h"
#include "sharedgame/protocol.h"

// Shared Client Game Headers.
#include "shared/cltypes.h"
#include "shared/clgame.h"




//
//=============================================================================
//
//	Client Game structures and definitions.
//
//=============================================================================
//-------------------
// Client View structure.
//
// Contains all current client view entities.
//-------------------
typedef struct clg_view_s {
    // Stores the entities.
    r_entity_t entities[MAX_ENTITIES];
    int num_entities;

    // Holds all the dynamic lights currently in the view frame.
#if USE_DLIGHTS
    rdlight_t dlights[MAX_DLIGHTS];
    int num_dlights;
#endif

    // Holds all the particles currently in the view frame.
    rparticle_t particles[MAX_PARTICLES];
    int num_particles;

    // Holds all the explosions currently in the view frame.
    explosion_t  explosions[MAX_EXPLOSIONS];

    // Holds all the lightstyles currently in the view frame.
#if USE_LIGHTSTYLES
    lightstyle_t lightstyles[MAX_LIGHTSTYLES];
#endif
} clg_view_t;
extern clg_view_t view;

//-------------------
// Client Game structure.
//
// This structure is used to contain all local client game module
// variables.
//
// Expand as you please.
//-------------------
typedef struct clientgame_s {
    // This is required for C to compile. It doesn't like to compile an empty
    // struct.
    //int nothingHereYet;

    // The player move parameters.
    pmoveParams_t pmoveParams;
} clientgame_t;

extern clientgame_t clg;

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
#define CL_PLAYER_MODEL_DISABLED     0
#define CL_PLAYER_MODEL_ONLY_GUN     1
#define CL_PLAYER_MODEL_FIRST_PERSON 2
#define CL_PLAYER_MODEL_THIRD_PERSON 3

//-------------------
// Core - Used to access the client's internals.
//-------------------
extern clgame_import_t  clgi;
extern client_state_t   *cl;
extern client_shared_t  *cs;

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
extern cvar_t* gender_auto;
extern cvar_t* info_fov;
extern cvar_t* info_hand;
extern cvar_t* info_gender;
extern cvar_t* info_msg;
extern cvar_t* info_name;
extern cvar_t* info_password;
extern cvar_t* info_skin;
extern cvar_t* info_spectator;
extern cvar_t* info_uf;

// Video.
extern cvar_t* vid_rtx;     // 1 if we're in RTX mode, 0 if not.

#endif // __CLGAME_LOCAL_H__