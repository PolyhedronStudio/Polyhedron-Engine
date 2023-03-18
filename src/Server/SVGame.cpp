/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// sv_game.c -- interface to the game dll

#include "Server.h"
#include "Models.h"
#include "Shared/SkeletalModelData.h"
#include "Common/SkeletalModelData.h"
#include "Common/EntitySkeleton.h"

#include "../Client/Client.h"

ServerGameExports    *ge;

static void PF_configstring(int index, const char *val);

/*
================
PF_FindIndex

================
*/
static int PF_FindIndex(const char *name, int start, int max)
{
    char *string;
    int i;

    if (!name || !name[0])
        return 0;

    for (i = 1; i < max; i++) {
        string = sv.configstrings[start + i];
        if (!string[0]) {
            break;
        }
        if (!strcmp(string, name)) {
            return i;
        }
    }

    if (i == max)
        Com_Error(ErrorType::Drop, "PF_FindIndex: overflow");
	
    PF_configstring(i + start, name);

    return i;
}

static int PF_PrecacheModel(const char *name)
{
    return PF_FindIndex(name, ConfigStrings::Models, MAX_MODELS);
}

static int PF_PrecacheSound(const char *name)
{
    return PF_FindIndex(name, ConfigStrings::Sounds, MAX_SOUNDS);
}

static int PF_PrecacheImage(const char *name)
{
    return PF_FindIndex(name, ConfigStrings::Images, MAX_IMAGES);
}

/*
===============
PF_Unicast

Sends the contents of the mutlicast buffer to a single client.
Archived in MVD stream.
===============
*/
static void PF_Unicast(Entity *ent, qboolean reliable)
{
    client_t    *client;
    int         cmd, flags, clientNumber;

    if (!ent) {
        goto clear;
    }

    clientNumber = NUM_FOR_EDICT(ent) - 1;
    if (clientNumber < 0 || clientNumber >= sv_maxclients->integer) {
        Com_WPrintf("%s to a non-client %d\n", __func__, clientNumber);
        goto clear;
    }

    client = svs.clientPool + clientNumber;
    if (client->connectionState <= ConnectionState::Zombie) {
        Com_WPrintf("%s to a free/zombie client %d\n", __func__, clientNumber);
        goto clear;
    }

    if (!msg_write.currentSize) {
        Com_DPrintf("%s with empty data\n", __func__);
        goto clear;
    }

    cmd = msg_write.data[0];

    flags = 0;
    if (reliable) {
        flags |= MSG_RELIABLE;
    }

    //if (cmd == ServerGameCommand::Layout) {
    //    flags |= MSG_COMPRESS;
    //}

    SV_ClientAddMessage(client, flags);

    // fix anti-kicking exploit for broken mods
    if (cmd == ServerCommand::Disconnect) {
        client->drop_hack = true;
        goto clear;
    }

clear:
    SZ_Clear(&msg_write);
}

/*
=================
PF_bprintf

Sends text to all active clients.
Archived in MVD stream.
=================
*/
static void PF_bprintf(int level, const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    client_t    *client;
    size_t      len;
    int         i;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::Print);//MSG_WriteByte(ServerCommand::Print);
    MSG_WriteUint8(level); //MSG_WriteByte(level);
    MSG_WriteData(string, len + 1);

    // echo to console
    if (COM_DEDICATED) {
        // mask off high bits
        for (i = 0; i < len; i++)
            string[i] &= 127;
        Com_Printf("%s", string);
    }

    FOR_EACH_CLIENT(client) {
        if (client->connectionState != ConnectionState::Spawned)
            continue;
        if (level >= client->messageLevel) {
            SV_ClientAddMessage(client, MSG_RELIABLE);
        }
    }

    SZ_Clear(&msg_write);
}


