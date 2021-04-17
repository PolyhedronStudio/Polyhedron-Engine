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

#ifndef __SHARED_SHARED_H__
#define __SHARED_SHARED_H__

//-----------------
// shared.h -- included first by ALL program modules
//-----------------

#if HAVE_CONFIG_H
#include "config.h"
#endif

//
//=============================================================================
//
//	Main Includes.
//
//=============================================================================
//
//-----------------
// C++ C STDIO
//-----------------
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <climits>
#include <ctime>
#include <cfloat>

//-----------------
// C++ STL
//-----------------
#include <numbers>
#include <string>
#include <numbers>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>

//-----------------
// System Endian include, if needed. 
//-----------------
#if HAVE_ENDIAN_H
#include <endian.h>
#endif

//-----------------
// Platform specific includes.
//-----------------
#include "shared/platform.h"


//
//=============================================================================
//
//	Core Types and Definitions.
//
//=============================================================================
// 
//-----------------
// Core
//-----------------
// "byte" - unsigned char
typedef unsigned char byte;

// "qboolean" - Bool.
typedef bool qboolean;

//-----------------
// Engine
//-----------------
// "qhandle_t" - int32_t - Used for storing handles to engine internal objects.
typedef int32_t qhandle_t;

// "qhandle_t" - int32_t - Used for error codes.
typedef int32_t qerror_t;

// "vec_t" - float - Used for vector components.
typedef float vec_t;

//-----------------
// Max String limits.
//-----------------
#define MAX_STRING_CHARS    4096    // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS   256     // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS     1024    // max length of an individual token
#define MAX_NET_STRING      2048    // max length of a string used in network protocol

#define MAX_QPATH           256      // max length of a quake game pathname
#define MAX_OSPATH          256     // max length of a filesystem pathname

//-----------------
// Per-level limits
//-----------------
#define MAX_CLIENTS         256     // absolute limit
#define MAX_EDICTS          2048    // N&C: POOL: Was 1024 // must change protocol to increase more
#define MAX_LIGHTSTYLES     256
#define MAX_MODELS          256     // these are sent over the net as bytes
#define MAX_SOUNDS          256     // so they cannot be blindly increased
#define MAX_IMAGES          256
#define MAX_ITEMS           256
#define MAX_GENERAL         (MAX_CLIENTS * 2) // general config strings

#define MAX_CLIENT_NAME     16

//-----------------
// General Utility Macros.
//-----------------
// Calculate the size of a typical C array, note that this method can be risky 
// to use. It's only here for backwards compatibility reasons. Do NOT use :)
#define Q_COUNTOF(a)        (sizeof(a) / sizeof(a[0]))

// Converts a floating point value to short, this brings certain limits with itself,
// such as losing precision, and a value limit of -4096/+4096
#define ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)

// Reversed of the above, converts a short to float.
#define SHORT2ANGLE(x)  ((x)*(360.0/65536))

//-----------------
// Fast "C" String Macros
//-----------------
#define Q_isupper(c)    ((c) >= 'A' && (c) <= 'Z')
#define Q_islower(c)    ((c) >= 'a' && (c) <= 'z')
#define Q_isdigit(c)    ((c) >= '0' && (c) <= '9')
#define Q_isalpha(c)    (Q_isupper(c) || Q_islower(c))
#define Q_isalnum(c)    (Q_isalpha(c) || Q_isdigit(c))
#define Q_isprint(c)    ((c) >= 32 && (c) < 127)
#define Q_isgraph(c)    ((c) > 32 && (c) < 127)
#define Q_isspace(c)    (c == ' ' || c == '\f' || c == '\n' || \
                         c == '\r' || c == '\t' || c == '\v')
// Tests if specified character is valid quake path character
#define Q_ispath(c)     (Q_isalnum(c) || (c) == '_' || (c) == '-')
// Tests if specified character has special meaning to quake console
#define Q_isspecial(c)  ((c) == '\r' || (c) == '\n' || (c) == 127)

