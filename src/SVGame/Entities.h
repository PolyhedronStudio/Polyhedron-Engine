/***
*
*	License here.
*
*	@file
*
*	Contains entity related utilities.
*
***/
#pragma once

// Include the entity bridge.
#include "Entities/Base/SVGEntityHandle.h"

// Include our base entity.
#include "Entities/Base/SVGBaseEntity.h"

//! Span for Entity* objects.
using EntitySpan = std::span<Entity>;
//! Vector for Entity* objects.
using EntityVector = std::vector<Entity*>;
//! Span for SVGBaseEntity* derived objects.
using BaseEntitySpan = std::span<SVGBaseEntity*>;
//! Vector for SVGBaseEntity* derived objects.
using BaseEntityVector = std::vector<SVGBaseEntity*>;

/**
*   @brief Selectively acquire a list of Entity* derived objects using entity filters.
* 
*   @return Returns a span containing all the entities from the range of [start] to [start + count]
*           that passed the filter process.
**/
template <std::size_t start, std::size_t count>
inline auto GetEntityRange() -> std::span<Entity, count> {
    return std::span(g_entities).subspan<start, count>();
}

/**
*   @brief Selectively acquire a list of SVGBaseEntity* derived objects using base entity filters.
* 
*   @return Returns a span containing all the base entities from the range of [start] to [start + count]
*           that passed the filter process.
**/
template <std::size_t start, std::size_t count>
inline auto GetBaseEntityRange() -> std::span<SVGBaseEntity*, count> {
    return std::span(g_baseEntities).subspan<start, count>();
}

/**
*   @brief Selectively acquire a list of Entity* derived objects using entity filters. Use the templated version where possible.
* 
*   @return Returns a span containing all the entities from the range of [start] to [start + count]
*           that passed the filter process.
**/
inline EntitySpan GetEntityRange(std::size_t start, std::size_t count) {
    return EntitySpan(g_entities).subspan(start, count);
}

/**
*   @brief Selectively acquire a list of SVGBaseEntity* derived objects using base entity filters. Use the templated version where possible.
* 
*   @return Returns a span containing all the base entities from the range of [start] to [start + count]
*           that passed the filter process.
**/
inline BaseEntitySpan GetBaseEntityRange(std::size_t start, std::size_t count) {
    return BaseEntitySpan(g_baseEntities).subspan(start, count);
}

//! Namespace containing the actual filter function implementations.
namespace EntityFilterFunctions {
    /**
    *   @brief Filter method for checking whether an entity is in use.
    *   @return Returns true in case the entity is in use.
    **/
    inline bool EntityInUse(const Entity& ent) { return ent.inUse; }
    /**
    *   @brief Filter method for checking whether an entity has a client attached to it.
    *   @return Returns true in case the entity has a client attached to it.
    **/
    inline bool EntityHasClient(const Entity& ent) { return static_cast<bool>(ent.client); }
    /**
    *   @brief Filter method for checking whether an entity has a class entity.
    *   @return Returns true in case the entity has a class entity.
    **/
    inline bool EntityHasClassEntity(const Entity& ent) { return static_cast<bool>(ent.classEntity); }

    /**
    *   @brief Filter method for checking whether a base entity has a client attached to it.
    *   @return Returns true in case the BaseEntity has a client attached to it.
    **/
    inline bool BaseEntityHasClient(SVGBaseEntity* ent) { return ent->GetClient(); }
    /**
    *   @brief Filter method for checking whether a base entity has a groundentity set.
    *   @return Returns true in case the BaseEntity has a groundentity set.
    **/
    inline bool BaseEntityHasGroundEntity(SVGBaseEntity* ent) { return ent->GetGroundEntity(); }
    /**
    *   @brief Filter method for checking whether a BaseEntity has a serverentity set.
    *   @return Returns true in case the BaseEntity has a serverentity set.
    **/
    inline bool BaseEntityHasServerEntity(SVGBaseEntity* ent) { return ent->GetServerEntity(); }
    /**
    *   @brief Filter method for checking whether a BaseEntity has a given targetname.
    *   @return Returns true if the BaseEntity has a given targetname.
    **/
    inline bool BaseEntityHasTargetName(SVGBaseEntity* ent) { return ent->GetTargetName() != "" && !ent->GetTargetName().empty(); }
    /**
    *   @brief Filter method for checking whether a BaseEntity is in use.
    *   @return Returns true if the BaseEntity is in use.
    **/
    inline bool BaseEntityInUse(SVGBaseEntity* ent) { return ent->IsInUse(); }
    /**
    *   @brief Filter method for checking whether a BaseEntity is a valid pointer or not.
    *   @return Returns true if the BaseEntity is a valid pointer. (Non nullptr)
    **/
    inline bool BaseEntityIsValidPointer(SVGBaseEntity* ent) { return ent != nullptr; }
};

//! Namespace containing the actual Entity filter functions to use and apply.
namespace EntityFilters {
    using namespace std::views;

    inline auto InUse = std::views::filter( &EntityFilterFunctions::EntityInUse );
    inline auto HasClient = std::views::filter( &EntityFilterFunctions::EntityHasClient );
    inline auto HasClassEntity = std::views::filter( &EntityFilterFunctions::EntityHasClassEntity );

    // TODO: Move these functions over into EntityFilterFunctions.
    inline auto HasClassName(const std::string& classname) {
        return std::ranges::views::filter(
            [classname /*need a copy!*/](Entity &ent) {
                return classname == ent.classname;
            }
        );
    }
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
    //! Wrapper of pipelined filter functions that are quite standard to apply.
    inline auto Standard = (InUse);
};
namespace ef = EntityFilters; // Shortcut, lesser typing.

//! Namespace containing the actual Base Entity filter functions to use and apply.
namespace BaseEntityFilters {
    using namespace std::ranges::views;

    inline auto IsValidPointer = std::views::filter( &EntityFilterFunctions::BaseEntityIsValidPointer );
    inline auto HasServerEntity = std::views::filter( &EntityFilterFunctions::BaseEntityHasServerEntity);
    inline auto HasGroundEntity = std::views::filter( &EntityFilterFunctions::BaseEntityHasGroundEntity);
    inline auto InUse = std::views::filter( &EntityFilterFunctions::BaseEntityInUse );
    inline auto HasClient = std::views::filter ( &EntityFilterFunctions::BaseEntityHasClient );

    // TODO: Move these functions over into EntityFilterFunctions.
    inline auto HasClassName(const std::string& classname) {
        return std::ranges::views::filter(
            [classname /*need a copy!*/](SVGBaseEntity* ent) {
                return ent->GetClassname() == classname;
            }
        );
    }

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

    //! Wrapper of pipelined filter functions that are quite standard to apply.
    inline auto Standard = (IsValidPointer | HasServerEntity | InUse);
};
namespace bef = BaseEntityFilters; // Shortcut, lesser typing.

//
// Entity SEARCH utilities.
//
Entity* SVG_PickTarget(char* targetName);
Entity* SVG_Find(Entity* from, int32_t fieldofs, const char* match); // C++20: Added const to char*

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
        edict->classname = entity->GetTypeInfo()->classname;
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
SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& classname);
qboolean SVG_FreeClassFromEntity(Entity* ent);