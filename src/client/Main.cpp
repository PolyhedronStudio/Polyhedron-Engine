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
// cl_main.c  -- client main loop
#include "RmlUI/RmlUI.h"

#include "Client.h"
#include "Client/UI/UI.h"
#include "Client/Sound/Vorbis.h"
#include "Client/GameModule.h"

cvar_t  *rcon_address;

cvar_t  *cl_noskins;
cvar_t  *cl_footsteps;
cvar_t  *cl_jumpsound;
cvar_t  *cl_timeout;
cvar_t  *cl_predict;
cvar_t  *cl_gun;
cvar_t* cl_warn_on_fps_rounding;
cvar_t  *cl_maxfps;
cvar_t  *cl_async;
cvar_t  *r_maxfps;
cvar_t  *cl_autopause;

cvar_t  *cl_kickangles;
cvar_t  *cl_rollhack;
cvar_t  *cl_noglow;
cvar_t  *cl_nolerp;

#ifdef _DEBUG
cvar_t  *cl_shownet;
cvar_t  *cl_showmiss;
cvar_t  *cl_showclamp;
#endif

cvar_t  *cl_player_model;
cvar_t  *cl_thirdperson_angle;
cvar_t  *cl_thirdperson_range;

//cvar_t  *cl_disable_particles;
//cvar_t  *cl_disable_explosions;
//cvar_t  *cl_chat_notify;
//cvar_t  *cl_chat_sound;
//cvar_t  *cl_chat_filter;
cvar_t  *cl_explosion_sprites;
cvar_t  *cl_explosion_frametime;

cvar_t  *cl_disconnectcmd;
cvar_t  *cl_changemapcmd;
cvar_t  *cl_beginmapcmd;

cvar_t  *cl_protocol;

cvar_t  *cl_vwep;

cvar_t  *cl_cinematics;

//
// userinfo
//
cvar_t  *info_password      = nullptr;
cvar_t  *info_spectator     = nullptr;
cvar_t  *info_name          = nullptr;
cvar_t  *info_skin          = nullptr;
cvar_t  *info_rate          = nullptr;
cvar_t  *info_fov           = nullptr;
cvar_t  *info_msg           = nullptr;
cvar_t  *info_hand          = nullptr;
cvar_t  *info_uf            = nullptr;
cvar_t  *info_in_bspmenu    = nullptr;

// N&C: Developer utilities.
cvar_t* dev_map;
#if USE_REF == REF_GL
extern cvar_t *gl_modulate_world;
extern cvar_t *gl_modulate_entities;
extern cvar_t *gl_brightness;
#endif


extern cvar_t *cl_renderdemo;
extern cvar_t *cl_renderdemo_fps;

ClientStatic cls;
ClientState  cl;

// N&C: Client shared structure. used to access entities etc in CG Module.
ClientShared cs;

// used for executing stringcmds
cmdbuf_t    cl_cmdbuf;
char        cl_cmdbuf_text[MAX_STRING_CHARS];

//======================================================================
// Opens the mainmenu only if a map has serverinfo var "in_bspmenu" set to 1
qboolean CL_InBSPMenu() {
    char serverinfo[MAX_INFO_STRING];
    Cvar_BitInfo(serverinfo, CVAR_SERVERINFO);

    int in_bspmenu = atoi(Info_ValueForKey(serverinfo, "in_bspmenu"));

    if (in_bspmenu == 1 && cls.connectionState >= ClientConnectionState::Active) {
        return true;
    } else {
        return false;
    }
}

void CL_OpenBSPMenu() {
    if (CL_InBSPMenu()) {
        UI_OpenMenu(UIMENU_GAME);
    }
}

void CL_CloseBSPMenu() {
    if (!CL_InBSPMenu()) {
        UI_OpenMenu(UIMENU_NONE);
    }
}

void CL_LoadBSPMenuMap(qboolean force = false) {
    // Open mainmenu map.
    if (force == false) {
        Cmd_ExecuteString(&cl_cmdbuf, "map mainmenu");
    } else {
        Cmd_ExecuteString(&cl_cmdbuf, "map mainmenu force");
    }
}

//======================================================================

enum RequestType {
    REQ_FREE,
    REQ_STATUS_CL,
    REQ_STATUS_UI,
    REQ_INFO,
    REQ_RCON
};

struct Request {
    RequestType type;
    NetAdr adr;
    unsigned time;
};

#define MAX_REQUESTS    64
#define REQUEST_MASK    (MAX_REQUESTS - 1)

static Request    clientRequests[MAX_REQUESTS];
static unsigned     nextRequest;

static Request *CL_AddRequest(const NetAdr *adr, RequestType type)
{
    Request *r;

    r = &clientRequests[nextRequest++ & REQUEST_MASK];
    r->adr = *adr;
    r->type = type;
    r->time = cls.realtime;

    return r;
}

static Request *CL_FindRequest(void)
{
    Request *r;
    int i, count;

    count = MAX_REQUESTS;
    if (count > nextRequest)
        count = nextRequest;

    // find the most recent request sent to this address
    for (i = 0; i < count; i++) {
        r = &clientRequests[(nextRequest - i - 1) & REQUEST_MASK];
        if (!r->type) {
            continue;
        }
        if (r->adr.type == NA_BROADCAST) {
            if (cls.realtime - r->time > 3000) {
                continue;
            }
            if (!NET_IsLanAddress(&net_from)) {
                continue;
            }
        } else {
            if (cls.realtime - r->time > 6000) {
                break;
            }
            if (!NET_IsEqualBaseAdr(&net_from, &r->adr)) {
                continue;
            }
        }

        return r;
    }

    return NULL;
}

//======================================================================

/*
===================
CL_ClientCommand
===================
*/
void CL_ClientCommand(const char *string)
{
    if (!cls.netChannel) {
        return;
    }

    Com_DDPrintf("%s: %s\n", __func__, string);

    MSG_WriteByte(clc_stringcmd);
    MSG_WriteString(string);
    MSG_FlushTo(&cls.netChannel->message);
}

/*
===================
CL_ForwardToServer

adds the current command line as a clc_stringcmd to the client message.
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
qboolean CL_ForwardToServer(void)
{
    const char    *cmd; // C++20: STRING: Added const to char*

    cmd = Cmd_Argv(0);
    if (cls.connectionState != ClientConnectionState::Active || *cmd == '-' || *cmd == '+') {
        return false;
    }

    CL_ClientCommand(Cmd_RawArgsFrom(0));
    return true;
}

/*
==================
CL_ForwardToServer_f
==================
*/
static void CL_ForwardToServer_f(void)
{
    if (cls.connectionState < ClientConnectionState::Connected) {
        Com_Printf("Can't \"%s\", not connected\n", Cmd_Argv(0));
        return;
    }

    if (cls.demo.playback) {
        return;
    }

    // don't forward the first argument
    if (Cmd_Argc() > 1) {
        CL_ClientCommand(Cmd_RawArgs());
    }
}

