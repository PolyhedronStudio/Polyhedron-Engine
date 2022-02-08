/*
// LICENSE HERE.

//
// SVGBaseEntity.cpp
//
//
*/

#include "../../ServerGameLocal.h"		// SVGame.

//
// EntityBridge.
//
#include "../../Entities.h"		// Entities.

/**
*	@brief	Helper function to acquire the SVGBaseEntity pointer from a server entity.
* 
*	@return	A valid pointer if the server entity is pointing to one. nullptr otherwise.
**/
static inline SVGBaseEntity* GetBaseClassentity(Entity* serverEntity) {
    // Reinterpret cast the classEntity pointer.
    if (serverEntity) {
	    if (serverEntity->classEntity != nullptr) {
	        return serverEntity->classEntity;
	    } else {
	        return nullptr;
	    }
    }

    // Return nullptr.
    return nullptr;
}




/**
*	@brief Simple constructor of an entity bridge that will accept a
*	class entity.
**/
SVGEntityHandle::SVGEntityHandle(SVGBaseEntity* baseEntity) : serverEntity(nullptr), entityID(0) {
    // Exploits the assignment operator.
    *this = baseEntity;
}

/**
*	@brief Simple constructor that will accept an other bridge entity.
**/
SVGEntityHandle::SVGEntityHandle(const SVGEntityHandle& other) : serverEntity(other.serverEntity), entityID(other.entityID) { }

/**
*	@brief Simple constructor that will accept a server entity and an entity number.
**/
SVGEntityHandle::SVGEntityHandle(Entity* entity, const uint32_t number) : serverEntity(entity), entityID(number) { }

/**
*	@brief	Acquire a pointer to the server entity.
*
*	@return	If the entityIDs match, a pointer to the server entity. 
*			Nullptr otherwise.
**/
Entity* SVGEntityHandle::Get() const {
    // Ensure we got a valid server entity pointer.
    if (serverEntity) {
		if (serverEntity->state.number == entityID) {
		    return serverEntity;
		} else {
		    return nullptr;
		}
    }

    // Immediately return a nullptr since we never made it to the inside of the
    // if clausule.
    return nullptr;
}

/**
*	@brief	Sets the server entity pointer and assigns its entity ID to this
*			handle in case of a non nullptr.
*
*	@return	The pointer passed to the function name.
**/
Entity* SVGEntityHandle::Set(Entity* entity) {
    // Set new pointer.
    serverEntity = entity;

    // In case of a valid entity ptr assign its number to our entityID.
    if (entity) {
    	entityID = entity->state.number;
    }

    // Return entity ptr.
    return entity;
}

/**
*	@return	The entityID stored in this "bridge" handle.
**/
const uint32_t SVGEntityHandle::ID() { return entityID; }

/**
*	@brief * operator implementations.
**/
SVGBaseEntity *SVGEntityHandle::operator*() { return (SVGBaseEntity*)GetBaseClassentity(Get()); }
const SVGBaseEntity* SVGEntityHandle::operator*() const { return (SVGBaseEntity*)GetBaseClassentity(Get()); }

bool SVGEntityHandle::operator==(const SVGBaseEntity* baseEntity) { 
    if (!baseEntity)
        return false;

    Entity* serverEntity = const_cast<SVGBaseEntity*>(baseEntity)->GetServerEntity();

    if (!serverEntity) {
        return false;
    }

    if (serverEntity->state.number != entityID) {
        return false;
    }

    return true;
}

/**
*	@brief	Assigns the SVGBaseEntity to this handle if it has a valid server entity.
*			If no valid SVGBaseEntity and server entity pointer are passed it unsets
*			this current handle to nullptr and entityID = 0.
**/
SVGBaseEntity* SVGEntityHandle::operator=(SVGBaseEntity* baseEntity) {
	// Ensure SVGBaseEntity pointer is valid.
	if (baseEntity) {
		// Acquire server entity pointer.
		serverEntity = baseEntity->GetServerEntity();

		// In case of a valid server entity pointer, assign entityID to its number.
		if (serverEntity) {
			entityID = serverEntity->state.number;
		}
	} else {
		// No valid SVGBaseEntity pointer, so reset this bridge handle.
		serverEntity = nullptr;
		entityID = 0;
	}

	// Last but not least, return.
	return baseEntity;
}

SVGBaseEntity* SVGEntityHandle::operator->() const { 
	return (SVGBaseEntity*)GetBaseClassentity(Get()); 
}