/***
*
*	License here.
*
*	@file
*
*	Client Game Media Interface Implementation.
* 
***/
#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Main.h"
#include "../TemporaryEntities.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Media.h"
#include "Screen.h"
#include "View.h"


//---------------
// ClientGameMedia::Initialize
//
//---------------
void ClientGameMedia::Initialize() {
    // Initialize FX Data.
    CLG_EffectsInit();

    // Initialize View Data.
    clge->view->Initialize();

    // Initialize the Screen Data.
    clge->screen->Initialize();
}


//---------------
// ClientGameMedia::Shutdown
//
//---------------
void ClientGameMedia::Shutdown() {
    // Shutdown View Data.
    clge->view->Shutdown();

    // Shutdown Screen Data.
    clge->screen->Shutdown();
}

//---------------
// ClientGameMedia::GetLoadStateName
//
//---------------
std::string ClientGameMedia::GetLoadStateName(int32_t loadState) {
    // One can put a switch here for the states.
    return "";
}

//---------------
// ClientGameMedia::LoadWorld
//
//---------------
void ClientGameMedia::LoadWorld() {
    int i;
    char* filename;

    //
    // Set Loadstate to: LOAD_MODELS.
    //
    clgi.SetClientLoadState(LOAD_MODELS);
    // Load Client Models.
    LoadModels();
    // Load World Models passed from server.
    for (i = 2; i < MAX_MODELS; i++) {
        // Fetch string (filename).
        filename = cl->configstrings[ConfigStrings::Models + i];
        // Ensure it has a name.
        if (!filename[0]) {
            break;
        }
        // Skip if it starts with a #, let engine handle this.
        if (filename[0] == '#') {
            continue;
        }
        // Register the model.
        cl->drawModels[i] = clgi.R_RegisterModel(filename);
    }

    //
    // Set Loadstate to: LOAD_IMAGES.
    //
    // Load Image passed from server.
    clgi.SetClientLoadState(LOAD_IMAGES);
    // Load client images here.
    LoadImages();
    for (i = 1; i < MAX_IMAGES; i++) {
        // Fetch string (filename).
        filename = cl->configstrings[ConfigStrings::Images + i];
        // Ensure it has a name.
        if (!filename[0]) {
            break;
        }
        // Regtister the image.
        cl->precaches.images[i] = clgi.R_RegisterPic2(filename);
    }

    //
    // Set Loadstate to: LOAD_SOUNDS.
    //
    clgi.SetClientLoadState(LOAD_SOUNDS);
    // Load client sounds here.
    LoadSounds();
    // Load sounds passed from the server.
    for (i = 1; i < MAX_SOUNDS; i++) {
        // Fetch string (filename).
        filename = cl->configstrings[ConfigStrings::Sounds + i];
        // Ensure it has a name.
        if (!filename[0])
            break;
        // Register the sound.
        cl->precaches.sounds[i] = clgi.S_RegisterSound(filename);
    }

    // Load in all player client infos. (This thus includes their player models and skins.)
    clgi.SetClientLoadState(LOAD_CLIENTS);
    for (i = 0; i < MAX_CLIENTS; i++) {
        filename = cl->configstrings[ConfigStrings::PlayerSkins + i];
        if (!filename[0]) {
            continue;
        }
        LoadClientInfo(&cl->clientInfo[i], filename);
    }

    // Load in our base client (the actual player its model and skin.)
    LoadClientInfo(&cl->baseClientInfo, "unnamed\\male/grunt");

    // Load and set sky media, rotation and axis to their ConfigString settings to 
    // finish off our world loading process.
    LoadAndConfigureSky();
}

//-
// 
void ClientGameMedia::LoadViewModels() {
    int         i;
    char        *name;

    cl->numWeaponModels = 1;
    strcpy(cl->weaponModels[0], "weapon.md2");

    // Only default model when vwep is off
    if (!cl_vwep->integer) {
        return;
    }
    
    // Acquire the weapon model config strings. This has to go for client side weapons.
    for (i = 2; i < MAX_MODELS; i++) {
        name = cl->configstrings[ConfigStrings::Models + i];
        if (!name[0]) {
            break;
        }
        if (name[0] != '#') {
            continue;
        }

        // special player weapon model
        strcpy(cl->weaponModels[cl->numWeaponModels++], name + 1);

        if (cl->numWeaponModels == MAX_CLIENTWEAPONMODELS) {
            break;
        }
    }
}

/**
*   @brief  Load and set sky media, rotation and axis to their ConfigString settings.
**/
void ClientGameMedia::LoadAndConfigureSky(void)
{
    float       rotate;
    vec3_t      axis;

    rotate = atof(cl->configstrings[ConfigStrings::SkyRotate]);
    if (sscanf(cl->configstrings[ConfigStrings::SkyAxis], "%f %f %f",
        &axis[0], &axis[1], &axis[2]) != 3) {
        Com_DPrint("Couldn't parse ConfigStrings::SkyAxis\n");
        VectorClear(axis);
    }

    clgi.R_SetSky(cl->configstrings[ConfigStrings::Sky], rotate, axis);
}

