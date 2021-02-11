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
// #include "common/cmodel.h"
// #include "common/cmd.h"
// #include "common/math.h"
#include "common/x86/fpu.h"
#include "common/msg.h"
#include "common/pmove.h"
#include "common/protocol.h"

// Shared Client Game Header.
#include "shared/cl_types.h"
#include "shared/cl_game.h"


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
    entity_t entities[MAX_ENTITIES];
    int num_entities;

    // Holds all the dynamic lights currently in the view frame.
#if USE_DLIGHTS
    dlight_t dlights[MAX_DLIGHTS];
    int num_dlights;
#endif

    // Holds all the particles currently in the view frame.
    particle_t particles[MAX_PARTICLES];
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
extern client_shared_t* cs;

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
extern cvar_t* cl_disable_explosions;
extern cvar_t* cl_explosion_sprites;
extern cvar_t* cl_explosion_frametime;
extern cvar_t* cl_disable_particles;
extern cvar_t* cl_footsteps;
extern cvar_t* cl_gibs;
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
extern cvar_t* vid_rtx;


//
//=============================================================================
//
//	Client Game Function declarations.
//
//=============================================================================
//
//
// clg_entities.c
//
// void CLG_ENT_Create(); or other CLG_ENT_?? What name shall we pick?
void CLG_AddPacketEntities(void);
void CLG_AddViewWeapon(void);
void CLG_CalcViewValues(void);

//
// clg_effects.c
//
void CLG_ClearEffects(void);
void CLG_EffectsInit(void);

cparticle_t* CLG_AllocParticle(void);
void CLG_AddParticles(void);
#if USE_DLIGHTS
cdlight_t* CLG_AllocDLight(int key);
void CLG_AddDLights(void);
void CLG_RunDLights(void);
#endif
#if USE_LIGHTSTYLES
void CLG_ClearLightStyles(void);
void CLG_AddLightStyles(void);
void CLG_RunLightStyles(void);
void CLG_SetLightStyle(int index, const char* s);
#endif

void CLG_MuzzleFlash(void);
void CLG_MuzzleFlash2(void);
void CLG_BFGExplosionParticles(vec3_t org);
void CLG_BfgParticles(entity_t* ent);
void CLG_BigTeleportParticles(vec3_t org);
void CLG_BlasterTrail(vec3_t start, vec3_t end);
void CLG_BlasterParticles(vec3_t org, vec3_t dir);
void CLG_BloodParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void CLG_BubbleTrail(vec3_t start, vec3_t end);
void CLG_DiminishingTrail(vec3_t start, vec3_t end, centity_t* old, int flags);
void CLG_ExplosionParticles(vec3_t org);
void CLG_FlagTrail(vec3_t start, vec3_t end, int color);
void CLG_FlyEffect(centity_t* ent, vec3_t origin);
void CLG_OldRailTrail(void);
void CLG_ParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void CLG_ParticleEffect2(vec3_t org, vec3_t dir, int color, int count);
void CLG_ParticleEffectWaterSplash(vec3_t org, vec3_t dir, int color, int count);
void CLG_RocketTrail(vec3_t start, vec3_t end, centity_t* old);
void CLG_TeleportParticles(vec3_t org);

//
// clg_main.c
//
void CLG_Init();
void CLG_Shutdown(void);

void CLG_ClientFrame(void);
void CLG_ClientBegin(void);
void CLG_ClearState(void);
void CLG_DemoSeek(void);

void CLG_UpdateUserInfo(cvar_t* var, from_t from);

void Com_Print(char *fmt, ...);
void Com_DPrint(char *fmt, ...);
void Com_WPrint(char *fmt, ...);
void Com_EPrint(char *fmt, ...);
void Com_Error (error_type_t code, char *fmt, ...);


//
// clg_media.c
//

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

void CLG_RegisterVWepModels();
void CLG_LoadClientInfo(clientinfo_t* ci, const char* str);
void CLG_SetSky(void);

