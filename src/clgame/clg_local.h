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
//
//
// Core - Used to access the client's internals.
//
extern clgame_import_t  clgi;
extern client_state_t   *cl;

//
// Game - Specific to the game itself.
//
// Stores parameters parsed from a temporary entity message.s
extern tent_params_t   teParameters;
// Stores parameters parsed from a muzzleflash message.
extern mz_params_t     mzParameters;

//
// CVars - Externed so they can be accessed all over the CG Module.
//
// Client Prediction?
extern cvar_t* cl_predict;
// Server Paused?
extern cvar_t* sv_paused;
// User Info.
extern cvar_t* info_fov;
extern cvar_t* info_uf;


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


//
// clg_effects.c
//
void CLG_ClearEffects(void);
void CLG_EffectsInit(void);

void CLG_AddParticles(void);
#if USE_DLIGHTS
void CLG_AddDLights(void);
#endif
#if USE_LIGHTSTYLES
void CLG_AddLightStyles(void);
#endif

void CLG_MuzzleFlash(void);
void CLG_MuzzleFlash2(void);

void CLG_RunDLights(void);


//
// clg_main.c
//
void CLG_Init();
void CLG_Shutdown(void);

void CLG_ClientFrame(void);
void CLG_ClearState(void);

void CLG_StartServerMessage(void);
qboolean CLG_ParseServerMessage(int serverCommand);
qboolean CLG_SeekDemoMessage(int demoCommand);
void CLG_EndServerMessage(int realTime);

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

void CLG_InitMedia(void);
char *CLG_GetMediaLoadStateName(load_state_t state);
void CLG_LoadScreenMedia(void);
void CLG_LoadWorldMedia(void);
void CLG_ShutdownMedia(void);

//
// clg_tent.c
//
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
void V_Init (void);
void V_Shutdown (void);

void V_AddLight (vec3_t org, float intensity, float r, float g, float b);
void V_AddLightStyle (int style, vec4_t value);

void CLG_CalcViewValues(void);

void CLG_PreRenderView (void);
void CLG_RenderView (void);
void CLG_PostRenderView (void);

#endif // __CLGAME_LOCAL_H__