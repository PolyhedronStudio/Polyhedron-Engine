// LICENSE HERE.

//
// clg_main.c
//
//
// Handles the main initialisation of the client game dll.
//
#include "clg_local.h"

#include "clg_effects.h"
#include "clg_entities.h"
#include "clg_input.h"
#include "clg_main.h"
#include "clg_media.h"
#include "clg_parse.h"
#include "clg_predict.h"
#include "clg_screen.h"
#include "clg_tents.h"
#include "clg_view.h"

// ClientGameExports interface implementations.
#include "ClientGameExports.h"
#include "exports/Core.h"
#include "exports/Entities.h"
#include "exports/Media.h"
#include "exports/Movement.h"
#include "exports/Prediction.h"
#include "exports/Screen.h"
#include "exports/ServerMessage.h"
#include "exports/View.h"

//
// Core.
//
// Contains the function pointers being passed in from the engine.
ClientGameImport clgi;
// Static export variable, lives as long as the client game dll lives.
IClientGameExports *clge;

// Pointer to the actual client frame state.
ClientState* cl = NULL;
// Pointer to the actual client shared data.
ClientShared* cs = NULL;

// Actual client game state (Contains the view for example).
clientgame_t clg;


//
// CVar.
//
cvar_t *cl_chat_notify          = nullptr;
cvar_t *cl_chat_sound           = nullptr;
cvar_t *cl_chat_filter          = nullptr;
cvar_t *cl_disable_particles    = nullptr;
cvar_t *cl_disable_explosions   = nullptr;
cvar_t *cl_explosion_sprites    = nullptr;
cvar_t *cl_explosion_frametime  = nullptr;
cvar_t *cl_footsteps            = nullptr;
cvar_t *cl_gunalpha             = nullptr;
cvar_t *cl_kickangles           = nullptr;
cvar_t *cl_monsterfootsteps     = nullptr;
cvar_t *cl_noglow               = nullptr;
cvar_t *cl_noskins              = nullptr;
cvar_t *cl_player_model         = nullptr;
cvar_t *cl_predict              = nullptr;
cvar_t *cl_rollhack             = nullptr;
cvar_t *cl_thirdperson_angle    = nullptr;
cvar_t *cl_thirdperson_range    = nullptr;
cvar_t *cl_vwep                 = nullptr;

// Refresh.
cvar_t* cvar_pt_beam_lights     = nullptr;

// Server.
cvar_t *sv_paused   = nullptr;

// User Info.
cvar_t *info_fov        = nullptr;
cvar_t *info_hand       = nullptr;
cvar_t *info_msg        = nullptr;
cvar_t *info_name       = nullptr;
cvar_t *info_password   = nullptr;
cvar_t *info_skin       = nullptr;
cvar_t *info_spectator  = nullptr;
cvar_t *info_uf         = nullptr;
cvar_t *info_in_bspmenu = nullptr; // Is set to 1  at boot time when loading mainmenu.bsp, and is set 
                                // to 1 when disconnecting from a server hence, once again, loading mainmenu.bsp

// Video.
cvar_t* vid_rtx = NULL;

//
//=============================================================================
//
//	CGAME API
//
//=============================================================================
//

