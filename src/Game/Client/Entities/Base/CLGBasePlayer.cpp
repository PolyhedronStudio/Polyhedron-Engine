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
	
	// When spawned, we aren't on any ground, make sure of that.
    SetGroundEntity(SGEntityHandle());
    // Set up the client entity accordingly.
    SetTakeDamage(TakeDamage::Aim);
    // Fresh movetype and solid.
    SetMoveType(MoveType::PlayerMove);
    SetSolid(Solid::OctagonBox);
    // Mass.
    SetMass(200);
    // Undead itself.
    SetDeadFlag(DeadFlags::Alive);
    // Set air finished time so it can respawn kindly.
    //SetAirFinishedTime(level.time + 12s);
    // Clip mask this client belongs to.
    SetClipMask(BrushContentsMask::PlayerSolid);
    // Fresh default model.
    SetModel("players/male/tris.md2");
	//SetModel( "models/monsters/stepdummy/stepdummy.iqm" );
    /*ent->pain = player_pain;*/
    // Fresh water level and type.
    SetWaterLevel(0);
    SetWaterType(0);
    // Fresh flags.
    SetFlags(GetFlags() & ~EntityFlags::NoKnockBack);
    SetServerFlags(GetServerFlags() & ~EntityServerFlags::DeadMonster);
    // Fresh player move bounding box.
    SetMins(vec3_scale(PM_MINS, PM_SCALE));
    SetMaxs(vec3_scale(PM_MAXS, PM_SCALE));
    // Fresh view height.
    SetViewHeight(22);
    // Zero out velocity in case it had any at all.
    SetVelocity(vec3_zero());

    // Fresh effects.
    Base::SetEffects(0);

    // Reset model indexes.
    SetModelIndex(255); // Use the skin specified by its model.
    SetModelIndex2(255);// Custom gun model.
    SetSkinNumber(GetNumber() - 1);	 // Skin is client number. //    ent->currentState.skinNumber = ent - g_entities - 1; // sknum is player num and weapon number  // weapon number will be added in changeweapon
    
    // Fresh frame for animations.
    SetAnimationFrame(0);
	//SwitchAnimation(2, level.time);

    // Set the die function.
    //SetDieCallback(&CLGBasePlayer::CLGBasePlayerDie);

    // Let it be known this client entity is in use again.
    SetInUse(true);


	SetNextThinkTime( level.time + FRAMETIME_S );
	SetThinkCallback( &CLGBasePlayer::CLGBasePacketEntityThinkStandard );
}

void CLGBasePlayer::CLGBasePlayerDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) {

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
