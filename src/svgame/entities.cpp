/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "g_local.h"			// Include SVGame header.
#include "entities.h"			// Entities header.
#include "player/client.h"		// Include Player Client header.

//
// SVG_SpawnClassEntity
//
//
#include "entities/base/SVGBaseEntity.h"
#include "entities/base/SVGBaseTrigger.h"
#include "entities/base/PlayerClient.h"
#include "entities/info/InfoPlayerStart.h"
#include "entities/trigger/TriggerAlways.h"
#include "entities/trigger/TriggerMultiple.h"
#include "entities/trigger/TriggerHurt.h"
#include "entities/Worldspawn.h"
#include "entities/Light.h"
#include "entities/misc/MiscExplosionBox.h"

//
//===============
// SVG_SpawnClassEntity
// 
// 
//=================
//
SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& className) {
    // Start with a nice nullptr.
    SVGBaseEntity* spawnEntity = nullptr;

    if (!ent)
        return nullptr;

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    if (className == "misc_explobox")
        spawnEntity = g_baseEntities[entityNumber] = new MiscExplosionBox(ent);
    else if (className == "info_player_start")
        spawnEntity = g_baseEntities[entityNumber] = new InfoPlayerStart(ent);
    else if (className == "light")
        spawnEntity = g_baseEntities[entityNumber] = new Light(ent);
    else if (className == "worldspawn")
        spawnEntity = g_baseEntities[entityNumber] = new WorldSpawn(ent);
    else if (className == "trigger_always")
        spawnEntity = g_baseEntities[entityNumber] = new TriggerAlways(ent);
    else if (className == "trigger_hurt")
        spawnEntity = g_baseEntities[entityNumber] = new TriggerHurt(ent);
    else if (className == "trigger_multiple")
        spawnEntity = g_baseEntities[entityNumber] = new TriggerMultiple(ent);
    else if (className == "PlayerClient")
        spawnEntity = g_baseEntities[entityNumber] = new PlayerClient(ent);
    else
        spawnEntity = g_baseEntities[entityNumber] = new SVGBaseEntity(ent);

    return spawnEntity;
}

//
//===============
// SVG_FreeClassEntity
// 
// Will remove the class entity, if it exists. For fully freeing an entity,
// look for SVG_FreeEntity instead. It automatically takes care of 
// classEntities too.
//=================
//
void SVG_FreeClassEntity(Entity* ent) {
    // Only proceed if it has a classEntity.
    if (!ent->classEntity)
        return;

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    // In case it exists in our base entitys, get rid of it, assign nullptr.
    if (g_baseEntities[entityNumber]) {
        delete g_baseEntities[entityNumber];
        ent->classEntity = g_baseEntities[entityNumber] = nullptr;
    }
}

//
//===============
// SVG_FreeEntity
// 
// Will remove the class entity, if it exists. Continues to then mark the
// entity as "free". (inUse = false)
//=================
//
void SVG_FreeEntity(Entity* ent)
{
    if (!ent)
        return;

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    // First of all, unlink the entity from this world.
    gi.UnlinkEntity(ent);        // unlink from world

    // Prevent freeing "special edicts". Clients, and the dead "client body queue".
    if ((ent - g_entities) <= (maxClients->value + BODY_QUEUE_SIZE)) {
        //      gi.DPrintf("tried to free special edict\n");
        return;
    }

    // Delete the actual entity pointer.
    SVG_FreeClassEntity(ent);

    // Clear the struct.
    *ent = {};
    
    // Reset classname to "freed" (It is, freed...)
    ent->className = "freed";

    // Store the freeTime, so we can prevent allocating a new entity with this ID too soon.
    // If we don't, we can expect client side LERP horror.
    ent->freeTime = level.time;

    // Last but not least, since it isn't in use anymore, let it be known.
    ent->inUse = false;
}

//
//===============
// SVG_PickTarget
// 
// Searches all active entities for the next one that holds
// the matching string at fieldofs (use the FOFS() macro) in the structure.
// 
// Searches beginning at the edict after from, or the beginning if NULL
// NULL will be returned if the end of the list is reached.
//
//===============
//
#define MAXCHOICES  8

Entity* SVG_PickTarget(char* targetName)
{
    Entity* ent = nullptr;
    int     num_choices = 0;
    Entity* choice[MAXCHOICES];

    // Can't go on without a target name, can we?
    if (!targetName) {
        gi.DPrintf("SVG_PickTarget called with NULL targetName\n");
        return NULL;
    }

    // Try and find the given entity that matches this targetName.
    while (1) {
        ent = SVG_Find(ent, FOFS(targetName), targetName);
        // If we can't find it, break out of this loop.
        if (!ent)
            break;

        // If we did find one, add it to our list of targets to choose from.
        choice[num_choices++] = ent;

        // Break out in case of maximum choice limit.
        if (num_choices == MAXCHOICES)
            break;
    }

    // If there is nothing to choose from, it means we never found an entity matching this targetname.
    if (!num_choices) {
        gi.DPrintf("SVG_PickTarget: target %s not found\n", targetName);
        return NULL;
    }

    // Return a random target use % to prevent out of bounds.
    return choice[rand() % num_choices];
}

//
//===============
// SVG_Find
// 
// Searches all active entities for the next one that holds
// the matching string at fieldofs (use the FOFS() macro) in the structure.
//
// Searches beginning at the edict after from, or the beginning if NULL
// NULL will be returned if the end of the list is reached.
//===============
//
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