/*
==================
CL_Pause_f
==================
*/
void CL_Pause_f(void)
{
    // activate manual pause
    if (cl_paused->integer == 2) {
        Cvar_Set("cl_paused", "0");
    } else {
        Cvar_Set("cl_paused", "2");
    }

	OGG_TogglePlayback();

    CL_CheckForPause();
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend(void)
{
    char tail[MAX_QPATH];
    char userinfo[MAX_INFO_STRING];
    int maxmsglen;

    if (cls.demo.playback) {
        return;
    }

    // if the local server is running and we aren't
    // then connect
    if (cls.connectionState < ClientConnectionState::Connecting && sv_running->integer > ServerState::Loading) {
        strcpy(cls.servername, "localhost");
        cls.serverAddress.type = NA_LOOPBACK;
        cls.serverProtocol = cl_protocol->integer;
        if (cls.serverProtocol < PROTOCOL_VERSION_DEFAULT ||
            cls.serverProtocol > PROTOCOL_VERSION_POLYHEDRON) {
            cls.serverProtocol = PROTOCOL_VERSION_POLYHEDRON;
        }

        // we don't need a challenge on the localhost
        cls.connectionState = ClientConnectionState::Connecting;
        cls.timeOfInitialConnect -= CONNECT_FAST;
        cls.connect_count = 0;

        cls.passive = false;

        Con_Popup(true);
    }

    // resend if we haven't gotten a reply yet
    if (cls.connectionState != ClientConnectionState::Connecting && cls.connectionState != ClientConnectionState::Challenging) {
        return;
    }

    if (cls.realtime - cls.timeOfInitialConnect < CONNECT_DELAY) {
        return;
    }

    cls.timeOfInitialConnect = cls.realtime;    // for retransmit requests
    cls.connect_count++;

    if (cls.connectionState == ClientConnectionState::Challenging) {
        Com_Printf("Requesting challenge... %i\n", cls.connect_count);
        OOB_PRINT(NS_CLIENT, &cls.serverAddress, "getchallenge\n");
        return;
    }

    //
    // We have gotten a challenge from the server, so try and connect.
    //
    Com_Printf("Requesting connection... %i\n", cls.connect_count);

    cls.userinfo_modified = 0;

    // use maximum allowed msglen for loopback
    maxmsglen = net_maxmsglen->integer;
    if (NET_IsLocalAddress(&cls.serverAddress)) {
        maxmsglen = MAX_PACKETLEN_WRITABLE;
    }

    // add protocol dependent stuff
    Q_snprintf(tail, sizeof(tail), " %d %d %d %d",
                maxmsglen, net_chantype->integer, USE_ZLIB_PACKET_COMPRESSION, // MSG: !! Changed from USE_ZLIB,
                PROTOCOL_VERSION_POLYHEDRON_CURRENT);
    cls.quakePort = net_qport->integer & 0xff;

    Cvar_BitInfo(userinfo, CVAR_USERINFO);
    Netchan_OutOfBand(NS_CLIENT, &cls.serverAddress,
                      "connect %i %i %i \"%s\"%s\n", cls.serverProtocol, cls.quakePort,
                      cls.challenge, userinfo, tail);
}

static void CL_RecentIP_g(genctx_t *ctx)
{
    NetAdr *a;
    int i, j;

    j = cls.recent_head - RECENT_ADDR;
    if (j < 0) {
        j = 0;
    }
    for (i = cls.recent_head - 1; i >= j; i--) {
        a = &cls.recent_addr[i & RECENT_MASK];
        if (a->type) {
            Prompt_AddMatch(ctx, NET_AdrToString(a));
        }
    }
}

static void CL_Connect_c(genctx_t *ctx, int argnum)
{
    if (argnum == 1) {
        CL_RecentIP_g(ctx);
        Com_Address_g(ctx);
    } else if (argnum == 2) {
        if (!ctx->partial[0] || (ctx->partial[0] == '1' && ctx->partial[1] == '3')) {
            Prompt_AddMatch(ctx, "1337");
            Prompt_AddMatch(ctx, "1340");
            Prompt_AddMatch(ctx, "1341");
        }
    }
}

//-----------------------------------------------------------------------------------------------------
// WID: This function is here to not break shit instantly, and try and use ENet to do the OOB
// sending and reading of.
//
// After that, the challenging and/or get going into the game.
//-----------------------------------------------------------------------------------------------------
/*
================
CL_EConnect_f

================
*/
static void CL_EConnect_f(void) {
    const char* server;// , * p; // C++20: STRING: Added const to char*
    char* p;
    NetAdr    address;
    int protocol;
    int argc = Cmd_Argc();

    if (argc < 2) {
    usage:
        Com_Printf("Usage: %s <server> [34|35|36]\n", Cmd_Argv(0));
        return;
    }

    if (argc > 2) {
        protocol = atoi(Cmd_Argv(2));
        if (protocol < PROTOCOL_VERSION_DEFAULT ||
            protocol > PROTOCOL_VERSION_POLYHEDRON) {
            goto usage;
        }
    } else {
        protocol = cl_protocol->integer;
        if (!protocol) {
            protocol = PROTOCOL_VERSION_POLYHEDRON;
        }
    }

    server = Cmd_Argv(1);

    // support quake2://<address>[/] scheme
    if (!Q_strncasecmp(server, "quake2://", 9)) {
        server += 9;
        if ((p = (char*)strchr(server, '/')) != NULL) {
            *p = 0;
        }
    }



    //if (!NET_StringToAdr(server, &address, PORT_SERVER)) {
    //    Com_Printf("Bad server address\n");
    //    return;
    //}

    // copy early to avoid potential cmd_argv[1] clobbering
    Q_strlcpy(cls.servername, server, sizeof(cls.servername));

    // if running a local server, kill it and reissue
    SV_Shutdown("Server was killed.\n", ERR_DISCONNECT);

    NET_Config(NET_CLIENT);

    CL_Disconnect(ERR_RECONNECT);

    cls.serverAddress = address;
    cls.serverProtocol = protocol;
    cls.protocolVersion = 0;
    cls.passive = false;
    cls.connectionState = ClientConnectionState::Challenging;
    cls.timeOfInitialConnect -= CONNECT_FAST;
    cls.connect_count = 0;

    Con_Popup(true);

    CL_CheckForResend();

    Cvar_Set("timedemo", "0");
}

/*
================
CL_Connect_f

================
*/
static void CL_Connect_f(void)
{
    const char* server;// , * p; // C++20: STRING: Added const to char*
    char* p;
    NetAdr    address;
    int protocol;
    int argc = Cmd_Argc();

    if (argc < 2) {
usage:
        Com_Printf("Usage: %s <server> [1337|1340|1341]\n", Cmd_Argv(0));
        return;
    }

    if (argc > 2) {
        protocol = atoi(Cmd_Argv(2));
        if (protocol < PROTOCOL_VERSION_DEFAULT ||
            protocol > PROTOCOL_VERSION_POLYHEDRON) {
            goto usage;
        }
    } else {
        protocol = cl_protocol->integer;
        if (!protocol) {
            protocol = PROTOCOL_VERSION_POLYHEDRON;
        }
    }

    server = Cmd_Argv(1);

    // support quake2://<address>[/] scheme
    if (!Q_strncasecmp(server, "quake2://", 9)) {
        server += 9;
        if ((p = (char*)strchr(server, '/')) != NULL) {
            *p = 0;
        }
    }

    if (!NET_StringToAdr(server, &address, PORT_SERVER)) {
        Com_Printf("Bad server address\n");
        return;
    }

    // copy early to avoid potential cmd_argv[1] clobbering
    Q_strlcpy(cls.servername, server, sizeof(cls.servername));

    // if running a local server, kill it and reissue
    SV_Shutdown("Server was killed.\n", ERR_DISCONNECT);

    NET_Config(NET_CLIENT);

    CL_Disconnect(ERR_RECONNECT);

    cls.serverAddress = address;
    cls.serverProtocol = protocol;
    cls.protocolVersion = 0;
    cls.passive = false;
    cls.connectionState = ClientConnectionState::Challenging;
    cls.timeOfInitialConnect -= CONNECT_FAST;
    cls.connect_count = 0;

    Con_Popup(true);

    CL_CheckForResend();

    Cvar_Set("timedemo", "0");
}

static void CL_FollowIP_f(void)
{
    NetAdr *a;
    int i, j;

    if (Cmd_Argc() > 1) {
        // optional second argument references less recent address
        j = atoi(Cmd_Argv(1)) + 1;
        clamp(j, 1, RECENT_ADDR);
    } else {
        j = 1;
    }

    i = cls.recent_head - j;
    if (i < 0) {
        Com_Printf("No IP address to follow.\n");
        return;
    }

    a = &cls.recent_addr[i & RECENT_MASK];
    if (a->type) {
        const char *s = NET_AdrToString(a);
        Com_Printf("Following %s...\n", s);
        Cbuf_InsertText(cmd_current, va("connect %s\n", s));
    }
}

static void CL_PassiveConnect_f(void)
{
    NetAdr address;

    if (cls.passive) {
        cls.passive = false;
        Com_Printf("No longer listening for passive connections.\n");
        return;
    }

    // if running a local server, kill it and reissue
    SV_Shutdown("Server was killed.\n", ERR_DISCONNECT);

    NET_Config(NET_CLIENT);

    CL_Disconnect(ERR_RECONNECT);

    if (!NET_GetAddress(NS_CLIENT, &address)) {
        return;
    }

    cls.passive = true;
    Com_Printf("Listening for passive connections at %s.\n",
               NET_AdrToString(&address));
}

void CL_SendRcon(const NetAdr *adr, const char *pass, const char *cmd)
{
    NET_Config(NET_CLIENT);

    CL_AddRequest(adr, REQ_RCON);

    Netchan_OutOfBand(NS_CLIENT, adr, "rcon \"%s\" %s", pass, cmd);
}


/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
static void CL_Rcon_f(void)
{
    NetAdr    address;

    if (Cmd_Argc() < 2) {
        Com_Printf("Usage: %s <command>\n", Cmd_Argv(0));
        return;
    }

    if (!rcon_password->string[0]) {
        Com_Printf("You must set 'rcon_password' before "
                   "issuing an rcon command.\n");
        return;
    }

    if (!cls.netChannel) {
        if (!rcon_address->string[0]) {
            Com_Printf("You must either be connected, "
                       "or set the 'rcon_address' cvar "
                       "to issue rcon commands.\n");
            return;
        }
        if (!NET_StringToAdr(rcon_address->string, &address, PORT_SERVER)) {
            Com_Printf("Bad address: %s\n", rcon_address->string);
            return;
        }
    } else {
        address = cls.netChannel->remoteNetAddress;
    }

    CL_SendRcon(&address, rcon_password->string, Cmd_RawArgs());
}

static void CL_Rcon_c(genctx_t *ctx, int argnum)
{
    Com_Generic_c(ctx, argnum - 1);
}

//
//===============
// CL_GetConnectionState
// 
// Returns the current state of the client.
//===============
//
uint32_t CL_GetConnectionState (void) {
    return cls.connectionState;
}

//
//===============
// CL_SetConnectionState
// 
// Sets the current state of the client.
//===============
//
void CL_SetConnectionState (uint32_t connectionState) {
    cls.connectionState = connectionState;
}

//
//===============
// CL_SetLoadState
// 
// Sets the current load state of the client.
//===============
//
void CL_SetLoadState (LoadState state) {
    CL_LoadState(state);
}

/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState(void)
{
    // Stop all sounds.
    S_StopAllSounds();
 
    // WID: Inform the CG Module.
    CL_GM_ClientClearState();

    // Wipe the entire cl structure
    BSP_Free(cl.bsp);
    memset(&cl, 0, sizeof(cl));
    memset(&cs.entities, 0, sizeof(cs.entities));
    // C++ Style, no more memset. I suppose I prefer this, if you do not, ouche.
    //cl = {};
    /*for (uint32_t i = 0; i < sizeof(cs.entities); i++) {
        cs.entities[i] = {};
    }*/

    // In case we are more than connected, reset it to just connected.
    if (cls.connectionState > ClientConnectionState::Connected) {
        cls.connectionState = ClientConnectionState::Connected;
        CL_CheckForPause();
        CL_UpdateFrameTimes();
    }

    // Unprotect game cvar
    fs_game->flags &= ~CVAR_ROM;

#if USE_REF == REF_GL
    // Unprotect our custom modulate cvars
    if(gl_modulate_world) gl_modulate_world->flags &= ~CVAR_CHEAT;
    if(gl_modulate_entities) gl_modulate_entities->flags &= ~CVAR_CHEAT;
    if(gl_brightness) gl_brightness->flags &= ~CVAR_CHEAT;
#endif
}

/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect(ErrorType type)
{
    if (!cls.connectionState) {
        return;
    }

    //cvar_t *info_in_bspmenu = Info_SetValueForKey("in_bspmenu")
    SCR_EndLoadingPlaque(); // get rid of loading plaque

    // N&C: Call into the CG Module to inform that we're disconnected.
    CL_GM_ClientDisconnect();

    if (cls.connectionState > ClientConnectionState::Disconnected && !cls.demo.playback) {
        EXEC_TRIGGER(cl_disconnectcmd);
        //Cbuf_AddText(&cmd_buffer, "bspmainmenu");
    }

#if 0
    if (cls.ref_initialized) {
        R_CinematicSetPalette(NULL);
    }
#endif

    //cls.timeOfInitialConnect = 0;
    //cls.connect_count = 0;
    cls.passive = false;
#if USE_ICMP
    cls.errorReceived = false;
#endif

    if (cls.netChannel) {
        // send a disconnect message to the server
        MSG_WriteByte(clc_stringcmd);
        MSG_WriteData("disconnect", 11);

        Netchan_Transmit(cls.netChannel, msg_write.currentSize, msg_write.data, 3);

        SZ_Clear(&msg_write);

        Netchan_Close(cls.netChannel);
        cls.netChannel = NULL;
    }

    // stop playback and/or recording
    CL_CleanupDemos();

    // stop download
    CL_CleanupDownloads();

    CL_ClearState();

    cls.connectionState = ClientConnectionState::Disconnected;

    cl.snd_is_underwater = false; // OAL: Moved to client.

    cls.userinfo_modified = 0;

    if (type == ERR_DISCONNECT || type == ERR_DROP) {
        //UI_OpenMenu(UIMENU_DEFAULT);
        Cmd_ExecuteCommand(&cl_cmdbuf);

        // Return to mainmenu map.
        if (!CL_InBSPMenu()) {
            CL_LoadBSPMenuMap();
            CL_OpenBSPMenu();
        }
    } else {
        Cvar_SetEx("in_bspmenu", "0", FROM_CODE);
        CL_CloseBSPMenu();
    }

    CL_CheckForPause();

    CL_UpdateFrameTimes();
}

/*
================
CL_Disconnect_f
================
*/
static void CL_Disconnect_f(void)
{
    // No disconnecting from our bsp mainmenu.
    if (CL_InBSPMenu()) {
        return;
    }

    if (cls.connectionState > ClientConnectionState::Disconnected) {
        Com_Error(ERR_DISCONNECT, "Disconnected from server");
    }
}

static void CL_ServerStatus_c(genctx_t *ctx, int argnum)
{
    if (argnum == 1) {
        CL_RecentIP_g(ctx);
        Com_Address_g(ctx);
    }
}

/*
================
CL_ServerStatus_f
================
*/
static void CL_ServerStatus_f(void)
{
    const char        *s; // C++20: STRING: Added const to char*
    NetAdr    adr;
    neterr_t    ret;

    if (Cmd_Argc() < 2) {
        if (!cls.netChannel) {
            Com_Printf("Usage: %s [address]\n", Cmd_Argv(0));
            return;
        }
        adr = cls.netChannel->remoteNetAddress;
    } else {
        s = Cmd_Argv(1);
        if (!NET_StringToAdr(s, &adr, PORT_SERVER)) {
            Com_Printf("Bad address: %s\n", s);
            return;
        }
    }

    CL_AddRequest(&adr, REQ_STATUS_CL);

    NET_Config(NET_CLIENT);

    ret = OOB_PRINT(NS_CLIENT, &adr, "status");
    if (ret == NET_ERROR) {
        Com_Printf("%s to %s\n", NET_ErrorString(), NET_AdrToString(&adr));
    }
}

/*
====================
SortPlayers
====================
*/
static int SortPlayers(const void *v1, const void *v2)
{
    const playerStatus_t *p1 = (const playerStatus_t *)v1;
    const playerStatus_t *p2 = (const playerStatus_t *)v2;

    return p2->score - p1->score;
}

/*
====================
CL_ParseStatusResponse
====================
*/
static void CL_ParseStatusResponse(serverStatus_t *status, const char *string)
{
    playerStatus_t *player;
    const char *s;
    size_t infolen;

    // Parse '\n' terminated infostring
    s = Q_strchrnul(string, '\n');

    // Due to off-by-one error in the original version of Info_SetValueForKey,
    // some servers produce infostrings up to 512 characters long. work this
    // bug around by cutting off the last character(s).
    infolen = s - string;
    if (infolen >= MAX_INFO_STRING)
        infolen = MAX_INFO_STRING - 1;

    // Copy infostring off
    memcpy(status->infostring, string, infolen);
    status->infostring[infolen] = 0;

    if (!Info_Validate(status->infostring))
        strcpy(status->infostring, "\\hostname\\badinfo");

    // Parse optional player list
    status->numPlayers = 0;
    while (status->numPlayers < MAX_STATUS_PLAYERS) {
        player = &status->players[status->numPlayers];
        player->score = atoi(COM_Parse(&s));
        player->ping = atoi(COM_Parse(&s));
        Q_strlcpy(player->name, COM_Parse(&s), sizeof(player->name));
        if (!s)
            break;
        status->numPlayers++;
    }

    // Sort players by frags
    qsort(status->players, status->numPlayers,
          sizeof(status->players[0]), SortPlayers);
}

static void CL_DumpStatusResponse(const serverStatus_t *status)
{
    int i;

    Com_Printf("Status response from %s\n\n", NET_AdrToString(&net_from));

    Info_Print(status->infostring);

    Com_Printf("\nNum Score Ping Name\n");
    for (i = 0; i < status->numPlayers; i++) {
        Com_Printf("%3i %5i %4i %s\n", i + 1,
                   status->players[i].score,
                   status->players[i].ping,
                   status->players[i].name);
    }
}

/*
====================
CL_ParsePrintMessage
====================
*/
static void CL_ParsePrintMessage(void)
{
    char string[MAX_NET_STRING];
    serverStatus_t status;
    Request *r;

    MSG_ReadString(string, sizeof(string));

    r = CL_FindRequest();
    if (r) {
        switch (r->type) {
        case REQ_STATUS_CL:
            CL_ParseStatusResponse(&status, string);
            CL_DumpStatusResponse(&status);
            break;
#if USE_UI
        case REQ_STATUS_UI:
            CL_ParseStatusResponse(&status, string);
            UI_StatusEvent(&status);
            break;
#endif
        case REQ_RCON:
            Com_Printf("%s", string);
            return; // rcon may come in multiple packets

        default:
            return;
        }

        if (r->adr.type != NA_BROADCAST)
            r->type = REQ_FREE;
        return;
    }

    // Finally, check is this is response from the server we are connecting to
    // and if so, start channenge cycle again
    if ((cls.connectionState == ClientConnectionState::Challenging || cls.connectionState == ClientConnectionState::Connecting) &&
        NET_IsEqualBaseAdr(&net_from, &cls.serverAddress)) {
        Com_Printf("%s", string);
        cls.connectionState = ClientConnectionState::Challenging;
        //cls.connect_count = 0;
        return;
    }

    Com_DPrintf("%s: dropped unrequested packet\n", __func__);
}

/*
=================
CL_ParseInfoMessage

Handle a reply from a ping
=================
*/
static void CL_ParseInfoMessage(void)
{
    char string[MAX_QPATH];
    Request *r;

    r = CL_FindRequest();
    if (!r)
        return;
    if (r->type != REQ_INFO)
        return;

    MSG_ReadString(string, sizeof(string));
    Com_Printf("%s", string);
    if (r->adr.type != NA_BROADCAST)
        r->type = REQ_FREE;
}

/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
/*
void CL_Packet_f (void)
{
    char    send[2048];
    int     i, l;
    char    *in, *out;
    NetAdr    adr;

    if (Cmd_Argc() != 3)
    {
        Com_Printf ("packet <destination> <contents>\n");
        return;
    }

    if (!NET_StringToAdr (Cmd_Argv(1), &adr))
    {
        Com_Printf ("Bad address\n");
        return;
    }
    if (!adr.port)
        adr.port = BigShort (PORT_SERVER);

    in = Cmd_Argv(2);
    out = send+4;
    send[0] = send[1] = send[2] = send[3] = (char)0xff;

    l = strlen (in);
    for (i=0; i<l; i++)
    {
        if (in[i] == '\\' && in[i+1] == 'n')
        {
            *out++ = '\n';
            i++;
        }
        else
            *out++ = in[i];
    }
    *out = 0;

    NET_SendPacket (NS_CLIENT, out-send, send, &adr);
}
*/

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
static void CL_Changing_f(void)
{
    int i, j;
    const char *s; // C++20: STRING: Added const to char*

    if (cls.connectionState < ClientConnectionState::Connected) {
        return;
    }

    if (cls.demo.recording)
        CL_Stop_f();

    S_StopAllSounds();

    Com_Printf("Changing map...\n");

    if (!cls.demo.playback) {
        EXEC_TRIGGER(cl_changemapcmd);
        Cmd_ExecTrigger("#cl_changelevel");
    }

    SCR_BeginLoadingPlaque();

    cls.connectionState = ClientConnectionState::Connected;   // not active anymore, but not disconnected
    cl.mapName[0] = 0;
    cl.configstrings[ConfigStrings::Name][0] = 0;

    CL_CheckForPause();

    CL_UpdateFrameTimes();

    // parse additional parameters
    j = Cmd_Argc();
    for (i = 1; i < j; i++) {
        s = Cmd_Argv(i);
        if (!strncmp(s, "map=", 4)) {
            Q_strlcpy(cl.mapName, s + 4, sizeof(cl.mapName));
        }
    }

    SCR_UpdateScreen();
}


/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
static void CL_Reconnect_f(void)
{
    if (cls.connectionState >= ClientConnectionState::Precached) {
        CL_Disconnect(ERR_RECONNECT);
    }

    if (cls.connectionState >= ClientConnectionState::Connected) {
        cls.connectionState = ClientConnectionState::Connected;

        if (cls.demo.playback) {
            return;
        }
        if (cls.download.file) {
            return; // if we are downloading, we don't change!
        }

        Com_Printf("Reconnecting...\n");

        CL_ClientCommand("new");
        return;
    }

    // issued manually at console
    if (cls.serverAddress.type == NA_UNSPECIFIED) {
        Com_Printf("No server to reconnect to.\n");
        return;
    }
    if (cls.serverAddress.type == NA_LOOPBACK) {
        Com_Printf("Can not reconnect to loopback.\n");
        return;
    }

    Com_Printf("Reconnecting...\n");

    cls.connectionState = ClientConnectionState::Challenging;
    cls.timeOfInitialConnect -= CONNECT_FAST;
    cls.connect_count = 0;

    SCR_UpdateScreen();
}

#ifdef USE_UI
/*
=================
CL_SendStatusRequest
=================
*/
void CL_SendStatusRequest(const NetAdr *address)
{
    NET_Config(NET_CLIENT);

    CL_AddRequest(address, REQ_STATUS_UI);

    OOB_PRINT(NS_CLIENT, address, "status");
}
#endif

/*
=================
CL_PingServers_f
=================
*/
static void CL_PingServers_f(void)
{
    NetAdr address;
    cvar_t *var;
    int i;

    NET_Config(NET_CLIENT);

    // send a broadcast packet
    memset(&address, 0, sizeof(address));
    address.type = NA_BROADCAST;
    address.port = BigShort(PORT_SERVER);

    Com_DPrintf("Pinging broadcast...\n");
    CL_AddRequest(&address, REQ_INFO);

    OOB_PRINT(NS_CLIENT, &address, "info 34");

    // send a packet to each address book entry
    for (i = 0; i < 64; i++) {
        var = Cvar_FindVar(va("adr%i", i));
        if (!var)
            break;

        if (!var->string[0])
            continue;

        if (!NET_StringToAdr(var->string, &address, PORT_SERVER)) {
            Com_Printf("Bad address: %s\n", var->string);
            continue;
        }

        Com_DPrintf("Pinging %s...\n", var->string);
        CL_AddRequest(&address, REQ_INFO);

        OOB_PRINT(NS_CLIENT, &address, "info 34");
    }
}

static void CL_Name_g(genctx_t *ctx)
{
    int i;
    ClientInfo *ci;
    char buffer[MAX_CLIENT_NAME];

    if (cls.connectionState < ClientConnectionState::Loading) {
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        ci = &cl.clientInfo[i];
        if (!ci->name[0]) {
            continue;
        }
        Q_strlcpy(buffer, ci->name, sizeof(buffer));
        if (COM_strclr(buffer) && !Prompt_AddMatch(ctx, buffer)) {
            break;
        }
    }
}


/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
static void CL_ConnectionlessPacket(void)
{
    char    string[MAX_STRING_CHARS];
    const char    *s, *c; // C++20: STRING: Added const to char*
    int     i, j, k;
    size_t  len;
    int type;

    MSG_BeginReading();
    MSG_ReadLong(); // skip the -1

    len = MSG_ReadStringLine(string, sizeof(string));
    if (len >= sizeof(string)) {
        Com_DPrintf("Oversize message received.  Ignored.\n");
        return;
    }

    Cmd_TokenizeString(string, false);

    c = Cmd_Argv(0);

    Com_DPrintf("%s: %s\n", NET_AdrToString(&net_from), string);

    // challenge from the server we are connecting to
    if (!strcmp(c, "challenge")) {
        int mask = 0;

        if (cls.connectionState < ClientConnectionState::Challenging) {
            Com_DPrintf("Challenge received while not connecting.  Ignored.\n");
            return;
        }
        if (!NET_IsEqualBaseAdr(&net_from, &cls.serverAddress)) {
            Com_DPrintf("Challenge from different address.  Ignored.\n");
            return;
        }
        if (cls.connectionState > ClientConnectionState::Challenging) {
            Com_DPrintf("Dup challenge received.  Ignored.\n");
            return;
        }

        cls.challenge = atoi(Cmd_Argv(1));
        cls.connectionState = ClientConnectionState::Connecting;
        cls.timeOfInitialConnect -= CONNECT_INSTANT; // fire immediately
        //cls.connect_count = 0;

        // Parse additional parameters
        int protocolFound = 0; // N&C: Added in to ensure we find a protocol, otherwise warn the player.

        j = Cmd_Argc();
        for (i = 2; i < j; i++) {
            s = Cmd_Argv(i);
            // Protocol version check, to ensure it is similar.
            if (!strncmp(s, "p=", 2)) {
                s += 2;
                k = strtoul(s, NULL, 10);
                Com_DPrintf("p=======%i", k);
                if (k == PROTOCOL_VERSION_POLYHEDRON) {
                    protocolFound = k;
                    break;
                }
                //while (*s) {
                //    k = strtoul(s, (char**)&s, 10);
                //    if (k == PROTOCOL_VERSION_POLYHEDRON) {
                //        protocolFound = true;
                //    }
                //    s = strchr(s, ',');
                //    if (s == NULL) {
                //        break;
                //    }
                //    s++;
                //}
            }
        }

        // Inform in case the protocol was not found, or not equal to our own.
        if (protocolFound != PROTOCOL_VERSION_POLYHEDRON) {
            Com_EPrintf("Challenging protocol: p=%i did not equal PROTOCOL_VERSION_POLYHEDRON(%i)\n", protocolFound, PROTOCOL_VERSION_POLYHEDRON);
        }

        // Setup to use our own protocol. However, if needed later on, here is a shot to change it.
        // This might be useful for backward compatibilities etc.
        cls.serverProtocol = PROTOCOL_VERSION_POLYHEDRON;

        Com_DPrintf("Selected protocol %d\n", cls.serverProtocol);

        CL_CheckForResend();
        return;
    }

    // server connection
    if (!strcmp(c, "client_connect")) {
        int anticheat = 0;
        char mapName[MAX_QPATH];
        qboolean got_server = false;

        if (cls.connectionState < ClientConnectionState::Connecting) {
            Com_DPrintf("Connect received while not connecting.  Ignored.\n");
            return;
        }
        if (!NET_IsEqualBaseAdr(&net_from, &cls.serverAddress)) {
            Com_DPrintf("Connect from different address.  Ignored.\n");
            return;
        }
        if (cls.connectionState > ClientConnectionState::Connecting) {
            Com_DPrintf("Dup connect received.  Ignored.\n");
            return;
        }

        // MSG: !! TODO: Look at demo code and see if we can remove NETCHAN_OLD.
        mapName[0] = 0;

        // parse additional parameters
        j = Cmd_Argc();
        for (i = 1; i < j; i++) {
            s = Cmd_Argv(i);
            if (!strncmp(s, "ac=", 3)) {
                s += 3;
                if (*s) {
                    anticheat = atoi(s);
                }
            } else if (!strncmp(s, "nc=", 3)) {
                s += 3;
                if (*s) {
                    type = atoi(s); // CPP: int to (netchan_type_t)
                    if (type != 1) {
                        Com_Error(ERR_DISCONNECT,
                                  "Server returned invalid netchan type");
                    }
                }
            } else if (!strncmp(s, "map=", 4)) {
                Q_strlcpy(mapName, s + 4, sizeof(mapName));
            } else if (!strncmp(s, "dlserver=", 9)) {
                if (!got_server) {
                    HTTP_SetServer(s + 9);
                    got_server = true;
                }
            }
        }

        if (!got_server) {
            HTTP_SetServer(NULL);
        }

        Com_Printf("Connected to %s (protocol %d).\n",
                   NET_AdrToString(&cls.serverAddress), cls.serverProtocol);
        if (cls.netChannel) {
            // this may happen after svc_reconnect
            Netchan_Close(cls.netChannel);
        }
        cls.netChannel = Netchan_Setup(NS_CLIENT, &cls.serverAddress,
                                    cls.quakePort, 1024, cls.serverProtocol);

        CL_ClientCommand("new");
        cls.connectionState = ClientConnectionState::Connected;
        cls.connect_count = 0;
        strcpy(cl.mapName, mapName);   // for levelshot screen
        return;
    }

    if (!strcmp(c, "passive_connect")) {
        if (!cls.passive) {
            Com_DPrintf("Passive connect received while not connecting.  Ignored.\n");
            return;
        }
        s = NET_AdrToString(&net_from);
        Com_Printf("Received passive connect from %s.\n", s);

        cls.serverAddress = net_from;
        cls.serverProtocol = cl_protocol->integer;
        Q_strlcpy(cls.servername, s, sizeof(cls.servername));
        cls.passive = false;

        cls.connectionState = ClientConnectionState::Challenging;
        cls.timeOfInitialConnect -= CONNECT_FAST;
        cls.connect_count = 0;

        CL_CheckForResend();
        return;
    }

    // print command from somewhere
    if (!strcmp(c, "print")) {
        CL_ParsePrintMessage();
        return;
    }

    // server responding to a status broadcast
    if (!strcmp(c, "info")) {
        CL_ParseInfoMessage();
        return;
    }

    Com_DPrintf("Unknown connectionless packet command.\n");
}

/*
=================
CL_PacketEvent
=================
*/
static void CL_PacketEvent(void)
{
    //
    // remote command packet
    //
    if (*(int *)msg_read.data == -1) {
        CL_ConnectionlessPacket();
        return;
    }

    if (cls.connectionState < ClientConnectionState::Connected) {
        return;
    }

    if (!cls.netChannel) {
        return;     // dump it if not connected
    }

    if (msg_read.currentSize < 8) {
        Com_DPrintf("%s: runt packet\n", NET_AdrToString(&net_from));
        return;
    }

    //
    // packet from server
    //
    if (!NET_IsEqualAdr(&net_from, &cls.netChannel->remoteNetAddress)) {
        Com_DPrintf("%s: sequenced packet without connection\n",
                    NET_AdrToString(&net_from));
        return;
    }

    if (!Netchan_Process(cls.netChannel))
        return;     // wasn't accepted for some reason

#if USE_ICMP
    cls.errorReceived = false; // don't drop
#endif

    CL_ParseServerMessage();

    // if recording demo, write the message out
    if (cls.demo.recording && !cls.demo.paused && CL_FRAMESYNC()) {
        CL_WriteDemoMessage(&cls.demo.buffer);
    }

    if (!cls.netChannel)
        return;     // might have disconnected

#ifdef _DEBUG
    CL_AddNetgraph();
#endif

    SCR_LagSample();
}

#if USE_ICMP
void CL_ErrorEvent(NetAdr *from)
{
    UI_ErrorEvent(from);

    //
    // error packet from server
    //
    if (cls.connectionState < ClientConnectionState::Connected) {
        return;
    }
    if (!cls.netChannel) {
        return;     // dump it if not connected
    }
    if (!NET_IsEqualBaseAdr(from, &cls.netChannel->remoteNetAddress)) {
        return;
    }
    if (from->port && from->port != cls.netChannel->remoteNetAddress.port) {
        return;
    }

    cls.errorReceived = true; // drop connection soon
}
#endif


//=============================================================================

void CL_UpdateUserinfo(cvar_t *var, from_t from)
{
    int i;
    if (!cls.netChannel) {
        return;
    }

    if (cls.serverProtocol != PROTOCOL_VERSION_POLYHEDRON) {
        // transmit at next oportunity
        cls.userinfo_modified = MAX_PACKET_USERINFOS;
        goto done;
    }

    if (cls.userinfo_modified == MAX_PACKET_USERINFOS) {
        // can't hold any more
        goto done;
    }


    // N&C: Allow the CG Module to work with it.
    CL_GM_ClientUpdateUserInfo(var, from);

    // check for the same variable being modified twice
    for (i = 0; i < cls.userinfo_modified; i++) {
        if (cls.userinfo_updates[i] == var) {
            Com_DDPrintf("%s: %u: %s [DUP]\n",
                         __func__, com_framenum, var->name);
            return;
        }
    }

    cls.userinfo_updates[cls.userinfo_modified++] = var;

done:
    Com_DDPrintf("%s: %u: %s [%d]\n",
                 __func__, com_framenum, var->name, cls.userinfo_modified);
}

/*
==============
CL_Userinfo_f
==============
*/
static void CL_Userinfo_f(void)
{
    char userinfo[MAX_INFO_STRING];

    Cvar_BitInfo(userinfo, CVAR_USERINFO);

    Com_Printf("User info settings:\n");
    Info_Print(userinfo);
}

/*
=================
CL_RestartSound_f

Restart the sound subsystem so it can pick up
new parameters and flush all sounds
=================
*/
static void CL_RestartSound_f(void)
{
    S_Shutdown();
    S_Init();

    // Restart the refresh system along so it can load all models.
    CL_RestartRefresh(false);
}

/*
=================
CL_PlaySound_f

Moved here from sound code so that command is always registered.
=================
*/
static void CL_PlaySound_c(genctx_t *ctx, int state)
{
    FS_File_g("sound", "*.wav", FS_SEARCH_SAVEPATH | FS_SEARCH_BYFILTER | FS_SEARCH_STRIPEXT, ctx);
}

static void CL_PlaySound_f(void)
{
    int     i;
    char name[MAX_QPATH];

    if (Cmd_Argc() < 2) {
        Com_Printf("Usage: %s <sound> [...]\n", Cmd_Argv(0));
        return;
    }

    for (i = 1; i < Cmd_Argc(); i++) {
        Cmd_ArgvBuffer(i, name, sizeof(name));
        COM_DefaultExtension(name, ".wav", sizeof(name));
        S_StartLocalSound(name);
    }
}

static int precache_spawncount;

//
//===============
// CL_UpdateListenerOrigin
//
// Updates the listener_ variables, this is called by the CG Module its
// CLG_RenderView function.
//===============
//
void CL_UpdateListenerOrigin(void) {
    VectorCopy(cl.refdef.vieworg, listener_origin);
    VectorCopy(cl.v_forward, listener_forward);
    VectorCopy(cl.v_right, listener_right);
    VectorCopy(cl.v_up, listener_up);
}

/*
=================
CL_Begin

Called after all downloads are done. Not used for demos.
=================
*/
void CL_Begin(void)
{
    Cvar_FixCheats();

    // N&C: Prepare media loading.
    CL_PrepareMedia();

    // TODO: Move over to the CG Module.
    LOC_LoadLocations();

    // Set state to precached and send over a begin command.
    CL_LoadState(LOAD_NONE);
    cls.connectionState = ClientConnectionState::Precached;
    CL_ClientCommand(va("begin %i\n", precache_spawncount));

    // Inform CG Module.
    CL_GM_ClientBegin();
}

/*
=================
CL_Precache_f

The server will send this command right
before allowing the client into the server
=================
*/
static void CL_Precache_f(void)
{
    if (cls.connectionState < ClientConnectionState::Connected) {
        return;
    }

    cls.connectionState = ClientConnectionState::Loading;
    CL_LoadState(LOAD_MAP);

    S_StopAllSounds();

    // Demos use different precache sequence
    if (cls.demo.playback) {
        CL_RegisterBspModels();
        CL_PrepareMedia();
        CL_LoadState(LOAD_NONE);
        cls.connectionState = ClientConnectionState::Precached;
        return;
    }

    precache_spawncount = atoi(Cmd_Argv(1));

    CL_ResetPrecacheCheck();
    CL_RequestNextDownload();

    if (cls.connectionState != ClientConnectionState::Precached) {
        cls.connectionState = ClientConnectionState::Connected;
    }
}

typedef struct {
    list_t entry;
    unsigned hits;
    char match[1];
} ignore_t;

static list_t cl_ignores;

static ignore_t *find_ignore(const char *match)
{
    ignore_t *ignore;

    LIST_FOR_EACH(ignore_t, ignore, &cl_ignores, entry) {
        if (!strcmp(ignore->match, match)) {
            return ignore;
        }
    }

    return NULL;
}

static void list_ignores(void)
{
    ignore_t *ignore;

    if (LIST_EMPTY(&cl_ignores)) {
        Com_Printf("No ignore filters.\n");
        return;
    }

    Com_Printf("Current ignore filters:\n");
    LIST_FOR_EACH(ignore_t, ignore, &cl_ignores, entry) {
        Com_Printf("\"%s\" (%u hit%s)\n", ignore->match,
                   ignore->hits, ignore->hits == 1 ? "" : "s");
    }
}

static void add_ignore(const char *match)
{
    ignore_t *ignore;
    size_t matchlen;

    // don't create the same ignore twice
    if (find_ignore(match)) {
        return;
    }

    matchlen = strlen(match);
    if (matchlen < 3) {
        Com_Printf("Match string \"%s\" is too short.\n", match);
        return;
    }

    // CPP: cast
    ignore = (ignore_t*)Z_Malloc(sizeof(*ignore) + matchlen);
    ignore->hits = 0;
    memcpy(ignore->match, match, matchlen + 1);
    List_Append(&cl_ignores, &ignore->entry);
}

static void remove_ignore(const char *match)
{
    ignore_t *ignore;

    ignore = find_ignore(match);
    if (!ignore) {
        Com_Printf("Can't find ignore filter \"%s\"\n", match);
        return;
    }

    List_Remove(&ignore->entry);
    Z_Free(ignore);
}

static void remove_all_ignores(void)
{
    ignore_t *ignore, *next;
    int count = 0;

    LIST_FOR_EACH_SAFE(ignore_t, ignore, next, &cl_ignores, entry) {
        Z_Free(ignore);
        count++;
    }

    Com_Printf("Removed %d ignore filter%s.\n", count, count == 1 ? "" : "s");
    List_Init(&cl_ignores);
}

static void CL_IgnoreText_f(void)
{
    if (Cmd_Argc() == 1) {
        list_ignores();
        return;
    }

    add_ignore(Cmd_ArgsFrom(1));
}

static void CL_UnIgnoreText_f(void)
{
    if (Cmd_Argc() == 1) {
        list_ignores();
        return;
    }

    if (LIST_EMPTY(&cl_ignores)) {
        Com_Printf("No ignore filters.\n");
        return;
    }

    if (!strcmp(Cmd_Argv(1), "all")) {
        remove_all_ignores();
        return;
    }

    remove_ignore(Cmd_ArgsFrom(1));
}

static void CL_IgnoreNick_c(genctx_t *ctx, int argnum)
{
    if (argnum == 1) {
        CL_Name_g(ctx);
    }
}

// properly escapes any special characters in nickname
static size_t parse_ignore_nick(int argnum, char *buffer)
{
    char temp[MAX_CLIENT_NAME];
    char *p, *s;
    int c;
    size_t len;

    Cmd_ArgvBuffer(argnum, temp, sizeof(temp));

    s = temp;
    p = buffer;
    len = 0;
    while (*s) {
        c = *s++;
        c &= 127;
        if (c == '?') {
            *p++ = '\\';
            *p++ = '?';
            len += 2;
        } else if (c == '*') {
            *p++ = '\\';
            *p++ = '*';
            len += 2;
        } else if (c == '\\') {
            *p++ = '\\';
            *p++ = '\\';
            len += 2;
        } else if (Q_isprint(c)) {
            *p++ = c;
            len++;
        }
    }

    *p = 0;

    return len;
}

static void CL_IgnoreNick_f(void)
{
    char nick[MAX_CLIENT_NAME * 2];
    char match[MAX_CLIENT_NAME * 3];

    if (Cmd_Argc() == 1) {
        list_ignores();
        return;
    }

    if (!parse_ignore_nick(1, nick)) {
        return;
    }

    Q_snprintf(match, sizeof(match), "%s: *", nick);
    add_ignore(match);

    Q_snprintf(match, sizeof(match), "(%s): *", nick);
    add_ignore(match);
}

static void CL_UnIgnoreNick_f(void)
{
    char nick[MAX_CLIENT_NAME * 2];
    char match[MAX_CLIENT_NAME * 3];

    if (Cmd_Argc() == 1) {
        list_ignores();
        return;
    }

    if (!parse_ignore_nick(1, nick)) {
        return;
    }

    Q_snprintf(match, sizeof(match), "%s: *", nick);
    remove_ignore(match);

    Q_snprintf(match, sizeof(match), "(%s): *", nick);
    remove_ignore(match);
}

/*
=================
CL_CheckForIgnore
=================
*/
qboolean CL_CheckForIgnore(const char *s)
{
    char buffer[MAX_STRING_CHARS];
    ignore_t *ignore;

    if (LIST_EMPTY(&cl_ignores)) {
        return false;
    }

    Q_strlcpy(buffer, s, sizeof(buffer));
    COM_strclr(buffer);

    LIST_FOR_EACH(ignore_t, ignore, &cl_ignores, entry) {
        if (Com_WildCmp(ignore->match, buffer)) {
            ignore->hits++;
            return true;
        }
    }

    return false;
}

static void CL_DumpClients_f(void)
{
    int i;

    if (cls.connectionState != ClientConnectionState::Active) {
        Com_Printf("Must be in a level to dump.\n");
        return;
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!cl.clientInfo[i].name[0]) {
            continue;
        }

        Com_Printf("%3i: %s\n", i, cl.clientInfo[i].name);
    }
}

