// LICENSE HERE.

//
// clg_parse.c
//
//
// Handles parsing of the Server Messages.
//
#include "clg_local.h"

#include "clg_effects.h"
#include "clg_main.h"
#include "clg_media.h"
#include "clg_parse.h"
#include "clg_screen.h"
#include "clg_tents.h"

//
//=============================================================================
//
//	SERVER MESSAGES.
//
//=============================================================================
//
// Variables used to store parsed message data in.
tent_params_t   teParameters;
mz_params_t     mzParameters;
snd_params_t    sndParameters;

//
//===============
// CLG_ParseLayout
// 
// Parses the layout string that's passed over from the server game module
// and copies it into the client state structure.
//===============
//
static void CLG_ParseLayout(void)
{
    clgi.MSG_ReadString(cl->layout, sizeof(cl->layout));
}

//
//===============
// CLG_ParseInventory
// 
// Parses the the inventory that's sent by the server game module.
//===============
//
static void CLG_ParseInventory(void)
{
    int        i;

    for (i = 0; i < MAX_ITEMS; i++) {
        cl->inventory[i] = clgi.MSG_ReadShort();
    }
}

//
//===============
// CLG_ParseInventory
// 
// Parse the temporary entity server message that keeps us up to date about
// the particles, explosions, and alike on the server side.
//
// The teParameters variable is prepared for the CLG_ParseTempEntity function.
//===============
//
static void CLG_ParseTempEntitiesPacket(void)
{
    teParameters.type = clgi.MSG_ReadByte();

    switch (teParameters.type) {
    case TempEntityEvent::Blaster:
    case TempEntityEvent::Gunshot:
    case TempEntityEvent::Shotgun:
    case TempEntityEvent::Blood:
    case TempEntityEvent::MoreBlood:
    case TempEntityEvent::Sparks:
    case TempEntityEvent::BulletSparks:
    case TempEntityEvent::ElectricSparks:
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.dir = clgi.MSG_ReadDirection();
        break;

    case TempEntityEvent::Splash:
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.dir = clgi.MSG_ReadDirection();
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TempEntityEvent::DebugTrail:
    case TempEntityEvent::BubbleTrail:
    case TempEntityEvent::BubbleTrail2:
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.pos2 = clgi.MSG_ReadPosition();
        break;

    case TempEntityEvent::Explosion2:
    case TempEntityEvent::Explosion1:
    case TempEntityEvent::NPExplosion1:
    case TempEntityEvent::BigExplosion1:
    case TempEntityEvent::PlainExplosion:
    case TempEntityEvent::TeleportEffect:
        teParameters.pos1 = clgi.MSG_ReadPosition();
        break;

    case TempEntityEvent::ForceWall:
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.pos2 = clgi.MSG_ReadPosition();
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TempEntityEvent::Steam:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.dir = clgi.MSG_ReadDirection();
        teParameters.color = clgi.MSG_ReadByte();
        teParameters.entity2 = clgi.MSG_ReadShort();
        if (teParameters.entity1 != -1) {
            teParameters.time = clgi.MSG_ReadLong();
        }
        break;

    case TempEntityEvent::Flare:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.pos1 = clgi.MSG_ReadPosition();
        teParameters.dir = clgi.MSG_ReadDirection();
        break;

    default:
        Com_Error(ERR_DROP, "%s: bad type", __func__);
    }
}

//
//===============
// CLG_ParseMuzzleFlashPacket
// 
// Parse the muzzleflash server message that keeps us up to date about
// the muzzleflashes for our own client, as well as worldly entities.
//
// The mzParameters variable is prepared for the CLG_MuzzleFlash(1/2) function.
//===============
//
static void CLG_ParseMuzzleFlashPacket(int mask)
{
    int entity, weapon;

    entity = clgi.MSG_ReadShort();
    if (entity < 1 || entity >= MAX_EDICTS)
        Com_Error(ERR_DROP, "%s: bad entity", __func__);

    weapon = clgi.MSG_ReadByte();
    mzParameters.silenced = weapon & mask;
    mzParameters.weapon = weapon & ~mask;
    mzParameters.entity = entity;
}

