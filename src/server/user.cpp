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
// sv_user.c -- server code for moving users

#include "server.h"

/*
============================================================

USER PLAYER MOVEMENT 

============================================================
*/

//
//===============
// SV_PreRunCmd
// 
// Done before running a player command.Clears the touch array
//===============
//
byte* playertouch;
size_t playertouchmax;

void SV_PreRunCmd(void)
{
    size_t max = (ge->num_edicts + 512 + 7) & ~7;
    if (max > playertouchmax)
    {
        playertouchmax = max;
        Z_Free(playertouch);
        playertouch = (byte*)Z_Malloc((playertouchmax >> 3) + 1); // CPP: Cast
    }
    memset(playertouch, 0, playertouchmax >> 3);
}
void SV_RunCmdCleanup(void)
{
    Z_Free(playertouch);
    playertouch = NULL;
    playertouchmax = 0;
}

/*
============================================================

USER STRINGCMD EXECUTION

sv_client and sv_player will be valid.
============================================================
*/

/*
================
SV_CreateBaselines

Entity entityBaselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
static void create_baselines(void)
{
    int        i;
    entity_t    *ent;
    PackedEntity *base, **chunk;

    // clear entityBaselines from previous level
    for (i = 0; i < SV_BASELINES_CHUNKS; i++) {
        base = sv_client->entityBaselines[i];
        if (!base) {
            continue;
        }
        memset(base, 0, sizeof(*base) * SV_BASELINES_PER_CHUNK);
    }

    for (i = 1; i < sv_client->pool->num_edicts; i++) {
        ent = EDICT_POOL(sv_client, i);

        if (!ent->inUse) {
            continue;
        }

        if (!ES_INUSE(&ent->state)) {
            continue;
        }

        ent->state.number = i;

        chunk = &sv_client->entityBaselines[i >> SV_BASELINES_SHIFT];
        if (*chunk == NULL) {
            *chunk = (PackedEntity*)SV_Mallocz(sizeof(*base) * SV_BASELINES_PER_CHUNK); // CPP: Cast
        }

        base = *chunk + (i & SV_BASELINES_MASK);
        MSG_PackEntity(base, &ent->state, true); // MSG: !! Removed: Q2PRO_SHORTANGLES - Modify MSG_PackEntity to always use shortangles.

        base->solid = sv.entities[i].solid32;
    }
}

static void write_plain_configstrings(void)
{
    int     i;
    char    *string;
    size_t  length;

    // write a packet full of data
    string = sv_client->configstrings;
    for (i = 0; i < ConfigStrings::MaxConfigStrings; i++, string += MAX_QPATH) {
        if (!string[0]) {
            continue;
        }
        length = strlen(string);
        if (length > MAX_QPATH) {
            length = MAX_QPATH;
        }
        // check if this configstring will overflow
        if (msg_write.cursize + length + 64 > sv_client->netchan->maxpacketlen) {
            SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
        }

        MSG_WriteByte(svc_configstring);
        MSG_WriteShort(i);
        MSG_WriteData(string, length);
        MSG_WriteByte(0);
    }

    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
}

static void write_baseline(PackedEntity *base)
{
    EntityStateMessageFlags flags = (EntityStateMessageFlags)(sv_client->esFlags | MSG_ES_FORCE); // CPP: Cast

    MSG_WriteDeltaEntity(NULL, base, flags);
}

static void write_plain_baselines(void)
{
    int i, j;
    PackedEntity *base;

    // write a packet full of data
    for (i = 0; i < SV_BASELINES_CHUNKS; i++) {
        base = sv_client->entityBaselines[i];
        if (!base) {
            continue;
        }
        for (j = 0; j < SV_BASELINES_PER_CHUNK; j++) {
            if (base->number) {
                // check if this baseline will overflow
                if (msg_write.cursize + 64 > sv_client->netchan->maxpacketlen) {
                    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
                }

                MSG_WriteByte(svc_spawnbaseline);
                write_baseline(base);
            }
            base++;
        }
    }

    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
}

#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB

static void write_compressed_gamestate(void)
{
    sizebuf_t   *buf = &sv_client->netchan->message;
    PackedEntity  *base;
    int         i, j;
    size_t      length;
    uint8_t     *patch;
    char        *string;

    MSG_WriteByte(svc_gamestate);

    // write configstrings
    string = sv_client->configstrings;
    for (i = 0; i < ConfigStrings::MaxConfigStrings; i++, string += MAX_QPATH) {
        if (!string[0]) {
            continue;
        }
        length = strlen(string);
        if (length > MAX_QPATH) {
            length = MAX_QPATH;
        }

        //// MSGFRAG: !! Check if this configstring will overflow
        //if (msg_write.cursize + length + 64 > sv_client->netchan->maxpacketlen) {
        //    SV_ClientAddMessage(sv_client, 0);
        //}

        MSG_WriteShort(i);
        MSG_WriteData(string, length);
        MSG_WriteByte(0);
    }
    MSG_WriteShort(ConfigStrings::MaxConfigStrings);   // end of configstrings

    // write entityBaselines
    for (i = 0; i < SV_BASELINES_CHUNKS; i++) {
        base = sv_client->entityBaselines[i];
        if (!base) {
            continue;
        }
        //// MSGFRAG: !! Check if this baseline will overflow
        //if (msg_write.cursize + 64 > sv_client->netchan->maxpacketlen) {
        //    SV_ClientAddMessage(sv_client, 0);
        //}
        for (j = 0; j < SV_BASELINES_PER_CHUNK; j++) {
            if (base->number) {
                write_baseline(base);
            }
            base++;
        }
    }
    MSG_WriteShort(0);   // end of entityBaselines

    SZ_WriteByte(buf, svc_zpacket);
    patch = (uint8_t*)SZ_GetSpace(buf, 2); // CPP: Cast
    SZ_WriteShort(buf, msg_write.cursize);

    deflateReset(&svs.z);
    svs.z.next_in = msg_write.data;
    svs.z.avail_in = (uInt)msg_write.cursize;
    svs.z.next_out = buf->data + buf->cursize;
    svs.z.avail_out = (uInt)(buf->maxsize - buf->cursize);
    SZ_Clear(&msg_write);

    if (deflate(&svs.z, Z_FINISH) != Z_STREAM_END) {
        SV_DropClient(sv_client, "deflate() failed on gamestate");
        return;
    }

    SV_DPrintf(0, "%s: comp: %lu into %lu\n",
               sv_client->name, svs.z.total_in, svs.z.total_out);

    patch[0] = svs.z.total_out & 255;
    patch[1] = (svs.z.total_out >> 8) & 255;
    buf->cursize += svs.z.total_out;
    
    //// MSGFRAG: !! Add final send.
    //SV_ClientAddMessage(sv_client, 0);
}

static inline int z_flush(byte *buffer)
{
    int ret;

    ret = deflate(&svs.z, Z_FINISH);
    if (ret != Z_STREAM_END) {
        return ret;
    }

    SV_DPrintf(0, "%s: comp: %lu into %lu\n",
               sv_client->name, svs.z.total_in, svs.z.total_out);

    MSG_WriteByte(svc_zpacket);
    MSG_WriteShort(svs.z.total_out);
    MSG_WriteShort(svs.z.total_in);
    MSG_WriteData(buffer, svs.z.total_out);

    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);

    return ret;
}

static inline void z_reset(byte *buffer)
{
    deflateReset(&svs.z);
    svs.z.next_out = buffer;
    svs.z.avail_out = (uInt)(sv_client->netchan->maxpacketlen - 5);
}

static void write_compressed_configstrings(void)
{
    int     i;
    size_t  length;
    byte    buffer[MAX_PACKETLEN_WRITABLE];
    char    *string;

    z_reset(buffer);

    // write a packet full of data
    string = sv_client->configstrings;
    for (i = 0; i < ConfigStrings::MaxConfigStrings; i++, string += MAX_QPATH) {
        if (!string[0]) {
            continue;
        }
        length = strlen(string);
        if (length > MAX_QPATH) {
            length = MAX_QPATH;
        }

        // check if this configstring will overflow
        if (svs.z.avail_out < length + 32) {
            // then flush compressed data
            if (z_flush(buffer) != Z_STREAM_END) {
                goto fail;
            }
            z_reset(buffer);
        }

        MSG_WriteByte(svc_configstring);
        MSG_WriteShort(i);
        MSG_WriteData(string, length);
        MSG_WriteByte(0);

        svs.z.next_in = msg_write.data;
        svs.z.avail_in = (uInt)msg_write.cursize;
        SZ_Clear(&msg_write);

        if (deflate(&svs.z, Z_SYNC_FLUSH) != Z_OK) {
            goto fail;
        }
    }

    // finally flush all remaining compressed data
    if (z_flush(buffer) != Z_STREAM_END) {
fail:
        SV_DropClient(sv_client, "deflate() failed on configstrings");
    }
}

#endif // USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB

static void stuff_cmds(list_t *list)
{
    StuffTextCommand *stuff;

    LIST_FOR_EACH(StuffTextCommand, stuff, list, entry) {
        MSG_WriteByte(svc_stufftext);
        MSG_WriteData(stuff->string, stuff->len);
        MSG_WriteByte('\n');
        MSG_WriteByte(0);
        SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
    }
}

static void stuff_junk(void)
{
    static const char junkchars[] =
        "!~#``&'()*`+,-./~01~2`3`4~5`67`89:~<=`>?@~ab~c"
        "d`ef~j~k~lm`no~pq`rst`uv`w``x`yz[`\\]^_`|~";
    char junk[8][16];
    int i, j, k;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 15; j++) {
            k = rand_byte() % (sizeof(junkchars) - 1);
            junk[i][j] = junkchars[k];
        }
        junk[i][15] = 0;
    }

    strcpy(sv_client->reconnectKey, junk[2]);
    strcpy(sv_client->reconnectValue, junk[3]);

    SV_ClientCommand(sv_client, "set %s set\n", junk[0]);
    SV_ClientCommand(sv_client, "$%s %s connect\n", junk[0], junk[1]);
    if (rand_byte() & 1) {
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[2], junk[3]);
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[4],
                         sv_force_reconnect->string);
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[5], junk[6]);
    } else {
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[4],
                         sv_force_reconnect->string);
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[5], junk[6]);
        SV_ClientCommand(sv_client, "$%s %s %s\n", junk[0], junk[2], junk[3]);
    }
    SV_ClientCommand(sv_client, "$%s %s \"\"\n", junk[0], junk[0]);
    SV_ClientCommand(sv_client, "$%s $%s\n", junk[1], junk[4]);
}

/*
================
SV_New_f

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_New_f(void)
{
    int32_t oldConnectionState;

    Com_DPrintf("New() from %s\n", sv_client->name);

    oldConnectionState = sv_client->connectionState;
    if (sv_client->connectionState < ConnectionState::Connected) {
        Com_DPrintf("Going from ConnectionState::Assigned to ConnectionState::Connected for %s\n",
                    sv_client->name);
        sv_client->connectionState = ConnectionState::Connected;
        sv_client->lastMessage = svs.realtime; // don't timeout
        time(&sv_client->timeOfInitialConnect);
    } else if (sv_client->connectionState > ConnectionState::Connected) {
        Com_DPrintf("New not valid -- already primed\n");
        return;
    }

    // stuff some junk, drop them and expect them to be back soon
    if (sv_force_reconnect->string[0] && !sv_client->reconnectKey[0] &&
        !NET_IsLocalAddress(&sv_client->netchan->remoteAddress)) {
        stuff_junk();
        SV_DropClient(sv_client, NULL);
        return;
    }

    SV_ClientCommand(sv_client, "\n");

    //
    // serverdata needs to go over for all types of servers
    // to make sure the protocol is right, and to set the gamedir
    //

    // create entityBaselines for this client
    create_baselines();

    // send the serverdata
    MSG_WriteByte(svc_serverdata);
    MSG_WriteLong(sv_client->protocol);
    // WID: This value was unset here, so it defaulted to the int64 max or so.
    sv_client->spawncount = 0;
    MSG_WriteLong(sv_client->spawncount);
    MSG_WriteByte(0);   // no attract loop
    MSG_WriteString(sv_client->gamedir);
    if (sv.serverState == ServerState::Pic || sv.serverState == ServerState::Cinematic)
        MSG_WriteShort(-1);
    else
        MSG_WriteShort(sv_client->slot);
    MSG_WriteString(&sv_client->configstrings[ConfigStrings::Name * MAX_QPATH]);

    MSG_WriteShort(sv_client->version);
    MSG_WriteByte(sv.serverState);

    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);

    SV_ClientCommand(sv_client, "\n");

    // send version string request
    if (oldConnectionState == ConnectionState::Assigned) {
        SV_ClientCommand(sv_client, "cmd \177c version $version\n");
        stuff_cmds(&sv_cmdlist_connect);
    }

    // send reconnect var request
    if (sv_force_reconnect->string[0] && !sv_client->reconnected) {
        SV_ClientCommand(sv_client, "cmd \177c connect $%s\n",
                         sv_client->reconnectKey);
    }

    Com_DPrintf("Going from ConnectionState::Connected to ConnectionState::Primed for %s\n",
                sv_client->name);
    sv_client->connectionState = ConnectionState::Primed;

    memset(&sv_client->lastClientUserCommand, 0, sizeof(sv_client->lastClientUserCommand));

    if (sv.serverState == ServerState::Pic || sv.serverState == ServerState::Cinematic)
        return;

#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    if (sv_client->has_zlib) {
        //if (sv_client->netchan->type == NETCHAN_NEW) {
            write_compressed_gamestate();
        //} else {
        //    // FIXME: Z_SYNC_FLUSH is not efficient for entityBaselines
        //    write_compressed_configstrings();
        //    write_plain_baselines();
        //}
    } else
#endif //USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    {
        write_plain_configstrings();
        write_plain_baselines();
    }

    // send next command
    SV_ClientCommand(sv_client, "precache %i\n", sv_client->spawncount);
}

/*
==================
SV_Begin_f
==================
*/
void SV_Begin_f(void)
{
    Com_DPrintf("Begin() from %s\n", sv_client->name);

    // handle the case of a level changing while a client was connecting
    if (sv_client->connectionState < ConnectionState::Primed) {
        Com_DPrintf("Begin not valid -- not yet primed\n");
        SV_New_f();
        return;
    }
    if (sv_client->connectionState > ConnectionState::Primed) {
        Com_DPrintf("Begin not valid -- already spawned\n");
        return;
    }

    if (!sv_client->versionString) {
        SV_DropClient(sv_client, "!failed version probe");
        return;
    }

    if (sv_force_reconnect->string[0] && !sv_client->reconnected) {
        SV_DropClient(sv_client, "!failed to reconnect");
        return;
    }

    Com_DPrintf("Going from ConnectionState::Primed to ConnectionState::Spawned for %s\n",
                sv_client->name);
    sv_client->connectionState = ConnectionState::Spawned;
    sv_client->sendDelta = 0;
    sv_client->clientUserCommandMiliseconds = 1800;
    sv_client->suppressCount = 0;
    sv_client->http_download = false;

    stuff_cmds(&sv_cmdlist_begin);

    // call the game begin function
    ge->ClientBegin(sv_player);

	// Try binding light here with message
	
	// The server needs to complete the autosave after the client has connected.
	// See SV_Map (commands.c) for more information.
	if (sv_pending_autosave)
	{
		SV_AutoSaveEnd();
		sv_pending_autosave = false;
	}
}

