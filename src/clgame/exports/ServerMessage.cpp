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

#include "shared/interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "ServerMessage.h"

qboolean ClientGameServerMessage::ParsePlayerSkin(char* name, char* model, char* skin, const char* str) {
    size_t len;
    char* t;

    // configstring parsing guarantees that playerskins can never
    // overflow, but still check the length to be entirely fool-proof
    len = strlen(str);
    if (len >= MAX_QPATH) {
        Com_Error(ERR_DROP, "%s: oversize playerskin", __func__);
    }

    // isolate the player's name
    t = (char*)strchr(str, '\\'); // CPP: WARNING: Cast from const char* to char*
    if (t) {
        len = t - str;
        strcpy(model, t + 1);
    } else {
        len = 0;
        strcpy(model, str);
    }

    // copy the player's name
    if (name) {
        memcpy(name, str, len);
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

    return false;

    default_skin:
        strcpy(skin, "grunt");
        return true;
    default_model:
        strcpy(model, "male");
        return true;
}

//---------------
// ClientGameServerMessage::UpdateConfigString
//
//---------------
qboolean ClientGameServerMessage::UpdateConfigString(int32_t index, const char* str) {
    if (index == ConfigStrings::AirAcceleration) {
        return true;
    }

#if USE_LIGHTSTYLES
    if (index >= ConfigStrings::Lights && index < ConfigStrings::Lights + MAX_LIGHTSTYLES) {
        CLG_SetLightStyle(index - ConfigStrings::Lights, str);
        return true;
    }
#endif
    // In case we aren't precaching, but got updated configstrings by the
    // server, we reload them.
    if (clgi.GetClienState() < ClientConnectionState::Precached) {
        return false;
    }

    if (index >= ConfigStrings::PlayerSkins && index < ConfigStrings::PlayerSkins + MAX_CLIENTS) {
        CLG_LoadClientInfo(&cl->clientInfo[index - ConfigStrings::PlayerSkins], str);
        return true;
    }

    return false;
}

//---------------
// ClientGameServerMessage::Start
//
//---------------
void ClientGameServerMessage::Start() {

}

//---------------
// ClientGameServerMessage::Parse
//
//---------------
qboolean ClientGameServerMessage::ParseMessage(int32_t serverCommand) {
    // Switch cmd.
    switch (serverCommand) {

        // Client Print Messages.
    case svc_print:
        CLG_ParsePrint();
        return true;
        break;

        // Client Center Screen Print messages.
    case svc_centerprint:
        CLG_ParseCenterPrint();
        return true;
        break;

        // Client temporary entities. (Particles, etc.)
    case SVG_CMD_TEMP_ENTITY:
        CLG_ParseTempEntitiesPacket();
        CLG_ParseTempEntity();
        return true;
        break;

        // Client Muzzle Flash.
    case SVG_CMD_MUZZLEFLASH:
        CLG_ParseMuzzleFlashPacket(0);
        CLG_MuzzleFlash();
        return true;
        break;
        // Entity Muzzle Flash.
    case SVG_CMD_MUZZLEFLASH2:
        CLG_ParseMuzzleFlashPacket(0);
        CLG_MuzzleFlash2();
        return true;
        break;

        // Client inventory updates.
    case SVG_CMD_INVENTORY:
        CLG_ParseInventory();
        return true;
        break;

        // Client layout (Cruel, limited, ugly UI...) updates
    case SVG_CMD_LAYOUT:
        CLG_ParseLayout();
        return true;
        break;
        // Fail by default.
    default:
        return false;
    }
}

//---------------
// ClientGameServerMessage::SeekDemoMessage
//
//---------------
qboolean ClientGameServerMessage::SeekDemoMessage(int32_t demoCommand) {
    // Switch cmd.
    switch (demoCommand) {
    case SVG_CMD_INVENTORY:
        CLG_ParseInventory();
        return true;
        break;
    case SVG_CMD_LAYOUT:
        CLG_ParseLayout();
        return true;
        break;
        // Return false for failure in case we've reached this checkpoint.
    default:
        return false;
    }
}

//---------------
// ClientGameServerMessage::End
//
//---------------
void ClientGameServerMessage::End(int32_t realTime) {

}