//
//=============================================================================
//
//	Endian Swap Library.
//
//=============================================================================
//
#include "shared/endian.h"


//
//=============================================================================
//
//	Math Library.
//
//=============================================================================
//
#include "shared/math.h"

//
//=============================================================================
//
//	Engine Tick Rate Settings.
//
//=============================================================================
//
//-----------------
//N&C 20hz tick
//-----------------
#define BASE_FRAMERATE          20 //10
#define BASE_FRAMETIME          50.0f //100
#define BASE_1_FRAMETIME        0.02f //0.01f   // 1/BASE_FRAMETIME
#define BASE_FRAMETIME_1000     0.05f //0.1f    // BASE_FRAMETIME/1000

// maximum variable FPS factor
#define MAX_FRAMEDIV    6

//-----------------
// Client FPS
//-----------------
// N&C: Moved here instead of client.h, for CG Module.
#define CL_FRAMETIME    BASE_FRAMETIME
#define CL_1_FRAMETIME  BASE_1_FRAMETIME
#define CL_FRAMEDIV     1
#define CL_FRAMESYNC    1
#if CGAME_INCLUDE
#define CL_KEYPS        &cl->frame.playerState
#define CL_OLDKEYPS     &cl->oldframe.playerState
#define CL_KEYLERPFRAC  cl->lerpfrac
#else
#define CL_KEYPS        &cl.frame.playerState
#define CL_OLDKEYPS     &cl.oldframe.playerState
#define CL_KEYLERPFRAC  cl.lerpfrac
#endif


//
//=============================================================================
//
//	Common Library.
//
//=============================================================================
//
typedef enum {
    ERR_FATAL,          // exit the entire game with a popup window
    ERR_DROP,           // print to console and disconnect from game
    ERR_DISCONNECT,     // like drop, but not an error
    ERR_RECONNECT       // make server broadcast 'reconnect' message
} error_type_t;

typedef enum {
    PRINT_ALL,          // general messages
    PRINT_TALK,         // print in green color
    PRINT_DEVELOPER,    // only print when "developer 1"
    PRINT_WARNING,      // print in yellow color
    PRINT_ERROR,        // print in red color
    PRINT_NOTICE        // print in cyan color
} print_type_t;



//-----------------
// WATISDEZE: We don't want these defined in clgame.h
//-----------------
#ifndef CGAME_INCLUDE
void    Com_LPrintf(print_type_t type, const char* fmt, ...)
q_printf(2, 3);
void    Com_Error(error_type_t code, const char* fmt, ...)
q_noreturn q_printf(2, 3);

#define Com_Printf(...) Com_LPrintf(PRINT_ALL, __VA_ARGS__)
#define Com_DPrintf(...) Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#define Com_WPrintf(...) Com_LPrintf(PRINT_WARNING, __VA_ARGS__)
#define Com_EPrintf(...) Com_LPrintf(PRINT_ERROR, __VA_ARGS__)
#endif // CGAME_INCLUDE

// game print flags
#define PRINT_LOW           0       // pickup messages
#define PRINT_MEDIUM        1       // death messages
#define PRINT_HIGH          2       // critical messages
#define PRINT_CHAT          3       // chat messages    

//-----------------
// Memory tags to allow dynamic memory to be cleaned up
// game DLLs have separate tag namespace starting at TAG_MAX
//-----------------
typedef enum {
    TAG_FREE,       // should have never been set
    TAG_STATIC,

    TAG_GENERAL,
    TAG_CMD,
    TAG_CVAR,
    TAG_FILESYSTEM,
    TAG_RENDERER,
    TAG_UI,
    TAG_SERVER,
    TAG_MVD,
    TAG_SOUND,
    TAG_CMODEL,

    TAG_MAX
} memtag_t;


//
//=============================================================================
//
//	User Input
//
//=============================================================================
//
#include "shared/keys.h"