static void dump_program(const char *text, const char *name)
{
    char buffer[MAX_OSPATH];

    if (cls.connectionState != ClientConnectionState::Active) {
        Com_Printf("Must be in a level to dump.\n");
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: %s <filename>\n", Cmd_Argv(0));
        return;
    }

    if (!*text) {
        Com_Printf("No %s to dump.\n", name);
        return;
    }

    if (FS_EasyWriteFile(buffer, sizeof(buffer), FS_MODE_WRITE | FS_FLAG_TEXT,
                         "layouts/", Cmd_Argv(1), ".txt", text, strlen(text))) {
        Com_Printf("Dumped %s program to %s.\n", name, buffer);
    }
}

static void CL_DumpStatusbar_f(void)
{
    dump_program(cl.configstrings[ConfigStrings::StatusBar], "status bar");
}

static void CL_DumpLayout_f(void)
{
    dump_program(cl.layout, "layout");
}

static const cmd_option_t o_writeconfig[] = {
    { "a", "aliases", "write aliases" },
    { "b", "bindings", "write bindings" },
    { "c", "cvars", "write archived cvars" },
    { "h", "help", "display this help message" },
    { "m", "modified", "write modified cvars" },
    { NULL }
};

static void CL_WriteConfig_c(genctx_t *ctx, int argnum)
{
    Cmd_Option_c(o_writeconfig, Cmd_Config_g, ctx, argnum);
}