//=============================================================================

void SV_CloseDownload(client_t *client)
{
    if (client->download.bytes) {
        Z_Free(client->download.bytes);
        client->download.bytes = NULL;
    }
    if (client->download.fileName) {
        Z_Free(client->download.fileName);
        client->download.fileName = NULL;
    }
    client->download.fileSize = 0;
    client->download.bytesSent = 0;
    client->download.command = 0;
    client->download.isPending = false;
}

/*
==================
SV_NextDownload_f
==================
*/
static void SV_NextDownload_f(void)
{
    if (!sv_client->download.bytes)
        return;

    sv_client->download.isPending = true;
}

/*
==================
SV_BeginDownload_f
==================
*/
static void SV_BeginDownload_f(void)
{
    char    name[MAX_QPATH];
    byte    *download;
    int     downloadcmd;
    ssize_t downloadsize, maxdownloadsize, result;
    int     offset = 0;
    cvar_t  *allow;
    size_t  len;
    qhandle_t f;

    len = Cmd_ArgvBuffer(1, name, sizeof(name));
    if (len >= MAX_QPATH) {
        goto fail1;
    }

    // hack for 'status' command
    if (!strcmp(name, "http")) {
        sv_client->http_download = true;
        return;
    }

    len = FS_NormalizePath(name, name);

    if (Cmd_Argc() > 2)
        offset = atoi(Cmd_Argv(2));     // downloaded offset

    // hacked by zoid to allow more conrol over download
    // first off, no .. or global allow check
    if (!allow_download->integer
        // check for empty paths
        || !len
        // check for illegal negative offsets
        || offset < 0
        // don't allow anything with .. path
        || strstr(name, "..")
        // leading dots, slashes, etc are no good
        || !Q_ispath(name[0])
        // trailing dots, slashes, etc are no good
        || !Q_ispath(name[len - 1])
        // MUST be in a subdirectory
        || !strchr(name, '/')) {
        Com_DPrintf("Refusing download of %s to %s\n", name, sv_client->name);
        goto fail1;
    }

    if (FS_pathcmpn(name, CONST_STR_LEN("players/")) == 0) {
        allow = allow_download_players;
    } else if (FS_pathcmpn(name, CONST_STR_LEN("models/")) == 0 ||
               FS_pathcmpn(name, CONST_STR_LEN("sprites/")) == 0) {
        allow = allow_download_models;
    } else if (FS_pathcmpn(name, CONST_STR_LEN("sound/")) == 0) {
        allow = allow_download_sounds;
    } else if (FS_pathcmpn(name, CONST_STR_LEN("maps/")) == 0) {
        allow = allow_download_maps;
    } else if (FS_pathcmpn(name, CONST_STR_LEN("textures/")) == 0 ||
               FS_pathcmpn(name, CONST_STR_LEN("env/")) == 0) {
        allow = allow_download_textures;
    } else if (FS_pathcmpn(name, CONST_STR_LEN("pics/")) == 0) {
        allow = allow_download_pics;
    } else {
        allow = allow_download_others;
    }

    if (!allow->integer) {
        Com_DPrintf("Refusing download of %s to %s\n", name, sv_client->name);
        goto fail1;
    }

    if (sv_client->download.bytes) {
        Com_DPrintf("Closing existing download for %s (should not happen)\n", sv_client->name);
        SV_CloseDownload(sv_client);
    }

    f = 0;
    downloadcmd = svc_download;

#if USE_ZLIB
    // prefer raw deflate stream from .pkz if supported
    if (sv_client->has_zlib && offset == 0) {
    //if (offset == 0) { //#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
        downloadsize = FS_FOpenFile(name, &f, FS_MODE_READ | FS_FLAG_DEFLATE);
        if (f) {
            Com_DPrintf("Serving compressed download to %s\n", sv_client->name);
            downloadcmd = svc_zdownload;
        }
    }
#endif

    if (!f) {
        downloadsize = FS_FOpenFile(name, &f, FS_MODE_READ);
        if (!f) {
            Com_DPrintf("Couldn't download %s to %s\n", name, sv_client->name);
            goto fail1;
        }
    }

    maxdownloadsize = MAX_LOADFILE;
#if 0
    if (sv_max_download_size->integer) {
        maxdownloadsize = Cvar_ClampInteger(sv_max_download_size, 1, MAX_LOADFILE);
    }
#endif

    if (downloadsize == 0) {
        Com_DPrintf("Refusing empty download of %s to %s\n", name, sv_client->name);
        goto fail2;
    }

    if (downloadsize > maxdownloadsize) {
        Com_DPrintf("Refusing oversize download of %s to %s\n", name, sv_client->name);
        goto fail2;
    }

    if (offset > downloadsize) {
        Com_DPrintf("Refusing download, %s has wrong version of %s (%d > %d)\n",
                    sv_client->name, name, offset, (int)downloadsize);
        SV_ClientPrintf(sv_client, PRINT_HIGH, "File size differs from server.\n"
                        "Please delete the corresponding .tmp file from your system.\n");
        goto fail2;
    }

    if (offset == downloadsize) {
        Com_DPrintf("Refusing download, %s already has %s (%d bytes)\n",
                    sv_client->name, name, offset);
        FS_FCloseFile(f);
        MSG_WriteByte(svc_download);
        MSG_WriteShort(0);
        MSG_WriteByte(100);
        SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
        return;
    }

    download = (byte*)SV_Malloc(downloadsize); // CPP: Cast
    result = FS_Read(download, downloadsize, f);
    if (result != downloadsize) {
        Com_DPrintf("Couldn't download %s to %s\n", name, sv_client->name);
        goto fail3;
    }

    FS_FCloseFile(f);

    sv_client->download.bytes = download;
    sv_client->download.fileSize = downloadsize;
    sv_client->download.bytesSent = offset;
    sv_client->download.fileName = SV_CopyString(name);
    sv_client->download.command = downloadcmd;
    sv_client->download.isPending = true;

    Com_DPrintf("Downloading %s to %s\n", name, sv_client->name);
    return;

fail3:
    Z_Free(download);
fail2:
    FS_FCloseFile(f);
fail1:
    MSG_WriteByte(svc_download);
    MSG_WriteShort(-1);
    MSG_WriteByte(0);
    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
}

