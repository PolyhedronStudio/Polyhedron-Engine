/***
*
*	License here.
*
*	@file
*
*	Contains implementations of filter functions that can be piped to the gameworld functions:
*		- GetServerEntityRange
*		- GetClassEntityRange
* 
*	They are best used whenever there is a need for acquiring, or looping, over a set range of entities.
*	An example usage of that is:
* 
*	for (auto &entity : GetClassEntityRange(0, MAX_EDICTS)
*			| cef::Standard	// This automatically checks for whether the pointer is valid, inUse = true, and whether the classEntity pointer is valid also.
*			| cef::HasKeyValue("target", "value")) { // Filters out any entity not having a key: target, value: targetValue.
*		// Do something with the entity... entity->Remove(); // Remove it for example.
*	}
* 
*	Note that as nice as this seems, so far there is no way to acquire the size of the returned ranges.
***/
#pragma once

// Include the entity bridge.
#include "Entities/Base/SVGEntityHandle.h"

// Include our base entity.
#include "Entities/Base/SVGBaseEntity.h"



//! Span for Entity* objects.
using ServerEntitySpan = std::span<Entity>;
//! Vector for Entity* objects.
using ServerEntityVector = std::vector<Entity*>;
//! Span for SVGBaseEntity* derived objects.
using ClassEntitySpan = std::span<SVGBaseEntity*>;
//! Vector for SVGBaseEntity* derived objects.
using ClassEntityVector = std::vector<SVGBaseEntity*>;



//! Namespace containing the actual filter function implementations.
namespace EntityFilterFunctions {
	/**
	*   @brief Filter method for checking whether an entity is in use.
	*   @return Returns true in case the entity is in use.
	**/
	inline bool ServerEntityInUse(const Entity& ent) { return ent.inUse; }
	/**
	*   @brief Filter method for checking whether an entity has a client attached to it.
	*   @return Returns true in case the entity has a client attached to it.
	**/
	inline bool ServerEntityHasClient(const Entity& ent) { return static_cast<bool>(ent.client); }
	/**
	*   @brief Filter method for checking whether an entity has a class entity.
	*   @return Returns true in case the entity has a class entity.
	**/
	inline bool ServerEntityHasClassEntity(const Entity& ent) { return static_cast<bool>(ent.classEntity); }

	/**
	*   @brief Filter method for checking whether a base entity has a client attached to it.
	*   @return Returns true in case the ClassEntity has a client attached to it.
	**/
	inline bool ClassEntityHasClient(SVGBaseEntity* ent) { return ent->GetClient(); }
	/**
	*   @brief Filter method for checking whether a base entity has a groundentity set.
	*   @return Returns true in case the ClassEntity has a groundentity set.
	**/
	inline bool ClassEntityHasGroundEntity(SVGBaseEntity* ent) { return ent->GetGroundEntity(); }
	/**
	*   @brief Filter method for checking whether a ClassEntity has a serverentity set.
	*   @return Returns true in case the ClassEntity has a serverentity set.
	**/
	inline bool ClassEntityHasServerEntity(SVGBaseEntity* ent) { return ent->GetServerEntity(); }
	/**
	*   @brief Filter method for checking whether a ClassEntity has a given targetname.
	*   @return Returns true if the ClassEntity has a given targetname.
	**/
	inline bool ClassEntityHasTargetName(SVGBaseEntity* ent) { return ent->GetTargetName() != "" && !ent->GetTargetName().empty(); }
	/**
	*   @brief Filter method for checking whether a ClassEntity is in use.
	*   @return Returns true if the ClassEntity is in use.
	**/
	inline bool ClassEntityInUse(SVGBaseEntity* ent) { return ent->IsInUse(); }
	/**
	*   @brief Filter method for checking whether a ClassEntity is a valid pointer or not.
	*   @return Returns true if the ClassEntity is a valid pointer. (Non nullptr)
	**/
	inline bool ClassEntityIsValidPointer(SVGBaseEntity* ent) { return ent != nullptr; }
};



//! Namespace containing the actual Entity filter functions to use and apply.
namespace ServerEntityFilters {
	using namespace std::views;

	inline auto InUse = std::views::filter(&EntityFilterFunctions::ServerEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::ServerEntityHasClient);
	inline auto HasClassEntity = std::views::filter(&EntityFilterFunctions::ServerEntityHasClassEntity);

	inline auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
		return std::ranges::views::filter([fieldKey, fieldValue /*need a copy!*/](Entity& ent) {
			auto& dictionary = ent.entityDictionary;

			if (dictionary.find(fieldKey) != dictionary.end()) {
				if (dictionary[fieldKey] == fieldValue) {
					return true;
				}
			}

			return false;
		});
	}
	//! Wrapper of pipelined filter functions that are quite standard to apply.
	inline auto Standard = (InUse);
};
//! Shorthand for ServerEntityFilters. Less typing.
namespace sef = ServerEntityFilters;



//! Namespace containing the actual Base Entity filter functions to use and apply.
namespace ClassEntityFilters {
	using namespace std::ranges::views;

	inline auto IsValidPointer = std::views::filter(&EntityFilterFunctions::ClassEntityIsValidPointer);
	inline auto HasServerEntity = std::views::filter(&EntityFilterFunctions::ClassEntityHasServerEntity);
	inline auto HasGroundEntity = std::views::filter(&EntityFilterFunctions::ClassEntityHasGroundEntity);
	inline auto InUse = std::views::filter(&EntityFilterFunctions::ClassEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::ClassEntityHasClient);

	// TODO: Move these functions over into EntityFilterFunctions.
	inline auto HasClassName(const std::string& classname) {
		return std::ranges::views::filter([classname /*need a copy!*/](SVGBaseEntity* ent) { return ent->GetClassname() == classname; });
	}

	inline auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
		return std::ranges::views::filter([fieldKey, fieldValue /*need a copy!*/](SVGBaseEntity* ent) {
			auto& dictionary = ent->GetEntityDictionary();

			if (dictionary.find(fieldKey) != dictionary.end()) {
				if (dictionary[fieldKey] == fieldValue) {
					return true;
				}
			}

			return false;
		});
	}

	template<typename ClassType> auto IsClassOf() {
		return std::ranges::views::filter([](SVGBaseEntity* ent) { return ent->IsClass<ClassType>(); });
	}

	template<typename ClassType> auto IsSubclassOf() {
		return std::ranges::views::filter([](SVGBaseEntity* ent) { return ent->IsSubclassOf<ClassType>(); });
	}

	inline auto WithinRadius(vec3_t origin, float radius, uint32_t excludeSolidFlags) {
		return std::ranges::views::filter([origin, radius, excludeSolidFlags /*need a copy!*/](SVGBaseEntity* ent) {
			// Find distances between entity origins.
			vec3_t entityOrigin = origin - (ent->GetOrigin() + vec3_scale(ent->GetMins() + ent->GetMaxs(), 0.5f));

			// Do they exceed our radius? Then we haven't find any.
			if (vec3_length(entityOrigin) > radius) {
			    return false;
			}

			// Cheers, we found our class entity.
			return true;
		});
	}

	//! Wrapper of pipelined filter functions that are quite standard to apply.
	inline auto Standard = (IsValidPointer | HasServerEntity | InUse);
};

//! Shorthand for ClassEntityFilters. Less typing.
namespace cef = ClassEntityFilters;  // Shortcut, lesser typing.
