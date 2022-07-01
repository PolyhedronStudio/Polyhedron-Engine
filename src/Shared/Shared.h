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

#pragma once

//-----------------
// shared.h -- included first by ALL program modules
//-----------------

#if HAVE_CONFIG_H
#include "Config.h"
#endif

/***
*
* 
*   C/C++ Standard Library Includes.
* 
* 
***/
/**
*   C STDIO.
**/
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstdint>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <cstring>
#include <ctime>


/**
*   C++ STL.
**/
// For std ranges and view support.
//#ifdef __clang__
//#undef _HAS_CXX23
//#define _HAS_CXX23 1
//#endif

#include <string>
#include <numbers>
#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <span>
#include <ranges>
#include <chrono>


/**
*   System Endian include, if needed. 
**/
#if HAVE_ENDIAN_H
#include <endian.h>
#endif

/**
*   Platform specific includes.
**/
#include "Platform.h"



/***
*
* 
*   Core Types & Definitions.
* 
* 
***/
/**
*   Core.
**/
//! "byte" - unsigned char
using byte = unsigned char;

//! "qboolean" - Bool.
using qboolean = uint32_t;


/**
*   Engine.
**/
// "qhandle_t" - int32_t - Used for storing handles to engine internal objects.
using qhandle_t = int32_t;

// "qhandle_t" - int32_t - Used for error codes.
using qerror_t = int32_t;

// "vec_t" - float - Used for vector components.
using vec_t = float;


/**
*   Max String limits.
**/
static constexpr int32_t MAX_STRING_CHARS  = 4096; // Maximum length of a string passed to Cmd_TokenizeString.
static constexpr int32_t MAX_STRING_TOKENS = 256;  // Maximum tokens resulting from Cmd_TokenizeString.
static constexpr int32_t MAX_TOKEN_CHARS   = 1024; // Maximum length of an individual token.
static constexpr int32_t MAX_NET_STRING    = 2048; // Maximum length of a string used in network protocol.

static constexpr int32_t MAX_QPATH     = 64;  // Maximum length of a quake game pathname.
static constexpr int32_t MAX_OSPATH    = 256; // Maximum length of a filesystem pathname.


/**
*   Per-level limits
**/
static constexpr int32_t MAX_CLIENTS       = 256;  //! Absolute limit.
static constexpr int32_t MAX_LIGHTSTYLES   = 256;
static constexpr int32_t MAX_MODELS        = 256;  //! These are sent over the net as bytes.
static constexpr int32_t MAX_SOUNDS        = 256;  //! So they cannot be blindly increased.
static constexpr int32_t MAX_IMAGES        = 256;
static constexpr int32_t MAX_ITEMS         = 256;
static constexpr int32_t MAX_GENERAL       = (MAX_CLIENTS * 2); //! General config strings.

static constexpr int32_t MAX_CLIENT_NAME = 16;     //! Maximum length of a client's in-game name.

//! The actual maximum amount of entities that we want to allow to be packetized.
static constexpr int32_t MAX_WIRED_POD_ENTITIES = 1024; //! Maximum amount of "wired" entities we can handle.

static constexpr int32_t MAX_CLIENT_POD_ENTITIES = MAX_WIRED_POD_ENTITIES + 3072; // Maximum amount of client-only entities we can handle.
static constexpr int32_t MAX_SERVER_POD_ENTITIES = MAX_WIRED_POD_ENTITIES + 3072; // Maximum amount of client-only entities we can handle.


/**
*   Max World Size.
**/
static constexpr int32_t MAX_WORLD_COORD = (16384);    // Maximum positive world coordinate.
static constexpr int32_t MIN_WORLD_COORD = (-16384);   // Maximum negative world coordinates.

static constexpr int32_t WORLD_SIZE = (MAX_WORLD_COORD - MIN_WORLD_COORD); // Max - Min = 16384.


/**
*   General Utility Macros.
**/
//! Calculate the size of a typical C array, note that this method can be risky 
//! to use. It's only here for backwards compatibility reasons. Do NOT use :)
#define Q_COUNTOF(a)        (sizeof(a) / sizeof(a[0]))

//! Converts a floating point value to short, this brings certain limits with itself,
//! such as losing precision, and a range limit value of -4096/+4096
static inline const short Angle2Short(const float& f) { return ((int)((f) * 65536/360) & 65535); }

//! Reversed of the above, converts a short to float.
static inline const float ShortToAngle(const short& s) { return ((s) * (360.0f / 65536)); }

