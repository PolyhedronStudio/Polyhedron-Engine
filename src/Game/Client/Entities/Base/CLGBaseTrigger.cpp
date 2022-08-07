/***
*
*	License here.
*
*	@file
*
*	ClientGame BaseTrigger Entity.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"


// Base Client Game Functionality.
#include "Game/Client/Debug.h"
#include "Game/Client/TemporaryEntities.h"

// Export classes.
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/View.h"

// Effects.
#include "Game/Client/Effects/ParticleEffects.h"

// Base Entity.
#include "Game/Client/Entities/Base/CLGBaseTrigger.h"

// Included for delayed use.
//#include "../Trigger/TriggerDelayedUse.h"

// Constructor/Deconstructor.
CLGBaseTrigger::CLGBaseTrigger(PODEntity *clEntity) : Base(clEntity) {}

// Interface functions. 
//
//

/**
*	
**/
void CLGBaseTrigger::InitBrushTrigger() {
	SetModel(GetModel());
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);
	
	SetServerFlags(EntityServerFlags::NoClient);
}

/**
*	
**/
void CLGBaseTrigger::InitPointTrigger() {
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
void CLGBaseTrigger::SpawnKey(const std::string& key, const std::string& value) {
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