void CLG_InitMedia(void);
char *CLG_GetMediaLoadStateName(load_state_t state);
void CLG_LoadScreenMedia(void);
void CLG_LoadWorldMedia(void);
void CLG_ShutdownMedia(void);

//
// clg_newfx.c
//
#if USE_DLIGHTS
void CLG_Flashlight(int ent, vec3_t pos);
void CLG_ColorFlash(vec3_t pos, int ent, int intensity, float r, float g, float b);
#endif
void CLG_DebugTrail(vec3_t start, vec3_t end);
void CLG_SmokeTrail(vec3_t start, vec3_t end, int colorStart, int colorRun, int spacing);
void CLG_ForceWall(vec3_t start, vec3_t end, int color);
void CLG_GenericParticleEffect(vec3_t org, vec3_t dir, int color, int count, int numcolors, int dirspread, float alphavel);
void CLG_BubbleTrail2(vec3_t start, vec3_t end, int dist);
void CLG_Heatbeam(vec3_t start, vec3_t forward);
void CLG_ParticleSteamEffect(vec3_t org, vec3_t dir, int color, int count, int magnitude);
void CLG_ParticleSteamEffect2(cl_sustain_t* self);
void CLG_TrackerTrail(vec3_t start, vec3_t end, int particleColor);
void CLG_Tracker_Shell(vec3_t origin);
void CLG_MonsterPlasma_Shell(vec3_t origin);
void CLG_Widowbeamout(cl_sustain_t* self);
void CLG_Nukeblast(cl_sustain_t* self);
void CLG_WidowSplash(void);
void CLG_Tracker_Explode(vec3_t  origin);
void CLG_TagTrail(vec3_t start, vec3_t end, int color);
void CLG_ColorExplosionParticles(vec3_t org, int color, int run);
void CLG_ParticleSmokeEffect(vec3_t org, vec3_t dir, int color, int count, int magnitude);
void CLG_BlasterParticles2(vec3_t org, vec3_t dir, unsigned int color);
void CLG_BlasterTrail2(vec3_t start, vec3_t end);
void CLG_IonripperTrail(vec3_t start, vec3_t ent);
void CLG_TrapParticles(entity_t* ent);
void CLG_ParticleEffect3(vec3_t org, vec3_t dir, int color, int count);


//
// clg_parse.c
//
qboolean CLG_UpdateConfigString(int index, const char* str);
void CLG_StartServerMessage(void);
qboolean CLG_ParseServerMessage(int serverCommand);
qboolean CLG_SeekDemoMessage(int demoCommand);
void CLG_EndServerMessage(int realTime);


//
// clg_predict.c
//
void CLG_CheckPredictionError(int frame, unsigned int cmd);
void CLG_PredictAngles(void);
void CLG_PredictMovement(unsigned int ack, unsigned int current);


//
// clg_screen.c
//
void SCR_Init(void);
void SCR_Shutdown(void);


//
// clg_tent.c
//
void CLG_ParseTempEntity(void);

void CLG_RegisterTempEntityModels(void);
void CLG_RegisterTempEntitySounds(void);

void CLG_ClearTempEntities(void);
void CLG_AddTempEntities(void);
void CLG_InitTempEntities(void);


//
// clg_tests.c
//
void CLG_ExecuteTests (void);


//
// clg_view.c
//
// For debugging purposes.
extern int         gun_frame;
extern qhandle_t   gun_model;

void V_Init(void);
void V_Shutdown(void);

void V_AddEntity(entity_t* ent);
void V_AddLight(vec3_t org, float intensity, float r, float g, float b);
void V_AddLightEx(vec3_t org, float intensity, float r, float g, float b, float radius);
void V_AddLightStyle (int style, vec4_t value);
void V_AddParticle(particle_t* p);

float CLG_CalcFOV(float fov_x, float width, float height);
void CLG_CalcViewValues(void);

void CLG_PreRenderView(void);
void CLG_ClearScene(void);
void CLG_RenderView(void);
void CLG_PostRenderView(void);

#endif // __CLGAME_LOCAL_H__