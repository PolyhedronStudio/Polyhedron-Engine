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

#ifndef __INC_SHARED__SVGAME_H__
#define __INC_SHARED__SVGAME_H__

#include "shared/list.h"
#include "sharedgame/pmove.h"
#include "sharedgame/protocol.h"

//
// game.h -- game dll information visible to server
//

#define SVGAME_API_VERSION_MAJOR VERSION_MAJOR
#define SVGAME_API_VERSION_MINOR VERSION_MINOR
#define SVGAME_API_VERSION_POINT VERSION_POINT

// edict->serverFlags
struct EntityServerFlags {
    static constexpr uint32_t NoClient = 0x00000001;    // Don't send entity to clients, even if it has effects
    static constexpr uint32_t DeadMonster = 0x00000002; // Treat as CONTENTS_DEADMONSTER for collision
    static constexpr uint32_t Monster = 0x00000004;     // Treat as CONTENTS_MONSTER for collision
};

// edict->solid values
struct Solid {
    static constexpr uint32_t Not       = 0;    // No interaction with other objects
    static constexpr uint32_t Trigger   = 1;    // Only touch when inside, after moving
    static constexpr uint32_t BoundingBox = 2;  // Touch on edge
    static constexpr uint32_t BSP       = 3;    // Bsp clip, touch on edge
};

//===============================================================

#define MAX_ENT_CLUSTERS    16


typedef struct entity_s entity_t;
typedef struct gclient_s gclient_t;


#ifndef GAME_INCLUDE

struct gclient_s {
    PlayerState  playerState;     // communicated by server to clients
    int             ping;

    // the game dll can add anything it wants after
    // this point in the structure
    int             clientNumber;
};


struct entity_s {
    EntityState  state;
    struct gclient_s    *client;
    qboolean    inUse;
    int         linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t      area;               // linked to a division node or leaf

    int         numClusters;       // if -1, use headNode instead
    int         clusterNumbers[MAX_ENT_CLUSTERS];
    int         headNode;           // unused if numClusters != -1
    int         areaNumber, areaNumber2;

    //================================

    int         serverFlags;            // EntityServerFlags::NoClient, EntityServerFlags::DeadMonster, EntityServerFlags::Monster, etc
    vec3_t      mins, maxs;
    vec3_t      absMin, absMax, size;
    uint32_t    solid;
    int         clipMask;
    entity_t     *owner;

    // the game dll can add anything it wants after
    // this point in the structure
};

#endif      // GAME_INCLUDE

//===============================================================

