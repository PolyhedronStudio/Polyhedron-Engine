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
// server.h

#include "shared/shared.h"
#include "shared/list.h"
#include "shared/svgame.h"

#include "common/bsp.h"
#include "common/cmd.h"
#include "common/cmodel.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/error.h"
#include "common/files.h"
#include "common/msg.h"
#include "common/net/net.h"
#include "common/net/chan.h"
#include "common/prompt.h"
#include "common/protocol.h"
#include "common/zone.h"

#include "client/client.h"
#include "server/server.h"
#include "system/system.h"

#if USE_ZLIB
#include <zlib.h>
#endif

//=============================================================================

#define SV_Malloc(size)         Z_TagMalloc(size, TAG_SERVER)
#define SV_Mallocz(size)        Z_TagMallocz(size, TAG_SERVER)
#define SV_CopyString(s)        Z_TagCopyString(s, TAG_SERVER)
#define SV_LoadFile(path, buf)  FS_LoadFileEx(path, buf, 0, TAG_SERVER)
#define SV_FreeFile(buf)        Z_Free(buf)

#ifdef _DEBUG
#define SV_DPrintf(level,...) \
    if (sv_debug && sv_debug->integer > level) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#else
#define SV_DPrintf(...)
#endif

#if USE_CLIENT
#define SV_PAUSED (sv_paused->integer != 0)
#else
#define SV_PAUSED 0
#endif

// Cheesy macro.
#define SV_InfoSet(var, val) \
    Cvar_FullSet(var, val, CVAR_SERVERINFO|CVAR_ROM, FROM_CODE)


//=============================================================================
#define EDICT_POOL(c, n) ((entity_t *)((byte *)(c)->pool->edicts + (c)->pool->entity_size*(n)))

#define EDICT_NUM(n) ((entity_t *)((byte *)ge->edicts + ge->entity_size*(n)))
#define NUM_FOR_EDICT(e) ((int)(((byte *)(e) - (byte *)ge->edicts) / ge->entity_size))


//=============================================================================
// Master/heartbeat settings.
static constexpr uint32_t MAX_MASTERS = 8;       // max recipients for heartbeat packets
static constexpr int32_t HEARTBEAT_SECONDS = 300;

// Baseline settings per packet.
static constexpr uint32_t SV_BASELINES_SHIFT = 6;
static constexpr uint32_t SV_BASELINES_PER_CHUNK = (1 << SV_BASELINES_SHIFT);
static constexpr uint32_t SV_BASELINES_MASK = (SV_BASELINES_PER_CHUNK - 1);
static constexpr uint32_t SV_BASELINES_CHUNKS = (MAX_EDICTS >> SV_BASELINES_SHIFT);

// Server FPS
constexpr uint32_t SV_FRAMERATE = BASE_FRAMERATE;
constexpr uint32_t SV_FRAMETIME = BASE_FRAMETIME;
constexpr uint32_t SV_FRAMEDIV = 1;
constexpr uint32_t SV_FRAMESYNC = 1;
#define SV_CLIENTSYNC(cl)   1

// Entity leaf settings.
static constexpr uint32_t MAX_TOTAL_ENT_LEAFS = 128;
//=============================================================================

//-----------------
// A client svc_frame message.
//-----------------
typedef struct {
    int         number;
    unsigned    num_entities;
    unsigned    first_entity;
    PlayerState playerState;
    int         clientNumber;
    int         areaBytes;
    byte        areaBits[MAX_MAP_AREA_BYTES];  // portalarea visibility bits
    unsigned    sentTime;                   // for ping calculations
    int         latency;
} ClientFrame;

//-----------------
// Server side Entity.
//-----------------
typedef struct {
    int         solid32;
} server_entity_t;


//-----------------
// Main server structure.
//-----------------
typedef struct {
    int32_t serverState;      // precache commands are only valid during load
    int32_t spawncount; // random number generated each server spawn

    int32_t  frameNumber;
    uint32_t frameResidual;

    char        mapcmd[MAX_QPATH];          // ie: *intro.cin+base

    char        name[MAX_QPATH];            // map name, or cinematic name
    cm_t        cm;
    char        *entityString;

    char        configstrings[ConfigStrings::MaxConfigStrings][MAX_QPATH];

    server_entity_t entities[MAX_EDICTS];

    unsigned    tracecount;
} server_t;


