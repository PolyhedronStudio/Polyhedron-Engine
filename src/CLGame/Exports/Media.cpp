#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Input.h"
#include "../Main.h"
#include "../Media.h"
#include "../Parse.h"
#include "../Predict.h"
#include "../Screen.h"
#include "../TemporaryEntities.h"
#include "../View.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Media.h"

//---------------
// ClientGameMedia::Initialize
//
//---------------
void ClientGameMedia::Initialize() {
    // Initialize FX Data.
    CLG_EffectsInit();

    // Initialize View Data.
    V_Init();

    // Initialize the Screen Data.
    SCR_Init();
}


//---------------
// ClientGameMedia::Shutdown
//
//---------------
void ClientGameMedia::Shutdown() {
    // Shutdown View Data.
    V_Shutdown();

    // Shutdown Screen Data.
    SCR_Shutdown();
}

//---------------
// ClientGameMedia::GetLoadStateName
//
//---------------
std::string ClientGameMedia::GetLoadStateName(LoadState loadState) {
    // One can put a switch here for the states.
    return "";
}

//---------------
// ClientGameMedia::LoadScreen
//
//---------------
void ClientGameMedia::LoadScreen() {
    SCR_RegisterMedia();
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
        CLG_LoadClientInfo(&cl->clientInfo[i], filename);
    }

    // Load in our base client (the actual player its model and skin.)
    CLG_LoadClientInfo(&cl->baseClientInfo, "unnamed\\male/grunt");

    // Last but not least, set the sky.
    CLG_SetSky();
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
        name = cl->configstrings[ConfigStrings::Models+ i];
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

//---------------
// ClientGameMedia::LoadModels
//
//---------------
void ClientGameMedia::LoadModels() {
    // Register view weapon models.
    LoadViewModels();

    // Register Temp Entity models.
    CLG_RegisterTempEntityModels();
}

//---------------
// ClientGameMedia::LoadImages
//
//---------------
void ClientGameMedia::LoadImages() {

}

//---------------
// ClientGameMedia::LoadSounds
//
//---------------
void ClientGameMedia::LoadSounds() {
    // Register temp entity sounds.
    CLG_RegisterTempEntitySounds();
}