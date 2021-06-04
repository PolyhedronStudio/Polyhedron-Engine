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

#include "g_local.h"          // Include SVGame header.
#include "entities.h"
#include "entities/base/SVGBaseEntity.h"
#include "player/client.h"      // Include Player Client header.
#include "player/view.h"        // Include Player View header.
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

void SVG_SpawnEntities(const char *mapName, const char *entities, const char *spawnpoint);

void SVG_RunEntity(SVGBaseEntity *ent);
void SVG_WriteGame(const char *filename, qboolean autosave);
void SVG_ReadGame(const char *filename);
void SVG_WriteLevel(const char *filename);
void SVG_ReadLevel(const char *filename);
void SVG_InitGame(void);
void SVG_RunFrame(void);


//===================================================================


void SVG_ShutdownGame(void)
{
    gi.DPrintf("==== SVG_ShutdownGame ====\n");

    // WatIs: C++-ify: Delete the edicts and clients arrays, they are allocated using new [], so need a delete[]
    //if (g_entities)
    //    delete[] g_entities;

    //if (game.clients)
    //    delete[] game.clients;

    // Shutdown the game.
    gi.FreeTags(TAG_LEVEL);
    gi.FreeTags(TAG_GAME);
}

/*
============
SVG_InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void SVG_InitGame(void)
{

    gi.DPrintf("==== InitServerGame ====\n");

    gun_x = gi.cvar("gun_x", "0", 0);
    gun_y = gi.cvar("gun_y", "0", 0);
    gun_z = gi.cvar("gun_z", "0", 0);

    //FIXME: sv_ prefix is wrong for these
    sv_rollspeed = gi.cvar("sv_rollspeed", "200", 0);
    sv_rollangle = gi.cvar("sv_rollangle", "2", 0);
    sv_maxvelocity = gi.cvar("sv_maxvelocity", "2000", 0);
    sv_gravity = gi.cvar("sv_gravity", "750", 0);

    // noset vars
    dedicated = gi.cvar("dedicated", "0", CVAR_NOSET);

    // latched vars
    sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

    maxClients = gi.cvar("maxClients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
    deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
    coop = gi.cvar("coop", "0", CVAR_LATCH);
    skill = gi.cvar("skill", "1", CVAR_LATCH);

    // change anytime vars
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
    bob_up  = gi.cvar("bob_up", "0.005", 0);
    bob_pitch = gi.cvar("bob_pitch", "0.002", 0);
    bob_roll = gi.cvar("bob_roll", "0.002", 0);

    // flood control
    flood_msgs = gi.cvar("flood_msgs", "4", 0);
    flood_persecond = gi.cvar("flood_persecond", "4", 0);
    flood_waitdelay = gi.cvar("flood_waitdelay", "10", 0);

    // dm map list
    sv_maplist = gi.cvar("sv_maplist", "", 0);

    // Monster footsteps.
	cl_monsterfootsteps = gi.cvar("cl_monsterfootsteps", "1", 0);

    // items
    SVG_InitItems();

    // initialize all entities for this game
    game.maxEntities = MAX_EDICTS;
    game.maxEntities = Clampi(game.maxEntities, (int)maxClients->value + 1, MAX_EDICTS);
    globals.entities = g_entities;
    globals.maxEntities = game.maxEntities;

    // initialize all clients for this game
    game.maxClients = maxClients->value;
    game.clients = (GameClient*)gi.TagMalloc(game.maxClients * sizeof(game.clients[0]), TAG_GAME); // CPP: Cast
    globals.numberOfEntities = game.maxClients + 1;
}


/*
=================
GetServerGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
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

//======================================================================


/*
=================
SVG_ClientEndServerFrames
=================
*/
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
SVG_CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
Entity *SVG_CreateTargetChangeLevel(char *map)
{
    Entity *ent;

    ent = SVG_Spawn();
    ent->className = (char*)"target_changelevel"; // C++20: Added a cast.
    Q_snprintf(level.nextMap, sizeof(level.nextMap), "%s", map);
    ent->map = level.nextMap;
    return ent;
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
    if ((int)dmflags->value & DeathMatchFlags::SameLevel) {
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
        SVG_RunEntity(baseEntity);
    }

    // See if it is time to end a deathmatch
    SVG_CheckDMRules();

    // See if needpass needs updated
    SVG_CheckNeedPass();

    // Build the playerstate_t structures for all players
    SVG_ClientEndServerFrames();
}