// CPP: These might need to be re-enabled on Linux.
#ifdef __cplusplus
extern "C" {
#endif

//q_exported ClientGameExport *GetClientGameAPI (ClientGameImport *clgimp)
//{
//    // Store a copy of the engine imported function pointer struct.
//    clgi = *clgimp;
//
//    // Store pointers to client  data.
//    cl  = clgimp->cl;
//    cs  = clgimp->cs;
//
//    // Setup the API version.
//    clge.apiversion = {
//        CGAME_API_VERSION_MAJOR,
//        CGAME_API_VERSION_MINOR,
//        CGAME_API_VERSION_POINT,
//    };
//
//    // Test if it is compatible, if not, return clge with only the apiversion set.
//    // The client will handle the issue from there on.
//    if (clgimp->apiversion.major != CGAME_API_VERSION_MAJOR ||
//        clgimp->apiversion.minor != CGAME_API_VERSION_MINOR) {
//        clge.apiversion = { CGAME_API_VERSION_MAJOR, CGAME_API_VERSION_MINOR, CGAME_API_VERSION_POINT };
//        return &clge;
//    }
//
//    // Setup the game export function pointers.
//    // Core.
//    clge.Init                       = CLG_Init;
//    clge.Shutdown                   = CLG_Shutdown;
//
//    clge.CalcFOV                    = CLG_CalculateFOV;
//    clge.UpdateOrigin             = CLG_UpdateOrigin;
//    clge.ClearState                 = CLG_ClearState;
//    clge.DemoSeek                   = CLG_DemoSeek;
//
//    clge.ClientBegin                = CLG_ClientBegin;
//    clge.ClientDeltaFrame           = CLG_ClientDeltaFrame;
//    clge.ClientFrame                = CLG_ClientFrame;
//    clge.ClientDisconnect           = CLG_ClientDisconnect;
//
//    clge.UpdateUserinfo             = CLG_UpdateUserInfo;
//
//    // Entities.
//    clge.EntityEvent                = CLG_EntityEvent;
//
//    // Movement Command.
//    clge.BuildFrameMoveCommand      = CLG_BuildFrameMoveCommand;
//    clge.FinalizeFrameMoveCommand   = CLG_FinalizeFrameMoveCommand;
//
//    // Media.
//    clge.InitMedia                  = CLG_InitMedia;
//    clge.GetMediaLoadStateName      = CLG_GetMediaLoadStateName;
//    clge.LoadScreenMedia            = CLG_LoadScreenMedia;
//    clge.LoadWorldMedia             = CLG_LoadWorldMedia;
//    clge.ShutdownMedia              = CLG_ShutdownMedia;
//
//    // Predict Movement (Client Side)
//    clge.CheckPredictionError       = CLG_CheckPredictionError;
//    clge.PredictAngles              = CLG_PredictAngles;
//    clge.PredictMovement            = CLG_PredictMovement;
//
//    // ServerMessage.
//    clge.UpdateConfigString         = CLG_UpdateConfigString;
//    clge.StartServerMessage         = CLG_StartServerMessage;
//    clge.ParseServerMessage         = CLG_ParseServerMessage;
//    clge.SeekDemoMessage            = CLG_SeekDemoMessage;
//    clge.EndServerMessage           = CLG_EndServerMessage;
//
//    // Screen.
//    clge.RenderScreen               = CLG_RenderScreen;
//    clge.ScreenModeChanged          = CLG_ScreenModeChanged;
//    clge.DrawLoadScreen             = CLG_DrawLoadScreen;
//    clge.DrawPauseScreen            = CLG_DrawPauseScreen;
//
//    // View.
//    clge.PreRenderView              = CLG_PreRenderView;
//    clge.ClearScene                 = CLG_ClearScene;
//    clge.RenderView                 = CLG_RenderView;
//    clge.PostRenderView             = CLG_PostRenderView;
//
//    // Return cgame function pointer struct.
//    return &clge;
//}
q_exported IClientGameExports* GetClientGameAPI(ClientGameImport* clgimp) {
    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store pointers to client  data.
    cl = clgimp->cl;
    cs = clgimp->cs;

    // Setup the API version.
    //clge.apiversion = {
    //    CGAME_API_VERSION_MAJOR,
    //    CGAME_API_VERSION_MINOR,
    //    CGAME_API_VERSION_POINT,
    //};

    // Test if it is compatible, if not, return clge with only the apiversion set.
    // The client will handle the issue from there on.
    if (clgimp->apiversion.major != CGAME_API_VERSION_MAJOR ||
        clgimp->apiversion.minor != CGAME_API_VERSION_MINOR) {
        //clge.apiversion = { CGAME_API_VERSION_MAJOR, CGAME_API_VERSION_MINOR, CGAME_API_VERSION_POINT };
        
        // WID: TODO: Gotta do something alternative here.
        return nullptr;
        //return &clge;
    }

    // Allocate the client game exports interface and its member implementations.
    clge = new ClientGameExports();
    clge->core = new ClientGameCore();
    clge->entities = new ClientGameEntities();
    clge->media = new ClientGameMedia();
    clge->movement = new ClientGameMovement();
    clge->prediction = new ClientGamePrediction();
    clge->screen = new ClientGameScreen();
    clge->serverMessage = new ClientGameServerMessage();
    clge->view = new ClientGameView();

    // Return cgame function pointer struct.
    return clge;
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
    ClientInfo* ci;

    if (clgi.GetClienState() < ClientConnectionState::Loading) {
        Com_Print("Must be in a level to load skins.\n");
        return;
    }

    CLG_RegisterVWepModels();

    for (i = 0; i < MAX_CLIENTS; i++) {
        s = cl->configstrings[ConfigStrings::PlayerSkins + i];
        if (!s[0])
            continue;
        ci = &cl->clientInfo[i];
        CLG_LoadClientInfo(ci, s);
        if (!ci->model_name[0] || !ci->skin_name[0])
            ci = &cl->baseClientInfo;
        Com_Print("client %d: %s --> %s/%s\n", i, s,
            ci->model_name, ci->skin_name);
        clgi.SCR_UpdateScreen();
    }
}

//---------------
// Minor command macro functions.
//
// They can have the same name as a cvar, but when a macro $ is used,
// it gains a higher priority than the cvar.
//
// Example: echo $cl_health
// will output the amount of health.
//
// These can be useful for debugging purposes, or for general use cases
// where commands may need to be constructed for menus etc.
//---------------
static size_t CL_Health_m(char* buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", cl->frame.playerState.stats[STAT_HEALTH]);
}

static size_t CL_Ammo_m(char* buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", cl->frame.playerState.stats[STAT_AMMO]);
}

static size_t CL_Armor_m(char* buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", cl->frame.playerState.stats[STAT_ARMOR]);
}

static size_t CL_WeaponModel_m(char* buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%s",
        cl->configstrings[cl->frame.playerState.gunIndex + ConfigStrings::Models]);
}