struct ConnectionState {
    static constexpr int32_t Free = 0;      // Can be reused for a new connection
    static constexpr int32_t Zombie = 1;    // Client has been disconnected, but don't reuse
                                            // Connection for a couple seconds
    static constexpr int32_t Assigned = 2;  // Client_t assigned, but no data received from client yet
    static constexpr int32_t Connected = 3; // Netchan fully established, but not in game yet
    static constexpr int32_t Primed = 4;    // Sent serverdata, client is precaching
    static constexpr int32_t Spawned = 5;   // Client is fully in game
};

constexpr uint32_t MSG_POOLSIZE = 1024;
constexpr uint32_t MSG_TRESHOLD = (64 - 10);   // keep pmsg_s 64 bytes aligned

constexpr uint32_t MSG_RELIABLE = 1;
constexpr uint32_t MSG_CLEAR = 2;
constexpr uint32_t MSG_COMPRESS = 4;

constexpr uint32_t MAX_SOUND_PACKET = 14;

//-----------------
// The actual networking message packets.
//-----------------
typedef struct {
    list_t              entry;
    uint16_t            cursize;    // Zero means sound packet
    union {
        uint8_t         data[MSG_TRESHOLD];
        struct {
            uint8_t     flags;
            uint8_t     index;
            uint16_t    sendchan;
            uint8_t     volume;
            uint8_t     attenuation;
            uint8_t     timeofs;
            vec3_t      pos;     // Saved in case entity is freed
        };
    };
} MessagePacket;

// This is best to match the actual server game frame rate.
static constexpr uint32_t SERVER_MESSAGES_TICKRATE = 20;

#define FOR_EACH_CLIENT(client) \
    LIST_FOR_EACH(client_t, client, &sv_clientlist, entry)

#define PL_S2C(cl) (cl->framesSent ? \
    (1.0f - (float)cl->framesAcknowledged / cl->framesSent) * 100.0f : 0.0f)
#define PL_C2S(cl) (cl->netchan->totalReceived ? \
    ((float)cl->netchan->totalDropped / cl->netchan->totalReceived) * 100.0f : 0.0f)
#define AVG_PING(cl) (cl->averagePingCount ? \
    cl->averagePingTime / cl->averagePingCount : cl->ping)

typedef struct {
    unsigned    time;
    unsigned    credit;
    unsigned    credit_cap;
    unsigned    cost;
} RateLimit;

