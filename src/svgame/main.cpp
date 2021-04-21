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
#include "player/client.h"    // Include Player Client header.

//-----------------
// Global Game Variables.
//
// These are used all throughout the code. To store game state related
// information, and callbacks to the engine server game API.
//-----------------
game_locals_t   game;
level_locals_t  level;
svgame_import_t   gi;       // CLEANUP: These were game_import_t and game_export_t
svgame_export_t   globals;  // CLEANUP: These were game_import_t and game_export_t
spawn_temp_t    st;

int sm_meat_index;
int snd_fry;
int meansOfDeath;

entity_t     *g_edicts;

cvar_t  *deathmatch;
cvar_t  *coop;
cvar_t  *dmflags;
cvar_t  *skill;
cvar_t  *fraglimit;
cvar_t  *timelimit;
cvar_t  *password;
cvar_t  *spectator_password;
cvar_t  *needpass;
cvar_t  *maxclients;
cvar_t  *maxspectators;
cvar_t  *maxentities;
cvar_t  *g_select_empty;
cvar_t  *dedicated;
cvar_t  *nomonsters;

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

cvar_t  *sv_flaregun;
cvar_t  *cl_monsterfootsteps;

void SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint);

void RunEntity(entity_t *ent);
void WriteGame(const char *filename, qboolean autosave);
void ReadGame(const char *filename);
void WriteLevel(const char *filename);
void ReadLevel(const char *filename);
void InitGame(void);
void G_RunFrame(void);


//===================================================================


void ShutdownGame(void)
{
    gi.DPrintf("==== ShutdownGame ====\n");

    // WatIs: C++-ify: Delete the edicts and clients arrays, they are allocated using new [], so need a delete[]
    //if (g_edicts)
    //    delete[] g_edicts;

    //if (game.clients)
    //    delete[] game.clients;

    // Shutdown the game.
    gi.FreeTags(TAG_LEVEL);
    gi.FreeTags(TAG_GAME);
}

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame(void)
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

	nomonsters = gi.cvar("nomonsters", "0", 0);

    // latched vars
    sv_cheats = gi.cvar("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
    gi.cvar("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

    maxclients = gi.cvar("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    maxspectators = gi.cvar("maxspectators", "4", CVAR_SERVERINFO);
    deathmatch = gi.cvar("deathmatch", "0", CVAR_LATCH);
    coop = gi.cvar("coop", "0", CVAR_LATCH);
    skill = gi.cvar("skill", "1", CVAR_LATCH);
    maxentities = gi.cvar("maxentities", "2048", CVAR_LATCH); // N&C: Pool

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

	// flare gun switch: 
	//   0 = no flare gun
	//   1 = spawn with the flare gun
	//   2 = spawn with the flare gun and some grenades
	sv_flaregun = gi.cvar("sv_flaregun", "1", 0);
	cl_monsterfootsteps = gi.cvar("cl_monsterfootsteps", "1", 0);

    // items
    InitItems();

    // initialize all entities for this game
    game.maxentities = maxentities->value;
    clamp(game.maxentities, (int)maxclients->value + 1, MAX_EDICTS);
    g_edicts = (entity_t*)gi.TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME); // CPP: Cast
    globals.edicts = g_edicts;
    globals.max_edicts = game.maxentities;

    // initialize all clients for this game
    game.maxclients = maxclients->value;
    game.clients = (gclient_t*)gi.TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME); // CPP: Cast
    globals.num_edicts = game.maxclients + 1;
}


