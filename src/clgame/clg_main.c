// LICENSE HERE.

//
// clg_main.c
//
//
// Handles the main initialisation of the client game dll.
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
client_state_t* cl = NULL;
// Pointer to the actual client shared data.
client_shared_t* cs = NULL;

// Actual client game state (Contains the view for example).
clientgame_t clg;

//
// Game.
//
//
// CVar.
//
cvar_t* cl_disable_particles = NULL;
cvar_t* cl_disable_explosions = NULL;
cvar_t* cl_explosion_sprites = NULL;
cvar_t* cl_explosion_frametime = NULL;
cvar_t  *cl_gibs = NULL;
cvar_t  *cl_gunalpha = NULL;
cvar_t  *cl_kickangles = NULL;
cvar_t  *cl_noglow = NULL;
cvar_t  *cl_noskins = NULL;
cvar_t  *cl_player_model = NULL;
cvar_t  *cl_predict = NULL;
cvar_t  *cl_rollhack = NULL;
cvar_t  *cl_thirdperson_angle = NULL;
cvar_t  *cl_thirdperson_range = NULL;
cvar_t  *cl_vwep = NULL;

// Refresh.
cvar_t* cvar_pt_beam_lights = NULL;

// Server.
cvar_t  *sv_paused = NULL;

// User Info.
cvar_t  *info_fov = NULL;
cvar_t  *info_hand = NULL;
cvar_t  *info_uf = NULL;

// Video.
cvar_t* vid_rtx = NULL;
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

    // Store pointers to client  data.
    cl  = clgimp->cl;
    cs  = clgimp->cs;

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
    clge.ClearState                 = CLG_ClearState;
    clge.DemoSeek                   = CLG_DemoSeek;

    clge.ClientBegin                = CLG_ClientBegin;
    clge.ClientFrame                = CLG_ClientFrame;

    // Media.
    clge.InitMedia                  = CLG_InitMedia;
    clge.GetMediaLoadStateName      = CLG_GetMediaLoadStateName;
    clge.LoadScreenMedia            = CLG_LoadScreenMedia;
    clge.LoadWorldMedia             = CLG_LoadWorldMedia;
    clge.ShutdownMedia              = CLG_ShutdownMedia;

    // ServerMessage.
    clge.UpdateConfigString         = CLG_UpdateConfigString;
    clge.StartServerMessage         = CLG_StartServerMessage;
    clge.ParseServerMessage         = CLG_ParseServerMessage;
    clge.SeekDemoMessage            = CLG_SeekDemoMessage;
    clge.EndServerMessage           = CLG_EndServerMessage;

    // View.
    clge.PreRenderView              = CLG_PreRenderView;
    clge.ClearScene                 = CLG_ClearScene;
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
//	CLGAME CMD FUNCTIONS
//
//=============================================================================
//
/*
=================
CL_Skins_f

Load or download any custom player skins and models
=================
*/
static void CL_Skins_f(void)
{
    int i;
    char* s;
    clientinfo_t* ci;

    if (clgi.GetClienState() < ca_loading) {
        Com_Print("Must be in a level to load skins.\n");
        return;
    }

    CLG_RegisterVWepModels();

    for (i = 0; i < MAX_CLIENTS; i++) {
        s = cl->configstrings[CS_PLAYERSKINS + i];
        if (!s[0])
            continue;
        ci = &cl->clientinfo[i];
        CLG_LoadClientinfo(ci, s);
        if (!ci->model_name[0] || !ci->skin_name[0])
            ci = &cl->baseclientinfo;
        Com_Print("client %d: %s --> %s/%s\n", i, s,
            ci->model_name, ci->skin_name);
        clgi.SCR_UpdateScreen();
    }
}


//
//=============================================================================
//
//	CLGAME INIT AND SHUTDOWN
//
//=============================================================================
//
static void CLG_UpdateGunSettings() {
    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED || info_hand->integer == 2) {
        clgi.UpdateSetting(CLS_NOGUN, 1);
    }
    else {
        clgi.UpdateSetting(CLS_NOGUN, 0);
    }
}

static void CLG_UpdateGibSettings() {
    clgi.UpdateSetting(CLS_NOGIBS, !cl_gibs->integer);
}

static void cl_gibs_changed(cvar_t* self)
{
    CLG_UpdateGibSettings();
}

static void cl_info_hand_changed(cvar_t* self)
{
    CLG_UpdateGunSettings();
}

static void cl_player_model_changed(cvar_t* self)
{
    CLG_UpdateGunSettings();
}