//
//=============================================================================
//
//  UI.
//
//=============================================================================
//
#include "shared/ui.h"

//-----------------
// Color defines, modify these as you please for custom colors.
//-----------------
#define U32_BLACK   MakeColor(  0,   0,   0, 255)
#define U32_RED     MakeColor(255,   0,   0, 255)
#define U32_GREEN   MakeColor(  0, 255,   0, 255)
#define U32_YELLOW  MakeColor(255, 255,   0, 255)
#define U32_BLUE    MakeColor(  0,   0, 255, 255)
#define U32_CYAN    MakeColor(  0, 255, 255, 255)
#define U32_MAGENTA MakeColor(255,   0, 255, 255)
#define U32_WHITE   MakeColor(255, 255, 255, 255)


//
//=============================================================================
//
//	Sound Channels & Attenuation
//
//=============================================================================
//
//-----------------
// Sound channels
// Channel 0 never willingly overrides
// Other channels (1-7) allways override a playing sound on that channel
//-----------------
#define CHAN_AUTO               0
#define CHAN_WEAPON             1
#define CHAN_VOICE              2
#define CHAN_ITEM               3
#define CHAN_BODY               4

//-----------------
// Modifier flags
//-----------------
#define CHAN_NO_PHS_ADD         8   // send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define CHAN_RELIABLE           16  // send by reliable message, not datagram

//-----------------
// Sound attenuation values
//-----------------
#define ATTN_NONE               0   // full volume the entire level
#define ATTN_NORM               1
#define ATTN_IDLE               2
#define ATTN_STATIC             3   // diminish very rapidly with distance


//
//=============================================================================
//
//	CVars (Console Variables)
//
//=============================================================================
//
#define CVAR_CHEAT          (1 << 5)  // can't be changed when connected
#define CVAR_PRIVATE        (1 << 6)  // never macro expanded or saved to config
#define CVAR_ROM            (1 << 7)  // can't be changed even from cmdline
#define CVAR_MODIFIED       (1 << 8)  // modified by user
#define CVAR_CUSTOM         (1 << 9)  // created by user
#define CVAR_WEAK           (1 << 10) // doesn't have value
#define CVAR_GAME           (1 << 11) // created by game library
#define CVAR_FILES          (1 << 13) // r_reload when changed
#define CVAR_REFRESH        (1 << 14) // vid_restart when changed
#define CVAR_SOUND          (1 << 15) // snd_restart when changed

#define CVAR_INFOMASK       (CVAR_USERINFO | CVAR_SERVERINFO)
#define CVAR_MODIFYMASK     (CVAR_INFOMASK | CVAR_FILES | CVAR_REFRESH | CVAR_SOUND)
#define CVAR_NOARCHIVEMASK  (CVAR_NOSET | CVAR_CHEAT | CVAR_PRIVATE | CVAR_ROM)
#define CVAR_EXTENDED_MASK  (~31)

// Only include here, in case CVar has not been defined yet? TODO: Investigate, this is CLG related.
#ifndef CVAR
#define CVAR

#define CVAR_ARCHIVE    1   // set to cause it to be saved to vars.rc
#define CVAR_USERINFO   2   // added to userinfo  when changed
#define CVAR_SERVERINFO 4   // added to serverinfo when changed
#define CVAR_NOSET      8   // don't allow change from console at all,
// but can be set from the command line
#define CVAR_LATCH      16  // save changes until server restart

struct cvar_s;
struct genctx_s;

typedef void (*xchanged_t)(struct cvar_s*);
typedef void (*xgenerator_t)(struct genctx_s*);

// nothing outside the cvar.*() functions should modify these fields!
typedef struct cvar_s {
    char* name;
    char* string;
    char* latched_string;    // for CVAR_LATCH vars
    int         flags;
    qboolean    modified;   // set each time the cvar is changed
    float       value;
    struct cvar_s* next;

    // ------ new stuff ------
    int         integer;
    char* default_string;
    xchanged_t      changed;
    xgenerator_t    generator;
    struct cvar_s* hashNext;
} cvar_t;
#endif      // CVAR


