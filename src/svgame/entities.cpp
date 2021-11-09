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
#include "entities/base/SVGBaseMover.h"
#include "entities/base/PlayerClient.h"
#include "entities/info/InfoPlayerStart.h"
#include "entities/Worldspawn.h"

#include <ranges>

//-----------------
// Entity Game Variables.
//
// TODO: Explain shit, lol.
//-----------------
// Actual Server Entity array.
Entity g_entities[MAX_EDICTS];

// BaseEntity array, matches similarly index wise.
SVGBaseEntity* g_baseEntities[MAX_EDICTS];

//
// This is the old method, or at least, where we started off with.
//
//auto FetchModernMethod(std::size_t start, std::size_t end) {
//    return BaseEntityRange(&g_baseEntities[start], &g_baseEntities[end]) |
//        std::views::filter([](SVGBaseEntity* ent) {
//            return ent != nullptr && ent->GetServerEntity() && ent->IsInUse();
//        }
//    );
//}
//
//auto FetchModernMethod2(std::size_t start, std::size_t end) {
//    //return std::span(&g_entities[start], &g_entities[end]) | std::views::filter([](auto& ent) { return ent.inUse; });
//    //std::span<Entity, MAX_EDICTS>(g_entities).subspan(start, end)
//    return std::span(&g_entities[start], &g_entities[end]) | std::views::filter([](auto& ent) { return ent.inUse; });
//}
//===================================================================
//

//
// This is the new method, let's roll!
//

