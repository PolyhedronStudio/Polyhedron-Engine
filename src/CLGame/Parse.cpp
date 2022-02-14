// LICENSE HERE.

//
// clg_parse.c
//
//
// Handles parsing of the Server Messages.
//
#include "ClientGameLocal.h"

#include "Effects.h"
#include "Main.h"
#include "Media.h"
#include "Parse.h"
#include "Screen.h"
#include "TemporaryEntities.h"

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
void CLG_ParseLayout(void)
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
void CLG_ParseInventory(void)
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
void CLG_ParseTempEntitiesPacket(void)
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
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.dir = clgi.MSG_ReadVector3();
        break;

    case TempEntityEvent::Splash:
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.dir = clgi.MSG_ReadVector3();
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TempEntityEvent::DebugTrail:
    case TempEntityEvent::BubbleTrail:
    case TempEntityEvent::BubbleTrail2:
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.position2 = clgi.MSG_ReadVector3();
        break;

    case TempEntityEvent::Explosion2:
    case TempEntityEvent::Explosion1:
    case TempEntityEvent::NoParticleExplosion1:
    case TempEntityEvent::BigExplosion1:
    case TempEntityEvent::PlainExplosion:
    case TempEntityEvent::TeleportEffect:
        teParameters.position1 = clgi.MSG_ReadVector3();
        break;

    case TempEntityEvent::ForceWall:
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.position2 = clgi.MSG_ReadVector3();
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TempEntityEvent::Steam:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.dir = clgi.MSG_ReadVector3();
        teParameters.color = clgi.MSG_ReadByte();
        teParameters.entity2 = clgi.MSG_ReadShort();
        if (teParameters.entity1 != -1) {
            teParameters.time = clgi.MSG_ReadLong();
        }
        break;

    case TempEntityEvent::Flare:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        teParameters.position1 = clgi.MSG_ReadVector3();
        teParameters.dir = clgi.MSG_ReadVector3();
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
void CLG_ParseMuzzleFlashPacket(int mask)
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

    if (cl->replyTime && clgi.GetRealTime() - cl->replyTime < 120000) {
        return;
    }

    cl->replyTime = clgi.GetRealTime();
    cl->replyDelta = 1024 + (rand() & 1023);
}
#endif
void CLG_ParsePrint(void)
{
    int level;
    char s[MAX_STRING_CHARS];
    const char* fmt; // C++20: STRING: Added const to char*

    level = clgi.MSG_ReadByte();
    clgi.MSG_ReadString(s, sizeof(s));

    //SHOWNET(2, "    %i \"%s\"\n", level, s);

    if (level != PRINT_CHAT) {
        Com_Print("%s", s);
        if (!clgi.IsDemoPlayback()) {
            COM_strclr(s);
            clgi.Cmd_ExecTrigger(s);
        }
        return;
    }

    if (clgi.CheckForIgnore(s)) {
        return;
    }

#if USE_AUTOREPLY
    if (!clgi.IsDemoPlayback()) {
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
void CLG_ParseCenterPrint(void)
{
    char s[MAX_STRING_CHARS];

    clgi.MSG_ReadString(s, sizeof(s));
    //SHOWNET(2, "    \"%s\"\n", s);
    SCR_CenterPrint(s);

    if (!clgi.IsDemoPlayback()) {
        COM_strclr(s);
        clgi.Cmd_ExecTrigger(s);
    }
}