/*
===============
CL_WriteConfig_f
===============
*/
static void CL_WriteConfig_f(void)
{
    char buffer[MAX_OSPATH];
    qboolean aliases = false, bindings = false, modified = false;
    int c, mask = 0;
    qhandle_t f;

    while ((c = Cmd_ParseOptions(o_writeconfig)) != -1) {
        switch (c) {
        case 'a':
            aliases = true;
            break;
        case 'b':
            bindings = true;
            break;
        case 'c':
            mask |= CVAR_ARCHIVE;
            break;
        case 'h':
            Cmd_PrintUsage(o_writeconfig, "<filename>");
            Com_Printf("Save current configuration into file.\n");
            Cmd_PrintHelp(o_writeconfig);
            return;
        case 'm':
            modified = true;
            mask = ~0;
            break;
        default:
            return;
        }
    }

    if (!cmd_optarg[0]) {
        Com_Printf("Missing filename argument.\n");
        Cmd_PrintHint();
        return;
    }

    if (!aliases && !bindings && !mask) {
        bindings = true;
        mask = CVAR_ARCHIVE;
    }

    f = FS_EasyOpenFile(buffer, sizeof(buffer), FS_MODE_WRITE | FS_FLAG_TEXT,
                        "configs/", cmd_optarg, ".cfg");
    if (!f) {
        return;
    }

    FS_FPrintf(f, "// generated by " APPLICATION "\n");

    if (bindings) {
        FS_FPrintf(f, "\n// key bindings\n");
        Key_WriteBindings(f);
    }
    if (aliases) {
        FS_FPrintf(f, "\n// command aliases\n");
        Cmd_WriteAliases(f);
    }
    if (mask) {
        FS_FPrintf(f, "\n//%s cvars\n", modified ? "modified" : "archived");
        Cvar_WriteVariables(f, mask, modified);
    }

    FS_FCloseFile(f);

    Com_Printf("Wrote %s.\n", buffer);
}

