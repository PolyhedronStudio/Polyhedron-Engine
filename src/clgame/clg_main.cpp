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

//
// Core.
//
// Contains the function pointers being passed in from the engine.
ClientGameImport clgi;
// Static export variable, lives as long as the client game dll lives.
ClientGameExport clge;

// Pointer to the actual client frame state.
ClientState* cl = NULL;
// Pointer to the actual client shared data.
ClientShared* cs = NULL;

// Actual client game state (Contains the view for example).
clientgame_t clg;


//
// CVar.
//
cvar_t *cl_chat_notify  = NULL;
cvar_t *cl_chat_sound   = NULL;
cvar_t *cl_chat_filter  = NULL;
cvar_t *cl_disable_particles    = NULL;
cvar_t *cl_disable_explosions   = NULL;
cvar_t *cl_explosion_sprites    = NULL;
cvar_t *cl_explosion_frametime  = NULL;
cvar_t *cl_footsteps    = NULL;
cvar_t *cl_gunalpha     = NULL;
cvar_t *cl_kickangles   = NULL;
cvar_t *cl_monsterfootsteps = NULL;
cvar_t *cl_noglow   = NULL;
cvar_t *cl_noskins  = NULL;
cvar_t *cl_player_model = NULL;
cvar_t *cl_predict  = NULL;
cvar_t *cl_rollhack = NULL;
cvar_t *cl_thirdperson_angle    = NULL;
cvar_t *cl_thirdperson_range    = NULL;
cvar_t *cl_vwep     = NULL;

// Refresh.
cvar_t* cvar_pt_beam_lights = NULL;

// Server.
cvar_t *sv_paused   = NULL;

// User Info.
cvar_t *gender_auto     = NULL;
cvar_t *info_fov        = NULL;
cvar_t *info_hand       = NULL;
cvar_t *info_gender     = NULL;
cvar_t *info_msg        = NULL;
cvar_t *info_name       = NULL;
cvar_t *info_password   = NULL;
cvar_t *info_skin       = NULL;
cvar_t *info_spectator  = NULL;
cvar_t *info_uf         = NULL;

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
//#ifdef __cplusplus
//extern "C" {
//#endif

q_exported ClientGameExport *GetClientGameAPI (ClientGameImport *clgimp)
{
    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store pointers to client  data.
    cl  = clgimp->cl;
    cs  = clgimp->cs;

    // Setup the API version.
    clge.apiversion = {
        CGAME_API_VERSION_MAJOR,
        CGAME_API_VERSION_MINOR,
        CGAME_API_VERSION_POINT,
    };

    // Test if it is compatible, if not, return clge with only the apiversion set.
    // The client will handle the issue from there on.
    if (clgimp->apiversion.major != CGAME_API_VERSION_MAJOR ||
        clgimp->apiversion.minor != CGAME_API_VERSION_MINOR) {
        clge.apiversion = { CGAME_API_VERSION_MAJOR, CGAME_API_VERSION_MINOR, CGAME_API_VERSION_POINT };
        return &clge;
    }

    // Setup the game export function pointers.
    // Core.
    clge.Init                       = CLG_Init;
    clge.Shutdown                   = CLG_Shutdown;

    clge.CalcFOV                    = CLG_CalculateFOV;
    clge.UpdateOrigin             = CLG_UpdateOrigin;
    clge.ClearState                 = CLG_ClearState;
    clge.DemoSeek                   = CLG_DemoSeek;

    clge.ClientBegin                = CLG_ClientBegin;
    clge.ClientDeltaFrame           = CLG_ClientDeltaFrame;
    clge.ClientFrame                = CLG_ClientFrame;
    clge.ClientDisconnect           = CLG_ClientDisconnect;

    clge.UpdateUserinfo             = CLG_UpdateUserInfo;

    // Entities.
    clge.EntityEvent                = CLG_EntityEvent;

    // Movement Command.
    clge.BuildFrameMoveCommand      = CLG_BuildFrameMoveCommand;
    clge.FinalizeFrameMoveCommand   = CLG_FinalizeFrameMoveCommand;

    // Media.
    clge.InitMedia                  = CLG_InitMedia;
    clge.GetMediaLoadStateName      = CLG_GetMediaLoadStateName;
    clge.LoadScreenMedia            = CLG_LoadScreenMedia;
    clge.LoadWorldMedia             = CLG_LoadWorldMedia;
    clge.ShutdownMedia              = CLG_ShutdownMedia;

    // Predict Movement (Client Side)
    clge.CheckPredictionError       = CLG_CheckPredictionError;
    clge.PredictAngles              = CLG_PredictAngles;
    clge.PredictMovement            = CLG_PredictMovement;

    // ServerMessage.
    clge.UpdateConfigString         = CLG_UpdateConfigString;
    clge.StartServerMessage         = CLG_StartServerMessage;
    clge.ParseServerMessage         = CLG_ParseServerMessage;
    clge.SeekDemoMessage            = CLG_SeekDemoMessage;
    clge.EndServerMessage           = CLG_EndServerMessage;

    // Screen.
    clge.RenderScreen               = CLG_RenderScreen;
    clge.ScreenModeChanged          = CLG_ScreenModeChanged;
    clge.DrawLoadScreen             = CLG_DrawLoadScreen;
    clge.DrawPauseScreen            = CLG_DrawPauseScreen;

    // View.
    clge.PreRenderView              = CLG_PreRenderView;
    clge.ClearScene                 = CLG_ClearScene;
    clge.RenderView                 = CLG_RenderView;
    clge.PostRenderView             = CLG_PostRenderView;

    // Return cgame function pointer struct.
    return &clge;
}

