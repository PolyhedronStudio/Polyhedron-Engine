// LICENSE HERE.

//
// clg_media.c
//
//
// Media load handling, usually happens when the renderer initializes, or
// restarts (think about changing screen mode, or other settings).
//
#include "clg_local.h"

//
//===============
// CLG_RegisterVWepModels
// 
// Register view weapon models, coming from server.
//===============
//
static void CLG_RegisterVWepModels()
{
    int         i;
    char        *name;

    cl->numWeaponModels = 1;
    strcpy(cl->weaponModels[0], "weapon.md2");

    // only default model when vwep is off
    // if (!cl_vwep->integer) {
    //     return;
    // }
    
    for (i = 2; i < MAX_MODELS; i++) {
        name = cl->configstrings[CS_MODELS + i];
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

//
//===============
// CLG_LoadClientModels
// 
// Load client models media here.
//===============
//
static void CLG_LoadClientModels(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
    
    // Register view weapon models.
    CLG_RegisterVWepModels();
}

//
//===============
// CLG_LoadClientImages
// 
// Load client image media here.
//===============
//
static void CLG_LoadClientImages(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
    
    // ...
}

//
//===============
// CLG_LoadClientSounds
// 
// Load client sound media here.
//===============
//
static void CLG_LoadClientSounds(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
    
    // ...
}

//
//=============================================================================
//
// CLIENT MODULE MEDIA ENTRY FUNCTIONS.
//
//=============================================================================
//

//
//===============
// CLG_GetMediaLoadStateName
//
// Return a string name for the custom load state type.
// Return NULL if unknown.
//===============
//
char *CLG_GetMediaLoadStateName(load_state_t state) {
    switch (state)
    {
        default:
            return NULL;
            break;
    }
}

//
//===============
// CLG_InitMedia
// 
// This is called upon every time the renderer initializes, or does a total
// hard restart.
//
// Use this to load in persistent data, such as 2D pics. Or use it to
// register certain CVars related to.
//===============
//
void CLG_InitMedia(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
    
    // Initialize View Data.
    V_Init();
}

//
//===============
// CLG_LoadScreenMedia
// 
// This is called when the client starts, but also when the renderer has had
// modified settings.
//
// It should register the basic screen media, 2D icons etc.
//===============
//
void CLG_LoadScreenMedia(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
}

//
//===============
// CLG_LoadWorldMedia
// 
// This is called when the client spawns into a server,
//
// It should register world related media here, such as particles that are
// used in-game, or view models, or sounds, etc.
//===============
//
void CLG_LoadWorldMedia(void)
{
    int i;
    char *name;

    Com_DPrint("[CG Module Callback] - %s\n", __func__);

    //
    // Set Loadstate to: LOAD_MODELS.
    //
    clgi.Com_SetClientLoadState(LOAD_MODELS);
    // Load Client Models.
    CLG_LoadClientModels();

    // THIS HERE DOES NOT WORK AT ALL FOR SOME SILLY REASON??
    // Load World Models passed from server.
    for (i = 2; i < MAX_MODELS; i++) {
        name = cl->configstrings[CS_MODELS + i];
        Com_DPrint("cl = %i, name == %s - %i\n", cl, (!name[0] ? "" : name), i);
        if (!name[0]) {
            break;
        }
        // Skip if it starts with a #, let engine handle this.
        if (name[0] == '#') {
            continue;
        } 
        cl->model_draw[i] = clgi.R_RegisterModel(name);
    }
    //
    // Set Loadstate to: LOAD_IMAGES.
    //
    // Load Image passed from server.
    clgi.Com_SetClientLoadState(LOAD_IMAGES);
    // POSSIBLE: Load client images here.
    CLG_LoadClientImages();
    for (i = 1; i < MAX_IMAGES; i++) {
        name = cl->configstrings[CS_IMAGES + i];
    Com_DPrint("name == %s - %i\n", (!name[0] ? "" : name), i);
        if (!name[0]) {
            break;
        }
        cl->image_precache[i] = clgi.R_RegisterPic2(name);
    }

    // //
    // // Set Loadstate to: LOAD_SOUNDS.
    // //
    clgi.Com_SetClientLoadState(LOAD_SOUNDS);
    // Load client sounds here.
    CLG_LoadClientSounds();
    // Load sounds passed from the server.
    for (i = 1; i < MAX_SOUNDS; i++) {
        name = cl->configstrings[CS_SOUNDS + i];
Com_DPrint("name == %s - %i\n", (!name[0] ? "" : name), i);
        if (!name[0])
            break;
        cl->sound_precache[i] = clgi.S_RegisterSound(name);
    }
}

//
//===============
// CLG_ShutdownMedia
// 
// This is called when the client stops the renderer.
// Use this to unload remaining data.
//===============
//
void CLG_ShutdownMedia (void) {
    Com_DPrint("[CG Module Callback] - %s\n", __func__);

    // Shutdown View Data.
    V_Shutdown();
}