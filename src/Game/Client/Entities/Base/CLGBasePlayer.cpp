/***
*
*	License here.
*
*	@file
*
*	Client Base Player Entity.
* 
***/
#include "../../ClientGameLocals.h"

// Base Client Game Functionality.
//#include "../Debug.h"
#include "../../TemporaryEntities.h"

// Export classes.
#include "../../Exports/Entities.h"
#include "../../Exports/View.h"

// Effects.
#include "../../Effects/ParticleEffects.h"

// Base Player.
#include "CLGBasePacketEntity.h"
#include "CLGBasePlayer.h"


/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
CLGBasePlayer::CLGBasePlayer(PODEntity* podEntity) : Base(podEntity) {//}, podEntity(clEntity) {
    
}



/**
*
*
*   Client Game Entity Interface Functions.
*
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void CLGBasePlayer::Precache() {
	Base::Precache();
}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void CLGBasePlayer::Spawn() {
	Base::Spawn();

	// Oh boy...
	SetSolid( Solid::Not );
	SetModelIndex( 0 );
	SetRenderEffects( 0 );
	SetEffects( 0 );
	SetFlags( 0 );
	SetServerFlags( 0 );
	SetInUse( true );

	SetNextThinkTime( level.time + FRAMETIME_S );
	SetThinkCallback( &CLGBasePlayer::CLGBasePacketEntityThinkStandard );
}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void CLGBasePlayer::Respawn() {
	Base::Respawn();
}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void CLGBasePlayer::PostSpawn() {
	Base::PostSpawn();
}

/**
*   @brief  General entity thinking routine.
**/
void CLGBasePlayer::Think() {
	Base::Think();

	Com_DPrint("CLGBasePlayer(#%i): is thinking man!\n", GetNumber());
}

/**
*   @brief  Act upon the parsed key and value.
**/
void CLGBasePlayer::SpawnKey(const std::string& key, const std::string& value) {
 //   // Deal with classname, set it anyway.
	//if ( key == "classname" ) {
	//	SetClassname( value );
	//} else {
	//    Com_DPrint(std::string("Warning: Entity[#" + std::to_string(GetNumber()) + ":" + GetClassname() + "] has unknown Key/Value['" + key + "','" + value + "']\n").c_str());
	//}
	Base::SpawnKey(key, value);
}



/***
*
* 
*   Client Game Entity Functions.
*
* 
***/
/**
*   @brief  Updates the entity with the data of the newly passed EntityState object.
**/
void CLGBasePlayer::UpdateFromState(const EntityState* state) {

}


/***
*
*
*   OnEventCallbacks.
*
*
***/
/**
*   @brief  Gets called right before the moment of deallocation happens.
**/
void CLGBasePlayer::OnDeallocate() {

}

/**
*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
**/
void CLGBasePlayer::PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) {
	Base::PrepareRefreshEntity(refreshEntityID, currentState, previousState, lerpFraction);
}
