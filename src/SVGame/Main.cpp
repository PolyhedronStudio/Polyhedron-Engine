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
#include "ServerGameLocal.h"          // Include SVGame header.

// Entities.
#include "Entities.h"
//#include "Entities/Base/SVGEntityHandle.h"
#include "Entities/Base/PlayerClient.h"

// Gamemodes.
#include "Gamemodes/IGamemode.h"
#include "Gamemodes/DefaultGamemode.h"
#include "Gamemodes/CoopGamemode.h"
#include "Gamemodes/DeathmatchGamemode.h"

// Gameworld.
#include "World/GameWorld.h"

// GameLocals.
#include "GameLocals.h"

// Player related.
#include "Player/Client.h"      // Include Player Client header.
#include "Player/View.h"        // Include Player View header.

// Physics related.
#include "Physics/StepMove.h"


//-----------------
// Global Game Variables.
//
// These are used all throughout the code. To store game state related
// information, and callbacks to the engine server game API.
//-----------------
GameLocals game;
LevelLocals level;
ServerGameImports gi;       // CLEANUP: These were game_import_t and game_export_t
ServerGameExports globals;  // CLEANUP: These were game_import_t and game_export_t
TemporarySpawnFields st;

int sm_meat_index;
int snd_fry;

//-----------------
// CVars.
//-----------------
cvar_t  *gamemode;      // Stores the gamemode string: "singleplayer", "deathmatch", "coop"
cvar_t  *gamemodeflags;
cvar_t  *skill;
cvar_t  *fraglimit;
cvar_t  *timelimit;
cvar_t  *password;
cvar_t  *spectator_password;
cvar_t  *needpass;
cvar_t  *maximumclients;
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
void SVG_CreatePlayerClientEntities();
static void SVG_SetupCVars();

void SVG_RunEntity(SVGBaseEntity *ent);
void SVG_WriteGame(const char *filename, qboolean autosave);
void SVG_ReadGame(const char *filename);
void SVG_WriteLevel(const char *filename);
void SVG_ReadLevel(const char *filename);
void SVG_InitGame(void);
void SVG_RunFrame(void);

//=============================================================================
//
//	SVGame Core API entry points.
//
//=============================================================================

/**
*   @brief  This will be called when the dll is initialize. This happens 
*           when a new game is started or a save game is loaded.
**/
void SVG_InitGame(void)
{
    gi.DPrintf("==== InitServerGame ====\n");

    // Initialise the type info system
    TypeInfo::SetupSuperClasses();

    // Setup CVars that we'll be working with.
    SVG_SetupCVars();

    // Setup our game locals object.
    game.Initialize();
}

//===============
// SVG_ShutdownGame
//
// Whenever a "game" or aka a "match" ends, this gets called.
//===============
void SVG_ShutdownGame(void) {
    gi.DPrintf("==== SVG_ShutdownGame ====\n");

    // Notify game object about the shutdown so it can let its members fire
    // their last act for cleaning up.
    game.Shutdown();

    // Free all memory that was tagged with TAG_LEVEL or TAG_GAME.
    gi.FreeTags(TAG_LEVEL);
    gi.FreeTags(TAG_GAME);
}

/**
*   @brief  Returns a pointer to the structure with all entry points
*           and global variables.
**/
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

//=============================================================================
//
//	SVGame Wrappers, these are here so that functions in q_shared.cpp can do
//  their thing. This should actually, need a nicer solution such as using a
//  .lib instead, that signifies the "core" utility.
//
//=============================================================================
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