static void SV_StopDownload_f(void)
{
    int percent;

    if (!sv_client->download.bytes)
        return;

    percent = sv_client->download.bytesSent * 100 / sv_client->download.fileSize;

    MSG_WriteByte(svc_download);
    MSG_WriteShort(-1);
    MSG_WriteByte(percent);
    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);

    Com_DPrintf("Download of %s to %s stopped by user request\n",
                sv_client->download.fileName, sv_client->name);
    SV_CloseDownload(sv_client);
}

//============================================================================

// special hack for end game screen in coop mode
static void SV_NextServer_f(void)
{
    char nextserver[MAX_QPATH];
    const char* v = Cvar_VariableString("nextserver"); // C++20: Added const.
    Q_strlcpy(nextserver, v, sizeof(nextserver));
    Cvar_Set("nextserver", "");
    
    if (sv.serverState != ServerState::Pic && sv.serverState != ServerState::Cinematic)
        return;     // can't nextserver while playing a normal game

    if (Cvar_VariableInteger("deathmatch"))
        return;

    sv.name[0] = 0; // make sure another doesn't sneak in

    if (!nextserver[0])
    {
        if (Cvar_VariableInteger("coop"))
            Cbuf_AddText(&cmd_buffer, "gamemap \"*nacstart\"\n");
        else
            Cbuf_AddText(&cmd_buffer, "killserver\n");
    }
    else
    {
        Cbuf_AddText(&cmd_buffer, nextserver);
        Cbuf_AddText(&cmd_buffer, "\n");
    }
}

