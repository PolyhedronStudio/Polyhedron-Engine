/***
*
*	License here.
*
*	@file
*
*	SharedGame Base Item implementation.
*
***/
//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"



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

	inline bool GameEntityHasClassName( GameEntity *geFilter, const std::string &classname ) {
		return geFilter->GetClassname() == classname; 
	}

	inline bool GameEntityHasHasKeyValue( GameEntity *geFilter, std::string& fieldKey, std::string& fieldValue ) {
		auto&& dictionary = geFilter->GetEntityDictionary();

		if (dictionary.find(fieldKey) != dictionary.end()) {
			if (dictionary[fieldKey] == fieldValue) {
				return true;
			}
		}

		return false;
	}
	/**
	*   @brief Filter method for checking whether a base entity has a client attached to it.
	*   @return Returns true in case the GameEntity has a client attached to it.
	**/
//	inline bool GameEntityHasClient(GameEntity* ent) { return ent->GetClient(); }
	/**
	*   @brief Filter method for checking whether a base entity has a groundentity set.
	*   @return Returns true in case the GameEntity has a groundentity set.
	**/
	//inline bool GameEntityHasGroundEntity(GameEntity* ent) { return ent->GetGroundEntityHandle(); }
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
	using namespace std::ranges;
	//auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue) {
	//	return views::filter([fieldKey, fieldValue /*need a copy!*/](PODEntity& ent) -> const bool {
	//		auto& dictionary = ent.spawnKeyValues;

	//		if (dictionary.find(fieldKey) != dictionary.end()) {
	//			if (dictionary[fieldKey] == fieldValue) {
	//				return true;
	//			}
	//		}

	//		return false;
	//	});
	//}
};
//! Shorthand for PODEntityFilters. Less typing.
namespace pef = PODEntityFilters;



//! Namespace containing the actual Base Entity filter functions to use and apply.
namespace GameEntityFilters {
	using namespace std::ranges;

	template<typename T1 = const std::string> 
	inline auto HasClassName( const std::string& classname ) {
		return views::filter([classname /*need a copy!*/]( GameEntity* geFilter ) -> const bool { 
			return (geFilter && geFilter->GetClassname() == classname ? true : false); 
		});
	}

	template<typename T2 = const vec3_t&, typename T3 = float, typename T4 = uint32_t>
	inline auto WithinRadius( const vec3_t &origin, float radius, uint32_t excludeSolidFlags) {
		return std::ranges::views::filter([origin, radius, excludeSolidFlags /*need a copy!*/]( GameEntity* geFilter ) -> const bool {
			// Find distances between entity origins.
			vec3_t entityOrigin = origin - (geFilter->GetOrigin() + vec3_scale( geFilter->GetMins() + geFilter->GetMaxs(), 0.5f ) );

			// Do they exceed our radius? Then we haven't find any.
			if (vec3_length(entityOrigin) > radius) {
			    return false;
			}

			// Cheers, we found our game entity.
			return true;
		});
	}
};