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
#include <string>
#include <numbers>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <span>
#include <ranges>

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
typedef uint32_t qboolean;

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
constexpr int32_t MAX_STRING_CHARS  = 4096;  // max length of a string passed to Cmd_TokenizeString
constexpr int32_t MAX_STRING_TOKENS = 256;  // max tokens resulting from Cmd_TokenizeString
constexpr int32_t MAX_TOKEN_CHARS   = 1024;   // max length of an individual token
constexpr int32_t MAX_NET_STRING    = 2048;    // max length of a string used in network protocol

constexpr int32_t MAX_QPATH     = 64;  // max length of a quake game pathname
constexpr int32_t MAX_OSPATH    = 256; // max length of a filesystem pathname

//-----------------
// Per-level limits
//-----------------
constexpr int32_t MAX_CLIENTS   = 256;   // absolute limit
constexpr int32_t MAX_EDICTS    = 1024; // N&C: POOL: Was 1024 // must change protocol to increase more
constexpr int32_t MAX_LIGHTSTYLES   = 256;
constexpr int32_t MAX_MODELS    = 256; // these are sent over the net as bytes
constexpr int32_t MAX_SOUNDS    = 256;// so they cannot be blindly increased
constexpr int32_t MAX_IMAGES    = 256;
constexpr int32_t MAX_ITEMS     = 256;
constexpr int32_t MAX_GENERAL   = (MAX_CLIENTS * 2);// general config strings

constexpr int32_t MAX_CLIENT_NAME = 16;

//-----------------
// Max World Size.
//-----------------
constexpr int32_t MAX_WORLD_COORD = (16384);
constexpr int32_t MIN_WORLD_COORD = (-16384);

constexpr int32_t WORLD_SIZE = (MAX_WORLD_COORD - MIN_WORLD_COORD);

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
#include "shared/tickrate.h"


//
//=============================================================================
//
//	Common Library.
//
//=============================================================================
//
#include "shared/common.h"



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
//	Key / Value Info Strings
//
//=============================================================================
//
constexpr uint32_t MAX_INFO_KEY = 64;
constexpr uint32_t MAX_INFO_VALUE = 64; 
constexpr uint32_t MAX_INFO_STRING = 512;

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
constexpr int32_t CHAN_AUTO = 0;
constexpr int32_t CHAN_WEAPON = 1;
constexpr int32_t CHAN_VOICE = 2;
constexpr int32_t CHAN_ITEM = 3;
constexpr int32_t CHAN_BODY = 4;

//-----------------
// Modifier flags
//-----------------
constexpr int32_t CHAN_NO_PHS_ADD = 8;   // send to all clients, not just ones in PHS (ATTN 0 will also do this)
constexpr int32_t CHAN_RELIABLE = 16;  // send by reliable message, not datagram

//-----------------
// Sound attenuation values
//-----------------
constexpr int32_t ATTN_NONE = 0;   // full volume the entire level
constexpr int32_t ATTN_NORM = 1;
constexpr int32_t ATTN_IDLE = 2;
constexpr int32_t ATTN_STATIC = 3;   // diminish very rapidly with distance


//
//=============================================================================
//
//	CVars (Console Variables)
//
//=============================================================================
//


constexpr uint32_t CVAR_CHEAT       = (1 << 5);  // can't be changed when connected
constexpr uint32_t CVAR_PRIVATE     = (1 << 6);  // never macro expanded or saved to config
constexpr uint32_t CVAR_ROM         = (1 << 7);  // can't be changed even from cmdline
constexpr uint32_t CVAR_MODIFIED    = (1 << 8);  // modified by user
constexpr uint32_t CVAR_CUSTOM      = (1 << 9);  // created by user
constexpr uint32_t CVAR_WEAK        = (1 << 10); // doesn't have value
constexpr uint32_t CVAR_GAME        = (1 << 11); // created by game library
constexpr uint32_t CVAR_FILES       = (1 << 13); // r_reload when changed
constexpr uint32_t CVAR_REFRESH     = (1 << 14); // vid_restart when changed
constexpr uint32_t CVAR_SOUND       = (1 << 15); // snd_restart when changed

constexpr uint32_t CVAR_ARCHIVE     = 1; // set to cause it to be saved to vars.rc
constexpr uint32_t CVAR_USERINFO    = 2; // added to userinfo  when changed
constexpr uint32_t CVAR_SERVERINFO  = 4; // added to serverinfo when changed
constexpr uint32_t CVAR_NOSET       = 8; // don't allow change from console at all,
// but can be set from the command line
constexpr uint32_t CVAR_LATCH       = 16;// save changes until server restart