//
// The new loop to be, but it won't run the item entities atm, for a lack of their base entity counterparts.
//
//void SVG_RunFrame(void)
//{
//    // We're moving the game a frame forward.
//    level.frameNumber++;
//
//    // Calculate the current frame time for this frame number.
//    level.time = level.frameNumber * FRAMETIME;
//
//    // Check for whether an intermission point wants to exit this level.
//    if (level.intermission.exitIntermission) {
//        SVG_ExitLevel();
//        return;
//    }
//
//    //
//    // Treat each object in turn
//    // even the world gets a chance to Think
//    //
//    SVGBaseEntity *ent = g_baseEntities[0];
//    for (int32_t i = 0 ; i < globals.numberOfEntities ; i++) {
//        // Fetch the entity.
//        ent = g_baseEntities[i];
//
//        // Need to be working with a valid base entity.
//        if (!ent)
//            continue;
//
//        if (!ent->GetServerEntity())
//            continue;
//
//        // Is it in use? If not, continue.
//        if (!ent->IsInUse())
//            continue;
//
//        // Let the level data know which entity we are processing right now.
//        level.currentEntity = ent;
//
//        // Backup origin as its Old Origin.
//        ent->SetOldOrigin(ent->GetOrigin());
//
//        // if the ground entity moved, make sure we are still on it
//        if (ent->GetGroundEntity() && (ent->GetLinkCount() != ent->GetGroundEntityLinkCount())) {
//            ent->SetGroundEntity(nullptr);
//            if (!(ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (ent->GetServerFlags() & EntityServerFlags::Monster)) {
//                SVG_StepMove_CheckGround(ent);
//            }
//        }
//
//        // Time to begin a server frame for all of our clients. (This has to ha
//        if (i > 0 && i <= maxClients->value) {
//            SVG_ClientBeginServerFrame(ent->GetServerEntity());
//            continue;
//        }
//
//        // Last but not least, "run" process the entity.
//        SVG_RunEntity(ent);
//    }
//
//    // See if it is time to end a deathmatch
//    SVG_CheckDMRules();
//
//    // See if needpass needs updated
//    SVG_CheckNeedPass();
//
//    // Build the playerstate_t structures for all players
//    SVG_ClientEndServerFrames();
//}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
//===============
// SVG_CenterPrint
//
// Wraps up gi.CenterPrintf for SVGBaseEntity, and nice std::string hurray.
//===============
//
void SVG_CenterPrint(SVGBaseEntity* ent, const std::string& str) {
    if (!ent)
        return;

    gi.CenterPrintf(ent->GetServerEntity(), "%s", str.c_str());
}

//
//===============
// SVG_CenterPrint
//
// Wraps up gi.Sound for SVGBaseEntity.
//===============
//
void SVG_Sound(SVGBaseEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset) {
    if (!ent)
        return;

    gi.Sound(ent->GetServerEntity(), channel, soundIndex, volume, attenuation, timeOffset);
}


//
//===============
// SVG_BoxEntities
//
// Returns an std::vector containing the found boxed entities. Will not exceed listCount.
//===============
//
std::vector<SVGBaseEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType) {
    Entity* boxedServerEntities[MAX_EDICTS];
    std::vector<SVGBaseEntity*> boxedBaseEntities;

    // Ensure the listCount can't exceed the max edicts.
    if (listCount > MAX_EDICTS) {
        listCount = MAX_EDICTS;
    }

    // Box the entities.
    int32_t numEntities = gi.BoxEntities(mins, maxs, boxedServerEntities, MAX_EDICTS, AREA_SOLID);

    // Go through the boxed entities list, and store there classEntities (SVGBaseEntity aka baseEntities).
    for (int32_t i = 0; i < numEntities; i++) {
        if (g_baseEntities[boxedServerEntities[i]->state.number] != nullptr)
            boxedBaseEntities.push_back(g_baseEntities[boxedServerEntities[i]->state.number]);
    }

    // Return our boxed base entities vector.
    return boxedBaseEntities;
}

//
//===============
// SVG_Trace
//
// The defacto trace function to use, for SVGBaseEntity and its derived family & friends.
//===============
//
SVGTrace SVG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, SVGBaseEntity* passent, const int32_t &contentMask) {
    // Fetch server entity in case one was passed to us.
    Entity* serverPassEntity = (passent ? passent->GetServerEntity() : NULL);

    // Execute server trace.
    trace_t trace = gi.Trace(start, mins, maxs, end, serverPassEntity, contentMask);

    // Convert results to Server Game Trace.
    SVGTrace svgTrace;
    svgTrace.allSolid = trace.allSolid;
    svgTrace.contents = trace.contents;
    svgTrace.endPosition = trace.endPosition;
    svgTrace.fraction = trace.fraction;
    svgTrace.offsets[0] = trace.offsets[0];
    svgTrace.offsets[1] = trace.offsets[1];
    svgTrace.offsets[2] = trace.offsets[2];
    svgTrace.offsets[3] = trace.offsets[3];
    svgTrace.offsets[4] = trace.offsets[4];
    svgTrace.offsets[5] = trace.offsets[5];
    svgTrace.offsets[6] = trace.offsets[6];
    svgTrace.offsets[7] = trace.offsets[7];
    svgTrace.plane = trace.plane;
    svgTrace.startSolid = trace.startSolid;
    svgTrace.surface = trace.surface;

    // Special.
    if (trace.ent) {
        uint32_t index = trace.ent->state.number;

        if (g_baseEntities[index] != NULL) {
            svgTrace.ent = g_baseEntities[index];
        } else {
            svgTrace.ent = g_entities[0].classEntity;
        }
    } else {
        svgTrace.ent = g_entities[0].classEntity;
    }

    return svgTrace;
}

//
//===============
// SVG_SetConfigString
//
// Sets the config string at the given index number.
//===============
//
void SVG_SetConfigString(const int32_t &configStringIndex, const std::string& configString) {
    gi.configstring(configStringIndex, configString.c_str());
}

//
//===============
// SVG_PrecacheModel
//
// Precaches the model and returns the model index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheModel(const std::string &filename) {
    return gi.ModelIndex(filename.c_str());
}

//
//===============
// SVG_PrecacheImage
//
// Precaches the image and returns the image index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheImage(const std::string& filename) {
    return gi.ImageIndex(filename.c_str());
}

//
//===============
// SVG_PrecacheSound
//
// Precaches the sound and returns the sound index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheSound(const std::string& filename) {
    return gi.SoundIndex(filename.c_str());
}