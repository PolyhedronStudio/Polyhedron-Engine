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

// Core.
#include "g_local.h"          // Include SVGame header.

// Entities.
#include "entities.h"
#include "entities/base/SVGBaseEntity.h"

// Gamemodes.
#include "gamemodes/IGameMode.h"
#include "gamemodes/DefaultGameMode.h"

// Player related.
#include "player/client.h"      // Include Player Client header.
#include "player/view.h"        // Include Player View header.

// Physics related.
#include "physics/stepmove.h"

//-----------------
// Global Game Variables.
//
// These are used all throughout the code. To store game state related
// information, and callbacks to the engine server game API.
//-----------------
GameLocals   game;
LevelLocals  level;
ServerGameImports   gi;       // CLEANUP: These were game_import_t and game_export_t
ServerGameExports   globals;  // CLEANUP: These were game_import_t and game_export_t
TemporarySpawnFields    st;

int sm_meat_index;
int snd_fry;
int meansOfDeath;

// Actual Server Entity array.
Entity g_entities[MAX_EDICTS];

// BaseEntity array, matches similarly index wise.
SVGBaseEntity* g_baseEntities[MAX_EDICTS];

//-----------------
// CVars.
//-----------------
cvar_t  *deathmatch;
cvar_t  *coop;
cvar_t  *dmflags;
cvar_t  *skill;
cvar_t  *fraglimit;
cvar_t  *timelimit;
cvar_t  *password;
cvar_t  *spectator_password;
cvar_t  *needpass;
cvar_t  *maxClients;
cvar_t  *maxspectators;
cvar_t  *maxEntities;
cvar_t  *g_select_empty;
cvar_t  *dedicated;

cvar_t  *filterban;

cvar_t  *sv_maxvelocity;
cvar_t  *sv_gravity;

cvar_t  *sv_rollspeed;
cvar_t  *sv_rollangle;
cvar_t  *gun_x;
cvar_t  *gun_y;
cvar_t  *gun_z;

cvar_t  *run_pitch;
cvar_t  *run_roll;
cvar_t  *bob_up;
cvar_t  *bob_pitch;
cvar_t  *bob_roll;

cvar_t  *sv_cheats;

cvar_t  *flood_msgs;
cvar_t  *flood_persecond;
cvar_t  *flood_waitdelay;

cvar_t  *sv_maplist;

cvar_t  *cl_monsterfootsteps;


//-----------------
// Funcs used locally.
//-----------------
void SVG_SpawnEntities(const char *mapName, const char *entities, const char *spawnpoint);

void SVG_InitializeServerEntities();
void SVG_InitializeGameMode();
void SVG_AllocateGameClients();
void SVG_InitializeCVars();

void SVG_RunEntity(SVGBaseEntity *ent);
void SVG_WriteGame(const char *filename, qboolean autosave);
void SVG_ReadGame(const char *filename);
void SVG_WriteLevel(const char *filename);
void SVG_ReadLevel(const char *filename);
void SVG_InitGame(void);
void SVG_RunFrame(void);


//
//=============================================================================
//
//	SVGame Core API entry points.
//
//=============================================================================
//
//
//===============
// SVG_InitGame
//
// This will be called when the dll is first loaded, which
// only happens when a new game is started or a save game
// is loaded.
//===============
//
void SVG_InitGame(void)
{
    // WID: Informed consent. One has to know, right? :D
    gi.DPrintf("==== InitServerGame ====\n");

    // Initialize and allocate core objects for this "games" map 'round'.
    SVG_InitializeCVars();
    SVG_InitItems();
    SVG_InitializeServerEntities();
    SVG_AllocateGameClients();
    SVG_InitializeGameMode();
}

//
//===============
// SVG_ShutdownGame
//
// Whenever a "game" or aka a "match" ends, this gets called.
//===============
//
void SVG_ShutdownGame(void) {
    // Informed consent, as always. We appreciate that, dang!
    gi.DPrintf("==== SVG_ShutdownGame ====\n");

    // Aight, delete C++ vars yo. (I know this check is not required, but I like it.)
    if (game.gameMode) {
        delete game.gameMode;
        game.gameMode = nullptr;
    }

    // These old school CVars gotta be deleted from their stash. 
    gi.FreeTags(TAG_LEVEL);
    gi.FreeTags(TAG_GAME);
}