/**
*   @brief  Loads up the data for the given client. Here you can set the default
*           models that it'll load, or totally disable clients from doing their
*           own.
*   
*           Think about a mod where you have a class system, you can load the info
*           here.
**/
void ClientGameMedia::LoadClientInfo(ClientInfo* ci, const char* str)
{
    int         i;
    char        model_name[MAX_QPATH];
    char        skin_name[MAX_QPATH];
    char        model_filename[MAX_QPATH];
    char        skin_filename[MAX_QPATH];
    char        weapon_filename[MAX_QPATH];
    char        icon_filename[MAX_QPATH];

    // Call upon parse player skin.
    ParsePlayerSkin(ci->name, model_name, skin_name, str);

    // model file
    Q_concat(model_filename, sizeof(model_filename),
        "players/", model_name, "/tris.md2", NULL);
    ci->model = clgi.R_RegisterModel(model_filename);
    if (!ci->model && PH_StringCompare(model_name, "male")) {
        strcpy(model_name, "male");
        strcpy(model_filename, "players/male/tris.md2");
        ci->model = clgi.R_RegisterModel(model_filename);
    }

    // skin file
    Q_concat(skin_filename, sizeof(skin_filename),
        "players/", model_name, "/", skin_name, ".pcx", NULL);
    ci->skin = clgi.R_RegisterSkin(skin_filename);

    // if we don't have the skin and the model was female,
    // see if athena skin exists
    if (!ci->skin && !PH_StringCompare(model_name, "female")) {
        strcpy(skin_name, "athena");
        strcpy(skin_filename, "players/female/athena.pcx");
        ci->skin = clgi.R_RegisterSkin(skin_filename);
    }

    // if we don't have the skin and the model wasn't male,
    // see if the male has it (this is for CTF's skins)
    if (!ci->skin && PH_StringCompare(model_name, "male")) {
        // change model to male
        strcpy(model_name, "male");
        strcpy(model_filename, "players/male/tris.md2");
        ci->model = clgi.R_RegisterModel(model_filename);

        // see if the skin exists for the male model
        Q_concat(skin_filename, sizeof(skin_filename),
            "players/male/", skin_name, ".pcx", NULL);
        ci->skin = clgi.R_RegisterSkin(skin_filename);
    }

    // if we still don't have a skin, it means that the male model
    // didn't have it, so default to grunt
    if (!ci->skin) {
        // see if the skin exists for the male model
        strcpy(skin_name, "grunt");
        strcpy(skin_filename, "players/male/grunt.pcx");
        ci->skin = clgi.R_RegisterSkin(skin_filename);
    }

    // weapon file
    for (i = 0; i < cl->numWeaponModels; i++) {
        Q_concat(weapon_filename, sizeof(weapon_filename),
            "players/", model_name, "/", cl->weaponModels[i], NULL);
        ci->weaponmodel[i] = clgi.R_RegisterModel(weapon_filename);
        if (!ci->weaponmodel[i] && !PH_StringCompare(model_name, "cyborg")) {
            // try male
            Q_concat(weapon_filename, sizeof(weapon_filename),
                "players/male/", cl->weaponModels[i], NULL);
            ci->weaponmodel[i] = clgi.R_RegisterModel(weapon_filename);
        }
    }

    // icon file
    Q_concat(icon_filename, sizeof(icon_filename),
        "/players/", model_name, "/", skin_name, "_i.pcx", NULL);
    ci->icon = clgi.R_RegisterPic2(icon_filename);

    strcpy(ci->model_name, model_name);
    strcpy(ci->skin_name, skin_name);

    // base info should be at least partially valid
    if (ci == &cl->baseClientInfo)
        return;

    // must have loaded all data types to be valid
    if (!ci->skin || !ci->icon || !ci->model || !ci->weaponmodel[0]) {
        ci->skin = 0;
        ci->icon = 0;
        ci->model = 0;
        ci->weaponmodel[0] = 0;
        ci->model_name[0] = 0;
        ci->skin_name[0] = 0;
    }
}

/**
*   @brief  Load client specific model media.
**/
void ClientGameMedia::LoadModels() {
    // Register view weapon models.
    LoadViewModels();

    // Register Temp Entity models.
    CLG_RegisterTempEntityModels();
}

/**
*   @brief  Load client specific image media.
**/
void ClientGameMedia::LoadImages() {

}

/**
*   @brief  Load client specific sound media here.
**/
void ClientGameMedia::LoadSounds() {
    // Register temp entity sounds.
    CLG_RegisterTempEntitySounds();
}


/**
*   @brief  Breaks up playerskin into name (optional), model and skin components.
*           If model or skin are found to be invalid, replaces them with sane defaults.
**/
void ClientGameMedia::ParsePlayerSkin(char* name, char* model, char* skin, const char* s) {
    size_t len;
    char* t;

    // configstring parsing guarantees that playerskins can never
    // overflow, but still check the length to be entirely fool-proof
    len = strlen(s);
    if (len >= MAX_QPATH) {
        Com_Error(ErrorType::Drop, "%s: oversize playerskin", __func__);
    }

    // isolate the player's name
    t = (char*)strchr(s, '\\'); // CPP: Cast
    if (t) {
        len = t - s;
        strcpy(model, t + 1);
    }
    else {
        len = 0;
        strcpy(model, s);
    }

    // copy the player's name
    if (name) {
        memcpy(name, s, len);
        name[len] = 0;
    }

    // isolate the model name
    t = strchr(model, '/');
    if (!t)
        t = strchr(model, '\\');
    if (!t)
        goto default_model;
    *t = 0;

    // isolate the skin name
    strcpy(skin, t + 1);

    // fix empty model to male
    if (t == model)
        strcpy(model, "male");

    // apply restrictions on skins
    if (cl_noskins->integer == 2 || !COM_IsPath(skin))
        goto default_skin;

    if (cl_noskins->integer || !COM_IsPath(model))
        goto default_model;

    return;

default_skin:
    if (!PH_StringCompare(model, "female")) {
        strcpy(model, "female");
        strcpy(skin, "athena");
    }
    else {
    default_model:
        strcpy(model, "male");
        strcpy(skin, "grunt");
    }
}