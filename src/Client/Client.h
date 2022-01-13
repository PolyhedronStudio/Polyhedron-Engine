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

// client.h -- primary header for client

#include "Shared/Shared.h"
#include "Shared/list.h"

#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CModel.h"
#include "Common/Common.h"
#include "Common/CVar.h"
#include "Common/Field.h"
#include "Common/Files.h"
//#include "Common/pmove.h"
#include "Common/Msg.h"
#include "Common/Net/NetChan.h"
#include "Common/Net/Net.h"
#include "Common/Prompt.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"
#include "Common/Zone.h"

#include "System/System.h"
#include "refresh/refresh.h"
#include "Server/Server.h"

#include "Client/Client.h"
#include "Client/Input.h"
#include "Client/Keys.h"
#include "Client/Sound/Sound.h"
#include "Client/UI.h"
#include "Client/Video.h"

// Shared Game includes.
#include "SharedGame/Protocol.h" // PMOVE: Remove once the game modules init pmove themselves using CLG_ParseServerData.

#if USE_ZLIB
#include <zlib.h>
#endif


//=============================================================================

// N&C: Most structures related to the client have been moved over here.
// They are shared to the client game dll, since it is tightly coupled.
#include "Shared/cltypes.h"

// N&C: TODO: REMOVE ONCE ALL OF THIS HAS MOVED TO THE GAME MODULE.
extern explosion_t  cl_explosions[MAX_EXPLOSIONS];
extern cl_entity_t  cl_entities[MAX_EDICTS];

extern    ClientState    cl;
extern    ClientShared   cs;

//=============================================================================
// 
// Client Specific FPS
// 
//=============================================================================
static constexpr double CL_FRAMETIME = BASE_FRAMETIME;
static constexpr double CL_1_FRAMETIME = BASE_1_FRAMETIME;
static constexpr int32_t CL_FRAMEDIV = BASE_FRAMERATE / 10.0;
inline qboolean CL_FRAMESYNC() {
    return !(cl.frame.number % CL_FRAMEDIV);
}

/*
==================================================================

the ClientStatic structure is persistant through an arbitrary number
of server connections

==================================================================
*/

// resend delay for challenge/connect packets
#define CONNECT_DELAY       3000u

#define CONNECT_INSTANT     CONNECT_DELAY
#define CONNECT_FAST        (CONNECT_DELAY - 1000u)

// Silly macros for download Queue.
#define FOR_EACH_DLQ(q) \
    LIST_FOR_EACH(dlqueue_t, q, &cls.download.queue, entry)
#define FOR_EACH_DLQ_SAFE(q, n) \
    LIST_FOR_EACH_SAFE(dlqueue_t, q, n, &cls.download.queue, entry)

typedef enum {
    // generic types
    DL_OTHER,
    DL_MAP,
    DL_MODEL,
#if USE_CURL
    // special types
    DL_LIST,
    DL_PAK
#endif
} dltype_t;

typedef enum {
    DL_PENDING,
    DL_RUNNING,
    DL_DONE
} dlstate_t;

typedef struct {
    list_t      entry;
    dltype_t    type;
    dlstate_t   state;
    char        path[1];
} dlqueue_t;

struct ClientStatic {
    int32_t    connectionState;
    keydest_t   key_dest;

    active_t    active;

    qboolean    ref_initialized;
    unsigned    disable_screen;

    int         userinfo_modified;
    cvar_t* userinfo_updates[MAX_PACKET_USERINFOS];
    // this is set each time a CVAR_USERINFO variable is changed
    // so that the client knows to send it to the server

    int         framecount;
    unsigned    realtime;           // always increasing, no clamping, etc
    float       frameTime;          // seconds since last frame

// preformance measurement
#define C_FPS   cls.measure.fps[0]
#define R_FPS   cls.measure.fps[1]
#define C_MPS   cls.measure.fps[2]
#define C_PPS   cls.measure.fps[3]
#define C_FRAMES    cls.measure.frames[0]
#define R_FRAMES    cls.measure.frames[1]
#define M_FRAMES    cls.measure.frames[2]
#define P_FRAMES    cls.measure.frames[3]
    struct {
        unsigned    time;
        int         frames[4];
        int         fps[4];
        int         ping;
    } measure;

    // connection information
    NetAdr    serverAddress;
    char        servername[MAX_OSPATH]; // name of server from original connect
    unsigned    timeOfInitialConnect;           // for connection retransmits
    int         connect_count;
    qboolean    passive;

#if USE_ZLIB
    z_stream    z;
#endif