static void CL_Say_c(genctx_t *ctx, int argnum)
{
    CL_Name_g(ctx);
}

static size_t CL_Mapname_m(char *buffer, size_t size)
{
    return Q_strlcpy(buffer, cl.mapName, size);
}

static size_t CL_Server_m(char *buffer, size_t size)
{
    return Q_strlcpy(buffer, cls.servername, size);
}

static size_t CL_Ups_m(char *buffer, size_t size)
{
    vec3_t vel;

    if (cl.frame.clientNumber == CLIENTNUM_NONE) {
        if (size) {
            *buffer = 0;
        }
        return 0;
    }

    if (!cls.demo.playback && cl.frame.clientNumber == cl.clientNumber &&
        cl_predict->integer) {
        vel = cl.predictedState.velocity;
    } else {
        // N&C: FF Precision.
        VectorCopy(cl.predictedState.velocity, vel);
       // VectorScale(cl.frame.playerState.pmove.velocity, 0.125f, vel);
    }

    return Q_scnprintf(buffer, size, "%d", (int)VectorLength(vel));
}

static size_t CL_Timer_m(char *buffer, size_t size)
{
    int hour, min, sec;

    sec = cl.time / 1000;
    min = sec / 60; sec %= 60;
    hour = min / 60; min %= 60;

    if (hour) {
        return Q_scnprintf(buffer, size, "%i:%i:%02i", hour, min, sec);
    }
    return Q_scnprintf(buffer, size, "%i:%02i", min, sec);
}