/*
=================
GetServerGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
svgame_export_t* GetServerGameAPI(svgame_import_t* import)
{
    gi = *import;

    // Setup the API version.
    globals.apiversion = {
        SVGAME_API_VERSION_MAJOR,
        SVGAME_API_VERSION_MINOR,
        SVGAME_API_VERSION_POINT,
    };

    globals.Init = InitGame;
    globals.Shutdown = ShutdownGame;
    globals.SpawnEntities = SpawnEntities;

    globals.WriteGame = WriteGame;
    globals.ReadGame = ReadGame;
    globals.WriteLevel = WriteLevel;
    globals.ReadLevel = ReadLevel;

    globals.ClientThink = ClientThink;
    globals.ClientConnect = ClientConnect;
    globals.ClientUserinfoChanged = ClientUserinfoChanged;
    globals.ClientDisconnect = ClientDisconnect;
    globals.ClientBegin = ClientBegin;
    globals.ClientCommand = ClientCommand;

    globals.PMoveInit = PMoveInit;
    globals.PMoveEnableQW = PMoveEnableQW;

    globals.RunFrame = G_RunFrame;

    globals.ServerCommand = ServerCommand;

    globals.entity_size = sizeof(entity_t);

    return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c can link
void Com_LPrintf(print_type_t type, const char *fmt, ...)
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

void Com_Error(error_type_t type, const char *fmt, ...)
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
ClientEndServerFrames
=================
*/
void ClientEndServerFrames(void)
{
    int     i;
    entity_t *ent;

    // calc the player views now that all pushing
    // and damage has been added
    for (i = 0 ; i < maxclients->value ; i++) {
        ent = g_edicts + 1 + i;
        if (!ent->inUse || !ent->client)
            continue;
        ClientEndServerFrame(ent);
    }

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
entity_t *CreateTargetChangeLevel(char *map)
{
    entity_t *ent;

    ent = G_Spawn();
    ent->classname = (char*)"target_changelevel"; // C++20: Added a cast.
    Q_snprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
    ent->map = level.nextmap;
    return ent;
}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel(void)
{
    entity_t     *ent;
    char *s, *t, *f;
    static const char *seps = " ,\n\r";

    // stay on same level flag
    if ((int)dmflags->value & DeathMatchFlags::SameLevel) {
        HUD_BeginIntermission(CreateTargetChangeLevel(level.mapname));
        return;
    }

    // see if it's in the map list
    if (*sv_maplist->string) {
        s = strdup(sv_maplist->string);
        f = NULL;
        t = strtok(s, seps);
        while (t != NULL) {
            if (Q_stricmp(t, level.mapname) == 0) {
                // it's in the list, go to the next one
                t = strtok(NULL, seps);
                if (t == NULL) { // end of list, go to first one
                    if (f == NULL) // there isn't a first one, same level
                        HUD_BeginIntermission(CreateTargetChangeLevel(level.mapname));
                    else
                        HUD_BeginIntermission(CreateTargetChangeLevel(f));
                } else
                    HUD_BeginIntermission(CreateTargetChangeLevel(t));
                free(s);
                return;
            }
            if (!f)
                f = t;
            t = strtok(NULL, seps);
        }
        free(s);
    }

    if (level.nextmap[0]) // go to a specific map
        HUD_BeginIntermission(CreateTargetChangeLevel(level.nextmap));
    else {  // search for a changelevel
        ent = G_Find(NULL, FOFS(classname), "target_changelevel");
        if (!ent) {
            // the map designer didn't include a changelevel,
            // so create a fake ent that goes back to the same level
            HUD_BeginIntermission(CreateTargetChangeLevel(level.mapname));
            return;
        }
        HUD_BeginIntermission(ent);
    }
}


/*
=================
CheckNeedPass
=================
*/
void CheckNeedPass(void)
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
CheckDMRules
=================
*/
void CheckDMRules(void)
{
    int         i;
    gclient_t   *cl;

    if (level.intermissiontime)
        return;

    if (!deathmatch->value)
        return;

    if (timelimit->value) {
        if (level.time >= timelimit->value * 60) {
            gi.BPrintf(PRINT_HIGH, "Timelimit hit.\n");
            EndDMLevel();
            return;
        }
    }

    if (fraglimit->value) {
        for (i = 0 ; i < maxclients->value ; i++) {
            cl = game.clients + i;
            if (!g_edicts[i + 1].inUse)
                continue;

            if (cl->resp.score >= fraglimit->value) {
                gi.BPrintf(PRINT_HIGH, "Fraglimit hit.\n");
                EndDMLevel();
                return;
            }
        }
    }
}


/*
=============
ExitLevel
=============
*/
void ExitLevel(void)
{
    int     i;
    entity_t *ent;
    char    command [256];

    Q_snprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
    gi.AddCommandString(command);
    level.changemap = NULL;
    level.exitintermission = 0;
    level.intermissiontime = 0;
    ClientEndServerFrames();

    // clear some things before going to next level
    for (i = 0 ; i < maxclients->value ; i++) {
        ent = g_edicts + 1 + i;
        if (!ent->inUse)
            continue;
        if (ent->health > ent->client->pers.maxHealth)
            ent->health = ent->client->pers.maxHealth;
    }

}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame(void)
{
    int     i;
    entity_t *ent;

    level.framenum++;
    level.time = level.framenum * FRAMETIME;

    // choose a client for monsters to target this frame
    AI_SetSightClient();

    // exit intermissions

    if (level.exitintermission) {
        ExitLevel();
        return;
    }

    //
    // treat each object in turn
    // even the world gets a chance to Think
    //
    ent = &g_edicts[0];
    for (i = 0 ; i < globals.num_edicts ; i++, ent++) {
        if (!ent->inUse)
            continue;

        level.current_entity = ent;

        VectorCopy(ent->s.origin, ent->s.old_origin);

        // if the ground entity moved, make sure we are still on it
        if ((ent->groundEntityPtr) && (ent->groundEntityPtr->linkCount != ent->groundEntityLinkCount)) {
            ent->groundEntityPtr = NULL;
            if (!(ent->flags & (FL_SWIM | FL_FLY)) && (ent->svFlags & SVF_MONSTER)) {
                M_CheckGround(ent);
            }
        }

        if (i > 0 && i <= maxclients->value) {
            ClientBeginServerFrame(ent);
            continue;
        }

        G_RunEntity(ent);
    }

    // see if it is time to end a deathmatch
    CheckDMRules();

    // see if needpass needs updated
    CheckNeedPass();

    // build the playerstate_t structures for all players
    ClientEndServerFrames();
}

/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
entity_t* G_Find(entity_t* from, int fieldofs, const char* match)
{
    char* s;

    if (!from)
        from = g_edicts;
    else
        from++;

    for (; from < &g_edicts[globals.num_edicts]; from++) {
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
G_FindEntitiesWithinRadius

Returns entities that have origins within a spherical area

G_FindEntitiesWithinRadius (origin, radius)
=================
*/
entity_t* G_FindEntitiesWithinRadius(entity_t* from, vec3_t org, float rad)
{
    vec3_t  eorg;
    int     j;

    if (!from)
        from = g_edicts;
    else
        from++;
    for (; from < &g_edicts[globals.num_edicts]; from++) {
        if (!from->inUse)
            continue;
        if (from->solid == Solid::Not)
            continue;
        for (j = 0; j < 3; j++)
            eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
        if (VectorLength(eorg) > rad)
            continue;
        return from;
    }

    return NULL;
}


/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES  8

entity_t* G_PickTarget(char* targetName)
{
    entity_t* ent = NULL;
    int     num_choices = 0;
    entity_t* choice[MAXCHOICES];

    if (!targetName) {
        gi.DPrintf("G_PickTarget called with NULL targetName\n");
        return NULL;
    }

    while (1) {
        ent = G_Find(ent, FOFS(targetName), targetName);
        if (!ent)
            break;
        choice[num_choices++] = ent;
        if (num_choices == MAXCHOICES)
            break;
    }

    if (!num_choices) {
        gi.DPrintf("G_PickTarget: target %s not found\n", targetName);
        return NULL;
    }

    return choice[rand() % num_choices];
}


void G_InitEntity(entity_t* e)
{
    e->inUse = true;
    e->classname = "noclass";
    e->gravity = 1.0;
    e->s.number = e - g_edicts;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to Think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
entity_t* G_Spawn(void)
{
    int         i;
    entity_t* e;

    e = &g_edicts[game.maxclients + 1];
    for (i = game.maxclients + 1; i < globals.num_edicts; i++, e++) {
        // the first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
        if (!e->inUse && (e->freeTime < 2 || level.time - e->freeTime > 0.5)) {
            G_InitEntity(e);
            return e;
        }
    }

    if (i == game.maxentities)
        gi.Error("ED_Alloc: no free edicts");

    globals.num_edicts++;
    G_InitEntity(e);
    return e;
}

/*
=================
G_FreeEntity

Marks the edict as free
=================
*/
void G_FreeEntity(entity_t* ed)
{
    gi.UnlinkEntity(ed);        // unlink from world

    if ((ed - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE)) {
        //      gi.DPrintf("tried to free special edict\n");
        return;
    }

    // C++-ify, reset the struct itself.
    memset(ed, 0, sizeof(*ed));
    //*ed = entity_t();
    ed->classname = "freed";
    ed->freeTime = level.time;
    ed->inUse = false;
}