    int         quakePort;          // a 16 bit value that allows quake servers
                                    // to work around address translating routers
    NetChannel* netChannel;
    int         serverProtocol;     // in case we are doing some kind of version hack
    int         protocolVersion;    // minor version

    int         challenge;          // from the server to use for connecting

#if USE_ICMP
    qboolean    errorReceived;  // got an ICMP error from server
#endif

#define RECENT_ADDR 4
#define RECENT_MASK (RECENT_ADDR - 1)

    NetAdr    recent_addr[RECENT_ADDR];
    int         recent_head;

    struct {
        list_t      queue;              // queue of paths we need
        int         pending;            // number of non-finished entries in queue
        dlqueue_t* current;           // path being downloaded
        int         percent;            // how much downloaded
        int         position;           // how much downloaded (in bytes)
        qhandle_t   file;               // UDP file transfer from server
        char        temp[MAX_QPATH + 4];// account 4 bytes for .tmp suffix
#if USE_ZLIB
        z_stream    z;                  // UDP download zlib stream
#endif
        string_entry_t* ignores;       // list of ignored paths
    } download;

    // demo recording info must be here, so it isn't cleared on level change
    struct {
        qhandle_t   playback;
        qhandle_t   recording;
        unsigned    time_start;
        unsigned    time_frames;
        int         last_server_frame;  // number of server frame the last svc_frame was written
        int         frames_written;     // number of frames written to demo file
        int         frames_dropped;     // number of svc_frames that didn't fit
        int         others_dropped;     // number of misc svc_* messages that didn't fit
        int         frames_read;        // number of frames read from demo file
        int         last_snapshot;      // number of demo frame the last snapshot was saved
        int         file_size;
        int         file_offset;
        int         file_percent;
        SizeBuffer   buffer;
        list_t      snapshots;
        qboolean    paused;
        qboolean    seeking;
        qboolean    eof;
        char		file_name[MAX_OSPATH];
    } demo;

};

extern ClientStatic    cls;
extern cmdbuf_t    cl_cmdbuf;
extern char        cl_cmdbuf_text[MAX_STRING_CHARS];

//=============================================================================

//
// cvars
//
extern cvar_t    *cl_gun;

extern cvar_t    *cl_predict;
extern cvar_t    *cl_footsteps;
extern cvar_t    *cl_noskins;
extern cvar_t    *cl_kickangles;
extern cvar_t    *cl_rollhack;
extern cvar_t    *cl_noglow;
extern cvar_t    *cl_nolerp;

#ifdef _DEBUG
#define SHOWNET(level, ...) \
    if (cl_shownet->integer > level) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#define SHOWCLAMP(level, ...) \
    if (cl_showclamp->integer > level) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#define SHOWMISS(...) \
    if (cl_showmiss->integer) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
extern cvar_t    *cl_shownet;
extern cvar_t    *cl_showmiss;
extern cvar_t    *cl_showclamp;
#else
#define SHOWNET(...)
#define SHOWCLAMP(...)
#define SHOWMISS(...)
#endif

extern cvar_t    *cl_vwep;

//extern cvar_t    *cl_disable_particles;
//extern cvar_t    *cl_disable_explosions;
extern cvar_t    *cl_explosion_sprites;
extern cvar_t    *cl_explosion_frametime;

extern cvar_t    *cl_chat_notify;
extern cvar_t    *cl_chat_sound;
extern cvar_t    *cl_chat_filter;

extern cvar_t    *cl_disconnectcmd;
extern cvar_t    *cl_changemapcmd;
extern cvar_t    *cl_beginmapcmd;

#define CL_PLAYER_MODEL_DISABLED     0
#define CL_PLAYER_MODEL_ONLY_GUN     1
#define CL_PLAYER_MODEL_FIRST_PERSON 2
#define CL_PLAYER_MODEL_THIRD_PERSON 3

extern cvar_t    *cl_player_model;
extern cvar_t    *cl_thirdperson_angle;
extern cvar_t    *cl_thirdperson_range;

extern cvar_t    *cl_async;

//
// userinfo
//
extern cvar_t    *info_password;
extern cvar_t    *info_spectator;
extern cvar_t    *info_name;
extern cvar_t    *info_skin;
extern cvar_t    *info_rate;
extern cvar_t    *info_fov;
extern cvar_t    *info_msg;
extern cvar_t    *info_hand;
extern cvar_t    *info_uf;
extern cvar_t    *info_in_bspmenu;

//=============================================================================

//
// console.c
//
#define CON_TIMES       16
#define CON_TIMES_MASK  (CON_TIMES - 1)