//
//===================================================================
//
// Testing...
#ifdef DEBUG_SHIT_FOR_ENTITIES_LULZ
#include "entities/info/InfoPlayerStart.h"
void DebugShitForEntitiesLulz() {
    gi.DPrintf("Entities - ===========================================\n");
    for (auto& entity : g_entities | EntityFilters::InUse) {
        gi.DPrintf("%s\n", entity.className);
    }
    gi.DPrintf("BaseEntities - ===========================================\n");
    for (auto* baseEntity : g_baseEntities | BaseEntityFilters::IsValidPointer | BaseEntityFilters::HasServerEntity | BaseEntityFilters::InUse) {
        gi.DPrintf("%s\n", baseEntity->GetClassName());
    }

    gi.DPrintf("Entity - info_player_start filter - ===========================================\n");
    // Hehe, only  fetch info_player_start
    for (auto& entity : GetEntityRange<0, MAX_EDICTS>()
        | ef::HasClassEntity
        | ef::InUse
        | ef::HasClassName("info_player_start")) {
        gi.DPrintf("Filtered out the entity #%i: %s\n", entity.state.number, entity.className);
    }

    gi.DPrintf("BaseEntity - info_player_start filter - ===========================================\n");
    // Hehe, only  fetch info_player_start
    for (auto* baseEntity : GetBaseEntityRange<0, MAX_EDICTS>()
        | bef::IsValidPointer
        | bef::HasServerEntity
        | bef::InUse
        | bef::IsClassOf<InfoPlayerStart>()) {
        gi.DPrintf("Filtered out the base entity #%i: %s\n", baseEntity->GetNumber(), baseEntity->GetClassName());
    }
}
#endif
//===============
// SVG_SpawnClassEntity
//
//=================
SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& className) {
    // Start with a nice nullptr.
    SVGBaseEntity* spawnEntity = nullptr;
    if ( nullptr == ent ) {
        return nullptr;
    }

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    // New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByMapName( className.c_str() );
    if ( nullptr == info ) { // Then try finding it by the C++ class name
        if ( nullptr == (info = TypeInfo::GetInfoByName( className.c_str() )) ) { 
            gi.DPrintf( "WARNING: unknown entity '%s'\n", className.c_str() );
            return nullptr; // Bail out, we didn't find one
        }
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know
    // Entity classes with 'DefineDummyMapClass' won't be reported here
    if ( nullptr != info->AllocateInstance && info->IsMapSpawnable() ) {
        return (g_baseEntities[entityNumber] = info->AllocateInstance( ent ));
    } else {
        if ( info->IsAbstract() ) {
            gi.DPrintf( "WARNING: tried to allocate an abstract class '%s'\n", info->className );
        } else if ( !info->IsMapSpawnable() ) {
            gi.DPrintf( "WARNING: tried to allocate a code-only class '%s'\n", info->className );
        }
        return nullptr;
    }
}

//===============
// SVG_FreeClassEntity
// 
// Will remove the class entity, if it exists. For fully freeing an entity,
// look for SVG_FreeEntity instead. It automatically takes care of 
// classEntities too.
//=================
void SVG_FreeClassEntity(Entity* ent) {
    // Special class entity handling IF it still has one.
    if (ent->classEntity) {
        // Remove the classEntity reference
        ent->classEntity->SetServerEntity(nullptr);
        ent->classEntity = nullptr;
    }

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    // In case it exists in our base entitys, get rid of it, assign nullptr.
    if (g_baseEntities[entityNumber]) {
        delete g_baseEntities[entityNumber];
        g_baseEntities[entityNumber] = nullptr;
    }
}


//===============
// SVG_FreeEntity
// 
// Will remove the class entity, if it exists. Continues to then mark the
// entity as "freed". (inUse = false)
//=================
void SVG_FreeEntity(Entity* ent)
{
    if (!ent)
        return;

    // Fetch entity number.
    int32_t entityNumber = ent->state.number;

    // First of all, unlink the entity from this world.
    gi.UnlinkEntity(ent);        // unlink from world

    // Prevent freeing "special edicts". Clients, and the dead "client body queue".
    if ((ent - g_entities) <= (maximumClients->value + BODY_QUEUE_SIZE)) {
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

    // Reset serverFlags.
    ent->serverFlags = 0;
}

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

//===============
// SVG_Find
// 
// Searches all active entities for the next one that holds
// the matching string at fieldofs (use the FOFS() macro) in the structure.
//
// Searches beginning at the edict after from, or the beginning if NULL
// NULL will be returned if the end of the list is reached.
//===============
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

//===============
// SVG_FindEntity
//
// Returns an entity that matches the given fieldKey and fieldValue in its 
// entity dictionary.
//===============
SVGBaseEntity* SVG_FindEntityByKeyValue(const std::string& fieldKey, const std::string& fieldValue, SVGBaseEntity* lastEntity) {
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
}

//===============
// SVG_FindEntitiesWithinRadius
// 
// Returns entities that have origins within a spherical area
// 
// SVG_FindEntitiesWithinRadius (origin, radius)
//===============
BaseEntityVector SVG_FindEntitiesWithinRadius(vec3_t origin, float radius, uint32_t excludeSolidFlags)
{
    BaseEntityVector entityList;

    // Iterate over all entities, see who is nearby, and who is not.
    for (auto* radiusEntity : GetBaseEntityRange<0, MAX_EDICTS>()
        | bef::IsValidPointer
        | bef::HasServerEntity
        | bef::InUse
        | bef::WithinRadius(origin, radius, excludeSolidFlags)) {

        // Push radiusEntity result item to the list.
        entityList.push_back(radiusEntity);
    }

    // The list might be empty, ensure to check for that ;-)
    return entityList;
}

//===============
// SVG_InitEntity
// 
// Reinitializes a ServerEntity for use.
//===============
void SVG_InitEntity(Entity* e)
{
    // From here on this entity is in use.
    e->inUse = true;

    // Set classname to "noclass", because it is.
    e->className = "noclass";

    // Reset gravity.
    //s->gravity = 1.0;

    // Last but not least, give it that ID number it so badly deserves for being initialized.
    e->state.number = e - g_entities;
}

//===============
// SVG_Spawn
// 
// Either finds a free server entity, or initializes a new one.
// Try to avoid reusing an entity that was recently freed, because it
// can cause the client to Think the entity morphed into something else
// instead of being removed and recreated, which can cause interpolated
// angles and bad trails.
//===============
Entity* SVG_Spawn(void)
{
    Entity *serverEntity = nullptr;
    int32_t i = 0;
    // Acquire a pointer to the entity we'll check for.
    serverEntity = &g_entities[game.maximumClients + 1];
    for (i = game.maximumClients + 1; i < globals.numberOfEntities; i++, serverEntity++) {
        // The first couple seconds of server time can involve a lot of
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

//=====================
// SVG_CreateTargetChangeLevel
//
// Returns the created target changelevel entity.
//=====================
Entity* SVG_CreateTargetChangeLevel(char* map) {
    Entity* ent;

    ent = SVG_Spawn();
    ent->className = (char*)"target_changelevel"; // C++20: Added a cast.
    Q_snprintf(level.nextMap, sizeof(level.nextMap), "%s", map);
    ent->map = level.nextMap;
    return ent;
}

//===============
// SVG_GetWorldServerEntity
// 
// Returns a pointer to the 'Worldspawn' ServerEntity.
//===============
Entity* SVG_GetWorldServerEntity() {
    return &g_entities[0];
};

//===============
// SVG_GetWorldClassEntity
// 
// Returns a pointer to the 'Worldspawn' ClassEntity.
//===============
SVGBaseEntity* SVG_GetWorldClassEntity() {
    return g_baseEntities[0];
};