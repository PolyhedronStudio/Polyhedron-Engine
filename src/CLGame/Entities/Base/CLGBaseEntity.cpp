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
    this->clientEntity = clEntity;
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
*   @brief  Sets the entity classname and generates a hashed string of it.
**/
void CLGBaseEntity::SetClassname(const std::string& classname) {
    // Sets the classname.
    this->classname = classname;

    // Hash it.
    hashedClassname = clgi.Com_HashStringLen(classname.c_str(), classname.size(), 64);
}
/**
*   @return A string containing the entity's classname.
**/
const std::string& CLGBaseEntity::GetClassname() {
    return classname;
}
/**
*   @return An uint32_t containing the hashed classname string.
**/
uint32_t CLGBaseEntity::GetHashedClassname() {
    return hashedClassname;
}


/**
*   @brief  Sets a pointer referring to this class' client entity.
**/
void CLGBaseEntity::SetClientEntity(ClientEntity* clEntity) {
    clientEntity = clEntity;
}
/**
*   @return The pointer referring to this class' client entity.
**/
ClientEntity* CLGBaseEntity::GetClientEntity() {
    return clientEntity;
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