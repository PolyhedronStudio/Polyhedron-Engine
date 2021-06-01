/*
// LICENSE HERE.

//
// SVGBaseTrigger.cpp
//
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../utils.h"       // Util funcs.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"

// Constructor/Deconstructor.
SVGBaseTrigger::SVGBaseTrigger(Entity* svEntity) : SVGBaseEntity(svEntity) {
	//
	// All callback functions best be nullptr.
	//
	//thinkFunction = nullptr;


	//
	// Set all entity pointer references to nullptr.
	//
	//activatorEntity = nullptr;
	//enemyEntity = nullptr;
	//groundEntity = nullptr;
	//oldEnemyEntity = nullptr;
	//teamChainEntity = nullptr;
	//teamMasterEntity = nullptr;

	//
	// Default values for members.
	//
	//moveType = MoveType::None;

	//// Velocity.
	//velocity = vec3_zero();
	//angularVelocity = vec3_zero();
	//mass = 0;
	//groundEntityLinkCount = 0;
	//health = 0;
	//maxHealth = 0;
	//deadFlag = DEAD_NO;
}
SVGBaseTrigger::~SVGBaseTrigger() {

}

// Interface functions. 
//
//===============
// SVGBaseTrigger::Precache
//
//===============
//
void SVGBaseTrigger::Precache() {
	gi.DPrintf("SVGBaseEntity::Precache();");
}

//
//===============
// SVGBaseTrigger::Spawn
//
//===============
//
void SVGBaseTrigger::Spawn() {
	SVGBaseEntity::Spawn();
}

//
//===============
// SVGBaseTrigger::Respawn
// 
//===============
//
void SVGBaseTrigger::Respawn() {
	SVGBaseEntity::Respawn();
}

//
//===============
// SVGBaseTrigger::PostSpawn
// 
//===============
//
void SVGBaseTrigger::PostSpawn() {
	SVGBaseEntity::PostSpawn();
}

//
//===============
// SVGBaseTrigger::Think
//
//===============
//
void SVGBaseTrigger::Think() {
	SVGBaseEntity::Think();
}

//
//===============
// SVGBaseTrigger::Use
//
// Execute the 'Use' callback in case we ran into any.
//===============
//
//void SVGBaseTrigger::Use(SVGBaseEntity* other, SVGBaseEntity* activator) {
//	// Safety check.
//	if (useFunction == nullptr)
//		return;
//
//	// Execute 'Die' callback function.
//	(this->*useFunction)(other, activator);
//}