static void cl_noskins_changed(cvar_t* self)
{
    int i;
    char* s;
    clientinfo_t* ci;

    if (clgi.GetClienState() < ca_loading) {
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        s = cl->configstrings[CS_PLAYERSKINS + i];
        if (!s[0])
            continue;
        ci = &cl->clientinfo[i];
        CLG_LoadClientinfo(ci, s);
    }
}
static void cl_vwep_changed(cvar_t* self)
{
    if (clgi.GetClienState() < ca_loading) {
        return;
    }
    // Register view weapon models again.
    CLG_RegisterVWepModels();
    //cl_noskins_changed(self);
}

//---------------
// Macro commands to register to the client.
//---------------
static const cmdreg_t c_cgmodule[] = {
    { "skins", CL_Skins_f },
    {NULL}
};

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

    // Register our commands.
    clgi.Cmd_Register(c_cgmodule);

    // Fetch cvars from the client.
    cl_gibs                  = clgi.Cvar_Get("cl_gibs", "1", 0);
    cl_gibs->changed         = cl_gibs_changed;
    cl_gunalpha              = clgi.Cvar_Get("cl_gunalpha", NULL, 0);
    cl_kickangles            = clgi.Cvar_Get("cl_kickangles", NULL, 0);
    cl_noglow                = clgi.Cvar_Get("cl_noglow", NULL, 0);
    cl_noskins               = clgi.Cvar_Get("cl_noskins", "0", 0);
    cl_noskins->changed      = cl_noskins_changed;
    cl_predict               = clgi.Cvar_Get("cl_predict", NULL, 0);
    cl_rollhack              = clgi.Cvar_Get("cl_rollhack", NULL, 0);
    sv_paused                = clgi.Cvar_Get("sv_paused", NULL, 0);

    // Create CVars.
    cl_player_model          = clgi.Cvar_Get("cl_player_model", va("%d", CL_PLAYER_MODEL_FIRST_PERSON), CVAR_ARCHIVE);
    cl_player_model->changed = cl_player_model_changed;
    cl_thirdperson_angle     = clgi.Cvar_Get("cl_thirdperson_angle", "0", 0);
    cl_thirdperson_range     = clgi.Cvar_Get("cl_thirdperson_range", "60", 0);

    cl_disable_particles     = clgi.Cvar_Get("cl_disable_particles", "0", 0);
    cl_disable_explosions    = clgi.Cvar_Get("cl_disable_explosions", "0", 0);
    cl_explosion_sprites     = clgi.Cvar_Get("cl_explosion_sprites", "1", 0);
    cl_explosion_frametime   = clgi.Cvar_Get("cl_explosion_frametime", "20", 0);
    cl_gibs                  = clgi.Cvar_Get("cl_gibs", "1", 0);
    cl_gibs->changed         = cl_gibs_changed;
    cl_vwep                  = clgi.Cvar_Get("cl_vwep", "1", CVAR_ARCHIVE);
    cl_vwep->changed         = cl_vwep_changed;

    //
    // User Info.
    //
    info_fov                = clgi.Cvar_Get("fov", "75", CVAR_USERINFO | CVAR_ARCHIVE);
    info_hand               = clgi.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    info_hand->changed      = cl_info_hand_changed;
    info_uf                 = clgi.Cvar_Get("uf", NULL, 0);

    // Video.
    vid_rtx     = clgi.Cvar_Get("vid_rtx", NULL, 0);

    // Initialize effects.
    CLG_EffectsInit();

    // Initialize temporary entities.
    CLG_InitTempEntities();
}

//
//===============
// CLG_ClientBegin
// 
// Called after all downloads are done. (Aka, a map has started.)
// Not used for demos.
//===============
//
void CLG_ClientBegin() {
    // Update settings.
    CLG_UpdateGunSettings();
    CLG_UpdateGibSettings();
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
    // Clear Effects.
    CLG_ClearEffects();

#if USE_LIGHTSTYLES
    CLG_ClearLightStyles();
#endif
    CLG_ClearTempEntities();
//    LOC_FreeLocations();
}

//
//===============
// CLG_DemoSeek
// 
// Called when a demo seeks to a certain position for playback.
// Used to clear the scene from effects and temporary entities.
// ===============
//
void CLG_DemoSeek(void) {
    // Clear Effects.
    CLG_ClearEffects();
    // Clear Temp Entities.
    CLG_ClearTempEntities();
}

//
//===============
// CLG_Shutdown
// 
// Handles the shutdown of the CG Module.
// ===============
//
void CLG_Shutdown(void) {
    // Deregister commands.
    clgi.Cmd_Deregister(c_cgmodule);
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