typedef struct client_s {
    list_t entry;

    // core info
    int32_t connectionState;
    entity_t *edict;     // EDICT_NUM(clientnum+1)
    int number;     // client slot number

    // client flags
    qboolean reconnected: 1;
    qboolean nodata: 1;
    qboolean has_zlib: 1;
    qboolean drop_hack: 1;
#if USE_ICMP
    qboolean unreachable: 1;
#endif
    qboolean http_download: 1;

    // userinfo
    char userinfo[MAX_INFO_STRING];  // name, etc
    char name[MAX_CLIENT_NAME];      // extracted from userinfo, high bits masked
    int32_t messageLevel;               // for filtering printed messages
    size_t rate;
    RateLimit ratelimitNameChange;       // for suppressing "foo changed name" flood

    // console var probes
    char *versionString;
    char reconnectKey[16];
    char reconnectValue[16];
    int32_t consoleQueries;

    // usercmd stuff
    uint32_t lastMessage;    // svs.realTime when packet was last received
    uint32_t lastActivity;   // svs.realTime when user activity was last seen
    int32_t lastFrame;      // for delta compression
    ClientUserCommand lastClientUserCommand;        // for filling in big drops
    int32_t clientUserCommandMiliseconds;   // every seconds this is reset, if user
                                    // commands exhaust it, assume time cheating
    int32_t numberOfMoves;      // reset every 10 seconds
    int32_t movesPerSecond;  // average movement FPS

    // Networking PING.
    int32_t ping;
    int32_t pingMinimum;
    int32_t pingMaximum;

    // Networking averages.
    int32_t averagePingTime;
    int32_t averagePingCount;

    // frame encoding
    ClientFrame frames[UPDATE_BACKUP];    // Updates can be delta'd from here
    uint32_t framesSent;
    uint32_t framesAcknowledged;
    uint32_t framesNoDelta;
    int32_t frameNumber;

    uint32_t frameFlags;

    // rate dropping
    size_t messageSizes[SERVER_MESSAGES_TICKRATE]; // Used to rate drop normal packets
    int32_t suppressCount; // Number of messages rate suppressed
    
    // Used to rate drop async packets
    uint32_t sendTime;
    uint32_t sendDelta;

    // current download
    struct {
        int32_t fileSize;   // total bytes (can't use EOF because of paks)
        char *fileName;  // name of the file

        byte *bytes;     // file being downloaded
        int32_t bytesSent;  // bytes sent

        int32_t command;    // svc_(z)download

        qboolean isPending;
    } download;

    // protocol stuff
    int32_t challenge;  // Challenge of this user, randomly generated
    int32_t protocol;   // Major version
    int32_t version;    // Minor version

    EntityStateMessageFlags esFlags; // Entity protocol flags

    // packetized messages
    list_t msg_free_list;
    list_t msg_unreliable_list;
    list_t msg_reliable_list;
    MessagePacket *msg_pool;
    size_t msg_unreliable_bytes;   // total size of unreliable datagram
    size_t msg_dynamic_bytes;      // total size of dynamic memory allocated

    // per-client baseline chunks
    PackedEntity *entityBaselines[SV_BASELINES_CHUNKS];

    // server state pointers (hack for MVD channels implementation)
    char *configstrings;
    char *gamedir, *mapname;
    EntityPool *pool;
    cm_t *cm;
    int32_t slot;
    int32_t spawncount;
    int32_t maxClients;

    // netchan type dependent methods
    void (*AddMessage)(struct client_s *, byte *, size_t, qboolean);
    void (*WriteFrame)(struct client_s *);
    void (*WriteDatagram)(struct client_s *);

    // netchan
    netchan_t *netchan;
    int32_t numpackets; // for that nasty packetdup hack

    // misc
    time_t timeOfInitialConnect; // time of initial connect
	int32_t lastValidCluster;
} client_t;

// a client can leave the server in one of four ways:
// dropping properly by quiting or disconnecting
// timing out if no valid messages are received for timeout.value seconds
// getting kicked off by the server operator
// a program error, like an overflowed reliable buffer

//=============================================================================

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
static constexpr uint32_t    MAX_CHALLENGES = 1024;

typedef struct {
    netadr_t adr;
    uint32_t challenge;
    uint32_t time;
} Challenge;

typedef struct {
    list_t      entry;
    netadr_t    addr;
    netadr_t    mask;
    uint32_t    hits;
    time_t      time;   // time of the last hit
    char        comment[1];
} AddressMatch;

typedef struct {
    list_t  entry;
    int32_t len;
    char    string[1];
} StuffTextCommand;

typedef enum {
    FA_IGNORE,
    FA_PRINT,
    FA_STUFF,
    FA_KICK,

    FA_MAX
} FilterAction;

typedef struct {
    list_t entry;
    
    FilterAction action;

    char *comment;
    char  string[1];
} FilterCommand;

typedef struct {
    list_t entry;
    netadr_t adr;

    uint32_t last_ack;
    time_t last_resolved;

    char name[1];
} master_t;

typedef struct {
    char            buffer[MAX_QPATH];
    char            *server;
    char            *spawnpoint;
    int32_t         serverState;
    int32_t         loadgame;
    qboolean        endofunit;
    cm_t            cm;
} MapCommand;

#define FOR_EACH_MASTER(m) \
    LIST_FOR_EACH(master_t, m, &sv_masterlist, entry)
#define FOR_EACH_MASTER_SAFE(m, n) \
    LIST_FOR_EACH_SAFE(master_t, m, n, &sv_masterlist, entry)

