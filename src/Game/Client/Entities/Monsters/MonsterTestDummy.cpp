/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../../ClientGameLocals.h"

// Base Entity.
#include "../Base/CLGBasePacketEntity.h"

// MonsterTestDummy
#include "MonsterTestDummy.h"


/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
MonsterTestDummy::MonsterTestDummy(PODEntity* clEntity) : Base(clEntity) {

}



/**
*
*   Client Game Entity Interface Functions.
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void MonsterTestDummy::Precache() {
	Base::Precache();
}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void MonsterTestDummy::Spawn() {
	Base::Spawn();

	// Here we should probably animate this sucker eh?
	SetNextThinkTime(level.time + 1s);
	SetThinkCallback(&MonsterTestDummy::FrameThink);
}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void MonsterTestDummy::Respawn() {
	Base::Respawn();
}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void MonsterTestDummy::PostSpawn() {
	Base::PostSpawn();
}

    // TEMP
void MonsterTestDummy::FrameThink() {
	// Acquire POD pointer.
	PODEntity *podEntity = GetPODEntity();

	// Print if we got a valid one.
	if (podEntity) {
	//	Com_DPrint("[clEntity: #%i - svEntity: #%i - Class: %s - Hash: %i] is Thinking\n", clEntity->clientEntityNumber, clEntity->current.number, GetClassname().c_str(), GetHashedClassname());
	}
	
	// Here we should probably animate this sucker eh?
	SetNextThinkTime(level.time + FRAMETIME_S);
	SetThinkCallback(&MonsterTestDummy::FrameThink);
}
/**
*   @brief  General entity thinking routine.
**/
void MonsterTestDummy::Think() {
	// Do Base Thinking.
	Base::Think();
}

/**
*   @brief  Act upon the parsed key and value.
**/
void MonsterTestDummy::SpawnKey(const std::string& key, const std::string& value) {

}



/***
*
*   Client Game Entity Functions.
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