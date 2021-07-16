/*
// LICENSE HERE.

//
// entities.h
//
// All entity related functionality resides here. Need to allocate a class?
// Find an entity? Anything else? You've hit the right spot.
//
// A "ClassEntity", or a CE, is always a member of a "ServerEntity", aka a SE.
//
// The actual game logic implementation thus goes in ClassEntities. An SE is
// merely a binding layer between SVGame and server.
//
*/
#ifndef __SVGAME_ENTITIES_H__
#define __SVGAME_ENTITIES_H__

// Include this guy here, gotta do so to make it work.
#include "entities/base/SVGBaseEntity.h"

//
// Filter function namespace that actually contains the entity filter implementations.
// 
namespace EntityFilterFunctions {
    // Returns true in case the (server-)Entity is in use.
    inline bool EntityInUse(const Entity& ent) { return ent.inUse; }
    // Returns true in case the (server-)Entity has a client attached to it.
    inline bool EntityHasClient(const Entity& ent) { return static_cast<bool>(ent.client); }
    // Returns true if the BaseEntity is NOT a nullptr.
    inline bool BaseEntityIsValidPointer(SVGBaseEntity* ent) { return ent != nullptr; }
    // Returns true in case the BaseEntity is properly linked to a server entity.
    inline bool BaseEntityHasServerEntity(SVGBaseEntity* ent) { return ent->GetServerEntity(); }
    // Returns true in case the BaseEntity has a client attached to it.
    inline bool BaseEntityInUse(SVGBaseEntity* ent) { return ent->IsInUse(); }
    // Returns true in case the (server-)Entity has a client attached to it.
    inline bool BaseEntityHasClient(SVGBaseEntity* ent) { return ent->GetClient(); }
};

//
// Actual filters to use with GetBaseEntityRange, ..., ... TODO: What other functions?
//
namespace EntityFilters {
    using namespace std::views;

    inline auto InUse = std::views::filter( &EntityFilterFunctions::EntityInUse );
    inline auto HasClient = std::views::filter(&EntityFilterFunctions::EntityHasClient);
};

//
// Actual filters to use with GetEntityRange, ..., ... TODO: What other functions?
//
namespace BaseEntityFilters {
    using namespace std::views;

    inline auto IsValidPointer = std::views::filter( &EntityFilterFunctions::BaseEntityIsValidPointer );
    inline auto HasServerEntity = std::views::filter( &EntityFilterFunctions::BaseEntityHasServerEntity);
    inline auto InUse = std::views::filter( &EntityFilterFunctions::BaseEntityInUse );
    inline auto HasClient = std::views::filter ( &EntityFilterFunctions::BaseEntityHasClient );
};

//
// C++ using magic.
//
using EntitySpan = std::span<Entity>;
using BaseEntitySpan = std::span<SVGBaseEntity*>;


//
// Entity SEARCH utilities.
//
Entity* SVG_PickTarget(char* targetName);
Entity* SVG_Find(Entity* from, int32_t fieldofs, const char* match); // C++20: Added const to char*

// Find entities within a given radius.
SVGBaseEntity* SVG_FindEntitiesWithinRadius(SVGBaseEntity* from, vec3_t org, float rad, uint32_t excludeSolidFlags = Solid::Not);
// Find entities based on their field(key), and field(value).
SVGBaseEntity* SVG_FindEntityByKeyValue(const std::string& fieldKey, const std::string& fieldValue, SVGBaseEntity* lastEntity = nullptr);

//
// Server Entity handling.
//
void    SVG_InitEntity(Entity* e);
void    SVG_FreeEntity(Entity* e);

Entity* SVG_GetWorldServerEntity();
Entity* SVG_Spawn(void);

Entity* SVG_CreateTargetChangeLevel(char* map);

// Admer: quick little template function to spawn entities, until we have this code in a local game class :)
template<typename entityClass>
inline entityClass* SVG_CreateClassEntity(Entity* edict = nullptr, bool allocateNewEdict = true) {
    entityClass* entity = nullptr;
    // If a null entity was passed, create a new one
    if (nullptr == edict) {
        if (allocateNewEdict) {
            edict = SVG_Spawn();
        } else {
            gi.DPrintf("WARNING: tried to spawn a class entity when the edict is null\n");
            return nullptr;
        }
    }
    // Abstract classes will have AllocateInstance as nullptr, hence we gotta check for that
    if (entityClass::ClassInfo.AllocateInstance) {
        entity = static_cast<entityClass*>(entityClass::ClassInfo.AllocateInstance(edict)); // Entities that aren't in the type info system will error out here
        edict->className = entity->GetTypeInfo()->className;
        edict->classEntity = entity;
        if (nullptr == g_baseEntities[edict->state.number]) {
            g_baseEntities[edict->state.number] = entity;
        } else {
            gi.DPrintf("ERROR: edict %i is already taken\n", edict->state.number);
        }
    }
    return entity;
}

//
// ClassEntity handling.
//
SVGBaseEntity* SVG_GetWorldClassEntity();
SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& className);
void SVG_FreeClassEntity(Entity* ent);

#endif // __SVGAME_ENTITIES_H__