//
// functions provided by the main engine
//
typedef struct {
    //---------------------------------------------------------------------
    // API Version.
    // 
    // The version numbers will always be equal to those that were set in 
    // CMake at the time of building the engine/game(dll/so) binaries.
    // 
    // In an ideal world, we comply to proper version releasing rules.
    // For Nail & Crescent, the general following rules apply:
    // --------------------------------------------------------------------
    // MAJOR: Ground breaking new features, you can expect anything to be 
    // incompatible at that.
    // 
    // MINOR : Everytime we have added a new feature, or if the API between
    // the Client / Server and belonging game counter-parts has actually 
    // changed.
    // 
    // POINT : Whenever changes have been made, and the above condition 
    // is not met.
    //---------------------------------------------------------------------
    struct {
        int32_t major;
        int32_t minor;
        int32_t point;
    } apiversion;

    // special messages
    void (* q_printf(2, 3) BPrintf)(int printlevel, const char *fmt, ...);
    void (* q_printf(1, 2) DPrintf)(const char *fmt, ...);
    void (* q_printf(3, 4) CPrintf)(entity_t *ent, int printlevel, const char *fmt, ...);
    void (* q_printf(2, 3) CenterPrintf)(entity_t *ent, const char *fmt, ...);
    void (*Sound)(entity_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs);
    void (*PositionedSound)(vec3_t origin, entity_t *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

    // config strings hold all the index strings, the lightstyles,
    // and misc data like the sky definition and cdtrack.
    // All of the current configstrings are sent to clients when
    // they connect, and changes are sent to all connected clients.
    void (*configstring)(int num, const char *string);

    void (* q_noreturn q_printf(1, 2) Error)(const char *fmt, ...);

    // the *index functions create configstrings and some internal server state
    int (*ModelIndex)(const char *name);
    int (*SoundIndex)(const char *name);
    int (*ImageIndex)(const char *name);

    void (*SetModel)(entity_t *ent, const char *name);

    // collision detection
    trace_t (* q_gameabi Trace)(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, entity_t *passent, int contentmask);
    int (*PointContents)(const vec3_t &point);
    qboolean (*InPVS)(const vec3_t &p1, const vec3_t &p2);
    qboolean (*InPHS)(const vec3_t &p1, const vec3_t &p2);
    void (*SetAreaPortalState)(int portalnum, qboolean open);
    qboolean (*AreasConnected)(int area1, int area2);

    // an entity will never be sent to a client or used for collision
    // if it is not passed to LinkEntity.  If the size, position, or
    // solidity changes, it must be relinked.
    void (*LinkEntity)(entity_t *ent);
    void (*UnlinkEntity)(entity_t *ent);     // call before removing an interactive edict
    int (*BoxEntities)(const vec3_t &mins, const vec3_t &maxs, entity_t **list, int maxcount, int areatype);

    // network messaging
    void (*Multicast)(const vec3_t *origin, int32_t to);
    void (*Unicast)(entity_t *ent, qboolean reliable);
    void (*WriteChar)(int c);
    void (*WriteByte)(int c);
    void (*WriteShort)(int c);
    void (*WriteLong)(int c);
    void (*WriteFloat)(float f);
    void (*WriteString)(const char *s);
    void (*WritePosition)(const vec3_t &pos);    // some fractional bits
    void (*WriteDirection)(const vec3_t &pos);         // single byte encoded, very coarse

    // managed memory allocation
    void *(*TagMalloc)(size_t size, unsigned tag);
    void (*TagFree)(void *block);
    void (*FreeTags)(unsigned tag);

    // console variable interaction
    cvar_t *(*cvar)(const char *var_name, const char *value, int flags);
    cvar_t *(*cvar_set)(const char *var_name, const char *value);
    cvar_t *(*cvar_forceset)(const char *var_name, const char *value);

    // ClientCommand and ServerCommand parameter access
    int (*argc)(void);
    const char *(*argv)(int n);     // C++20: char*
    const char *(*args)(void);      // concatenation of all argv >= 1 // C++20: char*

    // N&C: Stuff Cmd.
    void (*StuffCmd) (entity_t* pent, const char* pszCommand); // C++20: STRING: Added const to char*
    
    // add commands to the server console as if they were typed in
    // for map changing, etc
    void (*AddCommandString)(const char *text);

    void (*DebugGraph)(float value, int color);
} svgame_import_t;

//
// functions exported by the game subsystem
//

typedef struct {
    struct entity_s  *edicts;
    int         entity_size;
    int         num_edicts;     // current number, <= max_edicts
    int         max_edicts;
} EntityPool;

typedef struct {
    //---------------------------------------------------------------------
    // API Version.
    // 
    // The version numbers will always be equal to those that were set in 
    // CMake at the time of building the engine/game(dll/so) binaries.
    // 
    // In an ideal world, we comply to proper version releasing rules.
    // For Nail & Crescent, the general following rules apply:
    // --------------------------------------------------------------------
    // MAJOR: Ground breaking new features, you can expect anything to be 
    // incompatible at that.
    // 
    // MINOR : Everytime we have added a new feature, or if the API between
    // the Client / Server and belonging game counter-parts has actually 
    // changed.
    // 
    // POINT : Whenever changes have been made, and the above condition 
    // is not met.
    //---------------------------------------------------------------------
    struct {
        int32_t major;
        int32_t minor;
        int32_t point;
    } apiversion;

    // the init function will only be called when a game starts,
    // not each time a level is loaded.  Persistant data for clients
    // and the server can be allocated in init
    void (*Init)(void);
    void (*Shutdown)(void);

    // each new level entered will cause a call to SpawnEntities
    void (*SpawnEntities)(const char *mapname, const char *entstring, const char *spawnpoint);

    // Read/Write Game is for storing persistant cross level information
    // about the world state and the clients.
    // WriteGame is called every time a level is exited.
    // ReadGame is called on a loadgame.
    void (*WriteGame)(const char *filename, qboolean autosave);
    void (*ReadGame)(const char *filename);

    // ReadLevel is called after the default map information has been
    // loaded with SpawnEntities
    void (*WriteLevel)(const char *filename);
    void (*ReadLevel)(const char *filename);

    qboolean (*ClientConnect)(entity_t *ent, char *userinfo);
    void (*ClientBegin)(entity_t *ent);
    void (*ClientUserinfoChanged)(entity_t *ent, char *userinfo);
    void (*ClientDisconnect)(entity_t *ent);
    void (*ClientCommand)(entity_t *ent);
    void (*ClientThink)(entity_t *ent, ClientUserCommand *cmd);

    void (*RunFrame)(void);

    // ServerCommand will be called when an "sv <command>" command is issued on the
    // server console.
    // The game can issue gi.argc() / gi.argv() commands to get the rest
    // of the parameters
    void (*ServerCommand)(void);

    //
    // global variables shared between game and server
    //

    // The edict array is allocated in the game dll so it
    // can vary in size from one game to another.
    //
    // The size will be fixed when ge->Init() is called
//    EntityPool pool;

    struct entity_s  *edicts;
    int         entity_size;
    int         num_edicts;     // current number, <= max_edicts
    int         max_edicts;
} svgame_export_t;

#endif // __INC_SHARED__SVGAME_H__