// the client is going to disconnect, so remove the connection immediately
static void SV_Disconnect_f(void)
{
    SV_DropClient(sv_client, "!?disconnected");
    SV_RemoveClient(sv_client);   // don't bother with zombie state
}

// dumps the serverinfo info string
static void SV_ShowServerInfo_f(void)
{
    char serverinfo[MAX_INFO_STRING];

    Cvar_BitInfo(serverinfo, CVAR_SERVERINFO);

    SV_ClientRedirect();
    Info_Print(serverinfo);
    Com_EndRedirect();
}

// dumps misc protocol info
static void SV_ShowMiscInfo_f(void)
{
    SV_ClientRedirect();
    SV_PrintMiscInfo();
    Com_EndRedirect();
}

static void SV_NoGameData_f(void)
{
    sv_client->nodata ^= 1;
}

static void SV_Lag_f(void)
{
    client_t *cl;

    if (Cmd_Argc() > 1) {
        SV_ClientRedirect();
        cl = SV_GetPlayer(Cmd_Argv(1), true);
        Com_EndRedirect();
        if (!cl) {
            return;
        }
    } else {
        cl = sv_client;
    }

    SV_ClientPrintf(sv_client, PRINT_HIGH,
                    "Lag stats for:       %s\n"
                    "RTT (min/avg/max):   %d/%d/%d ms\n"
                    "Server to client PL: %.2f%% (approx)\n"
                    "Client to server PL: %.2f%%\n",
                    cl->name, cl->pingMinimum, AVG_PING(cl), cl->pingMaximum,
                    PL_S2C(cl), PL_C2S(cl));
}