//#ifdef __cplusplus
//}; // Extern "C"
//#endif

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
    //cl_noskins_changed(self);
}

//
//===============
// CLG_Init
// 
// Handles the initialisation of the CG Module.
//===============
//
void CLG_Init() {
    // Begin init log.
    Com_Print("\n%s\n", "==== InitCLGame ====");

    // Register user input.
    CLG_RegisterInput();

    // Here we fetch cvars that were created by the client.
    // These are nescessary for certain CG Module functionalities.
    cl_kickangles            = clgi.Cvar_Get("cl_kickangles", NULL, 0);
    cl_noglow                = clgi.Cvar_Get("cl_noglow", NULL, 0);
    cl_noskins               = clgi.Cvar_Get("cl_noskins", "0", 0);
    cl_noskins->changed      = cl_noskins_changed;
    cl_predict               = clgi.Cvar_Get("cl_predict", NULL, 0);
    cl_rollhack              = clgi.Cvar_Get("cl_rollhack", NULL, 0);
    sv_paused                = clgi.Cvar_Get("sv_paused", NULL, 0);

    // Create the CG Module its own cvars here.
    gender_auto              = clgi.Cvar_Get("gender_auto", "1", CVAR_ARCHIVE);

    cl_footsteps             = clgi.Cvar_Get("cl_footsteps", "1", 0);
    cl_gunalpha              = clgi.Cvar_Get("cl_gunalpha", "1", 0);
    // TODO: This one was never implemented at all!!
    cl_monsterfootsteps      = clgi.Cvar_Get("cl_monsterfootsteps", "1", 0);
    cl_player_model          = clgi.Cvar_Get("cl_player_model", va("%d", CL_PLAYER_MODEL_FIRST_PERSON), CVAR_ARCHIVE);
    cl_player_model->changed = cl_player_model_changed;
    cl_thirdperson_angle     = clgi.Cvar_Get("cl_thirdperson_angle", "0", 0);
    cl_thirdperson_range     = clgi.Cvar_Get("cl_thirdperson_range", "60", 0);

    cl_chat_notify           = clgi.Cvar_Get("cl_chat_notify", "1", 0);
    cl_chat_sound            = clgi.Cvar_Get("cl_chat_sound", "1", 0);
    cl_chat_sound->changed   = cl_chat_sound_changed;
    cl_chat_sound_changed(cl_chat_sound);
    cl_chat_filter           = clgi.Cvar_Get("cl_chat_filter", "0", 0);

    cl_disable_particles     = clgi.Cvar_Get("cl_disable_particles", "0", 0);
    cl_disable_explosions    = clgi.Cvar_Get("cl_disable_explosions", "0", 0);
    cl_explosion_sprites     = clgi.Cvar_Get("cl_explosion_sprites", "1", 0);
    cl_explosion_frametime   = clgi.Cvar_Get("cl_explosion_frametime", "20", 0);
    cl_vwep                  = clgi.Cvar_Get("cl_vwep", "1", CVAR_ARCHIVE);
    cl_vwep->changed         = cl_vwep_changed;

    //
    // User Info.
    //
    info_name               = clgi.Cvar_Get("name", "Player", CVAR_USERINFO | CVAR_ARCHIVE);
    info_fov                = clgi.Cvar_Get("fov", "75", CVAR_USERINFO | CVAR_ARCHIVE);
    info_gender             = clgi.Cvar_Get("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
    info_gender->modified   = false; // clear this so we know when user sets it manually
    info_hand               = clgi.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    info_skin               = clgi.Cvar_Get("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
    info_uf                 = clgi.Cvar_Get("uf", "", CVAR_USERINFO);
    
    // TODO: not sure what this is for, don't see it used anywhere.
    info_msg                = clgi.Cvar_Get("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
    info_password           = clgi.Cvar_Get("password", "", CVAR_USERINFO);
    info_spectator          = clgi.Cvar_Get("spectator", "0", CVAR_USERINFO);

    // Video.
    vid_rtx                 = clgi.Cvar_Get("vid_rtx", NULL, 0);

    // Register our macros.
    clgi.Cmd_AddMacro("cl_health", CL_Health_m);
    clgi.Cmd_AddMacro("cl_ammo", CL_Ammo_m);
    clgi.Cmd_AddMacro("cl_armor", CL_Armor_m);
    clgi.Cmd_AddMacro("cl_weaponmodel", CL_WeaponModel_m);

    // Register our commands.
    clgi.Cmd_Register(cmd_cgmodule);

    // Generate a random user name to avoid new users being kicked out of MP servers.
    // The default N&C config files set the user name to "Player", same as the cvar initialization above.
    if (Q_strcasecmp(info_name->string, "Player") == 0)
    {
        int random_number = rand() % 10000;
        char buf[MAX_CLIENT_NAME];
        Q_snprintf(buf, sizeof(buf), "n00b-%04d", random_number);
        clgi.Cvar_Set("name", buf);
    }

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
//===============
// CLG_Shutdown
// 
// Handles the shutdown of the CG Module.
// ===============
//
void CLG_Shutdown(void) {
    // Deregister commands.
    clgi.Cmd_Deregister(cmd_cgmodule);
}


//
//===============
// CLG_UpdateUserInfo
// 
// Called when the client has changed user info.
// Here we can fix up the gender for example before all data gets applied and
// send to the other clients.
//===============
//
void CLG_UpdateUserInfo(cvar_t* var, from_t from) {
    // If there is a skin change, and the gender setting is set to auto find it...
    if (var == info_skin && from > FROM_CONSOLE && gender_auto->integer) {
        char* p;
        char sk[MAX_QPATH];

        Q_strlcpy(sk, info_skin->string, sizeof(sk));
        if ((p = strchr(sk, '/')) != NULL)
            *p = 0;
        if (Q_stricmp(sk, "male") == 0 || Q_stricmp(sk, "cyborg") == 0)
            clgi.Cvar_Set("gender", "male");
        else if (Q_stricmp(sk, "female") == 0 || Q_stricmp(sk, "crackhor") == 0)
            clgi.Cvar_Set("gender", "female");
        else
            clgi.Cvar_Set("gender", "none");
        info_gender->modified = false;
    }
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