//
//=============================================================================
//
//	Config Strings
//
//=============================================================================
//
//-----------------
// Config Strings(CS) are a general means of communication from the server to
// all connected clients. A config string can be at most MAX_QPATH characters.
//-----------------
#define CS_NAME             0
#define CS_CDTRACK          1
#define CS_SKY              2
#define CS_SKYAXIS          3       // %f %f %f format
#define CS_SKYROTATE        4
#define CS_STATUSBAR        5       // display program string

#define CS_AIRACCEL         29      // air acceleration control
#define CS_MAXCLIENTS       30
#define CS_MAPCHECKSUM      31      // for catching cheater maps

#define CS_MODELS           32
#define CS_SOUNDS           (CS_MODELS+MAX_MODELS)
#define CS_IMAGES           (CS_SOUNDS+MAX_SOUNDS)
#define CS_LIGHTS           (CS_IMAGES+MAX_IMAGES)
#define CS_ITEMS            (CS_LIGHTS+MAX_LIGHTSTYLES)
#define CS_PLAYERSKINS      (CS_ITEMS+MAX_ITEMS)
#define CS_GENERAL          (CS_PLAYERSKINS+MAX_CLIENTS)
#define MAX_CONFIGSTRINGS   (CS_GENERAL+MAX_GENERAL)

// Some mods actually exploit CS_STATUSBAR to take space up to CS_AIRACCEL
#define CS_SIZE(cs) \
    ((cs) >= CS_STATUSBAR && (cs) < CS_AIRACCEL ? \
      MAX_QPATH * (CS_AIRACCEL - (cs)) : MAX_QPATH)


//
//=============================================================================
//
//	Player Move - Shared between Client, Server, and Game Modules.
//
//=============================================================================
//
//-----------------
// Brief Water Level.
//-----------------
typedef enum {
    WATER_UNKNOWN = -1,
    WATER_NONE,
    WATER_FEET,
    WATER_WAIST,
    WATER_UNDER
} pm_water_level_t;

//-----------------
// General player movement and capabilities classification.
//-----------------
typedef enum {
    PM_NORMAL, // Walking, jumping, falling, swimming, etc.
    PM_HOOK_PULL,   // Pull hook
    PM_HOOK_SWING,  // Swing hook
    PM_SPECTATOR,   // Free-flying movement with acceleration and friction
    // All slots up till 32 are free for custom game PM_ defines.
    PM_DEAD = 32, // No movement, but the ability to rotate in place
    PM_FREEZE,    // No movement at all
    PM_GIB,       // No movement, different bounding box
} pm_type_t;

//-----------------
// Player movement flags.The game is free to define up to 16 bits.
//-----------------
constexpr int32_t PMF_ENGINE        = (1 << 0);         // Engine flags first.
constexpr int32_t PMF_TIME_TELEPORT = (PMF_ENGINE << 1);// time frozen in place
constexpr int32_t PMF_NO_PREDICTION = (PMF_ENGINE << 2);// temporarily disables client side prediction
constexpr int32_t PMF_GAME          = (PMF_ENGINE << 3);// Game flags start from here.

//-----------------
// This structure needs to be communicated bit-accurate from the server to the 
// client to guarantee that prediction stays in sync, so no floats are used.
// 
// If any part of the game code modifies this struct, it will result in a 
// prediction error of some degree.
//-----------------
typedef struct {
    pm_type_t    type;

    vec3_t      origin;
    vec3_t      velocity;

    uint16_t    flags;       // Ducked, jump_held, etc
    uint16_t    time;        // Each unit = 8 ms
    uint16_t    gravity;
    int16_t     delta_angles[3];    // Add to command angles to get view direction
    // Changed by spawns, rotating objects, and teleporters

    // View offsets. (Only Z is used atm, beware.)
    vec3_t view_offset;
} pm_state_t;