#if USE_PACKETDUP
static void SV_PacketdupHack_f(void)
{
    int numdups = sv_client->numpackets - 1;

    if (Cmd_Argc() > 1) {
        numdups = atoi(Cmd_Argv(1));
        if (numdups < 0 || numdups > sv_packetdup_hack->integer) {
            SV_ClientPrintf(sv_client, PRINT_HIGH,
                            "Packetdup of %d is not allowed on this server.\n", numdups);
            return;
        }

        sv_client->numpackets = numdups + 1;
    }

    SV_ClientPrintf(sv_client, PRINT_HIGH,
                    "Server is sending %d duplicate packet%s to you.\n",
                    numdups, numdups == 1 ? "" : "s");
}
#endif

static void SV_CvarResult_f(void)
{
    const char *c, *v;

    c = Cmd_Argv(1); // C++20: Added a cast.
    if (!strcmp(c, "version")) {
        if (!sv_client->versionString) {
            v = (char*)Cmd_RawArgsFrom(2); // C++20: Added a cast.
            if (COM_DEDICATED) {
                Com_Printf("%s[%s]: %s\n", sv_client->name,
                           NET_AdrToString(&sv_client->netchan->remoteAddress), v);
            }
            sv_client->versionString = SV_CopyString(v);
        }
    } else if (!strcmp(c, "connect")) {
        if (sv_client->reconnectKey[0]) {
            if (!strcmp(Cmd_Argv(2), sv_client->reconnectValue)) {
                sv_client->reconnected = true;
            }
        }
    } else if (!strcmp(c, "console")) {
        if (sv_client->consoleQueries > 0) {
            Com_Printf("%s[%s]: \"%s\" is \"%s\"\n", sv_client->name,
                       NET_AdrToString(&sv_client->netchan->remoteAddress),
                       Cmd_Argv(2), Cmd_RawArgsFrom(3));
            sv_client->consoleQueries--;
        }
    }
}

