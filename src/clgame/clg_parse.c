// LICENSE HERE.

//
// clg_parse.c
//
//
// Handles parsing of the Server Messages.
//
#include "clg_local.h"


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
    case TE_BLOOD:
    case TE_GUNSHOT:
    case TE_SPARKS:
    case TE_BULLET_SPARKS:
    case TE_SCREEN_SPARKS:
    case TE_SHIELD_SPARKS:
    case TE_SHOTGUN:
    case TE_BLASTER:
    case TE_GREENBLOOD:
    case TE_BLASTER2:
    case TE_FLECHETTE:
    case TE_HEATBEAM_SPARKS:
    case TE_HEATBEAM_STEAM:
    case TE_MOREBLOOD:
    case TE_ELECTRIC_SPARKS:
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadDir(teParameters.dir);
        break;

    case TE_SPLASH:
    case TE_LASER_SPARKS:
    case TE_WELDING_SPARKS:
    case TE_TUNNEL_SPARKS:
        teParameters.count = clgi.MSG_ReadByte();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadDir(teParameters.dir);
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TE_BLUEHYPERBLASTER:
    case TE_RAILTRAIL:
    case TE_BUBBLETRAIL:
    case TE_DEBUGTRAIL:
    case TE_BUBBLETRAIL2:
    case TE_BFG_LASER:
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadPos(teParameters.pos2);
        break;

    case TE_GRENADE_EXPLOSION:
    case TE_GRENADE_EXPLOSION_WATER:
    case TE_EXPLOSION2:
    case TE_PLASMA_EXPLOSION:
    case TE_ROCKET_EXPLOSION:
    case TE_ROCKET_EXPLOSION_WATER:
    case TE_EXPLOSION1:
    case TE_EXPLOSION1_NP:
    case TE_EXPLOSION1_BIG:
    case TE_BFG_EXPLOSION:
    case TE_BFG_BIGEXPLOSION:
    case TE_BOSSTPORT:
    case TE_PLAIN_EXPLOSION:
    case TE_CHAINFIST_SMOKE:
    case TE_TRACKER_EXPLOSION:
    case TE_TELEPORT_EFFECT:
    case TE_DBALL_GOAL:
    case TE_WIDOWSPLASH:
    case TE_NUKEBLAST:
        clgi.MSG_ReadPos(teParameters.pos1);
        break;

    case TE_PARASITE_ATTACK:
    case TE_MEDIC_CABLE_ATTACK:
    case TE_HEATBEAM:
    case TE_MONSTER_HEATBEAM:
        teParameters.entity1 = clgi.MSG_ReadShort();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadPos(teParameters.pos2);
        break;

    case TE_GRAPPLE_CABLE:
        teParameters.entity1 = clgi.MSG_ReadShort();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadPos(teParameters.pos2);
        clgi.MSG_ReadPos(teParameters.offset);
        break;

    case TE_LIGHTNING:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.entity2 = clgi.MSG_ReadShort();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadPos(teParameters.pos2);
        break;

    case TE_FLASHLIGHT:
        clgi.MSG_ReadPos(teParameters.pos1);
        teParameters.entity1 = clgi.MSG_ReadShort();
        break;

    case TE_FORCEWALL:
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadPos(teParameters.pos2);
        teParameters.color = clgi.MSG_ReadByte();
        break;

    case TE_STEAM:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadDir(teParameters.dir);
        teParameters.color = clgi.MSG_ReadByte();
        teParameters.entity2 = clgi.MSG_ReadShort();
        if (teParameters.entity1 != -1) {
            teParameters.time = clgi.MSG_ReadLong();
        }
        break;

    case TE_WIDOWBEAMOUT:
        teParameters.entity1 = clgi.MSG_ReadShort();
        clgi.MSG_ReadPos(teParameters.pos1);
        break;

    case TE_FLARE:
        teParameters.entity1 = clgi.MSG_ReadShort();
        teParameters.count = clgi.MSG_ReadByte();
        clgi.MSG_ReadPos(teParameters.pos1);
        clgi.MSG_ReadDir(teParameters.dir);
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
// Returns qtrue in case of having handled it, otherwise returns qfalse
// and lets the client work it out.
//===============
//
qboolean CLG_UpdateConfigString(int index, const char *str) {
#if USE_LIGHTSTYLES
    if (index >= CS_LIGHTS && index < CS_LIGHTS + MAX_LIGHTSTYLES) {
        CLG_SetLightStyle(index - CS_LIGHTS, str);
        return qtrue;
    }
#endif
    // In case we aren't precaching, but got updated configstrings by the
    // server, we reload them.
    if (clgi.GetClienState() < ca_precached) {
        return qfalse;
    }
    
    if (index >= CS_PLAYERSKINS && index < CS_PLAYERSKINS + MAX_CLIENTS) {
        CLG_LoadClientinfo(&cl->clientinfo[index - CS_PLAYERSKINS], str);
        return qtrue;
    }

    return qfalse;
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

        // Client temporary entities. (Particles, etc.)
        case svc_temp_entity:
            CLG_ParseTempEntitiesPacket();
            CLG_ParseTempEntity();
            return qtrue;
        break;

        // Client Muzzle Flash.
        case svc_muzzleflash:
            CLG_ParseMuzzleFlashPacket(MZ_SILENCED);
            CLG_MuzzleFlash();
            return qtrue;
        break;
        // Entity Muzzle Flash.
        case svc_muzzleflash2:
            CLG_ParseMuzzleFlashPacket(0);
            CLG_MuzzleFlash2();
            return qtrue;
        break;

        // Client inventory updates.
        case svc_inventory:
            CLG_ParseInventory();
            return qtrue;
        break;

        // Client layout (Cruel, limited, ugly UI...) updates
        case svc_layout :
            CLG_ParseLayout();
            return qtrue;
        break;
    // Fail by default.
    default:
        return qfalse;
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
    case svc_inventory:
        CLG_ParseInventory();
        return qtrue;
        break;
    case svc_layout:
        CLG_ParseLayout();
        return qtrue;
        break;
    // Return false for failure in case we've reached this checkpoint.
    default:
        return qfalse;
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