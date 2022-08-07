#pragma once

class ISharedGameEntity;
class IClientGameEntity;
class IServerGameEntity;

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
	inline bool PODEntityInUse( const PODEntity& ent );
	/**
	*   @brief Filter method for checking whether an entity has a client attached to it.
	*   @return Returns true in case the entity has a client attached to it.
	**/
	inline bool PODEntityHasClient( const PODEntity& ent );
	/**
	*   @brief Filter method for checking whether an entity has a game entity.
	*   @return Returns true in case the entity has a game entity.
	**/
	inline bool PODEntityHasGameEntity( const PODEntity& ent );
	/**
	*	@brief	Checks the parsed bsp spawn string EntityDictionary for a matching key.
	*	@return	True if it exists, false if it does not.
	**/
	inline bool GameEntityHasKeyValue( const std::string& fieldKey, const std::string& fieldValue );

	/**
	*   @brief Filter method for checking whether a GameEntity has a matching classname.
	*   @return True if the filtered entity has a matching classname, false otherwise.
	**/
	inline bool GameEntityHasClassName( const std::string &classname  );
	/**
	*   @brief Filter method for checking whether a GameEntity has a client attached to it.
	*   @return Returns true in case the GameEntity has a client attached to it.
	**/
	inline bool GameEntityHasClient( GameEntity* geFilter );
	/**
	*   @brief Filter method for checking whether a GameEntity has a groundentity set.
	*   @return Returns true in case the GameEntity has a groundentity set.
	**/
	inline bool GameEntityHasGroundEntity( GameEntity* geFilter );
	/**
	*	@brief	Checks the parsed bsp spawn string EntityDictionary for a matching key.
	*	@return	True if it exists, false if it does not.
	**/
	inline bool GameEntityHasKeyValue( GameEntity* geFilter, const std::string& fieldKey, const std::string& fieldValue );
	/**
	*   @brief Filter method for checking whether a GameEntity has a serverentity set.
	*   @return Returns true in case the GameEntity has a serverentity set.
	**/
	inline bool GameEntityHasPODEntity( GameEntity* geFilter );
	/**
	*   @brief Filter method for checking whether a GameEntity has a given targetname.
	*   @return Returns true if the GameEntity has a given targetname.
	**/
	inline bool GameEntityHasTargetName( GameEntity* geFilter );
	/**
	*   @brief Filter method for checking whether a GameEntity is in use.
	*   @return Returns true if the GameEntity is in use.
	**/
	inline bool GameEntityInUse( GameEntity* geFilter );
	/**
	*   @brief Filter method for checking whether a GameEntity is a valid pointer or not.
	*   @return Returns true if the GameEntity is a valid pointer. (Non nullptr)
	**/
	inline bool GameEntityIsValidPointer( GameEntity* geFilter );
	/**
	*   @brief Filter method for checking whether a GameEntity is within the specified radius.
	*   @return Returns true if the GameEntity is within the radius, false otherwise.
	**/
	inline bool GameEntityWithinRadius( const vec3_t &origin, float radius, uint32_t excludeSolidFlags );
};



//! Namespace containing the actual Entity filter functions to use and apply.
namespace PODEntityFilters {
	using namespace std::views;

	inline auto InUse = std::views::filter(&EntityFilterFunctions::PODEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::PODEntityHasClient);
	inline auto HasGameEntity = std::views::filter(&EntityFilterFunctions::PODEntityHasGameEntity );

	//auto HasKeyValue(const std::string& fieldKey, const std::string& fieldValue);

	//! Wrapper of pipelined filter functions that are quite standard to apply.
	inline auto Standard = (InUse);
};
//! Shorthand for PODEntityFilters. Less typing.
namespace pef = PODEntityFilters;



//! Namespace containing the actual Base Entity filter functions to use and apply.
namespace GameEntityFilters {
	using namespace std::ranges;

	inline auto IsValidPointer = std::views::filter(&EntityFilterFunctions::GameEntityIsValidPointer);
	inline auto HasServerEntity = std::views::filter(&EntityFilterFunctions::GameEntityHasPODEntity);
	inline auto HasGroundEntity = std::views::filter(&EntityFilterFunctions::GameEntityHasGroundEntity);
	inline auto InUse = std::views::filter(&EntityFilterFunctions::GameEntityInUse);
	inline auto HasClient = std::views::filter(&EntityFilterFunctions::GameEntityHasClient);

	// TODO: Move these functions over into EntityFilterFunctions.
	template<typename T1>
	inline auto HasClassName(const T1& classname) {
		return std::ranges::views::filter([classname /*need a copy!*/](GameEntity* ent) { return ent->GetClassname() == classname; });
	}

	template<typename T1, typename T2>
	inline auto HasKeyValue(const T1& fieldKey, const T2& fieldValue) {
		return std::ranges::views::filter([fieldKey, fieldValue /*need a copy!*/](GameEntity* ent) {
			auto dictionary = ent->GetEntityDictionary();

			if (dictionary.find(fieldKey) != dictionary.end()) {
				if (dictionary[fieldKey] == fieldValue) {
					return true;
				}
			}

			return false;
		});
	}

	template<typename ClassType> auto IsClassOf() {
		return std::ranges::views::filter(
			[](GameEntity* ent) { 
				return ent->IsClass<ClassType>(); 
			}
		);
	}

	template<typename ClassType> auto IsSubclassOf() {
		return std::ranges::views::filter(
			[](GameEntity* ent) { 
				return ent->IsSubclassOf<ClassType>(); 
			}
		);
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