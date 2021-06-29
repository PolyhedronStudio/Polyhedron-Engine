/*
// LICENSE HERE.

//
// SVGBaseMover.cpp
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
#include "SVGBaseMover.h"

// Constructor/Deconstructor.
SVGBaseMover::SVGBaseMover(Entity* svEntity) : SVGBaseTrigger(svEntity) {
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
SVGBaseMover::~SVGBaseMover() {

}

// Interface functions. 
//
//===============
// SVGBaseMover::Precache
//
//===============
//
void SVGBaseMover::Precache() {
	SVGBaseEntity::Precache();
}

//
//===============
// SVGBaseMover::Spawn
//
//===============
//
void SVGBaseMover::Spawn() {
	SVGBaseTrigger::Spawn();


}

//
//===============
// SVGBaseMover::Respawn
// 
//===============
//
void SVGBaseMover::Respawn() {
	SVGBaseTrigger::Respawn();
}

//
//===============
// SVGBaseMover::PostSpawn
// 
//===============
//
void SVGBaseMover::PostSpawn() {
	SVGBaseTrigger::PostSpawn();
}

//
//===============
// SVGBaseMover::Think
//
//===============
//
void SVGBaseMover::Think() {
	SVGBaseTrigger::Think();
}

//
//===============
// SVGBaseMover::SpawnKey
//
//===============
//
void SVGBaseMover::SpawnKey(const std::string& key, const std::string& value) {
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

//
//===============
// SVGBaseMover::SetMoveDirection
//
//===============
//
void SVGBaseMover::SetMoveDirection(const vec3_t& angles) {
	vec3_t VEC_UP = { 0, -1, 0 };
	vec3_t MOVEDIR_UP = { 0, 0, 1 };
	vec3_t VEC_DOWN = { 0, -2, 0 };
	vec3_t MOVEDIR_DOWN = { 0, 0, -1 };

	//if (VectorCompare(angles, VEC_UP)) {
	//	VectorCopy(MOVEDIR_UP, moveDirection);
	//} else if (VectorCompare(angles, VEC_DOWN)) {
	//	VectorCopy(MOVEDIR_DOWN, moveDirection);
	//} else {
	//	AngleVectors(angles, &moveDirection, NULL, NULL);
	//}

	////VectorClear(angles);
	//SetAngles(vec3_zero());

	if (vec3_equal(angles, VEC_UP)) {
		moveDirection = MOVEDIR_UP;
	} else if (vec3_equal(angles, VEC_DOWN)) {
		moveDirection = MOVEDIR_DOWN;
	} else {
		AngleVectors(angles, &moveDirection, NULL, NULL);
	}

	SetAngles(vec3_zero());
}