#define CON_TOTALLINES          1024    // total lines in console scrollback
#define CON_TOTALLINES_MASK     (CON_TOTALLINES - 1)

#define CON_LINEWIDTH   100     // fixed width, do not need more

typedef enum {
    CHAT_NONE,
    CHAT_DEFAULT,
    CHAT_TEAM
} chatMode_t;

typedef enum {
    CON_POPUP,
    CON_DEFAULT,
    CON_CHAT,
    CON_REMOTE
} consoleMode_t;

typedef struct console_s {
    qboolean    initialized;

    char    text[CON_TOTALLINES][CON_LINEWIDTH];
    int     current;        // line where next message will be printed
    int     x;              // offset in current line for next print
    int     display;        // bottom of console displays this line
    int     color;
    int     newline;

    int     linewidth;      // characters across screen
    int     vidWidth, vidHeight;
    float   scale;

    unsigned    times[CON_TIMES];   // cls.realtime time the line was generated
                                    // for transparent notify lines
    qboolean    skipNotify;

    qhandle_t   backImage;
    qhandle_t   charsetImage;

    float   currentHeight;  // aproaches scr_conlines at scr_conspeed
    float   destHeight;     // 0.0 to 1.0 lines of console to display

    commandPrompt_t chatPrompt;
    commandPrompt_t prompt;

    chatMode_t chat;
    consoleMode_t mode;
    NetAdr remoteNetAddress;
    char *remotePassword;

    LoadState loadstate;
} console_t;
extern console_t con;
//=============================================================================

//
// main.c
//

void CL_Init(void);
void CL_Quit_f(void);
void CL_Disconnect(ErrorType type);
void CL_Begin(void);
void CL_CheckForResend(void);
void CL_ClearState(void);
void CL_RestartFilesystem(qboolean total);
void CL_RestartRefresh(qboolean total);
void CL_ClientCommand(const char *string);
void CL_SendRcon(const NetAdr *adr, const char *pass, const char *cmd);
void CL_CheckForPause(void);
void CL_UpdateFrameTimes(void);
qboolean CL_CheckForIgnore(const char* s);
void CL_WriteConfig(void);
uint32_t    CL_GetConnectionState (void);               // WATISDEZE Added for CG Module.
void        CL_SetConnectionState (uint32_t state);     // WATISDEZE Added for CG Module.
void        CL_SetLoadState (LoadState state);          // WATISDEZE Added for CG Module.

qboolean      CL_InBSPMenu();
void          CL_LoadBSPMenuMap(qboolean force);
void          CL_OpenBSPMenu();
void          CL_CloseBSPMenu();

//
// precache.c
//
void CL_ParsePlayerSkin(char *name, char *model, char *skin, const char *s);
void CL_LoadState(LoadState state);
void CL_RegisterBspModels(void);
void CL_PrepareMedia(void);
void CL_UpdateConfigstring(int index);


//
// download.c
//
qerror_t CL_QueueDownload(const char *path, dltype_t type);
qboolean CL_IgnoreDownload(const char *path);
void CL_FinishDownload(dlqueue_t *q);
void CL_CleanupDownloads(void);
void CL_LoadDownloadIgnores(void);
void CL_HandleDownload(byte *data, int size, int percent, int compressed);
qboolean CL_CheckDownloadExtension(const char *ext);
void CL_StartNextDownload(void);
void CL_RequestNextDownload(void);
void CL_ResetPrecacheCheck(void);
void CL_InitDownloads(void);


//
// input.c
//
void IN_Init(void);
void IN_Shutdown(void);
void IN_Frame(void);
void IN_Activate(void);
qboolean CL_GetMouseMotion(int* deltaX, int* deltaY);

void CL_RegisterInput(void);
void CL_UpdateCmd(int msec);
void CL_FinalizeCmd(void);
void CL_SendCmd(void);


//
// parse.c
//

extern tent_params_t    te;
extern mz_params_t      mz;
extern snd_params_t     snd;

void CL_ParseServerMessage(void);
void CL_SeekDemoMessage(void);


//
// entities.c
//
void CL_DeltaFrame(void);

#ifdef _DEBUG
void CL_CheckEntityPresent(int entnum, const char *what);
#endif

// the sound code makes callbacks to the client for entitiy position
// information, so entities can be dynamically re-spatialized
vec3_t CL_GetEntitySoundOrigin(int ent);
vec3_t CL_GetViewVelocity(void);

//
// view.c
//
extern    int       gun_frame;
extern    qhandle_t gun_model;