static size_t CL_DemoPos_m(char *buffer, size_t size)
{
    int sec, min, frameNumber;

    if (cls.demo.playback)
        frameNumber = cls.demo.frames_read;
    else
        frameNumber = 0;  

    sec = frameNumber / 10; frameNumber %= 10;
    min = sec / 60; sec %= 60;

    return Q_scnprintf(buffer, size,
                       "%d:%02d.%d", min, sec, frameNumber);
}

static size_t CL_Fps_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", C_FPS);
}

static size_t R_Fps_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", R_FPS);
}

static size_t CL_Mps_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", C_MPS);
}

static size_t CL_Pps_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", C_PPS);
}

static size_t CL_Ping_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%i", cls.measure.ping);
}

static size_t CL_Lag_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%.2f%%", cls.netChannel ?
                       ((float)cls.netChannel->totalDropped /
                        cls.netChannel->totalReceived) * 100.0f : 0);
}

static size_t CL_Cluster_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%i", cl.refdef.feedback.viewcluster);
}

static size_t CL_ClusterThere_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%i", cl.refdef.feedback.lookatcluster);
}

static size_t CL_NumLightPolys_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%i", cl.refdef.feedback.num_light_polys);
}

static size_t CL_Material_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%s", cl.refdef.feedback.view_material);
}

static size_t CL_Material_Override_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%s", cl.refdef.feedback.view_material_override);
}

static size_t CL_ViewPos_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "(%.1f, %.1f, %.1f)", cl.refdef.vieworg[0], cl.refdef.vieworg[1], cl.refdef.vieworg[2]);
}

static size_t CL_ViewDir_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "(%.3f, %.3f, %.3f)", cl.v_forward[0], cl.v_forward[1], cl.v_forward[2]);
}

static size_t CL_HdrColor_m(char* buffer, size_t size) {
    const float* color = cl.refdef.feedback.hdr_color;
    return Q_scnprintf(buffer, size, "(%.5f, %.5f, %.5f)", color[0], color[1], color[2]);
}

static size_t CL_ResolutionScale_m(char* buffer, size_t size) {
    return Q_scnprintf(buffer, size, "%d", cl.refdef.feedback.resolution_scale);
}

int CL_GetFps()
{
	return C_FPS;
}

int CL_GetResolutionScale()
{
	return cl.refdef.feedback.resolution_scale;
}

/*
===============
CL_WriteConfig

Writes key bindings and archived cvars to config.cfg
===============
*/
void CL_WriteConfig(void)
{
    qhandle_t f;
    qerror_t ret;

    ret = FS_FOpenFile(COM_CONFIG_CFG, &f, FS_MODE_WRITE | FS_FLAG_TEXT);
    if (!f) {
        Com_EPrintf("Couldn't open %s for writing: %s\n",
                    COM_CONFIG_CFG, Q_ErrorString(ret));
        return;
    }

    FS_FPrintf(f, "// generated by " APPLICATION ", do not modify\n");

    Key_WriteBindings(f);
    Cvar_WriteVariables(f, CVAR_ARCHIVE, false);

    FS_FCloseFile(f);
}

/*
====================
CL_RestartFilesystem

Flush caches and restart the VFS.
====================
*/
void CL_RestartFilesystem(qboolean total)
{
    int32_t clientConnectionState;

    if (!cl_running->integer) {
        FS_Restart(total);
        return;
    }

    Com_DPrintf("%s(%d)\n", __func__, total);

    // temporary switch to loading state
    clientConnectionState = cls.connectionState;
    if (cls.connectionState >= ClientConnectionState::Precached) {
        cls.connectionState = ClientConnectionState::Loading;
    }

    Con_Popup(false);

    UI_Shutdown();

    S_StopAllSounds();
    S_FreeAllSounds();

    // write current config before changing game directory
    CL_WriteConfig();

    if (cls.ref_initialized) {
        R_Shutdown(false);

        FS_Restart(total);

        R_Init(false);

        // Load client screen media first.
        SCR_RegisterMedia();
        // N&C: Inform the CG Module about the registration of media.
        CL_GM_LoadScreenMedia();
        Con_RegisterMedia();
        UI_Init();
    } else {
        FS_Restart(total);
    }

    if (clientConnectionState == ClientConnectionState::Disconnected) {
        
        //UI_OpenMenu(UIMENU_DEFAULT);
    } else if (clientConnectionState >= ClientConnectionState::Loading && clientConnectionState <= ClientConnectionState::Active) {
        CL_LoadState(LOAD_MAP);
        CL_PrepareMedia();
        CL_LoadState(LOAD_NONE);
    } else if (clientConnectionState == ClientConnectionState::Cinematic) {
        cl.precaches.images[0] = R_RegisterPic2(cl.mapName);
    }

    CL_LoadDownloadIgnores();

    // switch back to original state
    cls.connectionState = clientConnectionState; // CPP:

    Con_Close(false);

    CL_UpdateFrameTimes();

    cvar_modified &= ~CVAR_FILES;
}

void CL_RestartRefresh(qboolean total)
{
    int32_t clientConnectionState;

    if (!cls.ref_initialized) {
        return;
    }

    // temporary switch to loading state
    clientConnectionState = cls.connectionState;
    if (cls.connectionState >= ClientConnectionState::Precached) {
        cls.connectionState = ClientConnectionState::Loading;
    }

    Con_Popup(false);

    S_StopAllSounds();

    if (total) {
        IN_Shutdown();
        CL_ShutdownRefresh();
        CL_InitRefresh();
        IN_Init();
    } else {
        UI_Shutdown();
        R_Shutdown(false);
        R_Init(false);
        
        // Load client screen media first.
        SCR_RegisterMedia();
        // N&C: Inform the CG Module about the registration of media.
        CL_GM_LoadScreenMedia();
        Con_RegisterMedia();
        UI_Init();

    }

    if (clientConnectionState == ClientConnectionState::Disconnected) {
        UI_OpenMenu(UIMENU_DEFAULT);
    } else if (clientConnectionState >= ClientConnectionState::Loading && clientConnectionState <= ClientConnectionState::Active) {
        CL_LoadState(LOAD_MAP);
        CL_PrepareMedia();
        CL_LoadState(LOAD_NONE);
    } else if (clientConnectionState == ClientConnectionState::Cinematic) {
        cl.precaches.images[0] = R_RegisterPic2(cl.mapName);
    }

    // switch back to original state
    cls.connectionState = clientConnectionState; // CPP:

    Con_Close(false);

    CL_UpdateFrameTimes();

    cvar_modified &= ~CVAR_FILES;
}

/*
====================
CL_ReloadRefresh

Flush caches and reload all models and textures.
====================
*/
static void CL_ReloadRefresh_f(void)
{
    CL_RestartRefresh(false);
}

/*
====================
CL_RestartRefresh

Perform complete restart of the renderer subsystem.
====================
*/
static void CL_RestartRefresh_f(void)
{
    CL_RestartRefresh(true);
}

// execute string in server command buffer
static void exec_server_string(cmdbuf_t *buf, const char *text)
{
    const char *s; // C++20: STRING: Added const to char*

    Cmd_TokenizeString(text, true);

    // execute the command line
    if (!Cmd_Argc()) {
        return;        // no tokens
    }

    Com_DPrintf("stufftext: %s\n", text);

    s = Cmd_Argv(0);

    // handle private client commands
    if (!strcmp(s, "changing")) {
        CL_Changing_f();
        return;
    }
    if (!strcmp(s, "precache")) {
        CL_Precache_f();
        return;
    }

    // forbid nearly every command from demos
    if (cls.demo.playback) {
        if (strcmp(s, "play")) {
            return;
        }
    }

    // execute regular commands
    Cmd_ExecuteCommand(buf);
}

static inline int fps_to_msec(int fps) {
#if 0
    return (1000 + fps / 2) / fps;
#else
    return 1000 / fps;
#endif
}

static void warn_on_fps_rounding(cvar_t* cvar) {
    static qboolean warned = false;
    int msec, real_maxfps;

    if (cvar->integer <= 0 || cl_warn_on_fps_rounding->integer <= 0)
        return;

    msec = fps_to_msec(cvar->integer);
    if (!msec)
        return;

    real_maxfps = 1000 / msec;
    if (cvar->integer == real_maxfps)
        return;

    Com_WPrintf("%s value `%d' is inexact, using `%d' instead.\n",
        cvar->name, cvar->integer, real_maxfps);
    if (!warned) {
        Com_Printf("(Set `%s' to `0' to disable this warning.)\n",
            cl_warn_on_fps_rounding->name);
        warned = true;
    }
}

static void cl_sync_changed(cvar_t *self)
{
    CL_UpdateFrameTimes();
}

static void cl_maxfps_changed(cvar_t* self) {
    CL_UpdateFrameTimes();
    warn_on_fps_rounding(self);
}

// allow downloads to be permanently disabled as a
// protection measure from malicious (or just stupid) servers
// that force downloads by stuffing commands
static void cl_allow_download_changed(cvar_t *self)
{
    if (self->integer == -1) {
        self->flags |= CVAR_ROM;
    }
}

// ugly hack for compatibility
static void cl_chat_sound_changed(cvar_t *self)
{
    if (!*self->string)
        self->integer = 0;
    else if (!Q_stricmp(self->string, "misc/talk.wav"))
        self->integer = 1;
    else if (!Q_stricmp(self->string, "misc/talk1.wav"))
        self->integer = 2;
    else if (!self->integer && !COM_IsUint(self->string))
        self->integer = 1;
}

static const cmdreg_t c_client[] = {
    { "cmd", CL_ForwardToServer_f },
    { "pause", CL_Pause_f },
    { "pingservers", CL_PingServers_f },
//    { "skins", CL_Skins_f },
    { "userinfo", CL_Userinfo_f },
    { "snd_restart", CL_RestartSound_f },
    { "play", CL_PlaySound_f, CL_PlaySound_c },
    //{ "changing", CL_Changing_f },
    { "disconnect", CL_Disconnect_f },
    { "connect", CL_Connect_f, CL_Connect_c },
    { "followip", CL_FollowIP_f },
    { "passive", CL_PassiveConnect_f },
    { "reconnect", CL_Reconnect_f },
    { "rcon", CL_Rcon_f, CL_Rcon_c },
    //{ "precache", CL_Precache_f },
    { "serverstatus", CL_ServerStatus_f, CL_ServerStatus_c },
    { "ignoretext", CL_IgnoreText_f },
    { "unignoretext", CL_UnIgnoreText_f },
    { "ignorenick", CL_IgnoreNick_f, CL_IgnoreNick_c },
    { "unignorenick", CL_UnIgnoreNick_f, CL_IgnoreNick_c },
    { "dumpclients", CL_DumpClients_f },
    { "dumpstatusbar", CL_DumpStatusbar_f },
    { "dumplayout", CL_DumpLayout_f },
    { "writeconfig", CL_WriteConfig_f, CL_WriteConfig_c },
//    { "msgtab", CL_Msgtab_f, CL_Msgtab_g },
    { "vid_restart", CL_RestartRefresh_f },
    { "r_reload", CL_ReloadRefresh_f },

    //
    // forward to server commands
    //
    // the only thing this does is allow command completion
    // to work -- all unknown commands are automatically
    // forwarded to the server
    { "say", NULL, CL_Say_c },
    { "say_team", NULL, CL_Say_c },

 //   { "wave" }, { "inven" }, { "kill" }, { "use" },
 //   { "drop" }, { "info" }, { "prog" },
 //   { "give" }, { "god" }, { "notarget" }, { "noclip" },
 //   { "invuse" }, { "invprev" }, { "invnext" }, { "invdrop" },
	//{ "weapnext" }, { "weapprev" },

    { NULL }
};

