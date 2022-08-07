/***
*
*	License here.
*
*	@file
*
*	EntityHandle implementation.
*
***/
//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"

/**
*	@brief	Helper function to acquire the GameEntity pointer from a server entity.
* 
*	@return	A valid pointer if the POD Entity is pointing to one. nullptr otherwise.
**/
#ifdef SHAREDGAME_SERVERGAME
#include "../../Server/World/ServerGameWorld.h"
//#include "../../Server/Entities/IServerGameEntity.h"
static IServerGameEntity* GetGameEntity(PODEntity* podEntity) {
    // Reinterpret cast the gameEntity pointer.
    if (podEntity) {
		SGGameWorld *gameWorld = GetGameWorld();
		return gameWorld->GetGameEntityByIndex(podEntity->currentState.number);
		//if (podEntity->gameEntity != nullptr) {
		//	// Static Cast, since ultimately unless we're a buffoon, this pointer is valid.
		//	return static_cast<IServerGameEntity*>(podEntity->gameEntity);
		//} else {
		//	return nullptr;
		//}
    }

    // Return nullptr.
    return nullptr;
}
#endif
#ifdef SHAREDGAME_CLIENTGAME
#include "../../Client/World/ClientGameWorld.h"
//#include "../../Client/Entities/IClientGameEntity.h"
static IClientGameEntity* GetGameEntity(PODEntity* podEntity) {
    // Reinterpret cast the gameEntity pointer.
    if (podEntity) {
		SGGameWorld *gameWorld = GetGameWorld();
		return gameWorld->GetGameEntityByIndex(podEntity->clientEntityNumber);
		//  if (podEntity->gameEntity != nullptr) {
		//      return static_cast<IClientGameEntity*>(podEntity->gameEntity);
		//  } else {
		//		SGGameWorld *gameWorld = GetGameWorld();
		//		return gameWorld->GetGameEntityByIndex(podEntity->clientEntityNumber);
		//      //return nullptr;
		//  }
    }

    // Return nullptr.
    return nullptr;
}
#endif


/**
*	@brief Simple constructor of an entity handle that will accept a
*	game entity.
**/
SGEntityHandle::SGEntityHandle(ISharedGameEntity* gameEntity) : podEntity(nullptr), entityID(0) {
    // Exploits the assignment operator.
    *this = gameEntity;
}

/**
*	@brief Simple constructor that will accept a reference to another handle entity.
**/
SGEntityHandle::SGEntityHandle(SGEntityHandle& other) : podEntity(other.podEntity), entityID(other.entityID) { }


/**
*	@brief Simple constructor that will accept a const reference to another handle entity.
**/
SGEntityHandle::SGEntityHandle(const SGEntityHandle& other) : podEntity(other.podEntity), entityID(other.entityID) { }

/**
*	@brief Simple constructor that will accept a server entity.
**/
SGEntityHandle::SGEntityHandle(PODEntity* podEntity) { 
    if (podEntity) {
        // Assign POD Entity.
        this->podEntity = podEntity;

        // Assign Entity ID.
#if defined(SHAREDGAME_CLIENTGAME)
        entityID    = podEntity->clientEntityNumber;
#elif defined(SHAREDGAME_SERVERGAME)
        entityID    = podEntity->currentState.number;
#endif
    } else {
        podEntity   = nullptr;
        entityID    = 0;
    }
}

/**
*	@brief Simple constructor that will accept a server entity and an entity number.
**/
SGEntityHandle::SGEntityHandle(PODEntity* _podEntity, const uint32_t number) : podEntity(_podEntity), entityID(number) { }