static const ucmd_t ucmds[] = {
    // auto issued
    { "new", SV_New_f },
    { "begin", SV_Begin_f },
    { "entityBaselines", NULL },
    { "configstrings", NULL },
    { "nextserver", SV_NextServer_f },
    { "disconnect", SV_Disconnect_f },

    // issued by hand at client consoles
    { "info", SV_ShowServerInfo_f },
    { "sinfo", SV_ShowMiscInfo_f },

    { "download", SV_BeginDownload_f },
    { "nextdl", SV_NextDownload_f },
    { "stopdl", SV_StopDownload_f },

    { "\177c", SV_CvarResult_f },
    { "nogamedata", SV_NoGameData_f },
    { "lag", SV_Lag_f },
#if USE_PACKETDUP
    { "packetdup", SV_PacketdupHack_f },
#endif

    { NULL, NULL }
};

static void handle_filtercmd(FilterCommand *filter)
{
    size_t len;

    switch (filter->action) {
    case FA_PRINT:
        MSG_WriteByte(svc_print);
        MSG_WriteByte(PRINT_HIGH);
        break;
    case FA_STUFF:
        MSG_WriteByte(svc_stufftext);
        break;
    case FA_KICK:
        SV_DropClient(sv_client, filter->comment[0] ?
                      filter->comment : "issued banned command");
        // fall through
    default:
        return;
    }

    len = strlen(filter->comment);
    MSG_WriteData(filter->comment, len);
    MSG_WriteByte('\n');
    MSG_WriteByte(0);

    SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
}

/*
==================
SV_ExecuteUserCommand
==================
*/
static void SV_ExecuteUserCommand(const char *s)
{
    const ucmd_t *u;
    FilterCommand *filter;
    const char *c;

    Cmd_TokenizeString(s, false);
    sv_player = sv_client->edict;

    c = Cmd_Argv(0); // C++20: Added a cast.
    if (!c[0]) {
        return;
    }

    if ((u = Com_Find(ucmds, c)) != NULL) {
        if (u->func) {
            u->func();
        }
        return;
    }

    if (sv.serverState == ServerState::Pic || sv.serverState == ServerState::Cinematic) {
        return;
    }

    if (sv_client->connectionState != ConnectionState::Spawned && !sv_allow_unconnected_cmds->integer) {
        return;
    }

    LIST_FOR_EACH(FilterCommand, filter, &sv_filterlist, entry) {
        if (!Q_stricmp(filter->string, c)) {
            handle_filtercmd(filter);
            return;
        }
    }

    if (!strcmp(c, "say") || !strcmp(c, "say_team")) {
        // don't timeout. only chat commands count as activity.
        sv_client->lastActivity = svs.realtime;
    }

    ge->ClientCommand(sv_player);
}

