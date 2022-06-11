/***
*
*	License here.
*
*	@file
*
*	Client Game BaseEntity.
* 
***/
#pragma once


/**
*	Includes.
**/
// Type Info System.
#include "Entities/TypeInfo.h"

// Shared Entity Handle.
#include "Entities/SGEntityHandle.h"

// Shared Entity Interface.
#include "Entities/ISharedGameEntity.h"


/**
*	Using ... = Types.
**/
//! This is the actual GameWorld POD array with a size based on which GameModule we are building for.
using PODGameWorldArray = PODEntity[MAX_POD_ENTITIES];

//! std::span for PODEntity* objects.
using PODEntitySpan = std::span<PODEntity>;
//! std::vector for PODEntity* objects.
using PODEntityVector = std::vector<PODEntity*>;
//! std::span for GameEntity* derived objects.
using GameEntitySpan = std::span<GameEntity*>;
//! std::vector for GameEntity* derived objects.
using GameEntityVector = std::vector<GameEntity*>;


/**
*	Shared Constants.
**/
//! Size of the dead body entity queue.
static constexpr int32_t BODY_QUEUE_SIZE = 8;


/**
*
*
*	Entity Functions.
*
*	
**/
/**
*	@return	If the pointer is valid, either clientEntityNumber or the current state
*			server entity number. -1 otherwise.
**/
static inline int32_t SG_GetEntityNumber(PODEntity *podEntity) {
	if (!podEntity) {
		return -1;
	}

#ifdef SHAREDGAME_CLIENTGAME
	return podEntity->clientEntityNumber;
#endif
#ifdef SHAREDGAME_SERVERGAME
	return podEntity->currentState.number;
#endif
}
static inline int32_t SG_GetEntityNumber(GameEntity* geEntity) {
	if (!geEntity) {
		return -1;
	}

	return SG_GetEntityNumber(geEntity->GetPODEntity());
}


/**
*
*
*	Entity Range Filter Functions.
*
*	
**/
//! Namespace containing the actual filter function implementations.
namespace EntityFilterFunctions {
	/**
	*   @brief Filter method for checking whether an entity is in use.
	*   @return Returns true in case the entity is in use.
	**/
	inline bool PODEntityInUse(const PODEntity& ent) { return ent.inUse; }
	/**
	*   @brief Filter method for checking whether an entity has a client attached to it.
	*   @return Returns true in case the entity has a client attached to it.
	**/
	inline bool PODEntityHasClient(const PODEntity& ent) { return static_cast<bool>(ent.client); }
	/**
	*   @brief Filter method for checking whether an entity has a game entity.
	*   @return Returns true in case the entity has a game entity.
	**/
	inline bool PODEntityHasGameEntity(const PODEntity& ent) { return static_cast<bool>(ent.gameEntity); }

	/**
	*   @brief Filter method for checking whether a base entity has a client attached to it.
	*   @return Returns true in case the GameEntity has a client attached to it.
	**/
	inline bool GameEntityHasClient(GameEntity* ent) { return ent->GetClient(); }
	/**
	*   @brief Filter method for checking whether a base entity has a groundentity set.
	*   @return Returns true in case the GameEntity has a groundentity set.
	**/
	inline bool GameEntityHasGroundEntity(GameEntity* ent) { return ent->GetGroundEntityHandle(); }
	/**
	*   @brief Filter method for checking whether a GameEntity has a serverentity set.
	*   @return Returns true in case the GameEntity has a serverentity set.
	**/
	inline bool GameEntityHasPODEntity(GameEntity* ent) { return ent->GetPODEntity(); }
	/**
	*   @brief Filter method for checking whether a GameEntity has a given targetname.
	*   @return Returns true if the GameEntity has a given targetname.
	**/
	inline bool GameEntityHasTargetName(GameEntity* ent) { return ent->GetTargetName() != "" && !ent->GetTargetName().empty(); }
	/**
	*   @brief Filter method for checking whether a GameEntity is in use.
	*   @return Returns true if the GameEntity is in use.
	**/
	inline bool GameEntityInUse(GameEntity* ent) { return ent->IsInUse(); }
	/**
	*   @brief Filter method for checking whether a GameEntity is a valid pointer or not.
	*   @return Returns true if the GameEntity is a valid pointer. (Non nullptr)
	**/
	inline bool GameEntityIsValidPointer(GameEntity* ent) { return ent != nullptr; }
};



//! Namespace containing the actual Entity filter functions to use and apply.
namespace PODEntityFilters {
	using namespace std::views;

	inline auto InUse = std::views::filter(&EntityFilterFunctions::PODEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::PODEntityHasClient);
	inline auto HasGameEntity = std::views::filter(&EntityFilterFunctions::PODEntityHasGameEntity);

	inline auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
		return std::ranges::views::filter([fieldKey, fieldValue /*need a copy!*/](PODEntity& ent) {
			auto& dictionary = ent.spawnKeyValues;

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
//! Shorthand for PODEntityFilters. Less typing.
namespace pef = PODEntityFilters;



//! Namespace containing the actual Base Entity filter functions to use and apply.
namespace GameEntityFilters {
	using namespace std::ranges::views;

	inline auto IsValidPointer = std::views::filter(&EntityFilterFunctions::GameEntityIsValidPointer);
	inline auto HasServerEntity = std::views::filter(&EntityFilterFunctions::GameEntityHasPODEntity);
	inline auto HasGroundEntity = std::views::filter(&EntityFilterFunctions::GameEntityHasGroundEntity);
	inline auto InUse = std::views::filter(&EntityFilterFunctions::GameEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::GameEntityHasClient);

	// TODO: Move these functions over into EntityFilterFunctions.
	inline auto HasClassName(const std::string& classname) {
		return std::ranges::views::filter([classname /*need a copy!*/](GameEntity* ent) { return ent->GetClassname() == classname; });
	}

	inline auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
		return std::ranges::views::filter([fieldKey, fieldValue /*need a copy!*/](GameEntity* ent) {
			auto&& dictionary = ent->GetEntityDictionary();

			if (dictionary.find(fieldKey) != dictionary.end()) {
				if (dictionary[fieldKey] == fieldValue) {
					return true;
				}
			}

			return false;
		});
	}

	template<typename ClassType> auto IsClassOf() {
		return std::ranges::views::filter([](GameEntity* ent) { return ent->IsClass<ClassType>(); });
	}

	template<typename ClassType> auto IsSubclassOf() {
		return std::ranges::views::filter([](GameEntity* ent) { return ent->IsSubclassOf<ClassType>(); });
	}

	inline auto WithinRadius(vec3_t origin, float radius, uint32_t excludeSolidFlags) {
		return std::ranges::views::filter([origin, radius, excludeSolidFlags /*need a copy!*/](GameEntity* ent) {
			// Find distances between entity origins.
			vec3_t entityOrigin = origin - (ent->GetOrigin() + vec3_scale(ent->GetMins() + ent->GetMaxs(), 0.5f));

			// Do they exceed our radius? Then we haven't find any.
			if (vec3_length(entityOrigin) > radius) {
			    return false;
			}

			// Cheers, we found our game entity.
			return true;
		});
	}

	//! Wrapper of pipelined filter functions that are quite standard to apply.
	inline auto Standard = (IsValidPointer | HasServerEntity | InUse);
};

//! Shorthand for GameEntityFilters. Less typing.
namespace cef = GameEntityFilters;  // Shortcut, lesser typing.