constexpr uint32_t CVAR_INFOMASK        = (CVAR_USERINFO | CVAR_SERVERINFO);
constexpr uint32_t CVAR_MODIFYMASK      = (CVAR_INFOMASK | CVAR_FILES | CVAR_REFRESH | CVAR_SOUND);
constexpr uint32_t CVAR_NOARCHIVEMASK   = (CVAR_NOSET | CVAR_CHEAT | CVAR_PRIVATE | CVAR_ROM);
constexpr uint32_t CVAR_EXTENDED_MASK   = (~31);

// Only include here, in case CVar has not been defined yet? TODO: Investigate, this is CLG related.
#ifndef CVAR
#define CVAR
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
struct ConfigStrings {
    static constexpr uint32_t Name              = 0;
    static constexpr uint32_t CdTrack           = 1;
    static constexpr uint32_t Sky               = 2;
    static constexpr uint32_t SkyAxis           = 3;       // %f %f %f format
    static constexpr uint32_t SkyRotate         = 4;
    static constexpr uint32_t StatusBar         = 5;       // display program string

    static constexpr uint32_t AirAcceleration   = 29;      // air acceleration control
    static constexpr uint32_t MaxClients        = 30;
    static constexpr uint32_t MapCheckSum       = 31;      // for catching cheater maps

    static constexpr uint32_t Models            = 32;
    static constexpr uint32_t Sounds            = (ConfigStrings::Models + MAX_MODELS);
    static constexpr uint32_t Images            = (ConfigStrings::Sounds + MAX_SOUNDS);
    static constexpr uint32_t Lights            = (ConfigStrings::Images+ MAX_IMAGES);
    static constexpr uint32_t Items             = (ConfigStrings::Lights+ MAX_LIGHTSTYLES);
    static constexpr uint32_t PlayerSkins       = (ConfigStrings::Items+ MAX_ITEMS);
    static constexpr uint32_t General           = (ConfigStrings::PlayerSkins + MAX_CLIENTS);
    static constexpr uint32_t MaxConfigStrings  = (ConfigStrings::General+ MAX_GENERAL);
};

// Some mods actually exploit ConfigStrings::StatusBar to take space up to ConfigStrings::AirAcceleration
inline static uint32_t CS_SIZE(uint32_t cs) {
    return ((cs) >= ConfigStrings::StatusBar && (cs) < ConfigStrings::AirAcceleration? \
        MAX_QPATH * (ConfigStrings::AirAcceleration- (cs)) : MAX_QPATH);
}


//
//=============================================================================
//
//	Player Move - Shared between Client, Server, and Game Modules.
//
//=============================================================================
//
#include "shared/pmove.h"



//
//=============================================================================
//
//	Messaging/Network
//
//=============================================================================
//
#include "shared/messaging.h"


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
#include "shared/qstring.h"
#include "shared/strings.h"


//
//=============================================================================
//
//	Collision Detection.
//
//=============================================================================
//
#include "shared/collision.h"


//
//=============================================================================
//
//	Entities & Related.
//
//=============================================================================
//
//-----------------
// EntityState->renderEffects
//
// The render effects are useful for tweaking the way how an entity is displayed.
// It may be favored for it to only be visible in mirrors, or fullbright, name it.
// 
// This is t he place to look for in-game entity rendering effects to apply.
//-----------------
typedef enum {
    ViewerModel     = (1 << 0),     // Don't draw through eyes, only mirrors.
    WeaponModel     = (1 << 1),     // Only draw through eyes.

    MinimalLight    = (1 << 2),     // Allways have some light. (Used for viewmodels)
    FullBright      = (1 << 3),     // Always draw the model at full light intensity.

    DepthHack       = (1 << 4),     // For view weapon Z crunching.
    Translucent     = (1 << 5),     // Translucent.

    FrameLerp       = (1 << 6),     // Linear Interpolation between animation frames.
    Beam            = (1 << 7),     // Special rendering hand: origin = to, oldOrigin = from.

    CustomSkin      = (1 << 8),     // If CustomSkin is set, ent->skin is an index in precaches.images.
    Glow            = (1 << 9),     // Pulse lighting. Used for items.
    RedShell        = (1 << 10),    // Red shell color effect.
    GreenShell      = (1 << 11),    // Green shell color effect.
    BlueShell       = (1 << 12),    // Blue shell color effect.

    InfraRedVisible = (1 << 13),    // Infrared rendering.
    DoubleShell     = (1 << 14),    // Double shell rendering.
    HalfDamShell    = (1 << 15),    // Half dam shell.
    UseDisguise     = (1 << 16),    // Use disguise.
} RenderEffects;

// PlayerState->refdef flags
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
struct UserFields {
    static constexpr int32_t AutoScreenshot = 1;
    static constexpr int32_t AutoRecord = 2;
    static constexpr int32_t LocalFieldOfView = 4;
    static constexpr int32_t MutePlayers = 8;
    static constexpr int32_t MuteObservers = 16;
    static constexpr int32_t MuteMiscellaneous = 32;
    static constexpr int32_t PlayerFieldOfView = 64;
};

#endif // __SHARED_SHARED_H__