/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

static qboolean    moveIssued;
static int         stringCmdCount;
static int         userinfoUpdateCount;

/*
==================
SV_ClientThink
==================
*/
static inline void SV_ClientThink(ClientUserCommand *cmd)
{
    ClientUserCommand *old = &sv_client->lastClientUserCommand;

    sv_client->clientUserCommandMiliseconds -= cmd->moveCommand.msec;
    sv_client->numberOfMoves++;

    if (sv_client->clientUserCommandMiliseconds < 0 && sv_enforcetime->integer) {
        Com_DPrintf("commandMsec underflow from %s: %d\n",
                    sv_client->name, sv_client->clientUserCommandMiliseconds);
        return;
    }

    if (cmd->moveCommand.buttons != old->moveCommand.buttons
        || cmd->moveCommand.forwardMove != old->moveCommand.forwardMove
        || cmd->moveCommand.rightMove != old->moveCommand.rightMove
        || cmd->moveCommand.upMove != old->moveCommand.upMove) {
        // don't timeout
        sv_client->lastActivity = svs.realtime;
    }

    ge->ClientThink(sv_player, cmd);
}

static void SV_SetLastFrame(int lastFrame)
{
    ClientFrame *frame;

    if (lastFrame > 0) {
        if (lastFrame >= sv_client->frameNumber)
            return; // ignore invalid acks

        if (lastFrame <= sv_client->lastFrame)
            return; // ignore duplicate acks

        if (sv_client->frameNumber - lastFrame <= UPDATE_BACKUP) {
            frame = &sv_client->frames[lastFrame & UPDATE_MASK];

            if (frame->number == lastFrame) {
                // save time for ping calc
                if (frame->sentTime <= com_eventTime)
                    frame->latency = com_eventTime - frame->sentTime;
            }
        }

        // count valid ack
        sv_client->framesAcknowledged++;
    }

    sv_client->lastFrame = lastFrame;
}

/*
==================
SV_ExecuteMove
==================
*/
static void SV_ExecuteMove(void)
{
    ClientUserCommand   oldest, oldcmd, newcmd;
    int         lastFrame;
    int         net_drop;

    if (moveIssued) {
        SV_DropClient(sv_client, "multiple clc_move commands in packet");
        return;     // someone is trying to cheat...
    }

    moveIssued = true;

    lastFrame = MSG_ReadLong();

    MSG_ReadDeltaUsercmd(NULL, &oldest);
    MSG_ReadDeltaUsercmd(&oldest, &oldcmd);
    MSG_ReadDeltaUsercmd(&oldcmd, &newcmd);

    if (sv_client->connectionState != ConnectionState::Spawned) {
        SV_SetLastFrame(-1);
        return;
    }

    SV_SetLastFrame(lastFrame);
    
    // Determine drop rate, on whether we should be predicting or not.
    net_drop = sv_client->netchan->dropped;
    if (net_drop > 2) {
        sv_client->frameFlags |= FF_CLIENTPRED;
    }

    if (net_drop < 20) {
        // Run last client user command multiple times if no backups are available
        while (net_drop > 2) {
            SV_ClientThink(&sv_client->lastClientUserCommand);
            net_drop--;
        }

        // Run backup client user commands.
        if (net_drop > 1)
            SV_ClientThink(&oldest);
        if (net_drop > 0)
            SV_ClientThink(&oldcmd);
    }

    // Run new cmd
    SV_ClientThink(&newcmd);

    // Store it.
    sv_client->lastClientUserCommand = newcmd;
}

/*
=================
SV_UpdateUserinfo

Ensures that userinfo is valid and name is properly set.
=================
*/
static void SV_UpdateUserinfo(void)
{
    char *s;

    if (!sv_client->userinfo[0]) {
        SV_DropClient(sv_client, "empty userinfo");
        return;
    }

    if (!Info_Validate(sv_client->userinfo)) {
        SV_DropClient(sv_client, "malformed userinfo");
        return;
    }

    // validate name
    s = Info_ValueForKey(sv_client->userinfo, "name");
    s[MAX_CLIENT_NAME - 1] = 0;
    if (COM_IsWhite(s) || (sv_client->name[0] && strcmp(sv_client->name, s) &&
                           SV_RateLimited(&sv_client->ratelimitNameChange))) {
        if (!sv_client->name[0]) {
            SV_DropClient(sv_client, "malformed name");
            return;
        }
        if (!Info_SetValueForKey(sv_client->userinfo, "name", sv_client->name)) {
            SV_DropClient(sv_client, "oversize userinfo");
            return;
        }
        if (COM_IsWhite(s))
            SV_ClientPrintf(sv_client, PRINT_HIGH, "You can't have an empty name.\n");
        else
            SV_ClientPrintf(sv_client, PRINT_HIGH, "You can't change your name too often.\n");
        SV_ClientCommand(sv_client, "set name \"%s\"\n", sv_client->name);
    }

    SV_UserinfoChanged(sv_client);
}

