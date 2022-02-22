/***
*
*	License here.
*
*	@file
*
*	EntityHandle implementation.
*
***/
#include "../../ServerGameLocal.h"	// SVGame.
#include "../../Entities.h"			// Entities.



/**
*	@brief	Helper function to acquire the SVGBaseEntity pointer from a server entity.
* 
*	@return	A valid pointer if the server entity is pointing to one. nullptr otherwise.
**/
static inline SVGBaseEntity* GetClassEntity(Entity* serverEntity) {
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
*	@brief Simple constructor of an entity handle that will accept a
*	class entity.
**/
SVGEntityHandle::SVGEntityHandle(SVGBaseEntity* classEntity) : serverEntity(nullptr), entityID(0) {
    // Exploits the assignment operator.
    *this = classEntity;
}

/**
*	@brief Simple constructor that will accept a reference to another handle entity.
**/
SVGEntityHandle::SVGEntityHandle(SVGEntityHandle& other) : serverEntity(other.serverEntity), entityID(other.entityID) { }


/**
*	@brief Simple constructor that will accept a const reference to another handle entity.
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
*	@return	The entityID stored in this handle.
**/
const uint32_t SVGEntityHandle::ID() { return entityID; }

/**
*	@brief * operator implementations.
**/
SVGBaseEntity* SVGEntityHandle::operator*() { return (SVGBaseEntity*)GetClassEntity(Get()); }
const SVGBaseEntity* SVGEntityHandle::operator*() const { return (SVGBaseEntity*)GetClassEntity(Get()); }

/**
*	@brief	Assigns the SVGBaseEntity to this handle if it has a valid server entity.
*			If no valid SVGBaseEntity and server entity pointer are passed it unsets
*			this current handle to nullptr and entityID = 0.
**/
SVGBaseEntity* SVGEntityHandle::operator=(SVGBaseEntity* classEntity) {
	// Ensure SVGBaseEntity pointer is valid.
    if (classEntity) {
		// Acquire server entity pointer.
		serverEntity = classEntity->GetServerEntity();

		// In case of a valid server entity pointer, assign entityID to its number.
		if (serverEntity) {
			entityID = serverEntity->state.number;
		}
	} else {
		// No valid SVGBaseEntity pointer, so reset this entity handle.
		serverEntity = nullptr;
		entityID = 0;
	}

	// Last but not least, return.
	return classEntity;
}


/**
*	@brief	Used to access the class entity its methods.
**/
SVGBaseEntity* SVGEntityHandle::operator->() const { return (SVGBaseEntity*)GetClassEntity(Get()); }

/**
*   @brief  Comparison check for whether this handle points to the same entity as 
*           the SVGBaseEntity pointer does.
* 
*   @return Returns true if SVGBaseEntity* != nullptr, its serverEntity pointer 
*           != nullptr, and their entity index number matches.
**/
bool SVGEntityHandle::operator==(const SVGBaseEntity* classEntity) {
    if (!classEntity) {
	    return false;
    }

    Entity* serverEntity = const_cast<SVGBaseEntity*>(classEntity)->GetServerEntity();

    if (!serverEntity) {
		return false;
    }

    if (serverEntity->state.number != entityID) {
		return false;
    }

    return true;
}

/**
*   @brief Used to check whether this entity handle has a valid server entity.
**/
SVGEntityHandle::operator bool() { return (serverEntity != nullptr); }