//
//===============
// SVG_FindEntity
//
// Returns an entity that matches the given fieldKey and fieldValue in its 
// entity dictionary.
//===============
//
SVGBaseEntity* SVG_FindEntityByKeyValue(const std::string& fieldKey, const std::string& fieldValue, SVGBaseEntity* lastEntity) {
    vec3_t  eorg;
    int     j;

    Entity* serverEnt = (lastEntity ? lastEntity->GetServerEntity() : nullptr);

    if (!lastEntity)
        serverEnt = g_entities;
    else
        serverEnt++;

    for (; serverEnt < &g_entities[globals.numberOfEntities]; serverEnt++) {
        // Fetch serverEntity its ClassEntity.
        SVGBaseEntity* classEntity = serverEnt->classEntity;

        // Ensure it has a class entity.
        if (!serverEnt->classEntity)
            continue;

        // Ensure it is in use.
        if (!classEntity->IsInUse())
            continue;

        // Start preparing for checking IF, its dictionary HAS fieldKey.
        auto dictionary = serverEnt->entityDictionary;

        if (dictionary.find(fieldKey) != dictionary.end()) {
            if (dictionary[fieldKey] == fieldValue) {
                return classEntity;
            }
        }
    }

    return nullptr;

    //if (!from)
    //    from = g_baseEntities;
    //else
    //    from++;

    //for (int32_t i = 0; from < &g_entities[globals.numberOfEntities]; from++) {
    //    s = *(char**)((byte*)from + fieldofs);
    //    if (!s)
    //        continue;
    //    if (!Q_stricmp(s, mat ch))
    //        return from;
    //}

    //// In case we have a last entity, we now know where to start.
    //int32_t start = (lastEntity != nullptr ? lastEntity->GetNumber() : 0);

    //// Very ugly, but I suppose... it has to be like this for now.
    //for (int32_t i = start; i < MAX_EDICTS; i++) {
    //    // Ensure this entity is in use. (Skip otherwise.)
    //    if (!g_entities[i].inUse)
    //        continue;

    //    // Ensure this entity has a valid class entity. (Skip otherwise.)
    //    if (!g_entities[i].classEntity)
    //        continue;

    //    // Start preparing for checking IF, its dictionary HAS fieldKey.
    //    auto dictionary = g_entities[i].entityDictionary;

    //    if (dictionary.find(fieldKey) != dictionary.end()) {
    //        if (dictionary[fieldKey] == fieldValue) {
    //            return g_entities[i].classEntity;
    //        }
    //    }
    //}

    //// We failed at finding any entity with the specific requirements, return nullptr.
    //return nullptr;
}

//
//===============
// SVG_FindEntitiesWithinRadius
// 
// Returns entities that have origins within a spherical area
// 
// SVG_FindEntitiesWithinRadius (origin, radius)
//===============
//
SVGBaseEntity* SVG_FindEntitiesWithinRadius(SVGBaseEntity* from, vec3_t origin, float radius, uint32_t excludeSolidFlags)
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

        // Ensure it is in use.
        if (!classEntity->IsInUse())
            continue;

        // And last but not least, we can't find entities within radious if they aren't solid.
        if (classEntity->GetSolid() & excludeSolidFlags)
            continue;

        // Find distances between entity origins.
        vec3_t entityOrigin = origin - (classEntity->GetOrigin() + vec3_scale(classEntity->GetMins() + classEntity->GetMaxs(), 0.5f));

        // Do they exceed our radius? Then we haven't find any.
        if (vec3_length(eorg) > radius)
            continue;

        // Cheers, we found our class entity.
        return classEntity;
    }

    return nullptr;
}

//
//===============
// SVG_InitEntity
// 
// Reinitializes a ServerEntity for use.
//===============
//
void SVG_InitEntity(Entity* e)
{
    // From here on this entity is in use.
    e->inUse = true;

    // Set classname to "noclass", because it is.
    e->className = "noclass";

    // Reset gravity.
//    e->gravity = 1.0;

    // Last but not least, give it that ID number it so badly deserves for being initialized.
    e->state.number = e - g_entities;
}

//
//===============
// SVG_Spawn
// 
// Either finds a free server entity, or initializes a new one.
// Try to avoid reusing an entity that was recently freed, because it
// can cause the client to Think the entity morphed into something else
// instead of being removed and recreated, which can cause interpolated
// angles and bad trails.
//===============
//
Entity* SVG_Spawn(void)
{
    Entity *serverEntity = nullptr;
    int32_t i = 0;
    // Acquire a pointer to the entity we'll check for.
    serverEntity = &g_entities[game.maxClients + 1];
    for (int32_t i = game.maxClients + 1; i < globals.numberOfEntities; i++, serverEntity++) {


        // the first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
        if (!serverEntity->inUse && (serverEntity->freeTime < 2 || level.time - serverEntity->freeTime > 0.5)) {
            SVG_InitEntity(serverEntity);
            return serverEntity;
        }
    }

    if (i == game.maxEntities)
        gi.Error("ED_Alloc: no free edicts");

    // If we've gotten past the gi.Error, it means we can safely increase the number of entities.
    globals.numberOfEntities++;
    SVG_InitEntity(serverEntity);

    return serverEntity;
}

//
//===============
// SVG_GetWorldServerEntity
// 
// Returns a pointer to the 'Worldspawn' ServerEntity.
//===============
//
Entity* SVG_GetWorldServerEntity() {
    return &g_entities[0];
};

//
//===============
// SVG_GetWorldClassEntity
// 
// Returns a pointer to the 'Worldspawn' ClassEntity.
//===============
//
SVGBaseEntity* SVG_GetWorldClassEntity() {
    return g_baseEntities[0];
};