/*
// LICENSE HERE.

//
// entities.h
//
// All entity related functionality resides here. Need to allocate a class?
// Find an entity? Anything else? You've hit the right spot.
//
// A "ClassEntity", or a CE, is always a member of a "ServerEntity", aka an SE.
//
// The actual game logic implementation thus goes in ClassEntities. An SE is
// merely a POD binding layer between SVGame and the server. (Important for
// networking.)
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
    // Returns true in case the (server-)Entity has a Class Entity attached to it.
    inline bool EntityHasClassEntity(const Entity& ent) { return static_cast<bool>(ent.classEntity); }

    // Returns true in case the (server-)Entity has a client attached to it.
    inline bool BaseEntityHasClient(SVGBaseEntity* ent) { return ent->GetClient(); }
    // Returns true in case the BaseEntity has a ground entity set to it.
    inline bool BaseEntityHasGroundEntity(SVGBaseEntity* ent) { return ent->GetGroundEntity(); }
    // Returns true in case the BaseEntity is properly linked to a server entity.
    inline bool BaseEntityHasServerEntity(SVGBaseEntity* ent) { return ent->GetServerEntity(); }
    // Returns true if the BaseEntity contains the sought for targetname.
    inline bool BaseEntityHasTargetName(SVGBaseEntity* ent) { return ent->GetTargetName() != "" && !ent->GetTargetName().empty(); }
    // Returns true in case the BaseEntity has a client attached to it.
    inline bool BaseEntityInUse(SVGBaseEntity* ent) { return ent->IsInUse(); }
    // Returns true if the BaseEntity is NOT a nullptr.
    inline bool BaseEntityIsValidPointer(SVGBaseEntity* ent) { return ent != nullptr; }

    // Returns true in case the BaseEntity has the queried for classname.
    //inline bool BaseEntityHasClass(SVGBaseEntity* ent, std::string classname) { return ent->GetClassName() == classname; }
};


//
// Actual filters to use with GetBaseEntityRange, ..., ... TODO: What other functions?
//
namespace EntityFilters {
    using namespace std::views;

    inline auto InUse = std::views::filter( &EntityFilterFunctions::EntityInUse );
    inline auto HasClient = std::views::filter( &EntityFilterFunctions::EntityHasClient );
    inline auto HasClassEntity = std::views::filter( &EntityFilterFunctions::EntityHasClassEntity );
    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    inline auto HasClassName(const std::string& classname) {
        return std::ranges::views::filter(
            [classname /*need a copy!*/](Entity &ent) {
                return classname == ent.className;
            }
        );
    }
    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    inline auto HasKeyValue(const std::string& fieldKey, const std::string &fieldValue) {
        return std::ranges::views::filter(
            [fieldKey, fieldValue /*need a copy!*/](Entity& ent) {
                auto& dictionary = ent.entityDictionary;

                if (dictionary.find(fieldKey) != dictionary.end()) {
                    if (dictionary[fieldKey] == fieldValue) {
                        return true;
                    }
                }

                return false;
            }
        );
    }

    inline auto Standard = (InUse);
};
namespace ef = EntityFilters; // Shortcut, lesser typing.


//
// Actual filters to use with GetEntityRange, ..., ... TODO: What other functions?
//
namespace BaseEntityFilters {
    using namespace std::views;

    // BaseEntity Filters to employ by pipelining. Very nice and easy method of doing loops.
    inline auto IsValidPointer = std::views::filter( &EntityFilterFunctions::BaseEntityIsValidPointer );
    inline auto HasServerEntity = std::views::filter( &EntityFilterFunctions::BaseEntityHasServerEntity);
    inline auto HasGroundEntity = std::views::filter( &EntityFilterFunctions::BaseEntityHasGroundEntity);
    inline auto InUse = std::views::filter( &EntityFilterFunctions::BaseEntityInUse );
    inline auto HasClient = std::views::filter ( &EntityFilterFunctions::BaseEntityHasClient );

    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    inline auto HasClassName(const std::string& classname) {
        return std::ranges::views::filter(
            [classname /*need a copy!*/](SVGBaseEntity* ent) {
                return ent->GetClassName() == classname;
            }
        );
    }

    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    inline auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
        return std::ranges::views::filter(
            [fieldKey, fieldValue /*need a copy!*/](SVGBaseEntity *ent) {
                auto& dictionary = ent->GetEntityDictionary();

                if (dictionary.find(fieldKey) != dictionary.end()) {
                    if (dictionary[fieldKey] == fieldValue) {
                        return true;
                    }
                }

                return false;
            }
        );
    }

    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    template <typename ClassType>
    auto IsClassOf() {
        return std::ranges::views::filter(
            [](SVGBaseEntity* ent) {
                return ent->IsClass<ClassType>();
            }
        );
    }

    template <typename ClassType>
    auto IsSubclassOf() {
        return std::ranges::views::filter(
            [](SVGBaseEntity* ent) {
                return ent->IsSubclassOf<ClassType>();
            }
        );
    }

    // WID: TODO: This one actually has to move into EntityFilterFunctions, and then
    // be referred to from here. However, I am unsure how to do that as of yet.
    inline auto WithinRadius(vec3_t origin, float radius, uint32_t excludeSolidFlags) {
        return std::ranges::views::filter(
            [origin, radius, excludeSolidFlags/*need a copy!*/](SVGBaseEntity* ent) {
                // Find distances between entity origins.
                vec3_t entityOrigin = origin - (ent->GetOrigin() + vec3_scale(ent->GetMins() + ent->GetMaxs(), 0.5f));

                // Do they exceed our radius? Then we haven't find any.
                if (vec3_length(entityOrigin) > radius)
                    return false;

                // Cheers, we found our class entity.
                return true;
            }
        );
    }

    //
    // Summed up pipelines to simplify life with.
    //
    // A wrapper for the most likely 3 widely used, and if forgotten, error prone filters.
    inline auto Standard = (IsValidPointer | HasServerEntity | InUse);
};
namespace bef = BaseEntityFilters; // Shortcut, lesser typing.


//
// C++ using magic.
//
using EntitySpan = std::span<Entity>;
using BaseEntitySpan = std::span<SVGBaseEntity*>;

using BaseEntityVector = std::vector<SVGBaseEntity*>;

// Returns a span containing all the entities in the range of [start] to [start + count].
template <std::size_t start, std::size_t count>
inline auto GetEntityRange() -> std::span<Entity, count> {
    return std::span(g_entities).subspan<start, count>();
}

// Returns a span containing all base entities in the range of [start] to [start + count].
template <std::size_t start, std::size_t count>
inline auto GetBaseEntityRange() -> std::span<SVGBaseEntity*, count> {
    return std::span(g_baseEntities).subspan<start, count>();
}

inline EntitySpan GetEntityRange(std::size_t start, std::size_t count) {
    return EntitySpan(g_entities).subspan(start, count);
}
inline BaseEntitySpan GetBaseEntityRange(std::size_t start, std::size_t count) {
    return BaseEntitySpan(g_baseEntities).subspan(start, count);
}


//
// Entity SEARCH utilities.
//
Entity* SVG_PickTarget(char* targetName);
Entity* SVG_Find(Entity* from, int32_t fieldofs, const char* match); // C++20: Added const to char*

// Find entities within a given radius.
// Moved to gamemodes. This allows for them to customize what actually belongs in a certain radius.
// All that might sound silly, but the key here is customization.
//BaseEntityVector SVG_FindEntitiesWithinRadius(vec3_t org, float rad, uint32_t excludeSolidFlags = Solid::Not);
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