//
//===============
//GetServerGameAPI
//
// Returns a pointer to the structure with all entry points
// and global variables
//===============
//
ServerGameExports* GetServerGameAPI(ServerGameImports* import)
{
    gi = *import;

    // Setup the API version.
    globals.apiversion = {
        SVGAME_API_VERSION_MAJOR,
        SVGAME_API_VERSION_MINOR,
        SVGAME_API_VERSION_POINT,
    };

    globals.Init = SVG_InitGame;
    globals.Shutdown = SVG_ShutdownGame;
    globals.SpawnEntities = SVG_SpawnEntities;

    globals.WriteGame = SVG_WriteGame;
    globals.ReadGame = SVG_ReadGame;
    globals.WriteLevel = SVG_WriteLevel;
    globals.ReadLevel = SVG_ReadLevel;

    globals.ClientThink = SVG_ClientThink;
    globals.ClientConnect = SVG_ClientConnect;
    globals.ClientUserinfoChanged = SVG_ClientUserinfoChanged;
    globals.ClientDisconnect = SVG_ClientDisconnect;
    globals.ClientBegin = SVG_ClientBegin;
    globals.ClientCommand = SVG_ClientCommand;

    globals.RunFrame = SVG_RunFrame;

    globals.ServerCommand = SVG_ServerCommand;

    globals.entitySize = sizeof(Entity);

    return &globals;
}



//
//=============================================================================
//
//	SVGame Wrappers, these are here so that functions in q_shared.cpp can do
//  their thing. This should actually, need a nicer solution such as using a
//  .lib instead, that signifies the "core" utility.
//
//=============================================================================
//
#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c can link
void Com_LPrintf(PrintType type, const char *fmt, ...)
{
    va_list     argptr;
    char        text[MAX_STRING_CHARS];

    if (type == PRINT_DEVELOPER) {
        return;
    }

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    gi.DPrintf("%s", text);
}

void Com_Error(ErrorType type, const char *fmt, ...)
{
    va_list     argptr;
    char        text[MAX_STRING_CHARS];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    gi.Error("%s", text);
}
#endif


//
//=============================================================================
//
//	Initialization utility functions.
//
//=============================================================================
//
//
//=====================
// SVG_InitializeCVars
//
// Initializes all server game cvars and/or fetches those belonging to the engine.
//=====================
//
void SVG_InitializeCVars() {
    // Debug weapon vars.
    gun_x = gi.cvar("gun_x", "0", 0);
    gun_y = gi.cvar("gun_y", "0", 0);
    gun_z = gi.cvar("gun_z", "0", 0);

    //FIXME: sv_ prefix is wrong for these
    sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
    sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
    sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
    sv_gravity = gi.cvar("sv_gravity", "750", 0);

    // Noset vars
    dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

    // Latched vars
    sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);

    maxClients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
    deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
    coop = gi.cvar("coop", "0", CVAR_LATCH);
    skill = gi.cvar("skill", "1", CVAR_LATCH);

    // Change anytime vars
    dmflags = gi.cvar("dmflags", "0", CVAR_SERVERINFO);
    fraglimit = gi.cvar("fraglimit", "0", CVAR_SERVERINFO);
    timelimit = gi.cvar("timelimit", "0", CVAR_SERVERINFO);
    password = gi.cvar("password", "", CVAR_USERINFO);
    spectator_password = gi.cvar("spectator_password", "", CVAR_USERINFO);
    needpass = gi.cvar("needpass", "0", CVAR_SERVERINFO);
    filterban = gi.cvar("filterban", "1", 0);

    g_select_empty = gi.cvar("g_select_empty", "0", CVAR_ARCHIVE);

    run_pitch = gi.cvar("run_pitch", "0.002", 0);
    run_roll = gi.cvar("run_roll", "0.005", 0);
    bob_up = gi.cvar("bob_up", "0.005", 0);
    bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
    bob_roll = gi.cvar("bob_roll", "0.002", 0);

    // Flood control
    flood_msgs = gi.cvar("flood_msgs", "4", 0);
    flood_persecond = gi.cvar("flood_persecond", "4", 0);
    flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);

    // DM map list
    sv_maplist = gi.cvar("sv_maplist", "", 0);

    // Monster footsteps.
    cl_monsterfootsteps = gi.cvar("cl_monsterfootsteps", "1", 0);
}