//-----------------
// usercmd_t is sent to the server each client frame
//-----------------
typedef struct usercmd_s {
    byte    msec;
    byte    buttons;
    short   angles[3];
    short   forwardmove, sidemove, upmove;
    byte    impulse;        // remove?
    byte    lightlevel;     // light level the player is standing on
} usercmd_t;


//
//=============================================================================
//
//	Messaging/Network
//
//=============================================================================
//
//-----------------
// Destination class for gi.Multicast()
//-----------------
typedef enum {
    MULTICAST_ALL,
    MULTICAST_PHS,
    MULTICAST_PVS,
    MULTICAST_ALL_R,
    MULTICAST_PHS_R,
    MULTICAST_PVS_R
} multicast_t;

//-----------------
// Connection State of the client.
//-----------------
typedef enum {
    ca_uninitialized,
    ca_disconnected,    // not talking to a server
    ca_challenging,     // sending getchallenge packets to the server
    ca_connecting,      // sending connect packets to the server
    ca_connected,       // netchan_t established, waiting for svc_serverdata
    ca_loading,         // loading level data
    ca_precached,       // loaded level data, waiting for svc_frame
    ca_active,          // game views should be displayed
    ca_cinematic        // running a cinematic
} connstate_t;

//-----------------
// Run State of the server.
//-----------------
typedef enum {
    ss_dead,            // no map loaded
    ss_loading,         // spawning level edicts
    ss_game,            // actively running
    ss_pic,             // showing static picture
    ss_broadcast,       // running MVD client
    ss_cinematic,
} server_state_t;

//-----------------
// entity_state_t->event values
// 
// Entity events are for effects that take place relative to an existing 
// entities origin. Very network efficient.
// 
// All muzzle flashes really should be converted to events...
//-----------------
typedef enum {
    EV_NONE,
    EV_ITEM_RESPAWN,
    EV_FOOTSTEP,
    EV_FALLSHORT,
    EV_FALL,
    EV_FALLFAR,
    EV_PLAYER_TELEPORT,
    EV_OTHER_TELEPORT
} entity_event_t;

//-----------------
// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
//-----------------
typedef struct entity_state_s {
    int     number;         // Entity index

    vec3_t  origin;
    vec3_t  angles;
    vec3_t  old_origin;     // For lerping
    int     modelindex;
    int     modelindex2, modelindex3, modelindex4;  // Weapons, CTF flags, etc
    int     frame;
    int     skinnum;
    unsigned int        effects;        // PGM - we're filling it, so it needs to be unsigned
    int     renderfx;
    int     solid;          // For client side prediction, 8*(bits 0-4) is x/y radius
                            // 8*(bits 5-9) is z down distance, 8(bits10-15) is z up
                            // gi.LinkEntity sets this properly
    int     sound;          // For looping sounds, to guarantee shutoff
    int     event;          // Impulse events -- muzzle flashes, footsteps, etc
                            // events only go out for a single frame, they
                            // are automatically cleared each frame
} entity_state_t;

//-----------------
// player_state_t is the information needed in addition to pm_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pm_state_t changes will be reletive to client
// frame rates
//-----------------
// Maximum amount of stats available to the player state.
#define MAX_STATS               32

typedef struct {
    pm_state_t   pmove;         // For prediction

    // These fields do not need to be communicated bit-precise

    vec3_t      viewAngles;     // For fixed views
    vec3_t      viewoffset;     // Add to pmovestate->origin
    vec3_t      kickAngles;    // Add to view direction to get render angles
                                // Set by weapon kicks, pain effects, etc

    vec3_t      gunangles;
    vec3_t      gunoffset;
    int         gunindex;
    int         gunframe;

    float       blend[4];       // RGBA full screen effect

    float       fov;            // Horizontal field of view

    int         rdflags;        // Refdef flags

    short       stats[MAX_STATS]; // Fast status bar updates
} player_state_t;


