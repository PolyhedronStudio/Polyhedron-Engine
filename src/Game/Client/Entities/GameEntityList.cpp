/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../ClientGameLocals.h"

// Base Entity.
#include "Base/CLGBaseEntity.h"

// GameEntity list.
#include "GameEntityList.h"



/**
*   @brief  Clears the list by deallocating all its members.
**/
void GameEntityList::Clear() {
	// Loop through the entities to notify about their deletion.
	for (auto& clgEntity : gameEntities) {
		if (clgEntity) {
			// Notify about deletion.
			clgEntity->OnDeallocate();

			// Delete it from memory.
			delete clgEntity;
			clgEntity = nullptr;
		}
	}
		
	// Clear out the list.
	gameEntities.clear();
}

/**
*   @brief  Creates, and assigns to the POD entity a new game entity of type 'classname'.
*   @return Pointer to the game entity object on sucess. On failure, nullptr.
**/
IClientGameEntity* GameEntityList::AllocateFromClassname(const std::string &classname, PODEntity* clEntity) {
    // Start with a nice nullptr.
    IClientGameEntity* spawnEntity = nullptr;

	// Safety check.
    if (!clEntity) {
		return nullptr;
    }

	// New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByMapName(classname.c_str());
    if (info == nullptr) {
		// Then try finding it by the C++ class name. (USE THIS FOR SPAWNING BSP STRING ENTITIES.)
		if ((info = TypeInfo::GetInfoByName(classname.c_str())) == nullptr) {
		//if ((info = TypeInfo::GetInfoByName("CLGBaseEntity")) == nullptr) {
			// Warn.
		    Com_DPrint("Warning: info = TypeInfo::GetInfoByName(\"%s\")) == nullptr\n", classname.c_str());

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && info->IsMapSpawnable()) {
		// Allocate and return a pointer to the new game entity object.
		clEntity->gameEntity = InsertAt(clEntity->clientEntityNumber, info->AllocateInstance(clEntity));

		// If it isn't a nullptr...
		if (!clEntity->gameEntity) {
			Com_DPrint("Warning: GameEntityList.InsertAt failed.\n");
			//return nullptr;

			// Perhaps instead of returning nullptr, this is where we should do a 
			// CLGBaseEntity instead.
			clEntity->gameEntity = new CLGBaseEntity(clEntity);
		}

		// Return game entity.
		return static_cast<IClientGameEntity*>(clEntity->gameEntity);
    } else {
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			Com_DPrint("Warning: tried to allocate an abstract class '%s'\n", info->classname);
		} else if (!info->IsMapSpawnable()) {
		    Com_DPrint("Warning: tried to allocate a code-only class '%s'\n", info->classname);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}

/**
*   @brief  Spawns, inserts and assignsa new game entity to the cliententity, 
*			based on the state's hashed classname.
*   @return Pointer to the game entity object on sucess. On failure, nullptr.
**/
IClientGameEntity* GameEntityList::CreateFromState(const EntityState& state, PODEntity* clEntity) {
    // Start with a nice nullptr.
    IClientGameEntity* spawnEntity = nullptr;

	// Safety check.
    if (!clEntity) {
		return nullptr;
    }

	if (clEntity->isLocal) {
		return GetByNumber(clEntity->clientEntityNumber);
	}
    // Get entity state number.
    const int32_t stateNumber = state.number;

	// Acquire hashedClassname.
	const uint32_t currentHashedClassname = state.hashedClassname;
	uint32_t previousHashedClassname = clEntity->previousState.hashedClassname;

	// If the previous and current entity number and classname hash are a match, 
	// update the current entity from state instead.
	if (currentHashedClassname == previousHashedClassname && stateNumber == clEntity->clientEntityNumber) {
		// Acquire a pointer to the already in-place game entity instead of allocating a new one.
		clEntity->gameEntity = GetByNumber(stateNumber);

		// Update it based on state and return its pointer.
		if (clEntity->gameEntity) {
			static_cast<IClientGameEntity*>(clEntity->gameEntity)->UpdateFromState(state);
		} else {
			Com_DPrint("Warning: hashed classnames and/or state and entity number mismatch:\n currentHash: %s, previousHash: %s, %i, %i\n", currentHashedClassname, previousHashedClassname, state.number, clEntity->clientEntityNumber);
		}

		return static_cast<IClientGameEntity*>(clEntity->gameEntity);
	}

    // New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByHashedMapName(currentHashedClassname);
    if (info == nullptr) {
		// Then try finding it by the C++ class name. (USE THIS FOR SPAWNING BSP STRING ENTITIES.)
		//if ((info = TypeInfo::GetInfoByName(classname.c_str())) == nullptr) {
		// 
		if ((info = TypeInfo::GetInfoByName("CLGBaseEntity")) == nullptr) {
			// Warn.
		    Com_DPrint("Warning: info = TypeInfo::GetInfoByName(\"CLGBaseEntity\")) == nullptr\n");

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && info->IsMapSpawnable()) {
		// Allocate and return a pointer to the new game entity object.
		clEntity->gameEntity = InsertAt(state.number, info->AllocateInstance(clEntity));

		// If it isn't a nullptr...
		if (!clEntity->gameEntity ) {
			Com_DPrint("Warning: GameEntityList.InsertAt failed.\n");
			return nullptr;
			//classEntity = new CLGBaseEntity(clEntity);
		}

		// Update its current state.
		static_cast<IClientGameEntity*>(clEntity->gameEntity)->UpdateFromState(state);

		// Return game entity.
		return static_cast<IClientGameEntity*>(clEntity->gameEntity);
    } else {
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			Com_DPrint("Warning: tried to allocate an abstract class '%s' (hash #%i) \n", info->classname, currentHashedClassname);
		} else if (!info->IsMapSpawnable()) {
		    Com_DPrint("Warning: tried to allocate a code-only class '%s' (hash #%i) \n", info->classname, currentHashedClassname);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}

/**
*   @return A pointer to the game entity who's index matches the state number.
**/
IClientGameEntity *GameEntityList::GetByNumber(int32_t number) {
	// Ensure ID is within bounds.
	if (number <= 0 || number > gameEntities.size()) {
		return nullptr;
	}

	// Return game entity that belongs to this ID.
	return gameEntities[number - 1];
}

/**
*   @brief  Inserts the game entity pointer at the number index of our game entity vector.
*   @param  force   When set to true it'll delete any previously allocated game entity occupying the given index.
*   @return Pointer to the entity being inserted. nullptr on failure.
**/
IClientGameEntity *GameEntityList::InsertAt(int32_t number, IClientGameEntity *clgEntity, bool force) {
	// Ensure that the number range is valid, otherwise return a nullptr.
	if (number <= 0 || number > gameEntities.capacity()) {
		return nullptr;
	}

	// We're using 0 
	const int32_t index = number; // - 1; We should actually allow index 0 and just add in a worldspawn class like on the server.

	// If the index is already occupied...
	if (gameEntities.size() > index && gameEntities[index] != nullptr) {
		// We check if we should delete it, or return a nullptr for failure.
		if (force) {
			// Notify about deletion.
			clgEntity->OnDeallocate();

			// Delete old game entity.
			delete gameEntities[index];
			gameEntities[index] = nullptr;

			// Insert & Return.
			return gameEntities[index] = clgEntity;
		} else {
			return nullptr;
		}
	}

	gameEntities.resize(number + 1);
	gameEntities[number] = clgEntity;
	// We didn't actually 
//	gameEntities.push_back(clgEntity);

	// Return object ptr.
	return clgEntity;
}