//
//=====================
// SVG_InitializeServerEntities
//
// Sets up the server entity aligned array.
//=====================
//
void SVG_InitializeServerEntities() {
    // Initialize all entities for this "game", aka map that is being played.
    game.maxEntities = MAX_EDICTS;
    game.maxEntities = Clampi(game.maxEntities, (int)maxClients->value + 1, MAX_EDICTS);
    globals.entities = g_entities;
    globals.maxEntities = game.maxEntities;
}

//
//=====================
// SVG_AllocateGameClients
//
// Allocates the "GameClient", aligned to the ServerClient data type array properly for
// the current game at play.
//=====================
//
void SVG_AllocateGameClients() {
    // Initialize all clients for this game
    game.maxClients = maxClients->value;
    game.clients = (GameClient*)gi.TagMalloc(game.maxClients * sizeof(game.clients[0]), TAG_GAME); // CPP: Cast
    globals.numberOfEntities = game.maxClients + 1;
}

//
//===============
// SVG_InitializeGameMode
//
// Allocate AND initialize the proper gamemode that is set for this "game".
//===============
//
void SVG_InitializeGameMode(void) {
    // Default gamemode.
    game.gameMode = new DefaultGameMode();

}


//
//=============================================================================
//
//	Main Entry Point callback functions for the SVGame DLL.
//
//=============================================================================
//
//
//=====================
// SVG_ClientEndServerFrames
//
// Called when the game is at the end of its run for this frame, and decides to now
// instantiate a process for updating each client by the current server frame.
//=====================
//
void SVG_ClientEndServerFrames(void)
{
    int     i;
    Entity *ent;

    // Go through each client and calculate their final view for the state.
    // (This happens here, so we can take into consideration objects that have
    // pushes the player. And of course, because damage has been added.)
    for (i = 0 ; i < maxClients->value ; i++) {
        ent = g_entities + 1 + i;
        if (!ent->inUse || !ent->client)
            continue;
        SVG_ClientEndServerFrame((PlayerClient*)ent->classEntity);
    }

}

/*
=================
SVG_EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void SVG_EndDMLevel(void)
{
    Entity     *ent;
    char *s, *t, *f;
    static const char *seps = " ,\n\r";

    // stay on same level flag
    if ((int)dmflags->value & GameModeFlags::SameLevel) {
        SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(level.mapName));
        return;
    }

    // see if it's in the map list
    if (*sv_maplist->string) {
        s = strdup(sv_maplist->string);
        f = NULL;
        t = strtok(s, seps);
        while (t != NULL) {
            if (Q_stricmp(t, level.mapName) == 0) {
                // it's in the list, go to the next one
                t = strtok(NULL, seps);
                if (t == NULL) { // end of list, go to first one
                    if (f == NULL) // there isn't a first one, same level
                        SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(level.mapName));
                    else
                        SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(f));
                } else
                    SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(t));
                free(s);
                return;
            }
            if (!f)
                f = t;
            t = strtok(NULL, seps);
        }
        free(s);
    }

    if (level.nextMap[0]) // go to a specific map
        SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(level.nextMap));
    else {  // search for a changelevel
        ent = SVG_Find(NULL, FOFS(className), "target_changelevel");
        if (!ent) {
            // the map designer didn't include a changelevel,
            // so create a fake ent that goes back to the same level
            SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(level.mapName));
            return;
        }
        SVG_HUD_BeginIntermission(ent);
    }
}


/*
=================
SVG_CheckNeedPass
=================
*/
void SVG_CheckNeedPass(void)
{
    int need;

    // if password or spectator_password has changed, update needpass
    // as needed
    if (password->modified || spectator_password->modified) {
        password->modified = spectator_password->modified = false;

        need = 0;

        if (*password->string && Q_stricmp(password->string, "none"))
            need |= 1;
        if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
            need |= 2;

        gi.cvar_set("needpass", va("%d", need));
    }
}