typedef struct server_static_s {
    qboolean    initialized;        // sv_init has completed
    unsigned    realtime;           // always increasing, no clamping, etc

    client_t    *client_pool;   // [maxClients]

    unsigned        num_entities;   // maxClients*UPDATE_BACKUP*MAX_PACKET_ENTITIES
    unsigned        next_entity;    // next state to use
    PackedEntity *entities;      // [num_entities]

#if USE_ZLIB
    z_stream        z;  // for compressing messages at once
#endif

    unsigned        last_heartbeat;

    RateLimit     ratelimit_status;
    RateLimit     ratelimit_auth;
    RateLimit     ratelimit_rcon;

    Challenge     challenges[MAX_CHALLENGES]; // to prevent invalid IPs from connecting
} ServerStatic;

//=============================================================================

extern list_t      sv_masterlist; // address of the master server
extern list_t      sv_banlist;
extern list_t      sv_blacklist;
extern list_t      sv_cmdlist_connect;
extern list_t      sv_cmdlist_begin;
extern list_t      sv_filterlist;
extern list_t      sv_clientlist; // linked list of non-free clients

extern ServerStatic  svs;        // persistant server info
extern server_t      sv;         // local server

extern cvar_t       *sv_hostname;
extern cvar_t       *sv_maxclients;
extern cvar_t       *sv_password;
extern cvar_t       *sv_reserved_slots;
extern cvar_t       *sv_airaccelerate;        // development tool
extern cvar_t       *sv_qwmod;                // atu QW Physics modificator
extern cvar_t       *sv_enforcetime;

extern cvar_t       *sv_force_reconnect;
extern cvar_t       *sv_iplimit;

#ifdef _DEBUG
extern cvar_t       *sv_debug;
extern cvar_t       *sv_pad_packets;
#endif
extern cvar_t       *sv_novis;
extern cvar_t       *sv_lan_force_rate;
extern cvar_t       *sv_calcpings_method;
extern cvar_t       *sv_changemapcmd;

extern cvar_t       *sv_strafejump_hack;
#if USE_PACKETDUP
extern cvar_t       *sv_packetdup_hack;
#endif
extern cvar_t       *sv_allow_map;
#if !USE_CLIENT
extern cvar_t       *sv_recycle;
#endif
extern cvar_t       *sv_enhanced_setplayer;

extern cvar_t       *sv_status_limit;
extern cvar_t       *sv_status_show;
extern cvar_t       *sv_auth_limit;
extern cvar_t       *sv_rcon_limit;
extern cvar_t       *sv_uptime;

extern cvar_t       *sv_allow_unconnected_cmds;

extern cvar_t       *map_override_path;

extern cvar_t       *sv_timeout;
extern cvar_t       *sv_zombietime;
extern cvar_t       *sv_ghostime;

extern client_t     *sv_client;
extern entity_t      *sv_player;

extern qboolean     sv_pending_autosave;


//===========================================================

//
// sv_main.c
//
void SV_DropClient(client_t *drop, const char *reason);
void SV_RemoveClient(client_t *client);
void SV_CleanClient(client_t *client);

void SV_InitOperatorCommands(void);

void SV_UserinfoChanged(client_t *cl);

qboolean SV_RateLimited(RateLimit *r);
void SV_RateRecharge(RateLimit *r);
void SV_RateInit(RateLimit *r, const char *s);

AddressMatch *SV_MatchAddress(list_t *list, netadr_t *address);

int SV_CountClients(void);

#if USE_ZLIB
voidpf SV_zalloc(voidpf opaque, uInt items, uInt size);
void SV_zfree(voidpf opaque, voidpf address);
#endif

//
// sv_init.c
//
void SV_ClientReset(client_t *client);
void SV_SpawnServer(MapCommand *cmd);
qboolean SV_ParseMapCmd(MapCommand *cmd);
void SV_InitGame();

//
// sv_send.c
//
typedef enum {RD_NONE, RD_CLIENT, RD_PACKET} redirect_t;
#define SV_OUTPUTBUF_LENGTH     (MAX_PACKETLEN_DEFAULT - 16)