static void SV_ParseFullUserinfo(void)
{
    size_t len;

    // malicious users may try sending too many userinfo updates
    if (userinfoUpdateCount >= MAX_PACKET_USERINFOS) {
        Com_DPrintf("Too many userinfos from %s\n", sv_client->name);
        MSG_ReadString(NULL, 0);
        return;
    }

    len = MSG_ReadString(sv_client->userinfo, sizeof(sv_client->userinfo));
    if (len >= sizeof(sv_client->userinfo)) {
        SV_DropClient(sv_client, "oversize userinfo");
        return;
    }

    Com_DDPrintf("%s(%s): %s [%d]\n", __func__,
                 sv_client->name, sv_client->userinfo, userinfoUpdateCount);

    SV_UpdateUserinfo();
    userinfoUpdateCount++;
}

static void SV_ParseDeltaUserinfo(void)
{
    char key[MAX_INFO_KEY], value[MAX_INFO_VALUE];
    size_t len;

    // malicious users may try sending too many userinfo updates
    if (userinfoUpdateCount >= MAX_PACKET_USERINFOS) {
        Com_DPrintf("Too many userinfos from %s\n", sv_client->name);
        MSG_ReadString(NULL, 0);
        MSG_ReadString(NULL, 0);
        return;
    }

    // optimize by combining multiple delta updates into one (hack)
    while (1) {
        len = MSG_ReadString(key, sizeof(key));
        if (len >= sizeof(key)) {
            SV_DropClient(sv_client, "oversize delta key");
            return;
        }

        len = MSG_ReadString(value, sizeof(value));
        if (len >= sizeof(value)) {
            SV_DropClient(sv_client, "oversize delta value");
            return;
        }

        if (userinfoUpdateCount < MAX_PACKET_USERINFOS) {
            if (!Info_SetValueForKey(sv_client->userinfo, key, value)) {
                SV_DropClient(sv_client, "malformed userinfo");
                return;
            }

            Com_DDPrintf("%s(%s): %s %s [%d]\n", __func__,
                         sv_client->name, key, value, userinfoUpdateCount);

            userinfoUpdateCount++;
        } else {
            Com_DPrintf("Too many userinfos from %s\n", sv_client->name);
        }

        if (msg_read.readcount >= msg_read.cursize)
            break; // end of message

        if (msg_read.data[msg_read.readcount] != clc_userinfo_delta)
            break; // not delta userinfo

        msg_read.readcount++;
    }

    SV_UpdateUserinfo();
}

static void SV_ParseClientCommand(void)
{
    char buffer[MAX_STRING_CHARS];
    size_t len;

    len = MSG_ReadString(buffer, sizeof(buffer));
    if (len >= sizeof(buffer)) {
        SV_DropClient(sv_client, "oversize stringcmd");
        return;
    }

    // malicious users may try using too many string commands
    if (stringCmdCount >= MAX_PACKET_STRINGCMDS) {
        Com_DPrintf("Too many stringcmds from %s\n", sv_client->name);
        return;
    }

    Com_DDPrintf("%s(%s): %s\n", __func__, sv_client->name, buffer);

    SV_ExecuteUserCommand(buffer);
    stringCmdCount++;
}

/*
===================
SV_ExecuteClientMessage

The current net_message is parsed for the given client
===================
*/
void SV_ExecuteClientMessage(client_t *client)
{
    int c;

    sv_client = client;
    sv_player = sv_client->edict;

    // only allow one move command
    moveIssued = false;
    stringCmdCount = 0;
    userinfoUpdateCount = 0;

    while (1) {
        if (msg_read.readcount > msg_read.cursize) {
            SV_DropClient(client, "read past end of message");
            break;
        }

        c = MSG_ReadByte();
        if (c == -1)
            break;

        switch (c & SVCMD_MASK) {
        default:
        case clc_nop:
            break;

        case clc_userinfo:
            SV_ParseFullUserinfo();
            break;

        case clc_move:
            SV_ExecuteMove();
            break;

        case clc_stringcmd:
            SV_ParseClientCommand();
            break;

        case clc_userinfo_delta:
            SV_ParseDeltaUserinfo();
            break;
        }

        if (client->connectionState <= ConnectionState::Zombie)
            break;    // disconnect command
    }

    sv_client = NULL;
    sv_player = NULL;
}

