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
#include "entities/base/SVGBaseEntity.h"
#include "player/client.h"      // Include Player Client header.
#include "player/view.h"        // Include Player View header.

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
        SVG_ClientEndServerFrame(ent);
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
        if (ent->health > ent->client->persistent.maxHealth)
            ent->health = ent->client->persistent.maxHealth;
    }

}

/*
================
SVG_RunFrame

Advances the world by 0.1 seconds
================
*/
void SVG_RunFrame(void)
{
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
    SVGBaseEntity *ent = g_baseEntities[0];
    for (int32_t i = 0 ; i < globals.numberOfEntities ; i++) {
        // Fetch the entity.
        ent = g_baseEntities[i];

        // Need to be working with a valid base entity.
        if (!ent)
            continue;

        if (!ent->GetServerEntity())
            continue;

        // Is it in use? If not, continue.
        if (!ent->IsInUse())
            continue;

        // Let the level data know which entity we are processing right now.
        level.currentEntity = ent;

        // Backup origin as its Old Origin.
        ent->SetOldOrigin(ent->GetOrigin());

        // if the ground entity moved, make sure we are still on it
        if ((ent->GetServerEntity()->groundEntityPtr) && (ent->GetServerEntity()->groundEntityPtr->linkCount != ent->GetServerEntity()->groundEntityLinkCount)) {
            ent->GetServerEntity()->groundEntityPtr = NULL;
            //if (!(ent->flags & (EntityFlags::Swim | EntityFlags::Fly)) && (ent->serverFlags & EntityServerFlags::Monster)) {
            //    M_CheckGround(ent);
            //}
        }

        // Time to begin a server frame for all of our clients. (This has to ha
        if (i > 0 && i <= maxClients->value) {
            SVG_ClientBeginServerFrame(ent->GetServerEntity());
            continue;
        }

        // Last but not least, "run" process the entity.
        SVG_RunEntity(ent);
    }

    // See if it is time to end a deathmatch
    SVG_CheckDMRules();

    // See if needpass needs updated
    SVG_CheckNeedPass();

    // Build the playerstate_t structures for all players
    SVG_ClientEndServerFrames();
}

/*
=============
SVG_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
Entity* SVG_Find(Entity* from, int fieldofs, const char* match)
{
    char* s;

    if (!from)
        from = g_entities;
    else
        from++;

    for (; from < &g_entities[globals.numberOfEntities]; from++) {
        if (!from->inUse)
            continue;
        s = *(char**)((byte*)from + fieldofs);
        if (!s)
            continue;
        if (!Q_stricmp(s, match))
            return from;
    }

    return NULL;
}


/*
=================
SVG_FindEntitiesWithinRadius

Returns entities that have origins within a spherical area

SVG_FindEntitiesWithinRadius (origin, radius)
=================
*/
SVGBaseEntity* SVG_FindEntitiesWithinRadius(SVGBaseEntity* from, vec3_t org, float rad)
{
    vec3_t  eorg;
    int     j;

    Entity* serverEnt = (from ? from->GetServerEntity() : nullptr);

    if (!from)
        serverEnt = g_entities;
    else
        serverEnt++;

    for (; serverEnt < &g_entities[globals.numberOfEntities]; serverEnt++) {
        // Fetch serverEntity its ClassEntity.
        SVGBaseEntity* classEntity = serverEnt->classEntity;

        // Ensure it has a class entity.
        if (!serverEnt->classEntity)
            continue;

        if (!classEntity->IsInUse())
            continue;

        if (classEntity->GetSolid() == Solid::Not)
            continue;
        //for (j = 0; j < 3; j++)
            //eorg[j] = org[j] - (from->state.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
        eorg = org - (classEntity->GetOrigin() + vec3_scale(classEntity->GetMins() + classEntity->GetMaxs(), 0.5f));
        if (vec3_length(eorg) > rad)
            continue;

        return classEntity;
    }

    return NULL;
}


/*
=============
SVG_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES  8

Entity* SVG_PickTarget(char* targetName)
{
    Entity* ent = NULL;
    int     num_choices = 0;
    Entity* choice[MAXCHOICES];

    if (!targetName) {
        gi.DPrintf("SVG_PickTarget called with NULL targetName\n");
        return NULL;
    }

    while (1) {
        ent = SVG_Find(ent, FOFS(targetName), targetName);
        if (!ent)
            break;
        choice[num_choices++] = ent;
        if (num_choices == MAXCHOICES)
            break;
    }

    if (!num_choices) {
        gi.DPrintf("SVG_PickTarget: target %s not found\n", targetName);
        return NULL;
    }

    return choice[rand() % num_choices];
}


void SVG_InitEntity(Entity* e)
{
    e->inUse = true;
    e->className = "noclass";
    e->gravity = 1.0;
    e->state.number = e - g_entities;
}

/*
=================
SVG_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to Think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
Entity* SVG_Spawn(void)
{
    int         i;
    Entity* e;

    e = &g_entities[game.maxClients + 1];
    for (i = game.maxClients + 1; i < globals.numberOfEntities; i++, e++) {
        // the first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
        if (!e->inUse && (e->freeTime < 2 || level.time - e->freeTime > 0.5)) {
            SVG_InitEntity(e);
            return e;
        }
    }

    if (i == game.maxEntities)
        gi.Error("ED_Alloc: no free edicts");

    globals.numberOfEntities++;
    SVG_InitEntity(e);
    return e;
}

/*
=================
SVG_FreeEntity

Marks the edict as free
=================
*/
void SVG_FreeEntity(Entity* ed)
{
    // First of all, unlink the entity from this world.
    gi.UnlinkEntity(ed);        // unlink from world

    if ((ed - g_entities) <= (maxClients->value + BODY_QUEUE_SIZE)) {
        //      gi.DPrintf("tried to free special edict\n");
        return;
    }

    // Delete the actual entity pointer.
    if (ed->state.number) {
        if (g_baseEntities[ed->state.number]) {
            delete g_baseEntities[ed->state.number];
            ed->classEntity = g_baseEntities[ed->state.number] = NULL;
        }
    }

    // C++-ify, reset the struct itself.
    //memset(ed, 0, sizeof(*ed));
    *ed = {};
    //*ed = Entity();
    ed->className = "freed";
    ed->freeTime = level.time;
    ed->inUse = false;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Returns a pointer to the world entity aka Worldspawn.
Entity* SVG_GetWorldServerEntity() {
    return &g_entities[0];
};

SVGBaseEntity* SVG_GetWorldEntity() {
    return g_baseEntities[0];
};

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
        boxedBaseEntities.push_back(boxedServerEntities[i]->classEntity);
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
    Entity* serverEntity = (passent ? passent->GetServerEntity() : NULL);

    // Execute server trace.
    trace_t trace = gi.Trace(start, mins, maxs, end, serverEntity, contentMask);

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
        svgTrace.ent = g_baseEntities[index];
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