/*
=================
SVG_CheckDMRules
=================
*/
void SVG_CheckDMRules(void)
{
    int         i;
    GameClient   *cl;

    if (level.intermission.time)
        return;

    if (!deathmatch->value)
        return;

    if (timelimit->value) {
        if (level.time >= timelimit->value * 60) {
            gi.BPrintf(PRINT_HIGH, "Timelimit hit.\n");
            SVG_EndDMLevel();
            return;
        }
    }

    if (fraglimit->value) {
        for (i = 0 ; i < maxClients->value ; i++) {
            cl = game.clients + i;
            if (!g_entities[i + 1].inUse)
                continue;

            if (cl->respawn.score >= fraglimit->value) {
                gi.BPrintf(PRINT_HIGH, "Fraglimit hit.\n");
                SVG_EndDMLevel();
                return;
            }
        }
    }
}


/*
=============
SVG_ExitLevel
=============
*/
void SVG_ExitLevel(void)
{
    int     i;
    Entity *ent;
    char    command [256];

    Q_snprintf(command, sizeof(command), "gamemap \"%s\"\n", level.intermission.changeMap);
    gi.AddCommandString(command);
    level.intermission.changeMap = NULL;
    level.intermission.exitIntermission = 0;
    level.intermission.time = 0;
    SVG_ClientEndServerFrames();

    // Clear some things before going to next level
    for (i = 0 ; i < maxClients->value ; i++) {
        ent = g_entities + 1 + i;
        if (!ent->inUse)
            continue;
        if (!ent->classEntity)
            continue;
        if (ent->classEntity->GetHealth() > ent->client->persistent.maxHealth)
            ent->classEntity->SetHealth(ent->client->persistent.maxHealth);
    }

}

/*
================
SVG_RunFrame

Advances the world by 0.05(FRAMETIME) seconds
================
*/
void SVG_RunFrame(void)
{
    int     i;
    Entity* serverEntity;
    SVGBaseEntity* baseEntity;

    // We're moving the game a frame forward.
    level.frameNumber++;

    // Calculate the current frame time for this frame number.
    level.time = level.frameNumber * FRAMETIME;

    // Check for whether an intermission point wants to exit this level.
    if (level.intermission.exitIntermission) {
        SVG_ExitLevel();
        return;
    }

    //
    // Treat each object in turn
    // even the world gets a chance to Think
    //
    serverEntity = &g_entities[0];
    for (i = 0; i < globals.numberOfEntities; i++, serverEntity++) {
        // Don't go on if it isn't in use.
        if (!serverEntity->inUse)
            continue;

        // Don't go on if there is no class entity.
        if (!serverEntity->classEntity)
            continue;
        
        // Admer: entity was marked for removal at the previous tick
        if ( serverEntity->serverFlags & EntityServerFlags::Remove )
        {
            SVG_FreeEntity( serverEntity );
            continue;
        }

        // Fetch SVGBaseEntity (or inherited variant) of this server entity.
        baseEntity = serverEntity->classEntity;

        // Let the level data know which entity we are processing right now.
        level.currentEntity = baseEntity;

        // Store previous(old) origin.
        serverEntity->state.oldOrigin = serverEntity->state.origin;

        // If the ground entity moved, make sure we are still on it
        if ((baseEntity->GetGroundEntity()) && (baseEntity->GetGroundEntity()->GetLinkCount() != baseEntity->GetGroundEntityLinkCount())) {
            baseEntity->SetGroundEntity(nullptr);

            if (!(baseEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (baseEntity->GetServerFlags() & EntityServerFlags::Monster)) {
                SVG_StepMove_CheckGround(baseEntity);
            }
        }

        // Time to begin a server frame for all of our clients. (This has to ha
        if (i > 0 && i <= maxClients->value) {
            SVG_ClientBeginServerFrame(serverEntity);
            continue;
        }

        // Last but not least, "run" process the entity.
        SVG_RunEntity(serverEntity->classEntity);
    }

    // See if it is time to end a deathmatch
    SVG_CheckDMRules();

    // See if needpass needs updated
    SVG_CheckNeedPass();

    // Build the playerstate_t structures for all players
    SVG_ClientEndServerFrames();
}