//
//===============
// CLG_ParsePrint
// 
// Parses print messages, this includes chat messages from other clients
// coming in from the server.
//===============
//
#if USE_AUTOREPLY
static void CLG_CheckForVersion(const char* s)
{
    char* p;

    p = (char*)strstr(s, ": "); // CPP: Cast
    if (!p) {
        return;
    }

    if (strncmp(p + 2, "!version", 8)) {
        return;
    }

    if (cl->reply_time && clgi.GetRealTime() - cl->reply_time < 120000) {
        return;
    }

    cl->reply_time = clgi.GetRealTime();
    cl->reply_delta = 1024 + (rand() & 1023);
}
#endif
static void CLG_ParsePrint(void)
{
    int level;
    char s[MAX_STRING_CHARS];
    const char* fmt; // C++20: STRING: Added const to char*

    level = clgi.MSG_ReadByte();
    clgi.MSG_ReadString(s, sizeof(s));

    //SHOWNET(2, "    %i \"%s\"\n", level, s);

    if (level != PRINT_CHAT) {
        Com_Print("%s", s);
        if (!clgi.IsDemoPlayback() && clgi.GetServerState() != ss_broadcast) {
            COM_strclr(s);
            clgi.Cmd_ExecTrigger(s);
        }
        return;
    }

    if (clgi.CheckForIgnore(s)) {
        return;
    }

#if USE_AUTOREPLY
    if (!clgi.IsDemoPlayback() && clgi.GetServerState() != ss_broadcast) {
        CLG_CheckForVersion(s);
    }
#endif

    clgi.CheckForIP(s);

    // disable notify
    if (!cl_chat_notify->integer) {
        clgi.Con_SkipNotify(true);
    }

    // filter text
    if (cl_chat_filter->integer) {
        COM_strclr(s);
        fmt = "%s\n";
    }
    else {
        fmt = "%s";
    }

    clgi.Com_LPrintf(PRINT_TALK, fmt, s);

    clgi.Con_SkipNotify(false);

    SCR_AddToChatHUD(s);

    // N&C: We don't need this stuff anymore..
    //// silence MVD spectator chat
    //if (cl.serverstate == ss_broadcast && !strncmp(s, "[MVD] ", 6))
    //    return;

    // play sound
    if (cl_chat_sound->integer > 1)
        clgi.S_StartLocalSound_("misc/talk1.wav");
    else if (cl_chat_sound->integer > 0)
        clgi.S_StartLocalSound_("misc/talk.wav");
}

//
//===============
// CL_ParseCenterPrint
// 
// Parses center print messages.
//===============
//
static void CLG_ParseCenterPrint(void)
{
    char s[MAX_STRING_CHARS];

    clgi.MSG_ReadString(s, sizeof(s));
    //SHOWNET(2, "    \"%s\"\n", s);
    SCR_CenterPrint(s);

    if (!clgi.IsDemoPlayback() && clgi.GetServerState() != ss_broadcast) {
        COM_strclr(s);
        clgi.Cmd_ExecTrigger(s);
    }
}

//
//=============================================================================
//
//	CLIENT MODULE 'PARSE' ENTRY POINTS.
//
//=============================================================================
//
//
//===============
// CLG_UpdateConfigString
// 
// Called when a configstring has been parsed and is ready to be
// loaded in again.
//
// Returns true in case of having handled it, otherwise returns false
// and lets the client work it out.
//===============
//
qboolean CLG_UpdateConfigString(int index, const char *str) {
    if (index == CS_AIRACCEL) {
        if (clg.pmoveParams.qwmode)
            clg.pmoveParams.airaccelerate = true;
        else
            clg.pmoveParams.airaccelerate = atoi(str) ? true : false;
        return true;
    }

#if USE_LIGHTSTYLES
    if (index >= CS_LIGHTS && index < CS_LIGHTS + MAX_LIGHTSTYLES) {
        CLG_SetLightStyle(index - CS_LIGHTS, str);
        return true;
    }
#endif
    // In case we aren't precaching, but got updated configstrings by the
    // server, we reload them.
    if (clgi.GetClienState() < ca_precached) {
        return false;
    }
    
    if (index >= CS_PLAYERSKINS && index < CS_PLAYERSKINS + MAX_CLIENTS) {
        CLG_LoadClientInfo(&cl->clientinfo[index - CS_PLAYERSKINS], str);
        return true;
    }

    return false;
}

//
//===============
// CLG_StartServerMessage
// 
// Called when a server message comes in.
//===============
//
void CLG_StartServerMessage (void) {

}

//
//===============
// CLG_StartServerMessage
// 
// Called when a server message hasn't been parsed by the client yet.
// Returns true on success, returns false in case of error.
//
// When returning false, the client will notify about the error.
//===============
//
qboolean CLG_ParseServerMessage (int serverCommand) {
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
        case svg_temp_entity:
            CLG_ParseTempEntitiesPacket();
            CLG_ParseTempEntity();
            return true;
        break;

        // Client Muzzle Flash.
        case svg_muzzleflash:
            CLG_ParseMuzzleFlashPacket(0);
            CLG_MuzzleFlash();
            return true;
        break;
        // Entity Muzzle Flash.
        case svg_muzzleflash2:
            CLG_ParseMuzzleFlashPacket(0);
            CLG_MuzzleFlash2();
            return true;
        break;

        // Client inventory updates.
        case svg_inventory:
            CLG_ParseInventory();
            return true;
        break;

        // Client layout (Cruel, limited, ugly UI...) updates
        case svg_layout:
            CLG_ParseLayout();
            return true;
        break;
    // Fail by default.
    default:
        return false;
    }
}

//
//===============
// CLG_SeekDemoMessage
//
// A variant of ParseServerMessage that skips over non-important action messages,
// used for seeking in demos.
//
// The main things that are important here, are events which influence 
// possible entities being displayed, HUD display, or having sounds played
// just to name a few examples.
//===============
//
qboolean CLG_SeekDemoMessage(int demoCommand) {
    // Switch cmd.
    switch (demoCommand) {
    case svg_inventory:
        CLG_ParseInventory();
        return true;
        break;
    case svg_layout:
        CLG_ParseLayout();
        return true;
        break;
    // Return false for failure in case we've reached this checkpoint.
    default:
        return false;
    }
}

//
//===============
// CLG_EndServerMessage
// 
// Called when the end of parsing a server message has been reached.
//===============
//
void CLG_EndServerMessage (int realTime) {

}