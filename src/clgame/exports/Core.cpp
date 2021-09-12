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

#include "shared/interfaces/IClientGameExports.h"
#include "ClientGameExports.h"
#include "Core.h"

//---------------
// ClientGameExportCore::Initialize
//
//---------------
void ClientGameExportCore::Initialize() {
    // Begin init log.
    Com_Print("\n%s\n", "==== InitCLGame ====");

    // Register user input.
    CLG_RegisterInput();

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
    gender_auto = clgi.Cvar_Get("gender_auto", "1", CVAR_ARCHIVE);

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
    info_fov = clgi.Cvar_Get("fov", "75", CVAR_USERINFO | CVAR_ARCHIVE);
    info_gender = clgi.Cvar_Get("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
    info_gender->modified = false; // clear this so we know when user sets it manually
    info_hand = clgi.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    info_skin = clgi.Cvar_Get("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
    info_uf = clgi.Cvar_Get("uf", "", CVAR_USERINFO);

    // TODO: not sure what this is for, don't see it used anywhere.
    info_msg = clgi.Cvar_Get("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
    info_password = clgi.Cvar_Get("password", "", CVAR_USERINFO);
    info_spectator = clgi.Cvar_Get("spectator", "0", CVAR_USERINFO);

    // Video.
    vid_rtx = clgi.Cvar_Get("vid_rtx", NULL, 0);

    // Register our macros.
    clgi.Cmd_AddMacro("cl_health", CL_Health_m);
    clgi.Cmd_AddMacro("cl_ammo", CL_Ammo_m);
    clgi.Cmd_AddMacro("cl_armor", CL_Armor_m);
    clgi.Cmd_AddMacro("cl_weaponmodel", CL_WeaponModel_m);

    // Register our commands.
    clgi.Cmd_Register(cmd_cgmodule);

    // Generate a random user name to avoid new users being kicked out of MP servers.
    // The default N&C config files set the user name to "Player", same as the cvar initialization above.
    if (Q_strcasecmp(info_name->string, "Player") == 0)     {
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

//---------------
// ClientGameExportCore::Shutdown
//
//---------------
void ClientGameExportCore::Shutdown() {

}