/*
===============
PF_dprintf

Debug print to server console.
===============
*/
static void PF_dprintf(const char *fmt, ...)
{
    char        msg[MAXPRINTMSG];
    va_list     argptr;

    va_start(argptr, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Com_Printf("%s", msg);
}


/*
===============
PF_cprintf

Print to a single client if the level passes.
Archived in MVD stream.
===============
*/
static void PF_cprintf(Entity *ent, int level, const char *fmt, ...)
{
    char        msg[MAX_STRING_CHARS];
    va_list     argptr;
    int         clientNumber;
    size_t      len;
    client_t    *client;

    va_start(argptr, fmt);
    len = Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(msg)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    if (!ent) {
        Com_LPrintf(level == PRINT_CHAT ? PrintType::Talk : PrintType::Regular, "%s", msg);
        return;
    }

    clientNumber = NUM_FOR_EDICT(ent) - 1;
    if (clientNumber < 0 || clientNumber >= sv_maxclients->integer) {
        Com_Error(ErrorType::Drop, "%s to a non-client %d", __func__, clientNumber);
    }

    client = svs.clientPool + clientNumber;
    if (client->connectionState <= ConnectionState::Zombie) {
        Com_WPrintf("%s to a free/zombie client %d\n", __func__, clientNumber);
        return;
    }

    MSG_WriteUint8(ServerCommand::Print);//MSG_WriteByte(ServerCommand::Print);
    MSG_WriteUint8(level);//MSG_WriteByte(level);
    MSG_WriteData(msg, len + 1);

    if (level >= client->messageLevel) {
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }


    SZ_Clear(&msg_write);
}


/*
===============
PF_centerprintf

Centerprint to a single client.
Archived in MVD stream.
===============
*/
static void PF_centerprintf(Entity *ent, const char *fmt, ...)
{
    char        msg[MAX_STRING_CHARS];
    va_list     argptr;
    int         n;
    size_t      len;

    if (!ent) {
        return;
    }

    n = NUM_FOR_EDICT(ent);
    if (n < 1 || n > sv_maxclients->integer) {
        Com_WPrintf("%s to a non-client %d\n", __func__, n - 1);
        return;
    }

    va_start(argptr, fmt);
    len = Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(msg)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::CenterPrint); //MSG_WriteByte(ServerCommand::CenterPrint);
    MSG_WriteData(msg, len + 1);

    PF_Unicast(ent, true);
}


