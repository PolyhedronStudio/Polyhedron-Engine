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
// Common/cmodel.h
// Common/cmd.h
//
#define CGAME_INCLUDE 1

// Shared.
#include "Shared/Shared.h"
#include "Shared/list.h"
#include "Shared/refresh.h"

// Common.
#include "Common/CModel.h"
#include "Common/Cmd.h"
#include "Common/Msg.h"
#include "Common/Protocol.h"

// Shared Game Headers.
#include "SharedGame/SharedGame.h" // Include SG Base.
#include "SharedGame/PMove.h"
#include "SharedGame/Protocol.h"

// Shared Client Game Headers.
#include "Shared/CLTypes.h"
#include "Shared/CLGame.h"

// "Shared" cl frametime.
// WID: TODO: Make these part of the ClientGameImports instead.
static constexpr double CLG_FRAMETIME = BASE_FRAMETIME;
static constexpr double CLG_1_FRAMETIME = BASE_1_FRAMETIME;
static constexpr int32_t CLG_FRAMEDIV = BASE_FRAMERATE / 10.0;
static inline qboolean CLG_FRAMESYNC() {
    extern ClientState *cl;
    return !(cl->frame.number % CLG_FRAMEDIV);
}

//
//=============================================================================
//
//	Client Game structures and definitions.
//
//=============================================================================
// 
//
// Custom client game trace struct, stores ClientEntity* instead.
//
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
    cplane_t plane = {};
    // The impacted surface, or nullptr.
    csurface_t* surface = nullptr;
    // The contents mask of the impacted brush, or 0.
    int32_t contents = 0;
    // The impacted entity, or nullptr.
    ClientEntity *ent = nullptr;
};

//-------------------
// Client View structure.
//
// Contains all current client view entities.
//-------------------
typedef struct clg_view_s {
    // Stores the entities.
    r_entity_t entities[MAX_ENTITIES];
    int32_t num_entities;

    // Holds all the dynamic lights currently in the view frame.
    rdlight_t dlights[MAX_DLIGHTS];
    int32_t num_dlights;

    // Holds all the particles currently in the view frame.
    rparticle_t particles[MAX_PARTICLES];
    int32_t num_particles;

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
    int nothingHereYet;
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

#endif // __CLGAME_LOCAL_H__