/*
=================
CL_InitLocal
=================
*/
static void CL_InitLocal(void)
{
    cvar_t *var;
    int i;

    cls.connectionState = ClientConnectionState::Disconnected;
    cls.timeOfInitialConnect -= CONNECT_INSTANT;

    // Initialize the rest of the client.
    CL_RegisterInput();
    CL_InitDemos();
    LOC_Init();
    CL_InitAscii();
    CL_InitDownloads();

    List_Init(&cl_ignores);

    Cmd_Register(c_client);

    for (i = 0; i < MAX_LOCAL_SERVERS; i++) {
        var = Cvar_Get(va("adr%i", i), "", CVAR_ARCHIVE);
        var->generator = Com_Address_g;
    }

    //
    // register our variables
    //
    cl_predict = Cvar_Get("cl_predict", "1", 0);
    cl_kickangles = Cvar_Get("cl_kickangles", "1", CVAR_CHEAT);
    cl_warn_on_fps_rounding = Cvar_Get("cl_warn_on_fps_rounding", "1", 0);
    cl_maxfps = Cvar_Get("cl_maxfps", "62", 0);
    cl_maxfps->changed = cl_maxfps_changed;
    cl_async = Cvar_Get("cl_async", "1", 0);
    cl_async->changed = cl_sync_changed;
    r_maxfps = Cvar_Get("r_maxfps", "0", 0);
    r_maxfps->changed = cl_maxfps_changed;
    cl_autopause = Cvar_Get("cl_autopause", "1", 0);
    cl_rollhack = Cvar_Get("cl_rollhack", "1", 0);
    cl_noglow = Cvar_Get("cl_noglow", "0", 0);
    cl_nolerp = Cvar_Get("cl_nolerp", "0", 0);

    // hack for timedemo
    com_timedemo->changed = cl_sync_changed;

    CL_UpdateFrameTimes();
    warn_on_fps_rounding(cl_maxfps);
    warn_on_fps_rounding(r_maxfps);

//#ifdef _DEBUG
    cl_shownet = Cvar_Get("cl_shownet", "0", 0);
    cl_showmiss = Cvar_Get("cl_showmiss", "0", 0);
    cl_showclamp = Cvar_Get("showclamp", "0", 0);
//#endif

    cl_timeout = Cvar_Get("cl_timeout", "120", 0);

    rcon_address = Cvar_Get("rcon_address", "", CVAR_PRIVATE);
    rcon_address->generator = Com_Address_g;

    cl_disconnectcmd = Cvar_Get("cl_disconnectcmd", "", 0);
    cl_changemapcmd = Cvar_Get("cl_changemapcmd", "", 0);
    cl_beginmapcmd = Cvar_Get("cl_beginmapcmd", "", 0);

    cl_protocol = Cvar_Get("cl_protocol", std::to_string(PROTOCOL_VERSION_POLYHEDRON).c_str(), 0);

    cl_cinematics = Cvar_Get("cl_cinematics", "1", CVAR_ARCHIVE);

    allow_download->changed = cl_allow_download_changed;
    cl_allow_download_changed(allow_download);

    //
    // userinfo
    //
    info_rate = Cvar_Get("rate", "30000", CVAR_USERINFO | CVAR_ARCHIVE);
    info_in_bspmenu = Cvar_Get("in_bspmenu", "0", CVAR_SERVERINFO | CVAR_ROM);
    //dev_maplist = Cvar_Get("dev_maplist", "dev_map_0 dev_map_1 dev_map_2 dev_map_3", CVAR_ARCHIVE);

    //
    // macros
    //
    Cmd_AddMacro("cl_mapname", CL_Mapname_m);
    Cmd_AddMacro("cl_server", CL_Server_m);
    Cmd_AddMacro("cl_timer", CL_Timer_m);
    Cmd_AddMacro("cl_demopos", CL_DemoPos_m);
    Cmd_AddMacro("cl_ups", CL_Ups_m);
    Cmd_AddMacro("cl_fps", CL_Fps_m);
    Cmd_AddMacro("r_fps", R_Fps_m);
    Cmd_AddMacro("cl_mps", CL_Mps_m);   // moves per second
    Cmd_AddMacro("cl_pps", CL_Pps_m);   // packets per second
    Cmd_AddMacro("cl_ping", CL_Ping_m);
    Cmd_AddMacro("cl_lag", CL_Lag_m);
    // N&C: Moved over to the client game.
    //Cmd_AddMacro("cl_health", CL_Health_m);
    //Cmd_AddMacro("cl_ammo", CL_Ammo_m);
    //Cmd_AddMacro("cl_armor", CL_Armor_m);
    //Cmd_AddMacro("cl_weaponmodel", CL_WeaponModel_m);
	Cmd_AddMacro("cl_cluster", CL_Cluster_m);
	Cmd_AddMacro("cl_clusterthere", CL_ClusterThere_m);
	Cmd_AddMacro("cl_lightpolys", CL_NumLightPolys_m);
	Cmd_AddMacro("cl_material", CL_Material_m);
	Cmd_AddMacro("cl_material_override", CL_Material_Override_m);
	Cmd_AddMacro("cl_viewpos", CL_ViewPos_m);
	Cmd_AddMacro("cl_viewdir", CL_ViewDir_m);
	Cmd_AddMacro("cl_hdr_color", CL_HdrColor_m);
	Cmd_AddMacro("cl_resolution_scale", CL_ResolutionScale_m);

    // N&C: Initialize the game progs.
    CL_GM_Init();

    // Fetch CVars that should've been initialized by CG Module.
    cl_player_model = Cvar_Get("cl_player_model", NULL, 0);     // The Vulkan renderer needs this...
}

/*
==================
CL_CheatsOK
==================
*/
qboolean CL_CheatsOK(void)
{
    // can cheat when disconnected or playing a demo
    if (cls.connectionState < ClientConnectionState::Connected || cls.demo.playback)
        return true;

    // can't cheat on remote servers
    if (!sv_running->integer)
        return false;

    // developer option
    if (Cvar_VariableInteger("cheats"))
        return true;

    // single player can cheat
    if (cls.connectionState > ClientConnectionState::Connected && cl.maximumClients == 1)
        return true;

    return false;
}

//============================================================================

/*
==================
CL_Activate
==================
*/
void CL_Activate(active_t active)
{
    if (cls.active != active) {
        Com_DDDPrintf("%s: %u\n", __func__, active);
        cls.active = active;
        cls.disable_screen = 0;
        Key_ClearStates();
        IN_Activate();
        S_Activate();
        CL_UpdateFrameTimes();
    }
}

static void CL_SetClientTime(void)
{
    int prevtime;

    if (com_timedemo->integer) {
        cl.time = cl.serverTime;
        cl.lerpFraction = 1.0f;
        return;
    }

    prevtime = cl.serverTime - CL_FRAMETIME;
    if (cl.time > cl.serverTime) {
        SHOWCLAMP(1, "high clamp %i\n", cl.time - cl.serverTime);
        cl.time = cl.serverTime;
        cl.lerpFraction = 1.0f;
    } else if (cl.time < prevtime) {
        SHOWCLAMP(1, "low clamp %i\n", prevtime - cl.time);
        cl.time = prevtime;
        cl.lerpFraction = 0;
    } else {
        cl.lerpFraction = (cl.time - prevtime) * CL_1_FRAMETIME;
    }

    SHOWCLAMP(2, "time %d %d, lerpFraction %.3f\n",
              cl.time, cl.serverTime, cl.lerpFraction);
}

static void CL_MeasureStats(void)
{
    int i;

    if (com_localTime - cls.measure.time < 1000) {
        return;
    }

    // measure average ping
    if (cls.netChannel) {
        int ack = cls.netChannel->incomingAcknowledged;
        int ping = 0;
        int j, k = 0;

        i = ack - 16 + 1;
        if (i < cl.initialSequence) {
            i = cl.initialSequence;
        }
        for (j = i; j <= ack; j++) {
            ClientUserCommandHistory *h = &cl.clientCommandHistory[j & CMD_MASK];
            if (h->timeReceived > h->timeSent) {
                ping += h->timeReceived - h->timeSent;
                k++;
            }
        }

        cls.measure.ping = k ? ping / k : 0;
    }

    // measure main/refresh frame counts
    for (i = 0; i < 4; i++) {
        cls.measure.fps[i] = cls.measure.frames[i];
        cls.measure.frames[i] = 0;
    }

    cls.measure.time = com_localTime;
}

#if USE_AUTOREPLY
static void CL_CheckForReply(void)
{
    if (!cl.replyDelta) {
        return;
    }

    if (cls.realtime - cl.replyTime < cl.replyDelta) {
        return;
    }

    CL_ClientCommand(va("say \"%s\"", com_version->string));

    cl.replyDelta = 0;
}
#endif

static void CL_CheckTimeout(void)
{
    unsigned delta;

    if (NET_IsLocalAddress(&cls.netChannel->remoteNetAddress)) {
        return;
    }

#if USE_ICMP
    if (cls.errorReceived) {
        delta = 5000;
        if (com_localTime - cls.netChannel->lastReceivedTime > delta)  {
            Com_Error(ERR_DISCONNECT, "Server connection was reset.");
        }
    }
#endif

    delta = cl_timeout->value * 1000;
    if (delta && com_localTime - cls.netChannel->lastReceivedTime > delta)  {
        // timeoutcount saves debugger
        if (++cl.timeoutCount > 5) {
            Com_Error(ERR_DISCONNECT, "Server connection timed out.");
        }
    } else {
        cl.timeoutCount = 0;
    }
}

