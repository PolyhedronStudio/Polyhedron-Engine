// LICENSE HERE.

//
// clg_main.c
//
//
// Handles the main initialisation of the client game dll.
//
#include "ClientGameLocal.h"

// ClientGame.
#include "Entities.h"
#include "Main.h"
#include "TemporaryEntities.h"

// ClientGameExports implementations.
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

// Pointer to the actual client frame state.
ClientState* cl     = nullptr;
// Pointer to the actual client shared data.
ClientShared* cs    = nullptr;

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
cvar_t* vid_rtx = nullptr;

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

q_exported IClientGameExports* GetClientGameAPI(ClientGameImport* clgimp) {
    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store pointers to client  data.
    cl = clgimp->cl;
    cs = clgimp->cs;

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
//	COMMON
//
//=============================================================================
// Prints a message of type PrintType::All. Using variable arg formatting.
void Com_Print(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::All, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Developer. Using variable arg formatting.
// Only prints when developer cvar is set to 1.
void Com_DPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Developer, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Warning. Using variable arg formatting.
void Com_WPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Warning, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Error. Using variable arg formatting.
void Com_EPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Error, "%s", buffer);
    va_end (args);
}

// Triggers an error code of type. Using variable arg formatting.
void Com_Error (int32_t errorType, const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_Error(errorType, "%s", buffer);
    va_end (args);
}

// Prints a message of a type of your own liking. Using variable arg formatting
void Com_LPrintf(int32_t printType, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(printType, "%s", buffer);
    va_end(args);
}