/**
*   Used to be 'Old "C" String Macros'. Renamed, but sticking around for legacy 
*   compatibility reasons over the entire code base.
**/
static inline bool PH_IsUpper(const char c) { return (c >= 'A' && c <= 'Z'); }
static inline bool PH_IsLower(const char c) { return (c >= 'a' && c <= 'z'); }
static inline bool PH_IsDigit(const char c) { return (c >= '0' && c <= '9'); }
static inline bool PH_IsAlpha(const char c) { return (PH_IsUpper(c) | PH_IsLower(c)); }
static inline bool PH_IsLetterOrNUmber(const char c) { return (PH_IsAlpha(c) == true || PH_IsDigit(c) == true); }
static inline bool PH_IsPrint(const char c) { return (c >= 32 && c < 127); }
static inline bool PH_IsGraph(const char c) { return (c > 32 && c < 127); }
static inline bool PH_IsSpace(const char c) { return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'); }

//! Tests if specified character is valid Polyhedron path character
static inline bool PH_IsPath(const char c) { return (PH_IsLetterOrNUmber(c) || c == '_' || c == '-'); }
//! Tests if specified character has special meaning to Polyhedron console
static inline bool PH_IsSpecial(const char c) { return (c == '\r' || c == '\n' || c == 127); }


/***
* 
*   Endian Swap Library.
* 
***/
#include "Endian.h"


/***
* 
*   Math Library.
* 
***/
#include "Math.h"


/***
* 
*   Tick Rate Configuration.
* 
***/
#include "TickRate.h"



/***
*
*   Common Library.
* 
***/
#include "Common.h"



/**
*   @brief  Memory tags to allow dynamic memory to be cleaned up
*           game DLLs have separate tag namespace starting at TAG_MAX
**/
enum memtag_t {
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
};



/***
*
*   Key / Value Info Strings 
* 
***/
#include "KeyValue.h"



/***
*
*   User Input
*  
***/
#include "Keys.h"



/***
*
*   User Interface.
* 
***/
#include "UI.h"

/**
*   Color defines, modify these as you please for custom colors.
**/
#define U32_BLACK   MakeColor(  0,   0,   0, 255)
#define U32_RED     MakeColor(215,  83,  65, 255)
#define U32_GREEN   MakeColor( 41, 171, 135, 255)
#define U32_YELLOW  MakeColor(255, 255,   0, 255)
#define U32_ORANGE  MakeColor(255, 165,   0, 255) // Used to be: U32_BLUE    MakeColor(  0,   0, 255, 255)
#define U32_CYAN    MakeColor(  0, 255, 255, 255)
#define U32_MAGENTA MakeColor(255,   0, 255, 255)
#define U32_WHITE   MakeColor(255, 255, 255, 255)




/***
*
*	Sound Channels & Attenuation
* 
***/
#include "Sound.h"



/***
*
*	CVars
*
***/
static constexpr uint32_t CVAR_CHEAT       = (1 << 5);  // can't be changed when connected
static constexpr uint32_t CVAR_PRIVATE     = (1 << 6);  // never macro expanded or saved to config
static constexpr uint32_t CVAR_ROM         = (1 << 7);  // can't be changed even from cmdline
static constexpr uint32_t CVAR_MODIFIED    = (1 << 8);  // modified by user
static constexpr uint32_t CVAR_CUSTOM      = (1 << 9);  // created by user
static constexpr uint32_t CVAR_WEAK        = (1 << 10); // doesn't have value
static constexpr uint32_t CVAR_GAME        = (1 << 11); // created by game library
static constexpr uint32_t CVAR_FILES       = (1 << 13); // r_reload when changed
static constexpr uint32_t CVAR_REFRESH     = (1 << 14); // vid_restart when changed
static constexpr uint32_t CVAR_SOUND       = (1 << 15); // snd_restart when changed

static constexpr uint32_t CVAR_ARCHIVE     = 1; // set to cause it to be saved to vars.rc
static constexpr uint32_t CVAR_USERINFO    = 2; // added to userinfo  when changed
static constexpr uint32_t CVAR_SERVERINFO  = 4; // added to serverinfo when changed
static constexpr uint32_t CVAR_NOSET       = 8; // don't allow change from console at all,
// but can be set from the command line
static constexpr uint32_t CVAR_LATCH       = 16;// save changes until server restart

static constexpr uint32_t CVAR_INFOMASK        = (CVAR_USERINFO | CVAR_SERVERINFO);
static constexpr uint32_t CVAR_MODIFYMASK      = (CVAR_INFOMASK | CVAR_FILES | CVAR_REFRESH | CVAR_SOUND);
static constexpr uint32_t CVAR_NOARCHIVEMASK   = (CVAR_NOSET | CVAR_CHEAT | CVAR_PRIVATE | CVAR_ROM);
static constexpr uint32_t CVAR_EXTENDED_MASK   = (~31);

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


/***
*
*	Config Strings.
*
***/
/**
*   @brief  Config Strings(CS) are a general means of communication from the server to
*           all connected clients. A config string can be at most MAX_QPATH characters.
**/
struct ConfigStrings {
    static constexpr uint32_t Name              = 0;
    static constexpr uint32_t CdTrack           = 1;
    static constexpr uint32_t Sky               = 2;
    static constexpr uint32_t SkyAxis           = 3;       // %f %f %f format
    static constexpr uint32_t SkyRotate         = 4;
    static constexpr uint32_t StatusBar         = 5;       // display program string

    static constexpr uint32_t Unused            = 29;      // Unused now, in the past it was the air acceleration control config string.
    static constexpr uint32_t MaxClients        = 30;
    static constexpr uint32_t MapCheckSum       = 31;      // for catching cheater maps

    static constexpr uint32_t Models            = 32;
    static constexpr uint32_t Sounds            = (ConfigStrings::Models + MAX_MODELS);
    static constexpr uint32_t Images            = (ConfigStrings::Sounds + MAX_SOUNDS);
    static constexpr uint32_t Lights            = (ConfigStrings::Images + MAX_IMAGES);
    static constexpr uint32_t Items             = (ConfigStrings::Lights + MAX_LIGHTSTYLES);
    static constexpr uint32_t PlayerSkins       = (ConfigStrings::Items + MAX_ITEMS);
    static constexpr uint32_t General           = (ConfigStrings::PlayerSkins + MAX_CLIENTS);
    static constexpr uint32_t MaxConfigStrings  = (ConfigStrings::General + MAX_GENERAL);
};

// Some mods actually exploit ConfigStrings::StatusBar to take space up to ConfigStrings::Unused
inline static uint32_t CS_SIZE(uint32_t cs) {
    return ((cs) >= ConfigStrings::StatusBar && (cs) < ConfigStrings::Unused ? 
        MAX_QPATH * (ConfigStrings::Unused - (cs)) : MAX_QPATH);
}

/***
*
*	Internal Skeletal Model Data (-Game Friendly.)
*
***/
#include "SkeletalModelData.h"



/***
*
*	Player Move - Shared between Client, Server, and Game Modules.
*
***/
#include "PlayerMove.h"



/***
*
*	Network Messaging.
*
***/
#include "Messaging.h"



/***
*
*	Filesystem.
*
***/
/**
*   @brief  FileInfo structured, shared between engine and game modules.
**/
typedef struct file_info_s {
    size_t  size;
    time_t  ctime;
    time_t  mtime;
    char    name[1];
} file_info_t;



/***
*
*	String Manipulation Utilities.
*
***/
#include "QString.h"
#include "Strings.h"


/***
*
*	C Style List.
*
***/
#include "List.h"


/***
*
*	Collision Detection.
*
***/
#include "CollisionModel.h"



/***
*
*	Entities & Related.
*
***/
#include "Entities.h"

// PlayerState->refdef flags
static constexpr int32_t RDF_UNDERWATER     = 1;    //! Warp the screen as apropriate.
static constexpr int32_t RDF_NOWORLDMODEL    = 2;   //! Used for player configuration screen.

//ROGUE
static constexpr int32_t RDF_IRGOGGLES       = 4;
static constexpr int32_t RDF_UVGOGGLES       = 8;
//ROGUE

// player_state->stats[] indexes
struct PlayerStats {
    static constexpr uint32_t HealthIcon        = 0;
    static constexpr uint32_t Health            = 1;
    static constexpr uint32_t PrimaryAmmoIcon   = 2;
    static constexpr uint32_t PrimaryAmmo       = 3;
    static constexpr uint32_t SecondaryAmmoIcon = 4;
    static constexpr uint32_t SecondaryAmmo     = 5;
    static constexpr uint32_t ClipAmmoIcon      = 6;
    static constexpr uint32_t ClipAmmo          = 7;
	static constexpr uint32_t Weapon			= 8;
	static constexpr uint32_t WeaponTime		= 9;
	static constexpr uint32_t PendingWeapon		= 10;
    static constexpr uint32_t ArmorIcon         = 11;
    static constexpr uint32_t Armor             = 12;
    static constexpr uint32_t SelectedItemIcon  = 13;
    static constexpr uint32_t PickupIcon        = 14;
    static constexpr uint32_t PickupString      = 15;
    static constexpr uint32_t TimerIcon         = 16;
    static constexpr uint32_t Timer             = 17;
    static constexpr uint32_t HelpIcon          = 18;
    static constexpr uint32_t SelectedItem      = 19;
    static constexpr uint32_t Layouts           = 20;
    static constexpr uint32_t Frags             = 21;
    static constexpr uint32_t Flashes           = 22; // Cleared each frame: 1 = health, 2 = armor, 3 = primary ammo, 4 = secondary ammo.
    static constexpr uint32_t ChaseClientID     = 23;
    static constexpr uint32_t IsSpectator       = 24;
};

/**
*   @brief  User Fields.
**/
struct UserFields {
    static constexpr int32_t AutoScreenshot     = 1;
    static constexpr int32_t AutoRecord         = 2;
    static constexpr int32_t LocalFieldOfView   = 4;
    static constexpr int32_t MutePlayers        = 8;
    static constexpr int32_t MuteObservers      = 16;
    static constexpr int32_t MuteMiscellaneous  = 32;
    static constexpr int32_t PlayerFieldOfView  = 64;
};