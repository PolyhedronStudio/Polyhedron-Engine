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
#include "EntityList.h"



/**
*   @brief  Clears the list by deallocating all its members.
**/
void EntityList::Clear() {
	// Loop through the entities to notify about their deletion.
	for (auto& clgEntity : classEntities) {
		if (clgEntity) {
			clgEntity->OnDeallocate();
		}
	}

	// Clear out the list.
	classEntities.clear();
}

/**
*   @return A pointer to the entity who's index matches the state number.
**/
CLGBaseEntity *EntityList::GetByStateNumber(int32_t number) {
	//// Filter to search with.
	//constexpr auto has_id = [&id](CLGBaseEntity* clgEntity) { 
	//	return (clgEntity != nullptr && clgEntity->GetEntityID() == id); 
	//};

	//// Find an entity matching entity ID in our vector.
	//auto clgEntity = std::find_if(entities.begin(), entities.end(), has_id);

	//// Return if found.
	//if (clgEntity != entities.end()) {
	//	return *clgEntity;
	//}
	
	// Ensure ID is within bounds.
	if (number < 0 || number > classEntities.size()) {
		return nullptr;
	}

	// Return class entity that belongs to this ID.
	return classEntities.at(number);
}


/**
*   @brief  Inserts the class entity pointer at the number index of our class entity vector.
*   @return Pointer to the entity being inserted.
**/
CLGBaseEntity* EntityList::InsertAtSlotNumber(CLGBaseEntity* clgEntity, int32_t number) {
	// Insert pointer at designated location.
	classEntities.insert(classEntities.begin() + number, clgEntity);

	// Return class entity that belongs to this ID.
	return classEntities.at(number);
}