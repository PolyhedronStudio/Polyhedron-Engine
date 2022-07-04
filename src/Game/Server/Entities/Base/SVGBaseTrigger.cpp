/***
*
*	License here.
*
*	@file
*
*	ServerGame BaseTrigger Entity.
* 
***/
#include "../../ServerGameLocals.h"	// SVGame.
#include "../../Effects.h"			// Effects.
#include "../../Entities.h"			// Entities.
#include "../../Utilities.h"		// Util funcs.

// Class Entities.
#include "SVGBaseTrigger.h"

// Included for delayed use.
#include "../Trigger/TriggerDelayedUse.h"

// Constructor/Deconstructor.
SVGBaseTrigger::SVGBaseTrigger(PODEntity *svEntity) : Base(svEntity) {}

// Interface functions. 
//
//

/**
*	
**/
void SVGBaseTrigger::InitBrushTrigger() {
	SetModel(GetModel());
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);
	
	SetServerFlags(EntityServerFlags::NoClient);
}

/**
*	
**/
void SVGBaseTrigger::InitPointTrigger() {
	const vec3_t HULL_MINS = { -16.f, -16.f, -36.f };
	const vec3_t HULL_MAXS = { 16.f,  16.f,  36.f };

	SetSize(HULL_MINS + HULL_MAXS);
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);

	// Ensure we got the proper no client flags.
	SetServerFlags(EntityServerFlags::NoClient);
}

/**
*	
**/
void SVGBaseTrigger::SpawnKey(const std::string& key, const std::string& value) {
	// Wait.
	if (key == "wait") {
		// Parse.
		ParseKeyValue(key, value, waitTime);
	}
	// Parent class spawnkey.
	else {
		Base::SpawnKey(key, value);
	}
}