//---------------
// Commands to register to the client.
//---------------
static const cmdreg_t cmd_cgmodule[] = {
    //
    // Client commands.
    //
    { "skins", CL_Skins_f },

    //
    // Forward to server commands
    //
    // The only thing this does is allow command completion
    // to work. All of the unknown commands are automatically
    // forwarded to the server for handling.

    // TODO: maybe some day, we'll move these over to the CG Module.
    //{ "say", NULL, CL_Say_c },
    //{ "say_team", NULL, CL_Say_c },

    { "wave" }, { "inven" }, { "kill" }, { "use" },
    { "drop" }, { "info" }, { "prog" },
    { "give" }, { "god" }, { "notarget" }, { "noclip" },
    { "invuse" }, { "invprev" }, { "invnext" }, { "invdrop" },
    { "weapnext" }, { "weapprev" },

    {NULL}
};

//
//=============================================================================
//
//	CLGAME INIT AND SHUTDOWN
//
//=============================================================================
//
//---------------
// Update functions for when cvars change.
//
// These will notify the server about the changes, this will apply to for
// example, spectators who are viewing a game. They'll see/hear the same
// as the actual player they are spectating due to the settings being shared
// to the server.
//---------------
static void cl_chat_sound_changed(cvar_t* self)
{
    if (!*self->string)
        self->integer = 0;
    else if (!Q_stricmp(self->string, "misc/talk.wav"))
        self->integer = 1;
    else if (!Q_stricmp(self->string, "misc/talk1.wav"))
        self->integer = 2;
    else if (!self->integer && !COM_IsUint(self->string))
        self->integer = 1;
}

static void cl_noskins_changed(cvar_t* self)
{
    int i;
    char* s;
    ClientInfo* ci;

    if (clgi.GetClienState() < ClientConnectionState::Loading) {
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        s = cl->configstrings[ConfigStrings::PlayerSkins + i];
        if (!s[0])
            continue;
        ci = &cl->clientInfo[i];
        CLG_LoadClientInfo(ci, s);
    }
}

static void cl_player_model_changed(cvar_t* self)
{

}

static void cl_vwep_changed(cvar_t* self)
{
    if (clgi.GetClienState() < ClientConnectionState::Loading) {
        return;
    }
    // Register view weapon models again.
    CLG_RegisterVWepModels();
    cl_noskins_changed(self);
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

}

//
//===============
// CLG_ClientDeltaFrame
// 
// Called each VALID client frame. Handle per VALID frame basis things here.
//===============
//
void CLG_ClientDeltaFrame(void) {
    // Called each time a valid client frame has been 
    SCR_SetCrosshairColor();
}

//
//===============
// CLG_ClientDisconnect
// 
// Called when the client gets disconnected, for whichever reasons.
//===============
//
void CLG_ClientDisconnect(void) {
    // Clear the chat hud.
    SCR_ClearChatHUD_f();
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
//=============================================================================
//
//	COMMON
//
//=============================================================================
// Prints a message of type PRINT_ALL. Using variable arg formatting.
void Com_Print(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ALL, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_DEVELOPER. Using variable arg formatting.
// Only prints when developer cvar is set to 1.
void Com_DPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_DEVELOPER, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_WARNING. Using variable arg formatting.
void Com_WPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_WARNING, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_ERROR. Using variable arg formatting.
void Com_EPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ERROR, "%s", buffer);
    va_end (args);
}

// Triggers an error code of type. Using variable arg formatting.
void Com_Error (ErrorType code, const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_Error(code, "%s", buffer);
    va_end (args);
}

// Prints a message of a type of your own liking. Using variable arg formatting
void Com_LPrintf(PrintType type, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(type, "%s", buffer);
    va_end(args);
}