/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../../ClientGameLocal.h"
#include "../../Main.h"

// Base Entity.
#include "CLGBaseEntity.h"



/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
CLGBaseEntity::CLGBaseEntity(ClientEntity* clEntity) {
    this->podEntity = clEntity;
}



/**
*
*   Client Class Entity Interface Functions.
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void CLGBaseEntity::Precache() {
}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void CLGBaseEntity::Spawn() {

}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void CLGBaseEntity::Respawn() {

}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void CLGBaseEntity::PostSpawn() {

}

/**
*   @brief  General entity thinking routine.
**/
void CLGBaseEntity::Think() {
	// Safety check.
    if (thinkFunction == nullptr) {
		return;
    }

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

/**
*   @brief  Act upon the parsed key and value.
**/
void CLGBaseEntity::SpawnKey(const std::string& key, const std::string& value) {

}



/***
*
*   Client Class Entity Functions.
*
***/
/**
*   @brief  Updates the entity with the data of the newly passed EntityState object.
**/
void CLGBaseEntity::UpdateFromState(const EntityState& state) {
    previousState = currentState;
    currentState = state;
}

/**
*   @brief  Stub.
**/
const std::string CLGBaseEntity::GetClassname() {
    // Returns this classname, the base entity.
    return GetTypeInfo()->classname;
}

/**
*   @return An uint32_t containing the hashed classname string.
**/
uint32_t CLGBaseEntity::GetHashedClassname() {
    return GetTypeInfo()->hashedMapClass;
}



/***
*
*   OnEventCallbacks.
*
***/
/**
*   @brief  Gets called right before the moment of deallocation happens.
**/
void CLGBaseEntity::OnDeallocate() {

}