/*
=================
CL_CheckForPause

=================
*/
void CL_CheckForPause(void)
{
    if (cls.connectionState != ClientConnectionState::Active) {
        // only pause when active
        Cvar_Set("cl_paused", "0");
        Cvar_Set("sv_paused", "0");
        return;
    }

    if (cls.key_dest & (KEY_CONSOLE | KEY_MENU)) {
        // only pause in single player and not in our mainmenu.bsp mode.
        if (cl_paused->integer == 0 && (!CL_InBSPMenu())) {
            Cvar_Set("cl_paused", "1");
			OGG_TogglePlayback();
        }
    } else if (cl_paused->integer == 1) {
        // only resume after automatic pause
        Cvar_Set("cl_paused", "0");
		OGG_TogglePlayback();
    }

    // hack for demo playback pause/unpause
    if (cls.demo.playback) {
        // don't pause when running timedemo!
        if (cl_paused->integer && !com_timedemo->integer) {
            if (!sv_paused->integer) {
                Cvar_Set("sv_paused", "1");
                IN_Activate();
            }
        } else {
            if (sv_paused->integer) {
                Cvar_Set("sv_paused", "0");
                IN_Activate();
            }
        }
    }
}

typedef enum {
    SYNC_TIMEDEMO,
    SYNC_MAXFPS,
    SYNC_SLEEP_20,
    SYNC_SLEEP_60,
    ASYNC_FULL
} sync_mode_t;

#ifdef _DEBUG
static const char* const sync_names[] = {
    "SYNC_TIMEDEMO",
    "SYNC_MAXFPS",
    "SYNC_SLEEP_20",
    "SYNC_SLEEP_60",
    "ASYNC_FULL"
};
#endif

static int ref_msec, phys_msec, main_msec;
static int ref_extra, phys_extra, main_extra;
static sync_mode_t sync_mode;

#define MIN_PHYS_HZ 20
#define MAX_PHYS_HZ 125
#define MIN_REF_HZ MIN_PHYS_HZ
#define MAX_REF_HZ 1000

static int fps_to_clamped_msec(cvar_t* cvar, int min, int max) {
    if (cvar->integer == 0)
        return fps_to_msec(max);
    else
        return fps_to_msec(Cvar_ClampInteger(cvar, min, max));
}

/*
==================
CL_UpdateFrameTimes

Called whenever async/fps cvars change, but not every frame
==================
*/
extern cvar_t *cl_renderdemo;
void CL_UpdateFrameTimes(void)
{
    if (!cls.connectionState) {
        return; // not yet fully initialized
    }

    phys_msec = ref_msec = main_msec = 0;
    ref_extra = phys_extra = main_extra = 0;

    if (com_timedemo->integer) {
        // timedemo just runs at full speed
        sync_mode = SYNC_TIMEDEMO;
    }
    else if (cls.active == ACT_MINIMIZED) {
        // run at 20 fps if minimized
        main_msec = fps_to_msec(20);
        sync_mode = SYNC_SLEEP_20;
    }
    else if (cls.active == ACT_RESTORED || cls.connectionState != ClientConnectionState::Active) {
        // run at 60 fps if not active
        main_msec = fps_to_msec(60);
        sync_mode = SYNC_SLEEP_60;
    }
    else if (cl_async->integer > 0) {
        // run physics and refresh separately
        phys_msec = fps_to_clamped_msec(cl_maxfps, MIN_PHYS_HZ, MAX_PHYS_HZ);
        ref_msec = fps_to_clamped_msec(r_maxfps, MIN_REF_HZ, MAX_REF_HZ);
        sync_mode = ASYNC_FULL;
    }
    else {
        // everything ticks in sync with refresh
        main_msec = fps_to_clamped_msec(cl_maxfps, MIN_PHYS_HZ, MAX_PHYS_HZ);
        sync_mode = SYNC_MAXFPS;
    }

    Com_DDPrintf("%s: mode=%s main_msec=%d ref_msec=%d, phys_msec=%d\n",
        __func__, sync_names[sync_mode], main_msec, ref_msec, phys_msec);

}

/*
==================
CL_Frame

==================
*/
unsigned int totaltime = 0;
unsigned int lasttime = 0;
unsigned CL_Frame(unsigned msec)
{
    qboolean phys_frame = true, ref_frame = true;

    time_after_ref = time_before_ref = 0;

    if (!cl_running->integer) {
        return UINT_MAX;
    }

    main_extra += msec;
    cls.realtime += msec;

    CL_ProcessEvents();

    ref_frame = phys_frame = true;
    switch (sync_mode) {
    case SYNC_TIMEDEMO:
        // timedemo just runs at full speed
        break;
    case SYNC_SLEEP_20:
        // don't run refresh at all
        ref_frame = false;
        // fall through
    case SYNC_SLEEP_60:
        // run at limited fps if not active
        if (main_extra < main_msec) {
            return main_msec - main_extra;
        }
        break;
    case ASYNC_FULL:
        // run physics and refresh separately
        phys_extra += msec;
        ref_extra += msec;

        if (phys_extra < phys_msec) {
            phys_frame = false;
        }
        else if (phys_extra > phys_msec * 4) {
            phys_extra = phys_msec;
        }

        if (ref_extra < ref_msec) {
            ref_frame = false;
        }
        else if (ref_extra > ref_msec * 4) {
            ref_extra = ref_msec;
        }
        // Return immediately if neither physics or refresh are scheduled
        if (!phys_frame && !ref_frame) {
            return min(phys_msec - phys_extra, ref_msec - ref_extra);
        }
        break;
    case SYNC_MAXFPS:
        // everything ticks in sync with refresh
        if (main_extra < main_msec) {
            if (!cl.sendPacketNow) {
                return 0;
            }
            ref_frame = false;
        }
        break;
    }

	if (cls.demo.playback && cl_renderdemo->integer && cl_paused->integer != 2)
		main_extra = main_msec;

    Com_DDDDPrintf("main_extra=%d ref_frame=%d ref_extra=%d "
                   "phys_frame=%d phys_extra=%d\n",
                   main_extra, ref_frame, ref_extra,
                   phys_frame, phys_extra);

    // Decide the simulation time
    cls.frameTime = main_extra * 0.001f;

    if (cls.frameTime > 1.0 / 5)
        cls.frameTime = 1.0 / 5;

	if (!sv_paused->integer && !(cls.demo.playback && cl_renderdemo->integer && cl_paused->integer == 2)) {
        cl.time += main_extra;
    }

    // Read next demo frame
    if (cls.demo.playback)
        CL_DemoFrame(main_extra);

    // Calculate local time
	if (cls.connectionState == ClientConnectionState::Active && !sv_paused->integer && !(cls.demo.playback && cl_renderdemo->integer && cl_paused->integer == 2))
        CL_SetClientTime();

#if USE_AUTOREPLY
    // Check for version reply
    CL_CheckForReply();
#endif

    // Resend a connection request if necessary
    CL_CheckForResend();

    // Read user intentions
    CL_UpdateCmd(main_extra);

    // Finalize pending cmd
    phys_frame |= cl.sendPacketNow;
    if (phys_frame) {
        CL_FinalizeCmd();
        phys_extra -= phys_msec;
        M_FRAMES++;

        // Don't let the time go too far off
        // this can happen due to cl.sendPacketNow
        if (phys_extra < -phys_msec * 4) {
            phys_extra = 0;
        }
    }

	if (cls.demo.playback && cl_renderdemo->integer && cl_paused->integer != 2)
	{
		Cvar_Set("cl_paused", "2");
		CL_CheckForPause();
	}

    // Send pending clientUserCommands
    CL_SendCmd();

    // Predict all unacknowledged movements
    CL_PredictMovement();

    Con_RunConsole();

    // Update RMLUI
    RMLUI_UpdateFrame();

    UI_Frame(main_extra);

    if (ref_frame) {
        // Update the screen
        if (host_speeds->integer)
            time_before_ref = Sys_Milliseconds();

        SCR_UpdateScreen();

        if (host_speeds->integer)
            time_after_ref = Sys_Milliseconds();

        ref_extra -= ref_msec;
        R_FRAMES++;

run_fx:
        // Update audio after the 3D view was drawn
        S_Update();

        // Advance local game effects for next frame
        CL_GM_ClientFrame();

        SCR_RunCinematic();
    } else if (sync_mode == SYNC_SLEEP_20) {
        // Force audio and effects update if not rendering
        CL_GM_ClientUpdateOrigin();
        goto run_fx;
    }

    // Check connection timeout
    if (cls.netChannel)
        CL_CheckTimeout();

    C_FRAMES++;

    CL_MeasureStats();

    cls.framecount++;

    main_extra = 0;
    return 0;
}

/*
============
CL_ProcessEvents
============
*/
qboolean CL_ProcessEvents(void)
{
    if (!cl_running->integer) {
        return false;
    }

    CL_RunRefresh();

    IN_Frame();

    NET_GetPackets(NS_CLIENT, CL_PacketEvent);

    // process console and stuffed commands
    Cbuf_Execute(&cmd_buffer);
    Cbuf_Execute(&cl_cmdbuf);

    HTTP_RunDownloads();

    return cl.sendPacketNow;
}

//============================================================================

/*
====================
CL_Init
====================
*/
void CL_Init(void)
{
    if (dedicated->integer) {
        return; // nothing running on the client
    }

    if (cl_running->integer) {
        return;
    }

    // all archived variables will now be loaded

    // start with full screen console
    cls.key_dest = KEY_CONSOLE;
    
    // N&C: Load our client game module here.
    CL_InitGameProgs();

#ifdef _WIN32
    CL_InitRefresh();
    S_Init();   // sound must be initialized after window is created
#else
    S_Init();
    CL_InitRefresh();
#endif

    CL_InitLocal();
    IN_Init();

#if USE_ZLIB
    if (inflateInit2(&cls.z, -MAX_WBITS) != Z_OK) {
        Com_Error(ERR_FATAL, "%s: inflateInit2() failed", __func__);
    }
#endif

    CL_LoadDownloadIgnores();

    HTTP_Init();

    // Initialize RMLUI
    RMLUI_Init();

    Con_PostInit();
    Con_RunConsole();

    cl_cmdbuf.from = FROM_STUFFTEXT;
    cl_cmdbuf.text = cl_cmdbuf_text;
    cl_cmdbuf.maximumSize = sizeof(cl_cmdbuf_text);
    cl_cmdbuf.exec = exec_server_string;

    Cvar_Set("cl_running", "1");

    // Open mainmenu map.
    CL_LoadBSPMenuMap();
}

/*
===============
CL_Shutdown

FIXME: this is a callback from Com_Quit and Com_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void CL_Shutdown(void)
{
    static qboolean isdown = false;

    if (isdown) {
        Com_Printf("CL_Shutdown: recursive shutdown\n");
        return;
    }
    isdown = true;

    if (!cl_running || !cl_running->integer) {
        return;
    }

    // Shutdown the RMLUI
    RMLUI_Shutdown();

    // N&C: Notify the CG Module.
    CL_GM_Shutdown();

    CL_Disconnect(ERR_FATAL);

#if USE_ZLIB
    inflateEnd(&cls.z);
#endif

    HTTP_Shutdown();

    S_Shutdown();  
    IN_Shutdown();
    Con_Shutdown();
    
    CL_ShutdownRefresh();
    // N&C: Unload the client game dll.
    CL_ShutdownGameProgs();

    CL_WriteConfig();

    memset(&cls, 0, sizeof(cls));

    Cvar_Set("cl_running", "0");

    isdown = false;
}