/*
===============
PF_error

Abort the server with a game error
===============
*/
static q_noreturn void PF_error( int32_t errorType, const char *fmt, ...)
{
    char        msg[MAXERRORMSG];
    va_list     argptr;

    va_start(argptr, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Com_Error(ErrorType::Drop, "SVGame Error: %s", msg);
}

/**
*	@brief	Loads in an IQM model that can be used server-side.
**/
static qhandle_t PF_RegisterModel(const char *name) {
	return SV_Model_RegisterModel(name);
}

/**
*	@return	A pointer to the model structure for given handle. (nullptr) on failure.
**/
static model_t *PF_GetModelByHandle(qhandle_t handle) {
	model_t *model = SV_Model_ForHandle(handle);

	if (!model) {
		// TODO: Warn.
		return nullptr;
	}

	//if (model->skeletalModelData) {
		//return &model->skeletalModelData;
	//}
	return model;
}

/**
*	@return	A pointer to the model structure for given handle. (nullptr) on failure.
**/
static SkeletalModelData *PF_GetSkeletalModelDataByHandle(qhandle_t handle) {
	model_t *model = SV_Model_ForHandle(handle);

	if (!model) {
		// TODO: Warn.
		return nullptr;
	}

	if (model->skeletalModelData) {
		return model->skeletalModelData;
	}

	return nullptr;
}

/**
*	@brief	Utility function to test whether an animation is existent and within range.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified action.
**/
SkeletalAnimation *PF_ES_GetAnimationByName(EntitySkeleton *entitySkeleton, const std::string &name) {
	return ES_GetAnimation( entitySkeleton, name );
}
SkeletalAnimation *PF_ES_GetAnimationByIndex(EntitySkeleton *entitySkeleton, const int32_t index) {
	return ES_GetAnimation( entitySkeleton, index );
}

/**
*	@brief	Utility function to easily get a pointer to an Action by name or index.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified Action.
**/
SkeletalAnimationAction *PF_ES_GetActionByName ( EntitySkeleton *entitySkeleton, const std::string &name ) {
	return ES_GetAction( entitySkeleton, name );
}
SkeletalAnimationAction *PF_ES_GetActionByIndex ( EntitySkeleton *entitySkeleton, const int32_t index ) {
	return ES_GetAction( entitySkeleton, index );
}

/*
=================
PF_setmodel

Also sets mins and maxs for inline bmodels
=================
*/
static void PF_setmodel(Entity *ent, const char *name)
{
    int         i;
    mmodel_t    *mod;

    if (!name)
        Com_Error(ErrorType::Drop, "PF_setmodel: NULL");

    i = PF_PrecacheModel(name);

    ent->currentState.modelIndex = i;

// if it is an inline model, get the size information for it
    if (name[0] == '*') {
        mod = CM_InlineModel(&sv.cm, name);
        VectorCopy(mod->mins, ent->mins);
        VectorCopy(mod->maxs, ent->maxs);
        PF_LinkEntity(ent);
    }

}

/*
===============
PF_configstring

If game is actively running, broadcasts configstring change.
===============
*/
static void PF_configstring(int index, const char *val)
{
    size_t len, maxlen;
    client_t *client;
    char *dst;

    if (index < 0 || index >= ConfigStrings::MaxConfigStrings)
        Com_Error(ErrorType::Drop, "%s: bad index: %d", __func__, index);

    if (sv.serverState == ServerState::Dead) {
        Com_WPrintf("%s: not yet initialized\n", __func__);
        return;
    }

    if (!val)
        val = "";

    // error out entirely if it exceedes array bounds
    len = strlen(val);
    maxlen = (ConfigStrings::MaxConfigStrings- index) * MAX_QPATH;
    if (len >= maxlen) {
        Com_Error(ErrorType::Drop,
                  "%s: index %d overflowed: %" PRIz " > %" PRIz, // CPP: String fix.
                  __func__, index, len, maxlen - 1);
    }

    // print a warning and truncate everything else
    maxlen = CS_SIZE(index);
    if (len >= maxlen) {
        Com_WPrintf(
            "%s: index %d overflowed: %" PRIz " > %" PRIz "\n",
            __func__, index, len, maxlen - 1);
        len = maxlen - 1;
    }

    dst = sv.configstrings[index];
    if (!strncmp(dst, val, len)) {
        return;
    }

    // change the string in sv
    memcpy(dst, val, len);
    dst[len] = 0;

    if (sv.serverState == ServerState::Loading) {
        return;
    }

    // send the update to everyone
    MSG_WriteUint8(ServerCommand::ConfigString); //MSG_WriteByte(ServerCommand::ConfigString);
    MSG_WriteInt16(index);//MSG_WriteShort(index);
    MSG_WriteData(val, len);
    MSG_WriteUint8(0);//MSG_WriteByte(0);

    FOR_EACH_CLIENT(client) {
        if (client->connectionState < ConnectionState::Primed) {
            continue;
        }
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SZ_Clear(&msg_write);
}

static qboolean PF_inVIS(vec3_t p1, vec3_t p2, int vis)
{
    mleaf_t *leaf1 = NULL, *leaf2 = NULL;
    static byte mask[VIS_MAX_BYTES];
    bsp_t *bsp = sv.cm.cache;

    if (!bsp) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
    }

    leaf1 = BSP_PointLeaf(bsp->nodes, p1);
    BSP_ClusterVis(bsp, mask, leaf1->cluster, vis);

    leaf2 = BSP_PointLeaf(bsp->nodes, p2);
    if (leaf2->cluster == -1)
        return false;
    if (!Q_IsBitSet(mask, leaf2->cluster))
        return false;
    if (!CM_AreasConnected(&sv.cm, leaf1->area, leaf2->area))
        return false;        // a door blocks it
    return true;
}

/*
=================
PF_inPVS

Also checks portalareas so that doors block sight
=================
*/
static qboolean PF_InPVS(const vec3_t& p1, const vec3_t& p2)
{
    return PF_inVIS(p1, p2, DVIS_PVS);
}

/*
=================
PF_inPHS

Also checks portalareas so that doors block sound
=================
*/
static qboolean PF_InPHS(const vec3_t& p1, const vec3_t& p2)
{
    return PF_inVIS(p1, p2, DVIS_PHS);
}

/*
==================
PF_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

If channel & 8, the sound will be sent to everyone, not just
things in the PHS.

FIXME: if entity isn't in PHS, they must be forced to be sent or
have the origin explicitly sent.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

Timeofs can range from 0.0 to 0.1 to cause sounds to be started
later in the frame than they normally would.

If origin is NULL, the origin is determined from the entity origin
or the midpoint of the entity box for bmodels.
==================
*/

#define CHECK_PARAMS \
    if (volume < 0 || volume > 1.0) \
        Com_Error(ErrorType::Drop, "%s: volume = %f", __func__, volume); \
    if (attenuation < 0 || attenuation > 4) \
        Com_Error(ErrorType::Drop, "%s: attenuation = %f", __func__, attenuation); \
    if (timeofs < 0 || timeofs > 0.255) \
        Com_Error(ErrorType::Drop, "%s: timeofs = %f", __func__, timeofs); \
    if (soundindex < 0 || soundindex >= MAX_SOUNDS) \
        Com_Error(ErrorType::Drop, "%s: soundindex = %d", __func__, soundindex);

static void PF_StartSound(Entity *edict, int channel,
                          int soundindex, float volume,
                          float attenuation, float timeofs)
{
    int         sendchan;
    int         flags;
    int         ent;
    vec3_t      origin;
    client_t    *client;
    static byte mask[VIS_MAX_BYTES];
    mleaf_t     *leaf;
    int         area;
    PlayerState      *ps;
    MessagePacket    *msg;

    if (!edict)
        return;

    CHECK_PARAMS

    ent = NUM_FOR_EDICT(edict);

    if (!edict->inUse) {
        Com_DPrintf("%s: entnum not in use: %d\n", __func__, ent);
        return;
    }

    sendchan = (ent << 3) | (channel & 7);

    // always send the entity number for channel overrides
    flags = SoundCommandBits::Entity;
    if (volume != DEFAULT_SOUND_PACKET_VOLUME)
        flags |= SoundCommandBits::Volume;
    if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
        flags |= SoundCommandBits::Attenuation;
    if (timeofs)
        flags |= SoundCommandBits::Offset;

    // if the sound doesn't attenuate,send it to everyone
    // (global radio chatter, voiceovers, etc)
    if (attenuation == Attenuation::None) {
        channel |= SoundChannel::IgnorePHS;
    }

    FOR_EACH_CLIENT(client) {
        // do not send sounds to connecting clients
        if (client->connectionState != ConnectionState::Spawned || client->download.bytes || client->nodata) {
            continue;
        }

        // PHS cull this sound
        if (!(channel & SoundChannel::IgnorePHS)) {
            // get client viewpos
            ps = &client->edict->client->playerState;
            // N&C: FF Precision.
            VectorAdd(ps->pmove.viewOffset, ps->pmove.origin, origin);
            //VectorMA(ps->viewOffset, 0.125f, ps->pmove.origin, origin);
            leaf = CM_PointLeaf(&sv.cm, origin);
            area = CM_LeafArea(leaf);
            if (!CM_AreasConnected(&sv.cm, area, edict->areaNumber)) {
                // doors can legally straddle two areas, so
                // we may need to check another one
                if (!edict->areaNumber2 || !CM_AreasConnected(&sv.cm, area, edict->areaNumber2)) {
                    continue;        // Blocked by a door
                }
            }
            BSP_ClusterVis(sv.cm.cache, mask, leaf->cluster, DVIS_PHS);
            if (!SV_EntityIsVisible(&sv.cm, edict, mask)) {
                continue; // not in PHS
            }
        }

        // use the entity origin unless it is a bmodel
        if (edict->solid == Solid::BSP) {
            VectorAverage(edict->mins, edict->maxs, origin);
            VectorAdd(edict->currentState.origin, origin, origin);
        } else {
            VectorCopy(edict->currentState.origin, origin);
        }

        // reliable sounds will always have position explicitly set,
        // as no one gurantees reliables to be delivered in time
        if (channel & SoundChannel::Reliable) {
            MSG_WriteUint8(ServerCommand::Sound); //MSG_WriteByte(ServerCommand::Sound);
            MSG_WriteUint8(flags | SoundCommandBits::Position);//MSG_WriteByte(flags | SoundCommandBits::Position);
            MSG_WriteUint8(soundindex); //MSG_WriteByte(soundindex);

            if (flags & SoundCommandBits::Volume)
                MSG_WriteUint8(volume * 255);//MSG_WriteByte(volume * 255);
            if (flags & SoundCommandBits::Attenuation)
                MSG_WriteUint8(attenuation * 64); //MSG_WriteByte(attenuation * 64);
            if (flags & SoundCommandBits::Offset)
                MSG_WriteUint8(timeofs * 1000); //MSG_WriteByte(timeofs * 1000);

            MSG_WriteUint16(sendchan);//MSG_WriteShort(sendchan);
            MSG_WriteVector3(origin);

            SV_ClientAddMessage(client, MSG_RELIABLE | MSG_CLEAR);
            continue;
        }

        if (LIST_EMPTY(&client->msg_free_list)) {
            Com_WPrintf("%s: %s: out of message slots\n",
                        __func__, client->name);
            continue;
        }

        // send origin for invisible entities
        if (edict->serverFlags & EntityServerFlags::NoClient) {
            flags |= SoundCommandBits::Position;
        }

        // default client doesn't know that bmodels have weird origins
        // MSG: !! Removed: PROTOCOL_VERSION_DEFAULT
        //if (edict->solid == Solid::BSP && client->protocolVersion == PROTOCOL_VERSION_DEFAULT) {
        //    flags |= SoundCommandBits::Position;
        //}

        msg = LIST_FIRST(MessagePacket, &client->msg_free_list, entry);

        msg->currentSize = 0;
        msg->packet.ps.flags = flags;
        msg->packet.ps.index = soundindex;
        msg->packet.ps.volume = volume * 255;
        msg->packet.ps.attenuation = attenuation * 64;
        msg->packet.ps.timeofs = timeofs * 1000;
        msg->packet.ps.sendchan = sendchan;
        // N&C: FF Precision.
        msg->packet.ps.pos = origin;
        //for (i = 0; i < 3; i++) {
        //    msg->pos[i] = origin[i] * 8;
        //}

        List_Remove(&msg->entry);
        List_Append(&client->msg_unreliable_list, &msg->entry);
        client->msg_unreliable_bytes += MAX_SOUND_PACKET;

        flags &= ~SoundCommandBits::Position;
    }
}

static void PF_PositionedSound(vec3_t origin, Entity *entity, int channel,
                               int soundindex, float volume,
                               float attenuation, float timeofs)
{
    int     sendchan;
    int     flags;
    int     ent;

    if (!origin)
        Com_Error(ErrorType::Drop, "%s: NULL origin", __func__);
    CHECK_PARAMS

    ent = NUM_FOR_EDICT(entity);
	
    sendchan = (ent << 3) | (channel & 7);

    // always send the entity number for channel overrides
    flags = SoundCommandBits::Entity | SoundCommandBits::Position;
    if (volume != DEFAULT_SOUND_PACKET_VOLUME)
        flags |= SoundCommandBits::Volume;
    if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
        flags |= SoundCommandBits::Attenuation;
    if (timeofs)
        flags |= SoundCommandBits::Offset;

    MSG_WriteUint8(ServerCommand::Sound);//MSG_WriteByte(ServerCommand::Sound);
    MSG_WriteUint8(flags); //MSG_WriteByte(flags);
    MSG_WriteUint8(soundindex); //MSG_WriteByte(soundindex);

    if (flags & SoundCommandBits::Volume)
        MSG_WriteUint8(volume * 255); //MSG_WriteByte(volume * 255);
    if (flags & SoundCommandBits::Attenuation)
        MSG_WriteUint8(attenuation * 64); //MSG_WriteByte(attenuation * 64);
    if (flags & SoundCommandBits::Offset)
        MSG_WriteUint8(timeofs * 1000);//MSG_WriteByte(timeofs * 1000);
    
    MSG_WriteUint16(sendchan);//MSG_WriteShort(sendchan);
    MSG_WriteVector3(origin);

    // if the sound doesn't attenuate,send it to everyone
    // (global radio chatter, voiceovers, etc)
    if (attenuation == Attenuation::None || (channel & SoundChannel::IgnorePHS)) {
        if (channel & SoundChannel::Reliable) {
            SV_Multicast(vec3_zero(), Multicast::All_R);
        } else {
            SV_Multicast(vec3_zero(), Multicast::All);
        }
    } else {
        if (channel & SoundChannel::Reliable) {
            SV_Multicast(origin, Multicast::PHS_R);
        } else {
            SV_Multicast(origin, Multicast::PHS);
        }
    }
}

static cvar_t *PF_cvar(const char *name, const char *value, int flags)
{
    if (flags & CVAR_EXTENDED_MASK) {
        Com_WPrintf("Game attemped to set extended flags on '%s', masked out.\n", name);
        flags &= ~CVAR_EXTENDED_MASK;
    }

    return Cvar_Get(name, value, flags | CVAR_GAME);
}

//
//===============
// PF_stuffcmd
// 
// Stuff Cmd implementation like other Quake engines have for the server module.
//===============
//
static void PF_stuffcmd(Entity* pent, const char* pszCommand) {
    MSG_WriteUint8(ServerCommand::StuffText);//MSG_WriteByte(ServerCommand::StuffText);
    MSG_WriteString(pszCommand);

    // Use the PF Unicast.
    PF_Unicast(pent, true);
}

static void PF_AddCommandString(const char *string)
{
    Cbuf_AddText(&cmd_buffer, string);
}

static void PF_SetAreaPortalState(int portalnum, qboolean open)
{
    if (!sv.cm.cache) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
    }
    CM_SetAreaPortalState(&sv.cm, portalnum, open);
}

