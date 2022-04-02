/***
*
*	License here.
*
*	@file
*
*	Client Game Core Interface Implementation.
* 
***/
#include "../ClientGameLocals.h"

#include "../Entities.h"
#include "../TemporaryEntities.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "Core.h"
#include "Media.h"
#include "Movement.h"

#include "../Effects/Particles.h"


/**
*   @brief  Load or download any custom player skins and models
**/
static void CL_Skins_f(void) {
    int i;
    char* s;
    ClientInfo* ci;

    if (clgi.GetClienState() < ClientConnectionState::Loading) {
        Com_Print("Must be in a level to load skins.\n");
        return;
    }

    clge->media->LoadViewModels();

    for (i = 0; i < MAX_CLIENTS; i++) {
        s = cl->configstrings[ConfigStrings::PlayerSkins + i];
        if (!s[0])
            continue;
        ci = &cl->clientInfo[i];
        clge->media->LoadClientInfo(ci, s);
        if (!ci->model_name[0] || !ci->skin_name[0])
            ci = &cl->baseClientInfo;
        Com_Print("client %d: %s --> %s/%s\n", i, s,
            ci->model_name, ci->skin_name);
        clgi.SCR_UpdateScreen();
    }
}

/**
*   @brief  Update functions for when cvars change.
* 
*           These will notify the server about the changes, this will apply to for
*           example, spectators who are viewing a game. They'll see/hear the same
*           as the actual player they are spectating due to the settings being shared
*           to the server.
**/
static void cl_chat_sound_changed(cvar_t* self) {
    if (!*self->string)
        self->integer = 0;
    else if (!PH_StringCompare(self->string, "misc/talk.wav"))
        self->integer = 1;
    //else if (!PH_StringCompare(self->string, "misc/talk1.wav"))
    //    self->integer = 2;
    else if (!self->integer && !COM_IsUint(self->string))
        self->integer = 1;
}

static void cl_noskins_changed(cvar_t* self) {
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
        clge->media->LoadClientInfo(ci, s);
    }
}

static void cl_player_model_changed(cvar_t* self) {

}

static void cl_vwep_changed(cvar_t* self) {
    if (clgi.GetClienState() < ClientConnectionState::Loading) {
        return;
    }

    // Register view weapon models again.
    clge->media->LoadViewModels();
    cl_noskins_changed(self);
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

    { "wave" }, { "inven" }, { "kill" }, { "use" }, { "reload" },
    { "drop" }, { "info" }, /* {"prog"}, TODO: Was this a mistake of the old perhaps? */
    { "give" }, { "god" }, { "notarget" }, { "noclip" },
    { "invuse" }, { "invprev" }, { "invnext" }, { "invdrop" },
    { "weapnext" }, { "weapprev" },

    {NULL}
};

/**
*	@brief	Initializes the client game.
**/
void ClientGameCore::Initialize() {
    // Begin init log.
    Com_Print("\n%s\n", "==== InitCLGame ====");

    // Register user input.
    clge->movement->RegisterInput();

    // Here we fetch cvars that were created by the client.
    // These are nescessary for certain CG Module functionalities.
    cl_kickangles = clgi.Cvar_Get("cl_kickangles", NULL, 0);
    cl_noglow = clgi.Cvar_Get("cl_noglow", NULL, 0);
    cl_noskins = clgi.Cvar_Get("cl_noskins", "0", 0);
    cl_noskins->changed = cl_noskins_changed;
    cl_predict = clgi.Cvar_Get("cl_predict", NULL, 0);
    cl_rollhack = clgi.Cvar_Get("cl_rollhack", NULL, 0);
    sv_paused = clgi.Cvar_Get("sv_paused", NULL, 0);

    // Create the CG Module its own cvars here.
    cl_footsteps = clgi.Cvar_Get("cl_footsteps", "1", 0);
    cl_gunalpha = clgi.Cvar_Get("cl_gunalpha", "1", 0);
    // TODO: This one was never implemented at all!!
    cl_monsterfootsteps = clgi.Cvar_Get("cl_monsterfootsteps", "1", 0);
    cl_player_model = clgi.Cvar_Get("cl_player_model", va("%d", CL_PLAYER_MODEL_FIRST_PERSON), CVAR_ARCHIVE);
    cl_player_model->changed = cl_player_model_changed;
    cl_thirdperson_angle = clgi.Cvar_Get("cl_thirdperson_angle", "0", 0);
    cl_thirdperson_range = clgi.Cvar_Get("cl_thirdperson_range", "60", 0);

    cl_chat_notify = clgi.Cvar_Get("cl_chat_notify", "1", 0);
    cl_chat_sound = clgi.Cvar_Get("cl_chat_sound", "1", 0);
    cl_chat_sound->changed = cl_chat_sound_changed;
    cl_chat_sound_changed(cl_chat_sound);
    cl_chat_filter = clgi.Cvar_Get("cl_chat_filter", "0", 0);

    cl_disable_particles = clgi.Cvar_Get("cl_disable_particles", "0", 0);
    cl_disable_explosions = clgi.Cvar_Get("cl_disable_explosions", "0", 0);
    cl_explosion_sprites = clgi.Cvar_Get("cl_explosion_sprites", "1", 0);
    cl_explosion_frametime = clgi.Cvar_Get("cl_explosion_frametime", "20", 0);
    cl_vwep = clgi.Cvar_Get("cl_vwep", "1", CVAR_ARCHIVE);
    cl_vwep->changed = cl_vwep_changed;

    //
    // User Info.
    //
    info_name = clgi.Cvar_Get("name", "Player", CVAR_USERINFO | CVAR_ARCHIVE);
    info_fov = clgi.Cvar_Get("fov", "85", CVAR_USERINFO | CVAR_ARCHIVE);
    info_hand = clgi.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    info_skin = clgi.Cvar_Get("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
    info_uf = clgi.Cvar_Get("uf", "", CVAR_USERINFO);
    // TODO: not sure what this is for, don't see it used anywhere.
    info_msg = clgi.Cvar_Get("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
    info_password = clgi.Cvar_Get("password", "", CVAR_USERINFO);
    info_spectator = clgi.Cvar_Get("spectator", "0", CVAR_USERINFO);

    //
    // Server Info.
    //
    info_in_bspmenu = clgi.Cvar_Get("in_bspmenu", "0", CVAR_SERVERINFO | CVAR_ROM);

    // Video.
    vid_rtx = clgi.Cvar_Get("vid_rtx", NULL, 0);

    // Register our commands.
    clgi.Cmd_Register(cmd_cgmodule);

    // Generate a random user name to avoid new users being kicked out of MP servers.
    // The default PH config files set the user name to "Player", same as the cvar initialization above.
    if (PH_StringCaseCompare(info_name->string, "Player") == 0)     {
        int random_number = rand() % 10000;
        char buf[MAX_CLIENT_NAME];
        Q_snprintf(buf, sizeof(buf), "n00b-%04d", random_number);
        clgi.Cvar_Set("name", buf);
    }

    // Initialize effects.
    Particles::Initialize();

    // Initialize temporary entities.
    CLG_InitTempEntities();
}

/**
*	@brief	Shuts down the client game.
**/
void ClientGameCore::Shutdown() {
    clgi.Cmd_Unregister(cmd_cgmodule);
}