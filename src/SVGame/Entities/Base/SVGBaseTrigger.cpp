/*
// LICENSE HERE.

//
// SVGBaseTrigger.cpp
//
//
*/
#include "../../ServerGameLocal.h"	// SVGame.
#include "../../Effects.h"			// Effects.
#include "../../Entities.h"			// Entities.
#include "../../Utilities.h"		// Util funcs.

// Class Entities.
#include "SVGBaseTrigger.h"

// Included for delayed use.
#include "../Trigger/TriggerDelayedUse.h"

// Constructor/Deconstructor.
SVGBaseTrigger::SVGBaseTrigger(Entity* svEntity) : Base(svEntity) {}

// Interface functions. 
//
//===============
// SVGBaseTrigger::Precache
//
//===============
//
void SVGBaseTrigger::Precache() {
	Base::Precache();
}

//
//===============
// SVGBaseTrigger::Spawn
//
//===============
//
void SVGBaseTrigger::Spawn() {
	Base::Spawn();
}

//
//===============
// SVGBaseTrigger::Respawn
// 
//===============
//
void SVGBaseTrigger::Respawn() {
	Base::Respawn();
}

//
//===============
// SVGBaseTrigger::PostSpawn
// 
//===============
//
void SVGBaseTrigger::PostSpawn() {
	Base::PostSpawn();
}

//
//===============
// SVGBaseTrigger::Think
//
//===============
//
void SVGBaseTrigger::Think() {
	Base::Think();
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
// SVGBaseTrigger::SpawnKey
//
//===============
//
void SVGBaseTrigger::SpawnKey(const std::string& key, const std::string& value) {
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
		Base::SpawnKey(key, value);
	}
}