static qboolean PF_AreasConnected(int area1, int area2)
{
    if (!sv.cm.cache) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
    }
    return CM_AreasConnected(&sv.cm, area1, area2);
}

static void *PF_TagMalloc(size_t size, unsigned tag)
{
    if (tag + TAG_MAX < tag) {
        Com_Error(ErrorType::Fatal, "%s: bad tag", __func__);
    }
    if (!size) {
        return NULL;
    }
    return memset(Z_TagMalloc(size, (memtag_t)(tag + TAG_MAX)), 0, size); // CPP: Cast
}

static void PF_FreeTags(unsigned tag)
{
    if (tag + TAG_MAX < tag) {
        Com_Error(ErrorType::Fatal, "%s: bad tag", __func__);
    }
    Z_FreeTags((memtag_t)(tag + TAG_MAX)); // CPP: Cast
}

static void PF_DebugGraph(float value, int color)
{
#if (defined _DEBUG) && USE_CLIENT
    SCR_DebugGraph(value, color);
#endif
}


//==============================================

static void *game_library;

/*
===============
SV_ShutdownGameProgs

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
void SV_ShutdownGameProgs(void)
{
    if (ge) {
        ge->Shutdown();
        ge = NULL;
    }
    if (game_library) {
        Sys_FreeLibrary(game_library);
        game_library = NULL;
    }
}

static void *_SV_LoadGameLibrary(const char *path)
{
    void *entry;

    entry = Sys_LoadLibrary(path, "GetServerGameAPI", &game_library);
    if (!entry)
        Com_EPrintf("Failed to load Server Game library: %s\n", Com_GetLastError());
    else
        Com_Printf("Loaded Server Game library from %s\n", path);

    return entry;
}

static void *SV_LoadGameLibrary(const char *game, const char *prefix)
{
    char path[MAX_OSPATH];
    size_t len;

    // WATISDEZE: Old game architecture dynamic link loading code.
    // len = Q_concat(path, sizeof(path), sys_libdir->string,
    //                PATH_SEP_STRING, game, PATH_SEP_STRING,
    //                prefix, "game" CPUSTRING LIBSUFFIX, NULL);
    // WATISDEZE: Updated to load the renamed "server game" module.
    len = Q_concat(path, sizeof(path), sys_libdir->string,
                   PATH_SEP_STRING, game, PATH_SEP_STRING,
                   prefix, "svgame" LIBSUFFIX, NULL);
    if (len >= sizeof(path)) {
        Com_EPrintf("Server Game library path length exceeded\n");
        return NULL;
    }

    if (os_access(path, F_OK)) {
        if (!*prefix)
            Com_Printf("Can't access %s: %s\n", path, strerror(errno));
        return NULL;
    }

    return _SV_LoadGameLibrary(path);
}

/*
===============
SV_InitGameProgs

Init the game subsystem for a new map
===============
*/
void SV_InitGameProgs(void)
{
    ServerGameImports   importAPI;
    ServerGameExports   *(*entry)(ServerGameImports *) = NULL;

    // unload anything we have now
    SV_ShutdownGameProgs();

	// Initialize server model system.
	SV_Model_Shutdown();
	SV_Model_Init();

    // for debugging or `proxy' mods
    if (sys_forcegamelib->string[0])
        entry = (ServerGameExports * (*)(ServerGameImports*))_SV_LoadGameLibrary(sys_forcegamelib->string); // CPP: DANGER: WARNING: Is this cast ok?

    // try game first (Mod)
    if (!entry && fs_game->string[0]) {
        entry = (ServerGameExports * (*)(ServerGameImports*))SV_LoadGameLibrary(fs_game->string, ""); // CPP: DANGER: WARNING: Is this cast ok?
    }

    // then try basepoly
    if (!entry) {
        entry = (ServerGameExports * (*)(ServerGameImports*))SV_LoadGameLibrary(BASEGAME, ""); // CPP: DANGER: WARNING: Is this cast ok?
    }

    // all paths failed
    if (!entry)
        Com_Error(ErrorType::Drop, "Failed to load Server Game library");

    // load a new game dll
    importAPI.apiversion = {
        SVGAME_API_VERSION_MAJOR,
        SVGAME_API_VERSION_MINOR,
        SVGAME_API_VERSION_POINT
    };
    importAPI.Multicast = SV_Multicast;
    importAPI.Unicast = PF_Unicast;
    importAPI.BPrintf = PF_bprintf;
    importAPI.DPrintf = PF_dprintf;
    importAPI.CPrintf = PF_cprintf;
    importAPI.CenterPrintf = PF_centerprintf;
    importAPI.Error = PF_error;

    importAPI.LinkEntity = PF_LinkEntity;
    importAPI.UnlinkEntity = PF_UnlinkEntity;
    importAPI.BoxEntities = SV_AreaEntities;
	importAPI.Clip = SV_Clip;
    importAPI.Trace = SV_Trace;
    importAPI.PointContents = SV_PointContents;

    importAPI.InPVS = PF_InPVS;
    importAPI.InPHS = PF_InPHS;

    importAPI.PrecacheModel = PF_PrecacheModel;
    importAPI.PrecacheSound = PF_PrecacheSound;
    importAPI.PrecacheImage = PF_PrecacheImage;
	
	importAPI.RegisterModel = PF_RegisterModel;
	importAPI.GetModelByHandle = PF_GetModelByHandle;
	importAPI.GetSkeletalModelDataByHandle = PF_GetSkeletalModelDataByHandle;
	importAPI.ES_CreateFromModel = ES_CreateFromModel;
	importAPI.ES_GetAnimationByName = PF_ES_GetAnimationByName;
	importAPI.ES_GetAnimationByIndex = PF_ES_GetAnimationByIndex;
	importAPI.ES_GetActionByName = PF_ES_GetActionByName;
	importAPI.ES_GetActionByIndex = PF_ES_GetActionByIndex;
	importAPI.ES_GetBlendAction = ES_GetBlendAction;
	importAPI.ES_GetBlendActionState = ES_GetBlendActionState;
	importAPI.SetModel = PF_setmodel;

    importAPI.configstring = PF_configstring;
    importAPI.Sound = PF_StartSound;
    importAPI.PositionedSound = PF_PositionedSound;

    importAPI.MSG_WriteInt8 = MSG_WriteInt8;//MSG_WriteChar;
    importAPI.MSG_WriteUint8 = MSG_WriteUint8;//MSG_WriteByte;
    importAPI.MSG_WriteInt16 = MSG_WriteInt16;
    importAPI.MSG_WriteUint16 = MSG_WriteUint16;
    importAPI.MSG_WriteInt32 = MSG_WriteInt32;
    importAPI.MSG_WriteInt64 = MSG_WriteInt64;
    importAPI.MSG_WriteIntBase128 = MSG_WriteIntBase128;
    importAPI.MSG_WriteUintBase128 = MSG_WriteUintBase128;
    importAPI.MSG_WriteString = MSG_WriteString;
    importAPI.MSG_WriteHalfFloat = MSG_WriteHalfFloat;
    importAPI.MSG_WriteFloat = MSG_WriteFloat;
    importAPI.MSG_WriteVector3 = MSG_WriteVector3;
    importAPI.MSG_WriteVector4 = MSG_WriteVector4;

    importAPI.TagMalloc = PF_TagMalloc;
    importAPI.TagFree = Z_Free;
    importAPI.FreeTags = PF_FreeTags;

    importAPI.cvar = PF_cvar;
    importAPI.cvar_set = Cvar_UserSet;
    importAPI.cvar_forceset = Cvar_Set;

    importAPI.argc = Cmd_Argc;
    importAPI.argv = Cmd_Argv;
    // original Cmd_Args() did actually return raw arguments
    importAPI.args = Cmd_RawArgs;
    importAPI.AddCommandString = PF_AddCommandString;

    // N&C: StuffCmd
    importAPI.StuffCmd = PF_stuffcmd;

    importAPI.DebugGraph = PF_DebugGraph;
    importAPI.SetAreaPortalState = PF_SetAreaPortalState;
    importAPI.AreasConnected = PF_AreasConnected;

    ge = entry(&importAPI);
    if (!ge) {
        Com_Error(ErrorType::Drop, "Server Game DLL returned NULL exports");
    }

    if (ge->apiversion.major != SVGAME_API_VERSION_MAJOR ||
        ge->apiversion.minor != SVGAME_API_VERSION_MINOR) {
        Com_Error(ErrorType::Drop, "Server Game DLL is version %i.%i.%i, expected %i.%i.%i",
            ge->apiversion.major, ge->apiversion.minor, ge->apiversion.point, SVGAME_API_VERSION_MAJOR, SVGAME_API_VERSION_MINOR, SVGAME_API_VERSION_POINT);
    }

    // initialize
    ge->Init();

    // sanitize entitySize
    if (ge->entitySize < sizeof(PODEntity) || ge->entitySize > SIZE_MAX / MAX_WIRED_POD_ENTITIES) {
        Com_Error(ErrorType::Drop, "Server Game DLL returned bad size of Entity");
    }

    // sanitize maxEntities
    //if (ge->entitySize <= sv_maxclients->integer || ge->entitySize > MAX_WIRED_POD_ENTITIES) {
    //    Com_Error(ErrorType::Drop, "Server Game DLL returned bad number of maxEntities %i   %i", ge->entitySize, sizeof(PODEntity));
    //}
}

