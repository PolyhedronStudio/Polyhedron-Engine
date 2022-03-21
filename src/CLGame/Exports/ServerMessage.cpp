/***
*
*	License here.
*
*	@file
*
*	Client Game ServerMessage Interface Implementation.
* 
***/
#include "../ClientGameLocal.h"

#include "../Entities.h"
#include "../Main.h"
#include "../TemporaryEntities.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "../HUD/ChatHUD.h"
#include "../Effects/LightStyles.h"
#include "../Effects/MuzzleFlashEffects.h"
#include "Media.h"
#include "Screen.h"
#include "ServerMessage.h"

/**
*   @brief  Breaks up playerskin into name(optional), modeland skin components.
*           If model or skin are found to be invalid, replaces them with sane defaults.
**/
qboolean ClientGameServerMessage::ParsePlayerSkin(char* name, char* model, char* skin, const char* str) {
    size_t len;
    char* t;
    const char *defaultModelname = "male";

    // configstring parsing guarantees that playerskins can never
    // overflow, but still check the length to be entirely fool-proof
    len = strlen(str);
    if (len >= MAX_QPATH) {
        Com_Error(ErrorType::Drop, "%s: oversize playerskin", __func__);
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
    if (!t) {
        t = strchr(model, '\\');
    }
    if (!t) {
        strcpy(model, defaultModelname);
        return true;
    }
    *t = 0;

    // isolate the skin name
    strcpy(skin, t + 1);

    // fix empty model to male
    if (t == model)
    strcpy(model, "male");

    // apply restrictions on skins
    if (cl_noskins->integer == 2 || !COM_IsPath(skin)) {
        strcpy(skin, "grunt");
        return true;
    }

    if (cl_noskins->integer || !COM_IsPath(model)) {
        strcpy(model, defaultModelname);
        return true;
    }

    return false;
}

/**
*   @brief  Breaks up playerskin into name(optional), modeland skin components.
*           If model or skin are found to be invalid, replaces them with sane defaults.
**/
qboolean ClientGameServerMessage::UpdateConfigString(int32_t index, const char* str) {
    // This used to be the unused AirAcceleration config string index.
    if (index == ConfigStrings::Unused) {
        return true;
    }

#if USE_LIGHTSTYLES
    if (index >= ConfigStrings::Lights && index < ConfigStrings::Lights + MAX_LIGHTSTYLES) {
        LightStyles::Set(index - ConfigStrings::Lights, str);
        return true;
    }
#endif
    // In case we aren't precaching, but got updated configstrings by the
    // server, we reload them.
    if (clgi.GetClienState() < ClientConnectionState::Precached) {
        return false;
    }

    if (index >= ConfigStrings::PlayerSkins && index < ConfigStrings::PlayerSkins + MAX_CLIENTS) {
        clge->media->LoadClientInfo(&cl->clientInfo[index - ConfigStrings::PlayerSkins], str);
        return true;
    }

    return false;
}

/**
*   @brief  Called at the start of receiving a server message.
**/
void ClientGameServerMessage::Start() {

}

/**
*   @brief  Actually parses the server message, and handles it accordingly.
*   @return True if the message was succesfully parsed. False in case the message was unkown, or corrupted, etc.
**/
qboolean ClientGameServerMessage::ParseMessage(int32_t serverCommand) {
    // Switch cmd.
    switch (serverCommand) {

        // Client Print Messages.
    case ServerCommand::Print:
        ParsePrint();
        return true;
        break;

        // Client Center Screen Print messages.
    case ServerCommand::CenterPrint:
        ParseCenterPrint();
        return true;
        break;

        // Client temporary entities. (Particles, etc.)
    case ServerGameCommand::TempEntity:
        ParseTempEntitiesPacket();
        CLG_ParseTempEntity();
        return true;
        break;

        // Client Muzzle Flash.
    case ServerGameCommand::MuzzleFlash:
        ParseMuzzleFlashPacket(0);
        MuzzleFlashEffects::ClientMuzzleFlash();
        return true;
        break;
        // Entity Muzzle Flash.
    case ServerGameCommand::MuzzleFlash2:
        ParseMuzzleFlashPacket(0);
        MuzzleFlashEffects::EntityMuzzleFlash();
        return true;
        break;

        // Client inventory updates.
    case ServerGameCommand::Inventory:
        ParseInventory();
        return true;
        break;

        // Client layout (Cruel, limited, ugly UI...) updates
    case ServerGameCommand::Layout:
        ParseLayout();
        return true;
        break;
        // Fail by default.
    default:
        return false;
    }
}

/**
*   @brief  Handles the demo message during playback.
*   @return True if the message was succesfully parsed. False in case the message was unkown, or corrupted, etc.
**/
qboolean ClientGameServerMessage::SeekDemoMessage(int32_t demoCommand) {
    // Switch cmd.
    switch (demoCommand) {
    case ServerGameCommand::Inventory:
        ParseInventory();
        return true;
        break;
    case ServerGameCommand::Layout:
        ParseLayout();
        return true;
        break;
        // Return false for failure in case we've reached this checkpoint.
    default:
        return false;
    }
}

/**
*   @brief  Called when we're done receiving a server message.
**/
void ClientGameServerMessage::End(int32_t realTime) {

}



/***
*
*   Non Interface Parse Functions.
*
***/
// Variables used to store parsed message data in.
tent_params_t   teParameters;
mz_params_t     mzParameters;
snd_params_t    sndParameters;

/**
*   @brief  Parses the client's inventory message.
**/
void ClientGameServerMessage::ParseInventory(void) {
    // Parse inventory.
    for (int32_t i = 0; i < MAX_ITEMS; i++) {
        cl->inventory[i] = clgi.MSG_ReadInt16();
    }
}

/**
*   @brief  Parses the client's layout message.
**/
void ClientGameServerMessage::ParseLayout(void) {
    // Parse layout.
//    clgi.MSG_ReadStringBuffer(cl->layout, sizeof(cl->layout));
}

/**
*   @brief  Parses a temp entities packet.
**/
void ClientGameServerMessage::ParseTempEntitiesPacket(void) {
    // Read out the type of temp entity effect.
    teParameters.type = clgi.MSG_ReadUint8();

    switch (teParameters.type) {
        case TempEntityEvent::Blaster:
        case TempEntityEvent::Gunshot:
        case TempEntityEvent::Shotgun:
        case TempEntityEvent::Blood:
        case TempEntityEvent::MoreBlood:
        case TempEntityEvent::Sparks:
        case TempEntityEvent::BulletSparks:
        case TempEntityEvent::ElectricSparks:
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.dir = clgi.MSG_ReadVector3(false);
            break;

        case TempEntityEvent::Splash:
            teParameters.count = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.dir = clgi.MSG_ReadVector3(false);
            teParameters.color = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            break;

        case TempEntityEvent::DebugTrail:
        case TempEntityEvent::BubbleTrailA:
        case TempEntityEvent::BubbleTrailB:
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.position2 = clgi.MSG_ReadVector3(false);
            break;

        case TempEntityEvent::Explosion2:
        case TempEntityEvent::Explosion1:
        case TempEntityEvent::NoParticleExplosion1:
        case TempEntityEvent::BigExplosion1:
        case TempEntityEvent::PlainExplosion:
        case TempEntityEvent::TeleportEffect:
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            break;

        case TempEntityEvent::ForceWall:
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.position2 = clgi.MSG_ReadVector3(false);
            teParameters.color = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            break;

        case TempEntityEvent::Steam:
            teParameters.entity1 = clgi.MSG_ReadInt16();//clgi.MSG_ReadShort();
            teParameters.count = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.dir = clgi.MSG_ReadVector3(false);
            teParameters.color = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            teParameters.entity2 = clgi.MSG_ReadInt16(); //clgi.MSG_ReadShort();
            if (teParameters.entity1 != -1) {
                teParameters.time = clgi.MSG_ReadInt32();//clgi.MSG_ReadLong();
            }
            break;

        case TempEntityEvent::Flare:
            teParameters.entity1 = clgi.MSG_ReadInt16();//clgi.MSG_ReadShort();
            teParameters.count = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();
            teParameters.position1 = clgi.MSG_ReadVector3(false);
            teParameters.dir = clgi.MSG_ReadVector3(false);
            break;

        default:
            Com_Error(ErrorType::Drop, "%s: bad type", __func__);
    }
}

/**
*   @brief  Parses a muzzleflash packet.
**/
void ClientGameServerMessage::ParseMuzzleFlashPacket(int32_t mask) {
    // Parse entity number.
    int32_t entity = clgi.MSG_ReadInt16();
    if (entity < 1 || entity >= MAX_EDICTS) {
        Com_Error(ErrorType::Drop, "%s: bad entity", __func__);
    }

    // Parse weapon ID.
    int32_t weapon = clgi.MSG_ReadUint8();

    // Setup muzzleflash parameters.
    mzParameters.silenced = weapon & mask;
    mzParameters.weapon = weapon & ~mask;
    mzParameters.entity = entity;
}

/**
*   @brief  Parses a print message.
**/
void ClientGameServerMessage::ParsePrint(void) {  
    // Read print level.
    int32_t printLevel = clgi.MSG_ReadUint8();//clgi.MSG_ReadByte();

    // Read string buffer.
    char stringBuffer[MAX_STRING_CHARS] = {};
    clgi.MSG_ReadStringBuffer(stringBuffer, sizeof(stringBuffer));//clgi.MSG_ReadString(s, sizeof(s));

    //SHOWNET(2, "    %i \"%s\"\n", level, s);

    if (printLevel != PRINT_CHAT) {
        Com_Print("%s", stringBuffer);
        if (!clgi.IsDemoPlayback()) {
            COM_strclr(stringBuffer);
            clgi.Cmd_ExecTrigger(stringBuffer);
        }
        return;
    }

    if (clgi.CheckForIgnore(stringBuffer)) {
        return;
    }

#if USE_AUTOREPLY
    if (!clgi.IsDemoPlayback()) {
        char* p;

        p = (char*)strstr(s, ": "); // CPP: Cast
        if (!p) {
            return;
        }

        if (strncmp(p + 2, "!version", 8)) {
            return;
        }

        if (cl->replyTime && clgi.GetRealTime() - cl->replyTime < 120000) {
            return;
        }

        cl->replyTime = clgi.GetRealTime();
        cl->replyDelta = 1024 + (rand() & 1023);
    }
#endif

    clgi.CheckForIP(stringBuffer);

    // Disable notify.
    if (!cl_chat_notify->integer) {
        clgi.Con_SkipNotify(true);
    }

    // Filter text for unprintable characters and add a newline ourselves right after doing so.
    const char* fmt;
    if (cl_chat_filter->integer) {
        COM_strclr(stringBuffer);
        fmt = "%s\n";
    } else {
        fmt = "%s";
    }

    clgi.Com_LPrintf(PrintType::Talk, fmt, stringBuffer);

    clgi.Con_SkipNotify(false);
    clge->screen->ChatPrint(stringBuffer);
    //SCR_AddToChatHUD(s);

    // play sound
    //if (cl_chat_sound->integer > 1) {
    //    clgi.S_StartLocalSound_("misc/talk1.wav");
    //} else if (cl_chat_sound->integer > 0) {
    //    clgi.S_StartLocalSound_("misc/talk.wav");
    //}
    clgi.S_StartLocalSound("misc/talk.wav");
}

/**
*   @brief  Parses a centerprint message.
**/
void ClientGameServerMessage::ParseCenterPrint(void) {
    // Read string buffer.
    char stringBuffer[MAX_STRING_CHARS] = {};
    clgi.MSG_ReadStringBuffer(stringBuffer, sizeof(stringBuffer));

    //SHOWNET(2, "    \"%s\"\n", s);
    clge->screen->CenterPrint(stringBuffer);

    if (!clgi.IsDemoPlayback()) {
        COM_strclr(stringBuffer);
        clgi.Cmd_ExecTrigger(stringBuffer);
    }
}