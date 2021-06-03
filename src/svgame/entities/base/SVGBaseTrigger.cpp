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
	SVGBaseEntity::Precache();
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
// SVGBaseTrigger::InitBrushTrigger
//
//===============
//
void SVGBaseTrigger::InitBrushTrigger() {
	SetModel(GetModel());
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);
	SetInUse(true);

	// Ensure we got the proper no client flags.
	SetServerFlags(EntityServerFlags::NoClient);
}

//
//===============
// SVGBaseTrigger::InitPointTrigger
//
//===============
//
void SVGBaseTrigger::InitPointTrigger() {
	const vec3_t HULL_MINS = { -16.f, -16.f, -36.f };
	const vec3_t HULL_MAXS = { 16.f,  16.f,  36.f };

	SetSize(HULL_MINS + HULL_MAXS);
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);

	// Ensure we got the proper no client flags.
	SetServerFlags(EntityServerFlags::NoClient);
}

//
//===============
// SVGBaseTrigger::InitPointTrigger
//
//===============
//
void SVGBaseTrigger::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	SVGBaseEntity::SpawnKey(key, value);

	if (key == "killtarget") {
		// Parsed string.
		std::string parsedString;

		// Parse.
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		killTargetStr = value;
	}
}
	//case "killtarget":
	//	m_strKillTarget = strValue;
	//	break;
	//case "message":
	//	m_strMessage = strValue;
	//	break;
	//case "master":
	//	m_strMaster = strValue;
	//	break;
	//case "team_no":
	//	m_iTeam = stoi(strValue);
	//	break;
	//case "delay":
	//	m_flDelay = stof(strValue);
	//	break;

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
