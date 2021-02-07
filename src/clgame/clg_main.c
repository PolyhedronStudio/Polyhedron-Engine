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

// Contains the function pointers being passed in from the engine.
clgame_import_t clgi;

// Static export variable, lives as long as the client game dll lives.
clgame_export_t clge;

// Pointer to the actual client frame state.
client_state_t *cl;
client_test_t *ct;

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


    // Do an API version check match.


    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store a pointer to the actual client state.
    cl  = clgimp->cl;
    ct  = clgimp->ct;

    // Setup the API version.
    clge.apiversion                 = CGAME_API_VERSION;
     
    for (int i = 0; i < MAX_MODELS; i++) {
        Com_DPrint("%s\n", ct->configstrings[CS_MODELS][i]);
    }

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

    // Media.
    clge.InitMedia                  = CLG_InitMedia;
    clge.GetMediaLoadStateName      = CLG_GetMediaLoadStateName;
    clge.LoadScreenMedia            = CLG_LoadScreenMedia;
    clge.LoadWorldMedia             = CLG_LoadWorldMedia;
    clge.ShutdownMedia              = CLG_ShutdownMedia;

    // ServerMessage.
    clge.StartServerMessage         = CLG_StartServerMessage;
    clge.ParseServerMessage         = CLG_ParseServerMessage;
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

    // Execute tests.
    CLG_ExecuteTests();
}

//
//===============
// CLG_Shutdown
// 
// Handles the shutdown of the CG Module.
// ===============
//
void CLG_Shutdown(void) {
    // Begin shutdown log.
    Com_Print("\n%s\n", "==== ShutdownCGame ====");
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