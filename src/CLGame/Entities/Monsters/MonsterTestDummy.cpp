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
#include "../Base/CLGBaseEntity.h"

// MonsterTestDummy
#include "MonsterTestDummy.h"


/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
MonsterTestDummy::MonsterTestDummy(ClientEntity* clEntity) : Base(clEntity) {

}



/**
*
*   Client Class Entity Interface Functions.
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void MonsterTestDummy::Precache() {

}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void MonsterTestDummy::Spawn() {

}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void MonsterTestDummy::Respawn() {

}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void MonsterTestDummy::PostSpawn() {

}

/**
*   @brief  General entity thinking routine.
**/
void MonsterTestDummy::Think() {

}

/**
*   @brief  Act upon the parsed key and value.
**/
void MonsterTestDummy::SpawnKey(const std::string& key, const std::string& value) {

}



/***
*
*   Client Class Entity Functions.
*
***/




/***
*
*   OnEventCallbacks.
*
***/
/**
*   @brief  Gets called right before the moment of deallocation happens.
**/
void MonsterTestDummy::OnDeallocate() {

}