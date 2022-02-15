/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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

#include "ServerGameLocal.h"          // Include SVGame header.

// Entities.
#include "Entities.h"
#include "entities/base/SVGBaseEntity.h"

// Gamemodes.
#include "Gamemodes/IGameMode.h"

// Player Client header.
#include "Player/Client.h"


/*
=============
ED_NewString
=============
*/
static char* ED_NewString(const char* string) {
    char* newb, * new_p;
    int     i, l;

    l = strlen(string) + 1;

    newb = (char*)gi.TagMalloc(l, TAG_LEVEL); // CPP: Cast

    new_p = newb;

    for (i = 0; i < l; i++) {
        if (string[i] == '\\' && i < l - 1) {
            i++;
            if (string[i] == 'n')
                *new_p++ = '\n';
            else
                *new_p++ = '\\';
        } else
            *new_p++ = string[i];
    }

    return newb;
}

/*
===============
ED_CallSpawn

Allocates the proper server game entity class. Then spawns the entity.
===============
*/
void ED_CallSpawn(Entity *ent)
{
    auto dictionary = ent->entityDictionary;
    //ent->classname = ED_NewString( ent->entityDictionary["classname"].c_str() );
    if (!ent->entityDictionary.contains("classname")) {
	    return;
    }
    ent->classEntity = SVG_SpawnClassEntity(ent, ent->entityDictionary["classname"]);

    // If we did not find the classname, then give up
    if ( nullptr == ent->classEntity ) {
        SVG_FreeEntity( ent );
        return;
    }

    // Initialise the entity with its respected keyvalue properties
    for ( const auto& keyValueEntry : ent->entityDictionary ) {
        ent->classEntity->SpawnKey( keyValueEntry.first, keyValueEntry.second );
    }

    // Precache and spawn, to set the entity up
    ent->classEntity->Precache();
    ent->classEntity->Spawn();
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
void ED_ParseEntity(const char** data, Entity* ent) {
    qboolean    init;
    char* key, * value;

    init = false;

    // go through all the dictionary pairs
    while (1) {
        // parse key
        key = COM_Parse(data);
        if (key[0] == '}')
            break;
        if (!*data)
            gi.Error("%s: EOF without closing brace", __func__);

        // parse value
        value = COM_Parse(data);
        if (!*data)
            gi.Error("%s: EOF without closing brace", __func__);

        if (value[0] == '}')
            gi.Error("%s: closing brace without data", __func__);

        init = true;

        // keynames with a leading underscore are used for utility comments,
        // and are immediately discarded by quake
        if (key[0] == '_')
            continue;

        ent->entityDictionary[key] = value;
    }

    if (!init)
        *ent = {};
}

/*
================
SVG_FindTeams

Chain together all entities with a matching team field.

All but the first will have the EntityFlags::TeamSlave flag set.
All but the last will have the teamchain field set to the next one
================
*/
void SVG_FindTeams(void)
{
    Entity* e, * e2;
    SVGBaseEntity *chain;
    int     i, j;
    int     c, c2;

    c = 0;
    c2 = 0;
    for (i = 1, e = g_entities + i; i < globals.numberOfEntities; i++, e++) {
        // Fetch class entity.
        SVGBaseEntity *classEntity = g_baseEntities[e->state.number];

        if (classEntity == NULL)
            continue;

        if (!classEntity->IsInUse())
            continue;
        if (classEntity->GetTeam().empty())
            continue;
        if (classEntity->GetFlags() & EntityFlags::TeamSlave)
            continue;
        chain = classEntity;
        classEntity->SetTeamMasterEntity(classEntity);
        c++;
        c2++;
        for (j = i + 1, e2 = e + 1 ; j < globals.numberOfEntities ; j++, e2++) {
            // Fetch class entity.
            SVGBaseEntity* classEntity2 = g_baseEntities[e->state.number];

            if (classEntity2 == NULL)
                continue;

            if (!classEntity2->IsInUse())
                continue;
            if (classEntity2->GetTeam().empty())
                continue;
            if (classEntity2->GetFlags() & EntityFlags::TeamSlave)
                continue;
            if (classEntity->GetTeam() == classEntity2->GetTeam()) {
                c2++;
                chain->SetTeamChainEntity(classEntity2);
                classEntity2->SetTeamMasterEntity(classEntity);
                chain = classEntity2;
                classEntity2->SetFlags(classEntity2->GetFlags() | EntityFlags::TeamSlave);
            }
        }
    }

    gi.DPrintf("%i teams with %i entities\n", c, c2);
}


/*
==============
SVG_SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
extern void SVG_CreateSVGBasePlayerEntities();

void SVG_SpawnEntities(const char *mapName, const char *entities, const char *spawnpoint)
{
    Entity     *ent;
    int         inhibit;
    char        *com_token;
    int         i;
    float       skill_level;

    // Do a skill check.
    skill_level = floor(skill->value);
    if (skill_level < 0)
        skill_level = 0;
    if (skill_level > 3)
        skill_level = 3;
    if (skill->value != skill_level)
        gi.cvar_forceset("skill", va("%f", skill_level));

    // Save client data.
    if (game.GetCurrentGamemode()) {
        game.GetCurrentGamemode()->StorePlayerPersistentData();
    }

    // Free level tag allocated data.
    gi.FreeTags(TAG_LEVEL);

    // Clear level state.
    level = {};

    // Clear out entities.
    for (int32_t i = 0; i < game.maxEntities; i++) {
        // Delete class entities, if any.
        if (g_baseEntities[i]) {
            delete g_baseEntities[i];
            g_baseEntities[i] = NULL;
        }

        g_entities[i] = {};
    }

    strncpy(level.mapName, mapName, sizeof(level.mapName) - 1);
    strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint) - 1);

    // Set client fields on player entities
    for (i = 0 ; i < game.GetMaxClients() ; i++)
        g_entities[i + 1].client = game.clients + i;

    ent = NULL;
    inhibit = 0;

    // Spawn SVGBasePlayer entities first.
    SVG_CreateSVGBasePlayerEntities();

// parse ents
    while (1) {
        // parse the opening brace
        com_token = COM_Parse(&entities);
        if (!entities)
            break;
        if (com_token[0] != '{')
            gi.Error("ED_LoadFromFile: found %s when expecting {", com_token);

        if (!ent)
            ent = g_entities;
        else
            ent = SVG_Spawn();
        ED_ParseEntity(&entities, ent);

        //// yet another map hack
        //if (!PH_StringCompare(level.mapName, "command") && !PH_StringCompare(ent->classname, "trigger_once") && !PH_StringCompare(ent->model, "*27"))
        //    ent->spawnFlags &= ~EntitySpawnFlags::NotHard;

        //// remove things (except the world) from different skill levels or deathmatch
        //if (ent != g_entities) {
        //    // Do a check for deathmatch, in case the entity isn't allowed there.
        //    if (deathmatch->value) {
        //        if (ent->spawnFlags & EntitySpawnFlags::NotDeathMatch) {
        //            SVG_FreeEntity(ent);
        //            inhibit++;
        //            continue;
        //        }
        //    } else {
        //        if ( /* ((coop->value) && (ent->spawnFlags & EntitySpawnFlags::NotCoop)) || */
        //            ((skill->value == 0) && (ent->spawnFlags & EntitySpawnFlags::NotEasy)) ||
        //            ((skill->value == 1) && (ent->spawnFlags & EntitySpawnFlags::NotMedium)) ||
        //            (((skill->value == 2) || (skill->value == 3)) && (ent->spawnFlags & EntitySpawnFlags::NotHard))
        //        ) {
        //            SVG_FreeEntity(ent);
        //            inhibit++;
        //            continue;
        //        }
        //    }

        //    ent->spawnFlags &= ~(EntitySpawnFlags::NotEasy | EntitySpawnFlags::NotMedium | EntitySpawnFlags::NotHard | EntitySpawnFlags::NotCoop | EntitySpawnFlags::NotDeathMatch);
        //}

        // Allocate the class entity, and call its spawn.
        ED_CallSpawn(ent);
    }

    // Post spawn entities.
    for (int32_t i = 0; i < MAX_EDICTS; i++) {
        if (g_baseEntities[i])
            g_baseEntities[i]->PostSpawn();
    }

    gi.DPrintf("%i entities inhibited\n", inhibit);

#ifdef DEBUG
    i = 1;
    ent = EDICT_NUM(i);
    while (i < globals.pool.numberOfEntities) {
        if (ent->inUse != 0 || ent->inUse != 1)
            Com_DPrintf("Invalid entity %d\n", i);
        i++, ent++;
    }
#endif

    SVG_FindTeams();

    SVG_PlayerTrail_Init();
}


//===================================================================

#if 0
// cursor positioning
xl <value>
xr <value>
yb <value>
yt <value>
xv <value>
yv <value>

// drawing
statpic <name>
pic <stat>
num <fieldwidth> <stat>
string <stat>

// control
if <stat>
ifeq <stat> <value>
ifbit <stat> <value>
endif

#endif