//
//=============================================================================
//
//	Filesystem
//
//=============================================================================
//
//-----------------
// FileInfo structured, shared between engine and game modules.
//-----------------
typedef struct file_info_s {
    size_t  size;
    time_t  ctime;
    time_t  mtime;
    char    name[1];
} file_info_t;


//
//=============================================================================
//
//	String Manipulation Utilities.
//
//=============================================================================
//
static inline int Q_tolower(int c) {
    if (Q_isupper(c)) {
        c += ('a' - 'A');
    }
    return c;
}

static inline int Q_toupper(int c) {
    if (Q_islower(c)) {
        c -= ('a' - 'A');
    }
    return c;
}

static inline char* Q_strlwr(char* s) {
    char* p = s;

    while (*p) {
        *p = Q_tolower(*p);
        p++;
    }

    return s;
}

static inline char* Q_strupr(char* s) {
    char* p = s;

    while (*p) {
        *p = Q_toupper(*p);
        p++;
    }

    return s;
}

static inline int Q_charhex(int c) {
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

// converts quake char to ASCII equivalent
static inline int Q_charascii(int c) {
    if (Q_isspace(c)) {
        // white-space chars are output as-is
        return c;
    }
    c &= 127; // strip high bits
    if (Q_isprint(c)) {
        return c;
    }
    switch (c) {
        // handle bold brackets
    case 16: return '[';
    case 17: return ']';
    }
    return '.'; // don't output control chars, etc
}

// portable case insensitive compare
int Q_strcasecmp(const char* s1, const char* s2);
int Q_strncasecmp(const char* s1, const char* s2, size_t n);
char* Q_strcasestr(const char* s1, const char* s2);

#define Q_stricmp   Q_strcasecmp
#define Q_stricmpn  Q_strncasecmp
#define Q_stristr   Q_strcasestr

char* Q_strchrnul(const char* s, int c);
void* Q_memccpy(void* dst, const void* src, int c, size_t size);
void Q_setenv(const char* name, const char* value);

char* COM_SkipPath(const char* pathname);
void COM_StripExtension(const char* in, char* out, size_t size);
void COM_FileBase(char* in, char* out);
void COM_FilePath(const char* in, char* out, size_t size);
size_t COM_DefaultExtension(char* path, const char* ext, size_t size);
char* COM_FileExtension(const char* in);

#define COM_CompareExtension(in, ext) \
    Q_strcasecmp(COM_FileExtension(in), ext)

qboolean COM_IsFloat(const char* s);
qboolean COM_IsUint(const char* s);
qboolean COM_IsPath(const char* s);
qboolean COM_IsWhite(const char* s);

char* COM_Parse(const char** data_p);
// data is an in/out parm, returns a parsed out token
size_t COM_Compress(char* data);

int SortStrcmp(const void* p1, const void* p2);
int SortStricmp(const void* p1, const void* p2);

size_t COM_strclr(char* s);

// buffer safe operations
size_t Q_strlcpy(char* dst, const char* src, size_t size);
size_t Q_strlcat(char* dst, const char* src, size_t size);

size_t Q_concat(char* dest, size_t size, ...) q_sentinel;

size_t Q_vsnprintf(char* dest, size_t size, const char* fmt, va_list argptr);
size_t Q_vscnprintf(char* dest, size_t size, const char* fmt, va_list argptr);
size_t Q_snprintf(char* dest, size_t size, const char* fmt, ...) q_printf(3, 4);
size_t Q_scnprintf(char* dest, size_t size, const char* fmt, ...) q_printf(3, 4);

// Inline utility.
inline const char* Vec3ToString(const vec3_t& v, qboolean rounded = true) {
    // 64 should be enough, no? This function shouldn't be used outside of
    // debugging purposes anyhow...
    static std::string str[64];
    static int strIndex = 0;

    str[strIndex = (strIndex > 7 ? 0 : strIndex + 1)] = vec3_to_str(v, rounded);
    return str[strIndex].c_str();
}

char* va(const char* format, ...) q_printf(1, 2);


//
//=============================================================================
//
//	Key / Value Info Strings
//
//=============================================================================
//
#define MAX_INFO_KEY        64
#define MAX_INFO_VALUE      64
#define MAX_INFO_STRING     512

char* Info_ValueForKey(const char* s, const char* key);
void    Info_RemoveKey(char* s, const char* key);
qboolean    Info_SetValueForKey(char* s, const char* key, const char* value);
qboolean    Info_Validate(const char* s);
size_t  Info_SubValidate(const char* s);
void    Info_NextPair(const char** string, char* key, char* value);
void    Info_Print(const char* infostring);


//
//=============================================================================
//
//	Collision Detection.
//
//=============================================================================
//
//-----------------
// Brush content Flags.
//-----------------
// Lower bits are stronger, and will eat weaker brushes completely
#define CONTENTS_SOLID          1       // An eye is never valid in a solid
#define CONTENTS_WINDOW         2       // Translucent, but not watery
#define CONTENTS_AUX            4
#define CONTENTS_LAVA           8
#define CONTENTS_SLIME          16
#define CONTENTS_WATER          32
#define CONTENTS_MIST           64
#define LAST_VISIBLE_CONTENTS   64

// Remaining contents are non-visible, and don't eat brushes
#define CONTENTS_AREAPORTAL     0x8000
#define CONTENTS_PLAYERCLIP     0x10000
#define CONTENTS_MONSTERCLIP    0x20000

#define CONTENTS_ORIGIN         0x1000000   // Removed before bsping an entity
#define CONTENTS_MONSTER        0x2000000   // Should never be on a brush, only in game
#define CONTENTS_DEADMONSTER    0x4000000
#define CONTENTS_DETAIL         0x8000000   // Brushes to be added after vis leafs
#define CONTENTS_TRANSLUCENT    0x10000000  // Auto set if any surface has trans
#define CONTENTS_LADDER         0x20000000

// Currents can be added to any other contents, and may be mixed
#define CONTENTS_CURRENT_0      0x40000
#define CONTENTS_CURRENT_90     0x80000
#define CONTENTS_CURRENT_180    0x100000
#define CONTENTS_CURRENT_270    0x200000
#define CONTENTS_CURRENT_UP     0x400000
#define CONTENTS_CURRENT_DOWN   0x800000

//-----------------
// Sets of content masks
//-----------------
#define CONTENTS_MASK_ALL                (-1)
#define CONTENTS_MASK_SOLID              (CONTENTS_SOLID|CONTENTS_WINDOW)
#define CONTENTS_MASK_PLAYERSOLID        (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define CONTENTS_MASK_DEADSOLID          (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW)
#define CONTENTS_MASK_MONSTERSOLID       (CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define CONTENTS_MASK_LIQUID              (CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define CONTENTS_MASK_OPAQUE             (CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define CONTENTS_MASK_SHOT               (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER)
#define CONTENTS_MASK_CURRENT            (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)

//-----------------
// Surface Flags.
//-----------------
#define SURF_LIGHT      0x1     // Value will hold the light strength
#define SURF_SLICK      0x2     // Effects game physics
#define SURF_SKY        0x4     // Don't draw, but add to skybox
#define SURF_WARP       0x8     // Turbulent water warp
#define SURF_TRANS33    0x10
#define SURF_TRANS66    0x20
#define SURF_FLOWING    0x40    // Scroll towards angle
#define SURF_NODRAW     0x80    // Don't bother referencing the texture
#define SURF_ALPHATEST  0x02000000  // used by kmquake2


//-----------------
// gi.BoxEntities() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
//-----------------
#define AREA_SOLID      1
#define AREA_TRIGGERS   2

//-----------------
// Surface Collision data.
//-----------------
typedef struct csurface_s {
    char        name[16];   // The actual material name used for this surface.
    int         flags;      // The surface flags.
    int         value;      // The content vlags. (TODO: Is this correct?)
} csurface_t;

//-----------------
// Traces are discrete movements through world space, clipped to the
// BSP planes they intersect.This is the basis for all collision detection
// within Quake.
//-----------------
typedef struct {
    // If true, the trace startedand ended within the same solid.
    qboolean    allSolid;
    // If true, the trace started within a solid, but exited it.
    qboolean    startSolid;
    // The fraction of the desired distance traveled(0.0 - 1.0).If
    // 1.0, no plane was impacted.
    float       fraction;

    // The destination position.
    vec3_t      endPosition;

    // The impacted plane, or empty.Note that a copy of the plane is
    // returned, rather than a pointer.This is because the plane may belong to
    // an inline BSP model or the box hull of a solid entity, in which case it must
    // be transformed by the entity's current position.
    cplane_t    plane;
    // The impacted surface, or `NULL`.
    csurface_t* surface;
    // The contents mask of the impacted brush, or 0.
    int         contents;

    // The impacted entity, or `NULL`.
    struct entity_s* ent;   // Not set by CM_*() functions

    // N&C: Custom added.
    vec3_t		offsets[8];	// [signbits][x] = either size[0][x] or size[1][x]
} trace_t;

// entity_state_t->renderfx flags
#define RF_MINLIGHT         1       // allways have some light (viewmodel)
#define RF_VIEWERMODEL      2       // don't draw through eyes, only mirrors
#define RF_WEAPONMODEL      4       // only draw through eyes
#define RF_FULLBRIGHT       8       // allways draw full intensity
#define RF_DEPTHHACK        16      // for view weapon Z crunching
#define RF_TRANSLUCENT      32
#define RF_FRAMELERP        64
#define RF_BEAM             128
#define RF_CUSTOMSKIN       256     // skin is an index in image_precache
#define RF_GLOW             512     // pulse lighting for bonus items
#define RF_SHELL_RED        1024
#define RF_SHELL_GREEN      2048
#define RF_SHELL_BLUE       4096

//ROGUE
#define RF_IR_VISIBLE       0x00008000      // 32768
#define RF_SHELL_DOUBLE     0x00010000      // 65536
#define RF_SHELL_HALF_DAM   0x00020000
#define RF_USE_DISGUISE     0x00040000
//ROGUE

// player_state_t->refdef flags
#define RDF_UNDERWATER      1       // warp the screen as apropriate
#define RDF_NOWORLDMODEL    2       // used for player configuration screen

//ROGUE
#define RDF_IRGOGGLES       4
#define RDF_UVGOGGLES       8
//ROGUE

// player_state->stats[] indexes
#define STAT_HEALTH_ICON        0
#define STAT_HEALTH             1
#define STAT_AMMO_ICON          2
#define STAT_AMMO               3
#define STAT_ARMOR_ICON         4
#define STAT_ARMOR              5
#define STAT_SELECTED_ICON      6
#define STAT_PICKUP_ICON        7
#define STAT_PICKUP_STRING      8
#define STAT_TIMER_ICON         9
#define STAT_TIMER              10
#define STAT_HELPICON           11
#define STAT_SELECTED_ITEM      12
#define STAT_LAYOUTS            13
#define STAT_FRAGS              14
#define STAT_FLASHES            15      // cleared each frame, 1 = health, 2 = armor
#define STAT_CHASE              16
#define STAT_SPECTATOR          17

//
// User Field.
//
#define UF_AUTOSCREENSHOT   1
#define UF_AUTORECORD       2
#define UF_LOCALFOV         4
#define UF_MUTE_PLAYERS     8
#define UF_MUTE_OBSERVERS   16
#define UF_MUTE_MISC        32
#define UF_PLAYERFOV        64

// WatIsDeze: Ifdef, for cgame dll.
#ifdef CGAME_INCLUDE
#include "common/cmodel.h"
#include "common/cmd.h"
#endif // CGAME_INCLUDE

#endif // SHARED_H