#define SV_ClientRedirect() \
    Com_BeginRedirect(RD_CLIENT, sv_outputbuf, MAX_STRING_CHARS - 1, SV_FlushRedirect)

#define SV_PacketRedirect() \
    Com_BeginRedirect(RD_PACKET, sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect)

extern char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

void SV_FlushRedirect(int redirected, char *outputbuf, size_t len);

void SV_SendClientMessages(void);
void SV_SendAsyncPackets(void);

void SV_Multicast(const vec3_t *origin, int32_t to);
void SV_ClientPrintf(client_t *cl, int level, const char *fmt, ...) q_printf(3, 4);
void SV_BroadcastPrintf(int level, const char *fmt, ...) q_printf(2, 3);
void SV_ClientCommand(client_t *cl, const char *fmt, ...) q_printf(2, 3);
void SV_BroadcastCommand(const char *fmt, ...) q_printf(1, 2);
void SV_ClientAddMessage(client_t *client, int flags);
void SV_ShutdownClientSend(client_t *client);
void SV_InitClientSend(client_t *newcl);

//
// sv_user.c
//
void SV_New_f(void);
void SV_Begin_f(void);
void SV_ExecuteClientMessage(client_t *cl);
void SV_CloseDownload(client_t *client);

//
// sv_ccmds.c
//

void SV_AddMatch_f(list_t *list);
void SV_DelMatch_f(list_t *list);
void SV_ListMatches_f(list_t *list);
client_t *SV_GetPlayer(const char *s, qboolean partial);
void SV_PrintMiscInfo(void);

//
// sv_ents.c
//

#define ES_INUSE(s) \
    ((s)->modelIndex || (s)->effects || (s)->sound || (s)->event)

void SV_BuildProxyClientFrame(client_t *client);
void SV_BuildClientFrame(client_t *client);
void SV_WriteFrameToClient(client_t *client);

//
// sv_game.c
//
extern    svgame_export_t    *ge;

void SV_InitGameProgs(void);
void SV_ShutdownGameProgs(void);
void SV_InitEntity(entity_t *e);

//void PF_PMove(PlayerMove *pm);

//
// sv_save.c
//
void SV_AutoSaveBegin(MapCommand *cmd);
void SV_AutoSaveEnd(void);
void SV_CheckForSavegame(MapCommand *cmd);
void SV_RegisterSavegames(void);
int SV_NoSaveGames(void);

//============================================================

//
// high level object sorting to reduce interaction tests
//

void SV_ClearWorld(void);
// called after the world model has been loaded, before linking any entities

void PF_UnlinkEntity(entity_t *ent);
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself

void SV_LinkEntity(cm_t *cm, entity_t *ent);
void PF_LinkEntity(entity_t *ent);
// Needs to be called any time an entity changes origin, mins, maxs,
// or solid.  Automatically unlinks if needed.
// sets ent->v.absMin and ent->v.absMax
// sets ent->leafnums[] for pvs determination even if the entity
// is not solid

int SV_AreaEntities(const vec3_t &mins, const vec3_t &maxs, entity_t **list, int maxcount, int areatype);
// fills in a table of edict pointers with edicts that have bounding boxes
// that intersect the given area.  It is possible for a non-axial bmodel
// to be returned that doesn't actually intersect the area on an exact
// test.
// returns the number of pointers filled in
// ??? does this always return the world?

qboolean SV_EntityIsVisible(cm_t *cm, entity_t *ent, byte *mask);

//===================================================================

//
// functions that interact with everything apropriate
//
int SV_PointContents(const vec3_t &p);
// returns the CONTENTS_* value from the world at the given point.
// Quake 2 extends this to also check entities, to allow moving liquids

trace_t q_gameabi SV_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end,
                           entity_t *passedict, int contentmask);
// mins and maxs are relative

// if the entire move stays in a solid volume, trace.allSolid will be set,
// trace.startSolid will be set, and trace.fraction will be 0

// if the starting point is in a solid, it will be allowed to move out
// to an open area

// passedict is explicitly excluded from clipping checks (normally NULL)

