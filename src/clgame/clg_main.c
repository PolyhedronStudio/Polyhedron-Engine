// LICENSE HERE.

//
// clg_main.c
//
//
// Handles the main initialisation of the client game dll.
// 
// Initialises the appropriate cvars, precaches data, and handles
// shutdowns.
//
#include "clg_local.h"

//
// Core.
//
// Contains the function pointers being passed in from the engine.
clgame_import_t clgi;
// Static export variable, lives as long as the client game dll lives.
clgame_export_t clge;
// Pointer to the actual client frame state.
client_state_t *cl = NULL;

//
// Game.
//
// The client side entity array. Filled each frame.
centity_t   clg_entities[MAX_EDICTS];

//
// CVar.
//
// Client Prediction? Y/N
cvar_t  *cl_predict = NULL;
// Server Paused? Y/N
cvar_t  *sv_paused = NULL;
// User Info.
cvar_t  *info_fov = NULL;
cvar_t  *info_uf = NULL;

//
//=============================================================================
//
//	CGAME API
//
//=============================================================================
//
#ifdef __cplusplus
extern "C" {
#endif

clgame_export_t *GetClientGameAPI (clgame_import_t *clgimp)
{
    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store a pointer to the actual client state.
    cl  = clgimp->cl;

    // Setup the API version.
    clge.apiversion                 = CGAME_API_VERSION;

    // Test if it is compatible, if not, return clge with only the apiversion set.
    // The client will handle the issue from there on.
    if (clgimp->apiversion != CGAME_API_VERSION) {
        clge.apiversion = -1;
        return &clge;
    }
    Com_DPrint("cl = %i - clgimp->cl = %i\n", cl, clgimp->cl);

    // Setup the game export function pointers.
    // Core.
    clge.Init                       = CLG_Init;
    clge.Shutdown                   = CLG_Shutdown;

    clge.CalcFOV                    = CLG_CalcFOV;
    clge.CalcViewValues             = CLG_CalcViewValues;
    clge.ClientFrame                = CLG_ClientFrame;
    clge.ClearState                 = CLG_ClearState;

    // Media.
    clge.InitMedia                  = CLG_InitMedia;
    clge.GetMediaLoadStateName      = CLG_GetMediaLoadStateName;
    clge.LoadScreenMedia            = CLG_LoadScreenMedia;
    clge.LoadWorldMedia             = CLG_LoadWorldMedia;
    clge.ShutdownMedia              = CLG_ShutdownMedia;

    // ServerMessage.
    clge.StartServerMessage         = CLG_StartServerMessage;
    clge.ParseServerMessage         = CLG_ParseServerMessage;
    clge.SeekDemoMessage            = CLG_SeekDemoMessage;
    clge.EndServerMessage           = CLG_EndServerMessage;

    // View.
    clge.PreRenderView              = CLG_PreRenderView;
    clge.RenderView                 = CLG_RenderView;
    clge.PostRenderView             = CLG_PostRenderView;

    // Return cgame function pointer struct.
    return &clge;
}

#ifdef __cplusplus
}; // Extern "C"
#endif


//
//=============================================================================
//
//	CGAME INIT AND SHUTDOWN
//
//=============================================================================
//
//
//===============
// CLG_Init
// 
// Handles the initialisation of the CG Module.
//===============
//
void CLG_Init() {
    // Begin init log.
    Com_Print("\n%s\n", "==== InitCGame ====");

    // Create cvars.
    // ...

    // Fetch cvars.
    cl_predict  = clgi.Cvar_Get("cl_predict", NULL, 0);
    sv_paused   = clgi.Cvar_Get("sv_paused", NULL, 0);

    info_fov = clgi.Cvar_Get("fov", NULL, 0);
    info_uf = clgi.Cvar_Get("uf", NULL, 0);

    Com_DPrint("cl_predict = %s\n", cl_predict->string);
    Com_DPrint("sv_paused = %s\n", sv_paused->string);
    Com_DPrint("info_fov = %s\n", info_fov->string);
    Com_DPrint("info_uf = %s\n", info_uf->string);

    // Initialize effects.
    CLG_EffectsInit();

    // Initialize temporary entities.
    CLG_InitTempEntities();
}

//
//===============
// CLG_ClientFrame
// 
// Called each client frame. Handle per frame basis things here.
//===============
//
void CLG_ClientFrame() {
    // Advance local effects.
#if USE_DLIGHTS
    CLG_RunDLights();
#endif
#if USE_LIGHTSTYLES
    CLG_RunLightStyles();
#endif
}


//
//===============
// CLG_ClearState
// 
// Handles clearing the current state when the client is disconnected
// for whichever reasons.
// ===============
//
void CLG_ClearState(void) {
    // Wipe out our client entities array, so it's clean in case of a
    // new connect.
    memset(&clg_entities, 0, sizeof(clg_entities));

    // Clear Effects.
    CLG_ClearEffects();

//    CL_ClearEffects();
//#if USE_LIGHTSTYLES
//    CL_ClearLightStyles();
//#endif
//    CL_ClearTEnts();
//    LOC_FreeLocations();
}

//
//===============
// CLG_Shutdown
// 
// Handles the shutdown of the CG Module.
// ===============
//
void CLG_Shutdown(void) {

}

//
//=============================================================================
//
//	COMMON
//
//=============================================================================
// Prints a message of type PRINT_ALL. Using variable arg formatting.
void Com_Print(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ALL, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_DEVELOPER. Using variable arg formatting.
// Only prints when developer cvar is set to 1.
void Com_DPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_DEVELOPER, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_WARNING. Using variable arg formatting.
void Com_WPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_WARNING, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_ERROR. Using variable arg formatting.
void Com_EPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ERROR, "%s", buffer);
    va_end (args);
}

// Triggers an error code of type. Using variable arg formatting.
void Com_Error (error_type_t code, char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_Error(code, "%s", buffer);
    va_end (args);
}

// Prints a message of a type of your own liking. Using variable arg formatting
void    Com_LPrintf(print_type_t type, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(type, "%s", buffer);
    va_end(args);
}