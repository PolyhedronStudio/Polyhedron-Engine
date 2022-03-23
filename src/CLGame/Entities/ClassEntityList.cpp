/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../ClientGameLocal.h"
#include "../Main.h"

// Base Entity.
#include "Base/CLGBaseEntity.h"

// Entity list.
#include "ClassEntityList.h"


/**
*   @brief  Clears the list by deallocating all its members.
**/
void ClassEntityList::Clear() {
	// Loop through the entities to notify about their deletion.
	for (auto& clgEntity : classEntities) {
		if (clgEntity) {
			// Notify about deletion.
			clgEntity->OnDeallocate();

			// Delete it from memory.
			delete clgEntity;
			clgEntity = nullptr;
		}
	}
		
	// Clear out the list.
	classEntities.clear();
}

/**
*   @brief  Spawns and inserts a new class entity determined by the hashedClassname for the state.number index, 
*			which belongs to the ClientEntity.
*   @return Pointer to the class entity object on sucess. On failure, nullptr.
**/
CLGBaseEntity* ClassEntityList::SpawnFromState(const EntityState& state, ClientEntity* clEntity) {
    // Start with a nice nullptr.
    CLGBaseEntity* spawnEntity = nullptr;

	// Safety check.
    if (!clEntity) {
		return nullptr;
    }

    // Get entity state number.
    const int32_t stateNumber = state.number;

	// Acquire hashedClassname.
	const uint32_t currentHashedClassname = state.hashedClassname;
	uint32_t previousHashedClassname = clEntity->prev.hashedClassname;

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
		// Allocate and return a pointer to the new class entity object.
		CLGBaseEntity *classEntity = InsertAt(state.number, info->AllocateInstance(clEntity));

		// If it isn't a nullptr...
		if (!classEntity) {
			Com_DPrint("Warning: ClassEntityList.InsertAt failed.\n");
			return nullptr;
			//classEntity = new CLGBaseEntity(clEntity);
		}

		// Update its current state.
		classEntity->UpdateFromState(state);

		// Return class entity.
		return classEntity;
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
*   @return A pointer to the entity who's index matches the state number.
**/
CLGBaseEntity *ClassEntityList::GetByNumber(int32_t number) {
	// Ensure ID is within bounds.
	if (number <= 0 || number > classEntities.size()) {
		return nullptr;
	}

	// Return class entity that belongs to this ID.
	return classEntities[number - 1];
}

/**
*   @brief  Inserts the class entity pointer at the number index of our class entity vector.
*   @param  force   When set to true it'll delete any previously allocated class entity occupying the given index.
*   @return Pointer to the entity being inserted. nullptr on failure.
**/
CLGBaseEntity *ClassEntityList::InsertAt(int32_t number, CLGBaseEntity *clgEntity, bool force) {
	// Ensure that the number range is valid, otherwise return a nullptr.
	if (number <= 0 || number > classEntities.capacity()) {
		return nullptr;
	}

	// If the index is already occupied...
	if (classEntities.size() >= number && classEntities[number] != nullptr) {
		// We check if we should delete it, or return a nullptr for failure.
		if (force) {
			delete classEntities[number];
			classEntities[number] = nullptr;
		} else {
			return nullptr;
		}
	}

	// We're good to go, let's insert and return a pointer to it.
	//classEntities.emplace(classEntities.begin() + number, clgEntity);
	classEntities.push_back(clgEntity);

	// Return object ptr.
	return clgEntity;
}