//=============================================================================
//
//	Initialization utility functions.
//
//=============================================================================
/**
*   @brief  Sets up all server game cvars and/or fetches those belonging to the engine.
**/
static void SVG_SetupCVars() {
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

    maximumclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
    gamemode = gi.cvar("gamemode", 0, 0);
    
    //deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
    //coop = gi.cvar("coop", "0", CVAR_LATCH);
    skill = gi.cvar("skill", "1", CVAR_SERVERINFO | CVAR_LATCH);

    // Change anytime vars
    gamemodeflags = gi.cvar("gamemodeflags", "0", CVAR_SERVERINFO);
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
// SVG_CreatePlayerClientEntities
//
// Allocate the client player class entities before hand. No need to redo this all over,
// that'd just be messy and complicate things more.
//=====================
//
void SVG_CreatePlayerClientEntities() {
    // Loop over the number of clients.
    const int32_t maximumClients = game.maxClients;

    // Allocate a classentity for each client in existence.
    for (int32_t i = 1; i < maximumClients + 1; i++) {
        // Fetch server entity.
        Entity* serverEntity = &g_entities[i];

        // Initialize entity.
        SVG_InitEntity(serverEntity);

        // Allocate player client class entity 
        PlayerClient *playerClientEntity = SVG_CreateClassEntity<PlayerClient>(serverEntity, false); //SVG_SpawnClassEntity(serverEntity, serverEntity->classname);
        
        // Be sure to reset their inuse, after all, they aren't in use.
        playerClientEntity->SetInUse(false);

        // Fetch client index.
        const int32_t clientIndex = i - 1; // Same as the older: serverEntity - g_entities - 1;

        // Assign the designated client to this PlayerClient entity.
        playerClientEntity->SetClient(&game.clients[clientIndex]);
    }

    SVGEntityHandle bridgeA = g_baseEntities[1];
    Entity *playerEnt = bridgeA.Get();
    auto baseClientPlayer = bridgeA;
    gi.DPrintf("=====================================================================\n");
    gi.DPrintf("=====================================================================\n");
    gi.DPrintf("=====================================================================\n");

    if (baseClientPlayer) {
	    gi.DPrintf("Found basePlayer: %s\n", baseClientPlayer->GetClassname());
    }
    if (playerEnt) {
	    gi.DPrintf("Found player entity: %i\n", playerEnt->state.number);
    }
    gi.DPrintf("=====================================================================\n");
    gi.DPrintf("=====================================================================\n");
    gi.DPrintf("=====================================================================\n");
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
    // Go through each client and calculate their final view for the state.
    // (This happens here, so we can take into consideration objects that have
    // pushed the player. And of course, because damage has been added.)
    for (int32_t clientIndex = 0; clientIndex < game.maxClients; clientIndex++) {
        // First, fetch entity state number.
        int32_t stateNumber = g_entities[1 + clientIndex].state.number;

        // Now, let's go wild. (Purposely, do not assume the pointer is a PlayerClient.)
        Entity *entity = &g_entities[stateNumber]; // WID: 1 +, because 0 == Worldspawn.
        //Entity* entity = g_entities + 1 + clientIndex;
                                                   // See if we're gooszsd to go, if not, continue for the next. 
        if (!entity || !entity->inUse || !entity->client)
            continue;

        // Ugly cast, yes, but at this point we know we can do this. And that, to do it, matters more than
        // ethics, because our morals say otherwise :D
        game.GetCurrentGamemode()->ClientEndServerFrame(entity);
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
    if ((int)gamemodeflags->value & GamemodeFlags::SameLevel) {
        SVG_HUD_BeginIntermission(SVG_CreateTargetChangeLevel(level.mapName));
        return;
    }

    // see if it's in the map list
    if (*sv_maplist->string) {
        s = strdup(sv_maplist->string);
        f = NULL;
        t = strtok(s, seps);
        while (t != NULL) {
            if (PH_StringCompare(t, level.mapName) == 0) {
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
        ent = SVG_Find(NULL, FOFS(classname), "target_changelevel");
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

        if (*password->string && PH_StringCompare(password->string, "none"))
            need |= 1;
        if (*spectator_password->string && PH_StringCompare(spectator_password->string, "none"))
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
    ServerClient   *cl;

    if (level.intermission.time)
        return;

    //if (!deathmatch->value)
    //    return;

    if (timelimit->value) {
        if (level.time >= timelimit->value * 60) {
            gi.BPrintf(PRINT_HIGH, "Timelimit hit.\n");
            SVG_EndDMLevel();
            return;
        }
    }

    if (fraglimit->value) {
        for (i = 0 ; i < maximumclients->value ; i++) {
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
================
SVG_RunFrame

Advances the world by FRAMETIME(for 50hz=0.019) seconds
================
*/
void SVG_RunFrame(void) {
    // We're moving the game a frame forward.
    level.frameNumber++;

    // Calculate the current frame time for this game its own frame number.
    level.time = level.frameNumber * FRAMETIME;

    // Check for whether an intermission point wants to exit this level.
    if (level.intermission.exitIntermission) {
        //SVG_ExitLevel();
        game.GetCurrentGamemode()->OnLevelExit();
        return;
    }

    //
    // Treat each object in turn
    // "even the world gets a chance to Think", it does.
    //
    // Loop through the server entities, and run the base entity frame if any exists.
    for (int32_t i = 0; i < globals.numberOfEntities; i++) {
        // Acquire state number.
        int32_t stateNumber = g_entities[i].state.number;

        // Fetch the corresponding base entity.
        //SVGBaseEntity* entity = g_baseEntities[stateNumber];
        SVGBaseEntity* entity = g_entities[i].classEntity;

        // Reset level current entity.
        level.currentEntity = nullptr;

        // Is it even valid?
        if (entity == nullptr)
            continue;
        
        // Does it have a server entity?
        if (!entity->GetServerEntity())
            continue;

        // Don't go on if it isn't in use.
        if (!entity->IsInUse())
            continue;

        // Admer: entity was marked for removal at the previous tick
        if (entity->GetServerFlags() & EntityServerFlags::Remove) {
            // Free server entity.
            SVG_FreeEntity(entity->GetServerEntity());

            // Be sure to unset the server entity on this SVGBaseEntity for the current frame.
            // 
            // Other entities may wish to point at this entity for the current tick. By unsetting
            // the server entity we can prevent malicious situations from happening.
            entity->SetServerEntity(nullptr);

            // Skip further processing of this entity, it's removed.
            continue;
        }

        // Let the level data know which entity we are processing right now.
        level.currentEntity = entity;

        // Store previous(old) origin.
        entity->SetOldOrigin(entity->GetOrigin());

        // If the ground entity moved, make sure we are still on it
        if ((entity->GetServerEntity() && entity->GetGroundEntity() && entity->GetGroundEntity()->GetServerEntity())
            && (entity->GetGroundEntity()->GetLinkCount() != entity->GetGroundEntityLinkCount())) {
            // Reset ground entity.
            entity->SetGroundEntity(nullptr);

            // Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
            if (!(entity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (entity->GetServerFlags() & EntityServerFlags::Monster)) {
                // Check for a new ground entity that resides below this entity.
                SVG_StepMove_CheckGround(entity);
            }
        }

        // Time to begin a server frame for all of our clients. (This has to ha
        if (i > 0 && i <= maximumclients->value) {
            // Ensure the entity is in posession of a client that controls it.
            ServerClient* client = entity->GetClient();
            if (!client) {
                continue;
            }

            // If the entity is NOT a PlayerClient (sub-)class, skip.
            if (!entity->GetTypeInfo()->IsSubclassOf(PlayerClient::ClassInfo)) {
                continue;
            }

            // Last but not least, begin its server frame.
            game.GetCurrentGamemode()->ClientBeginServerFrame(dynamic_cast<PlayerClient*>(entity), client);

            continue;
        }

        // Last but not least, "run" process the entity.
        SVG_RunEntity(entity);
    }

    // See if it is time to end a deathmatch.
    SVG_CheckDMRules();

    // See if needpass needs updated.
    SVG_CheckNeedPass();

    // Build the playerstate_t structures for all players in this frame.
    SVG_ClientEndServerFrames();
}