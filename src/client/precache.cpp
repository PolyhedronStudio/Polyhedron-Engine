/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

//
// cl_precache.c
//

#include "client.h"
#include "client/gamemodule.h"
#include "client/sound/vorbis.h"

/*
================
CL_ParsePlayerSkin

Breaks up playerskin into name (optional), model and skin components.
If model or skin are found to be invalid, replaces them with sane defaults.
================
*/
void CL_ParsePlayerSkin(char *name, char *model, char *skin, const char *s)
{
    // Let client game module handle this.
    CL_GM_ParsePlayerSkin(name, model, skin, s);
}


/*
=================
CL_RegisterBspModels

Registers main BSP file and inline models
=================
*/
void CL_RegisterBspModels(void)
{
    qerror_t ret;
    char *name;
    int i;

    ret = BSP_Load(cl.configstrings[ConfigStrings::Models+ 1], &cl.bsp);
    if (cl.bsp == NULL) {
        Com_Error(ERR_DROP, "Couldn't load %s: %s",
                  cl.configstrings[ConfigStrings::Models+ 1], Q_ErrorString(ret));
    }

#if USE_MAPCHECKSUM
    if (cl.bsp->checksum != atoi(cl.configstrings[ConfigStrings::MapCheckSum])) {
        if (cls.demo.playback) {
            Com_WPrintf("Local map version differs from demo: %i != %s\n",
                        cl.bsp->checksum, cl.configstrings[ConfigStrings::MapCheckSum]);
        } else {
            Com_Error(ERR_DROP, "Local map version differs from server: %i != %s",
                      cl.bsp->checksum, cl.configstrings[ConfigStrings::MapCheckSum]);
        }
    }
#endif

    for (i = 1; i < MAX_MODELS; i++) {
        name = cl.configstrings[ConfigStrings::Models+ i];
        if (!name[0]) {
            break;
        }
        if (name[0] == '*')
            cl.clipModels[i] = BSP_InlineModel(cl.bsp, name);
        else
            cl.clipModels[i] = NULL;
    }
}


/*
=================
CL_PrepareMedia

Call before entering a new level, or after changing dlls
=================
*/
void CL_PrepareMedia(void)
{
    if (!cls.ref_initialized)
        return;
    if (!cl.mapName[0])
        return;     // no map loaded


    // register models, pics, and skins
    R_BeginRegistration(cl.mapName);
    // register sounds.
    S_BeginRegistration();

    // N&C: Pass over loading to the CG Module so it can actively
    // manage the load state. This is useful for load screen information.
    CL_GM_LoadWorldMedia();

    // Ensure to register these too, for client side prediction.
    CL_RegisterBspModels();
    
    // The sound engine can now free unneeded stuff
    S_EndRegistration();

    // the renderer can now free unneeded stuff
    R_EndRegistration(cl.mapName);

    // clear any lines of console text
    Con_ClearNotify_f();

    SCR_UpdateScreen();

	int cdtrack = atoi(cl.configstrings[ConfigStrings::CdTrack]);
    OGG_PlayTrack(cdtrack);
}

/*
=================
CL_UpdateConfigstring

A configstring update has been parsed.
=================
*/
void CL_UpdateConfigstring(int index)
{
    const char *s = cl.configstrings[index];

    // Let the CG Module handle the string.
    // If it returns TRUE it has succeeded.
    // If it returns false, we move on.
    if (CL_GM_UpdateConfigString(index, s)) {
        // We're done here.
        return;
    }

    if (index == ConfigStrings::MaxClients) {
        cl.maximumClients = atoi(s);
        return;
    }

    if (index == ConfigStrings::Models+ 1) {
        size_t len = strlen(s);

        if (len <= 9) {
            Com_Error(ERR_DROP, "%s: bad world model: %s", __func__, s);
        }
        memcpy(cl.mapName, s + 5, len - 9);   // skip "maps/"
        cl.mapName[len - 9] = 0; // cut off ".bsp"
        return;
    }

    // Anything processed after this if statement is done so only when we're
    // not fully precached yet. 
    if (cls.connectionState < ClientConnectionState::Precached) {
        return;
    }

    // TODO: Move all over to CG Module and ONLY
    // handle the BSP Model loading. EXAMPLE:
    //if (index >= ConfigStrings::Models+ 2 && index < ConfigStrings::Models+ MAX_MODELS) {
    //    if (*s == '*') {
    //        int i = index - ConfigStrings::Models;
    //        cl.drawModels[i] = R_RegisterModel(s);
    //        cl.clipModels[i] = BSP_InlineModel(cl.bsp, s);
    //    }
    //    return;
    //}
    
    if (index >= ConfigStrings::Models+ 2 && index < ConfigStrings::Models+ MAX_MODELS) {
        int i = index - ConfigStrings::Models;

        cl.drawModels[i] = R_RegisterModel(s);
        if (*s == '*')
            cl.clipModels[i] = BSP_InlineModel(cl.bsp, s);
        else
            cl.clipModels[i] = NULL;
        return;
    }
    if (index >= ConfigStrings::Sounds&& index < ConfigStrings::Sounds+ MAX_SOUNDS) {
        cl.precaches.sounds[index - ConfigStrings::Sounds] = S_RegisterSound(s);
        return;
    }
    if (index >= ConfigStrings::Images&& index < ConfigStrings::Images+ MAX_IMAGES) {
        cl.precaches.images[index - ConfigStrings::Images] = R_RegisterPic2(s);
        return;
    }
}