/**
*	@brief	Acquire a pointer to the server entity.
*
*	@return	If the entityIDs match, a pointer to the server entity. 
*			Nullptr otherwise.
**/
PODEntity* SGEntityHandle::Get() const {
    // Ensure we got a valid server entity pointer.
    if (podEntity) {
#if defined(SHAREDGAME_CLIENTGAME)
        if (podEntity->clientEntityNumber == entityID) {
#elif defined(SHAREDGAME_SERVERGAME)
		if (podEntity->currentState.number == entityID) {
#endif
            return podEntity;
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
PODEntity* SGEntityHandle::Set(PODEntity* entity) {
    // Set new pointer.
    podEntity = entity;

    // In case of a valid entity ptr assign its number to our entityID.
    if (entity) {
#if defined(SHAREDGAME_CLIENTGAME)
        entityID = entity->clientEntityNumber;
#elif defined(SHAREDGAME_SERVERGAME)
    	entityID = entity->currentState.number;
#endif
    }

    // Return entity ptr.
    return entity;
}

/**
*	@return	The entityID stored in this handle.
**/
const uint32_t SGEntityHandle::ID() { return entityID; }
const uint32_t SGEntityHandle::ID() const { return entityID; }

/**
*	@brief * operator implementations.
**/
GameEntity* SGEntityHandle::operator*() { return (GameEntity*)GetGameEntity(Get()); }
const GameEntity* SGEntityHandle::operator*() const { return (GameEntity*)GetGameEntity(Get()); }

/**
*	@brief	Assigns the GameEntity to this handle if it has a valid server entity.
*			If no valid GameEntity and server entity pointer are passed it unsets
*			this current handle to nullptr and entityID = 0.
**/
ISharedGameEntity* SGEntityHandle::operator=(ISharedGameEntity* gameEntity) {
	// Ensure GameEntity pointer is valid.
    if (gameEntity) {
		// Acquire server entity pointer.
		podEntity = gameEntity->GetPODEntity();

		// In case of a valid server entity pointer, assign entityID to its number.
		if (podEntity) {
#if defined(SHAREDGAME_CLIENTGAME)
            entityID = podEntity->clientEntityNumber;
#elif defined(SHAREDGAME_SERVERGAME)
    	    entityID = podEntity->currentState.number;
#endif
		}
	} else {
		// No valid GameEntity pointer, so reset this entity handle.
		podEntity = nullptr;
		entityID = 0;
	}

	// Last but not least, return.
	return gameEntity;
}


/**
*	@brief	Used to access the game entity its methods.
**/
GameEntity* SGEntityHandle::operator->() const { return (GameEntity*)GetGameEntity(Get()); }

/**
*   @brief  Comparison check for whether this handle points to the same POD Entity as 
*           the GameEntity pointer does.
* 
*   @return Returns true if GameEntity* != nullptr, its podEntity pointer 
*           != nullptr, and their entity index number matches.
**/
bool SGEntityHandle::operator == (const ISharedGameEntity* gameEntity) {
    if (!gameEntity) {
	    return false;
    }

    PODEntity* podEntity = const_cast<ISharedGameEntity*>(gameEntity)->GetPODEntity();
	
    if (!podEntity) {
		return false;
    }

#if defined(SHAREDGAME_CLIENTGAME)
    if (podEntity->clientEntityNumber != entityID) {
#elif defined(SHAREDGAME_SERVERGAME)
    if (podEntity->currentState.number != entityID) {
#endif
		return false;
    }

    return true;
}

/**
*   @brief Used to check whether this entity handle is valid valid server entity.
**/
SGEntityHandle::operator bool() {
	// Ensure the POD Entity is a valid pointer, and matches the stored ID(Entity Number).
	bool validPODEntity = (Get() ? true: false);

	// When valid, move on to test for a valid gameEntity pointer.
	if (validPODEntity) {
		// Get the pointer if valid.
		GameEntity *validGameEntity = GetGameEntity(Get());

		// The Game Entity is valid ONLY if its Number matches the stored ID(Entity Number).
		if (validGameEntity && validGameEntity->GetNumber() == ID()) {
			return true;
		}
	}
	
	// Return false, it's not valid.
	return false;
}

/**
*   @brief Used to check whether this entity handle is valid valid server entity.
**/
SGEntityHandle::operator bool() const {
	// Ensure the POD Entity is a valid pointer, and matches the stored ID(Entity Number).
	bool validPODEntity = (Get() ? true: false);

	// When valid, move on to test for a valid gameEntity pointer.
	if (validPODEntity) {
		// Get the pointer if valid.
		GameEntity *validGameEntity = GetGameEntity(Get());

		// The Game Entity is valid ONLY if its Number matches the stored ID(Entity Number).
		if (validGameEntity && validGameEntity->GetNumber() == ID()) {
			return true;
		}
	}
	
	// Return false, it's not valid.
	return false;
}