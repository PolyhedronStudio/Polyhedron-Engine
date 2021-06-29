/*
// LICENSE HERE.

//
// SVGBasePusher.cpp
//
//
*/
#include "../../g_local.h"		// SVGame.
#include "../../effects.h"		// Effects.
#include "../../entities.h"		// Entities.
#include "../../utils.h"		// Util funcs.

// Class Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBasePusher.h"

// Constructor/Deconstructor.
SVGBasePusher::SVGBasePusher(Entity* svEntity) : SVGBaseTrigger(svEntity) {
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
	acceleration = 0;
	deceleration = 0;
	speed = 0;
	startPosition = { 0.f, 0.f, 0.f };
	endPosition = { 0.f, 0.f, 0.f };
}
SVGBasePusher::~SVGBasePusher() {

}

// Interface functions. 
//
//===============
// SVGBasePusher::Precache
//
//===============
//
void SVGBasePusher::Precache() {
	SVGBaseEntity::Precache();
}

//
//===============
// SVGBasePusher::Spawn
//
//===============
//
void SVGBasePusher::Spawn() {
	SVGBaseTrigger::Spawn();


}

//
//===============
// SVGBasePusher::Respawn
// 
//===============
//
void SVGBasePusher::Respawn() {
	SVGBaseTrigger::Respawn();
}

//
//===============
// SVGBasePusher::PostSpawn
// 
//===============
//
void SVGBasePusher::PostSpawn() {
	SVGBaseTrigger::PostSpawn();
}

//
//===============
// SVGBasePusher::Think
//
//===============
//
void SVGBasePusher::Think() {
	SVGBaseTrigger::Think();
}

//
//===============
// SVGBasePusher::SpawnKey
//
//===============
//
void SVGBasePusher::SpawnKey(const std::string& key, const std::string& value) {
	// Wait.
	if (key == "wait") {
		// Parsed float.
		float parsedFloat = 0.f;

		// Parse.
		ParseFloatKeyValue(key, value, parsedFloat);

		// Assign.
		SetWaitTime(parsedFloat);
	}
	// Parent class spawnkey.
	else {
		SVGBaseTrigger::SpawnKey(key, value);
	}
}