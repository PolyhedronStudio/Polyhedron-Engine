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

    uint32_t stateNumber = ent->state.number;

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
    //Entity* e, * e2;
    //SVGBaseEntity *chain;
    //int     i, j;
    //int     c, c2;

    //c = 0;
    //c2 = 0;
    //for (i = 1, e = g_entities + i; i < globals.numberOfEntities; i++, e++) {
    //    // Fetch class entity.
    //    SVGBaseEntity *classEntity = g_baseEntities[e->state.number];

    //    if (classEntity == NULL)
    //        continue;

    //    if (!classEntity->IsInUse())
    //        continue;
    //    if (classEntity->GetTeam().empty())
    //        continue;
    //    if (classEntity->GetFlags() & EntityFlags::TeamSlave)
    //        continue;
    //    chain = classEntity;
    //    classEntity->SetTeamMasterEntity(classEntity);
    //    c++;
    //    c2++;
    //    for (j = i + 1, e2 = e + 1 ; j < globals.numberOfEntities ; j++, e2++) {
    //        // Fetch class entity.
    //        SVGBaseEntity* classEntity2 = g_baseEntities[e->state.number];

    //        if (classEntity2 == NULL)
    //            continue;

    //        if (!classEntity2->IsInUse())
    //            continue;
    //        if (classEntity2->GetTeam().empty())
    //            continue;
    //        if (classEntity2->GetFlags() & EntityFlags::TeamSlave)
    //            continue;
    //        if (classEntity->GetTeam() == classEntity2->GetTeam()) {
    //            c2++;
    //            chain->SetTeamChainEntity(classEntity2);
    //            classEntity2->SetTeamMasterEntity(classEntity);
    //            chain = classEntity2;
    //            classEntity2->SetFlags(classEntity2->GetFlags() | EntityFlags::TeamSlave);
    //        }
    //    }
    //}

    //gi.DPrintf("%i teams with %i entities\n", c, c2);
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
