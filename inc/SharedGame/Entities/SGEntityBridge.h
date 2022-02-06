/***
*
*	License here.
*
*	@file
*
*	EntityBridge implementation.
*
***/
#pragma once

// Pre-define.
class SGBaseEntity;
class SGEntityBridge;


/**
*	Helper function to cast from an entity handle pointer to a SGBaseEntity pointer.
**/
template<typename BaseEntityClass> static inline BaseEntityClass EntityBridgeCast(SGEntityBridge& bridge) { 
	return static_cast<BaseEntityClass>( static_cast<SGBaseEntity*>(bridge) ); 
}

/**
*	@brief	Helper function to acquire the SGBaseEntity pointer from a server entity.
* 
*	@return	A valid pointer if the server entity is pointing to one. nullptr otherwise.
**/
static inline SGBaseEntity* GetBaseClassentity(Entity* serverEntity) {
	// Reinterpret cast the classEntity pointer.
	if (serverEntity) {
		return reinterpret_cast<SGBaseEntity*>(serverEntity->classEntity);
	}

	// Return nullptr.
	return nullptr;
}


/**
*	@brief A bridge between class entity and server entity.
* 
*	@details An Entity "Bridge" is used to link a "Classentity" to a "ServerEntity".
*	It is used as a handle to acquire the class, and server entity pointers with.
*
*	By storing the entity number and a pointer to a classentity it is capable of
*	acting like a safe handle. If the entity number does not match with the one
*	of the currently pointing to classentity, the * operator will kindly return
*	us a nullptr.
* 
*	It can be constructed in the following manners:
*		- By passing a SGBaseEntity pointer.
*		- By passing a const reference to another handle.
*		- By passing a server entity, and an entity number.
**/ 
class SGEntityBridge {
public:
	// Friend itself so it can access private members in its methods.
	friend class SGEntityBridge;

    /**
	*	@brief Simple constructor of an entity bridge that will accept a
	*	class entity.
	**/
    SGEntityBridge(SGBaseEntity* baseEntity) : serverEntity(nullptr), entityID(0) {
		// Exploits the assignment operator.
		*this = baseEntity;
	}

    /**
	*	@brief Simple constructor that will accept an other bridge entity.
	**/
    SGEntityBridge(const SGEntityBridge& other) : serverEntity(other.serverEntity), entityID(other.entityID) { }

	/**
	*	@brief Simple constructor that will accept a server entity and an entity number.
	**/
    SGEntityBridge(Entity* entity, uint32_t number) : serverEntity(entity), entityID(number) {}

	/**
	*	@brief Default destructor
	**/
    ~SGEntityBridge() = default;

public:
    /**
	*	@brief	Acquire a pointer to the server entity.
    *
	*	@return	If the entityIDs match, a pointer to the server entity. 
	*			Nullptr otherwise.
	**/
    inline Entity* Get() const {
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
	inline Entity* Set(Entity* entity) {
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
	inline const uint32_t ID() {
		return entityID;
	}

	/**
	*	@brief * operator implementations.
	**/
	inline operator SGBaseEntity*() { 
		return (SGBaseEntity*)GetBaseClassentity(Get());
	}
	inline operator const SGBaseEntity* const() { 
		return (SGBaseEntity*)GetBaseClassentity(Get()); 
	}

	/**
	*	@brief	Assigns the SGBaseEntity to this handle if it has a valid server entity.
	*			If no valid SGBaseEntity and server entity pointer are passed it unsets
	*			this current handle to nullptr and entityID = 0.
	**/
	inline SGBaseEntity* operator=(SGBaseEntity* baseEntity) {
		// Ensure SGBaseEntity pointer is valid.
		if (baseEntity) {
			// Acquire server entity pointer.
		    serverEntity = baseEntity->GetServerEntity();

			// In case of a valid server entity pointer, assign entityID to its number.
			if (serverEntity) {
				entityID = serverEntity->state.number;
			}
		} else {
			// No valid SGBaseEntity pointer, so reset this bridge handle.
			serverEntity = nullptr;
			entityID = 0;
		}

		// Last but not least, return.
		return baseEntity;
	}

	inline SGBaseEntity* operator->() const { 
		return (SGBaseEntity*)GetBaseClassentity(Get());
	}
    
private:
	// Actual pointer referring to the server entity.
	Entity *serverEntity = nullptr;

	// Stores the server entity's number(ID). Used to determine whether the
	// currently stored classentity pointer is still pointing to a valid entity.
	uint32_t entityID = 0;
};