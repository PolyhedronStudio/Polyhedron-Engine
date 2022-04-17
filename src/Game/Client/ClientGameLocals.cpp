/***
*
*	License here.
*
*	@file
* 
*   Client Game Main.
*
***/
#include "ClientGameLocals.h"

// ClientGame.
#include "Entities.h"
#include "TemporaryEntities.h"
#include "../World/ClientGameworld.h"

// ClientGameExports implementations.
#include "Exports/Core.h"
#include "Exports/Entities.h"
#include "Exports/Media.h"
#include "Exports/Movement.h"
#include "Exports/Prediction.h"
#include "Exports/Screen.h"
#include "Exports/ServerMessage.h"
#include "Exports/View.h"



/**
*   Contains the function pointers being passed in from the engine.
**/
ClientGameImport clgi;



/**
*   Pointer to the client frame state, and the client shared data.
**/
// Pointer to the actual client frame state.
ClientState* cl     = nullptr;
// Pointer to the actual client shared data.
ClientShared* cs    = nullptr;



/**
*   Core CVars.
**/
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



/**
*	Local ClientGame objects.
**/
ClientGameLocals game;
LevelLocals level;


/**
*
*
*   Client Game Locals.
*
*
**/
/**
*   @return A pointer to the gameworld object. The big man in charge.
**/
ClientGameworld *GetGameworld() {
    return game.world;
}

//**
//*   @return A pointer to the gamemode object. The man's little helper.
//**/
//IGamemode *GetGamemode() {
//    if (game.world) {
//        return game.world->GetGamemode();
//    } else {
//        return nullptr;
//    }
//}



/**
*   @return A pointer to the gameworld object. The big man in charge.
**/
ClientGameworld *ClientGameLocals::GetGameworld() {
    return world;
}


/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
//IGamemode *GetGamemode() {
//    if (game.world) {
//        return game.world->GetGamemode();
//    } else {
//        return nullptr;
//    }
//}

/**
*	@brief Initializes the gameworld and its member objects.
**/
void ClientGameLocals::Initialize() {
    // Create and initialize the world object.
    // 
    // Since it manages entities and clients it does the following things:
    // Allocate and reserve the clients array based on maxclients cvar.
    // Parse the BSP entity string to create, precache and spawn each game entity instances.
    CreateWorld();
}

/**
*	@brief Shutsdown the gamelocal.
**/
void ClientGameLocals::Shutdown() {
    // Uninitialize world and destroy its object.
    DestroyWorld();
}



/**
*   @brief Create the world member object and initialize it.
**/
void ClientGameLocals::CreateWorld() {
    // Create game world object.
    world = new ClientGameworld();

    // Initialize it.
    world->Initialize();
}

/**
*   @brief De-initialize the world and destroy it.
**/
void ClientGameLocals::DestroyWorld() {
    // Give the gameworld a chance to finalize anything.
    if (world) { 
        world->Shutdown();

        // Delete game world from memory.
        delete world;
        world = nullptr;
    }
}


/**
*   @return A pointer to the gameworld its current gamemode object.
**/
//IGamemode* ClientGameLocals::GetGamemode() { 
//    return world->GetGamemode(); 
//}



/**
*   @brief  Code shortcut for accessing gameworld's client array.
* 
*   @return A pointer to the gameworld's clients array.
**/
ServerClient* ClientGameLocals::GetClients() { return nullptr; } // { return world->GetClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxClients.
* 
*   @return The maximum allowed clients in this game.
**/
int32_t ClientGameLocals::GetMaxClients() { return 4; /* ?? */ } // { return world->GetMaxClients(); }
/**
*   @brief  Code shortcut for acquiring gameworld's maxEntities.
* 
*   @return The maximum allowed entities in this game.
**/
int32_t ClientGameLocals::GetMaxEntities() { return world->GetMaxEntities(); }