void V_Init(void);
void V_Shutdown(void);
void V_RenderView(void);
//void V_AddEntity(r_entity_t *ent);
//void V_AddParticle(rparticle_t *p);
//#if USE_DLIGHTS
//void V_AddLight(const vec3_t &org, float intensity, float r, float g, float b);
//void V_AddLightEx(const vec3_t& org, float intensity, float r, float g, float b, float radius);
//#else
//#define V_AddLight(org, intensity, r, g, b)
//#define V_AddLightEx(org, intensity, r, g, b, radius)
//#endif
#if USE_LIGHTSTYLES
void V_AddLightStyle(int style, const vec4_t &value);
#endif

//
// predict.c
//
void CL_PredictMovement(void);
void CL_CheckPredictionError(void);

//
// demo.c
//
void CL_InitDemos(void);
void CL_CleanupDemos(void);
void CL_DemoFrame(int msec);
qboolean CL_WriteDemoMessage(SizeBuffer *buf);
void CL_EmitDemoFrame(void);
void CL_EmitDemoSnapshot(void);
void CL_FirstDemoFrame(void);
void CL_Stop_f(void);
demoInfo_t *CL_GetDemoInfo(const char *path, demoInfo_t *info);


//
// locs.c
//
void LOC_Init(void);
void LOC_LoadLocations(void);
void LOC_FreeLocations(void);
void LOC_UpdateCvars(void);
void LOC_AddLocationsToScene(void);


//
// console.c
//
void Con_Init(void);
void Con_PostInit(void);
void Con_Shutdown(void);
void Con_DrawConsole(void);
void Con_RunConsole(void);
void Con_Print(const char *txt);
void Con_ClearNotify_f(void);
void Con_ToggleConsole_f(void);
void Con_ClearTyping(void);
void Con_Close(qboolean force);
void Con_Popup(qboolean force);
void Con_SkipNotify(qboolean skip);
void Con_RegisterMedia(void);
void Con_CheckResize(void);

void Key_Console(int key);
void Key_Message(int key);
void Char_Console(int key);
void Char_Message(int key);


//
// refresh.c
//
void    CL_InitRefresh(void);
void    CL_ShutdownRefresh(void);
void    CL_RunRefresh(void);


//
// screen.c
//
extern rect_t      scr_vrect;        // position of render window

void    SCR_Init(void);
void    SCR_Shutdown(void);
void    SCR_UpdateScreen(void);
void    SCR_SizeUp(void);
void    SCR_SizeDown(void);
void    SCR_CenterPrint(const char *str);
void    SCR_FinishCinematic(void);
void    SCR_PlayCinematic(const char *name);
void    SCR_RunCinematic();
void    SCR_BeginLoadingPlaque(void);
void    SCR_EndLoadingPlaque(void);
void    SCR_DebugGraph(float value, int color);
void    SCR_TouchPics(void);
void    SCR_RegisterMedia(void);
void    SCR_ModeChanged(void);
void    SCR_LagSample(void);
void    SCR_LagClear(void);
void    SCR_SetCrosshairColor(void);
qhandle_t SCR_GetFont(void);
void    SCR_SetHudAlpha(float alpha);

float   SCR_FadeAlpha(unsigned startTime, unsigned visTime, unsigned fadeTime);
int     SCR_DrawStringEx(int x, int y, int flags, size_t maxlen, const char *s, qhandle_t font);
void    SCR_DrawStringMulti(int x, int y, int flags, size_t maxlen, const char *s, qhandle_t font);

void    SCR_ClearChatHUD_f(void);
void    SCR_AddToChatHUD(const char *text);

#ifdef _DEBUG
void CL_AddNetgraph(void);
#endif


//
// ascii.c
//
void CL_InitAscii(void);


//
// http.c
//
#if USE_CURL
void HTTP_Init(void);
void HTTP_Shutdown(void);
void HTTP_SetServer(const char *url);
qerror_t HTTP_QueueDownload(const char *path, dltype_t type);
void HTTP_RunDownloads(void);
void HTTP_CleanupDownloads(void);
#else
#define HTTP_Init()                     (void)0
#define HTTP_Shutdown()                 (void)0
#define HTTP_SetServer(url)             (void)0
#define HTTP_QueueDownload(path, type)  Q_ERR_NOSYS
#define HTTP_RunDownloads()             (void)0
#define HTTP_CleanupDownloads()         (void)0
#endif

//
// gtv.c
//

//
// crc.c
//
byte COM_BlockSequenceCRCByte(byte *base, size_t length, int sequence);
