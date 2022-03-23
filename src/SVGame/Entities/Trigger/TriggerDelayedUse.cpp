/*
// LICENSE HERE.

//
// TriggerDelayedUse.cpp
//
// This trigger will always fire.  It is activated by the world.
//
*/
#include "../../ServerGameLocals.h"	// SVGame.
#include "../../Effects.h"			// Effects.
#include "../../Entities.h"			// Entities.
#include "../../Utilities.h"		// Util funcs.

// Class entities.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"

// Delayed use.
#include "TriggerDelayedUse.h"

// Gameworld.
#include "../../World/Gameworld.h"
//
// Spawn Flags.
// 

// Constructor/Deconstructor.
TriggerDelayedUse::TriggerDelayedUse(Entity* svEntity) 
	: SVGBaseTrigger(svEntity) {
	//
	// All callback functions best be nullptr.
	//

	//
	// Set all entity pointer references to nullptr.
	//

	//
	// Default values for members.
	//
	//lastHurtTime = 0.f;
}
TriggerDelayedUse::~TriggerDelayedUse() {

}

// Interface functions. 
//
//===============
// TriggerDelayedUse::Precache
//
//===============
//
void TriggerDelayedUse::Precache() {
	Base::Precache();
}

//
//===============
// TriggerDelayedUse::Spawn
//
//===============
//
void TriggerDelayedUse::Spawn() {
	// Spawn base trigger.
	Base::Spawn();

	// Initialize Brush Trigger.
	//InitPointTrigger();
	SetMoveType(MoveType::None);
	SetSolid(Solid::Not);

	// Link entity for collision.
	LinkEntity();
}

//
//===============
// TriggerDelayedUse::Respawn
// 
//===============
//
void TriggerDelayedUse::Respawn() {
	Base::Respawn();
}

//
//===============
// TriggerDelayedUse::PostSpawn
// 
//===============
//
void TriggerDelayedUse::PostSpawn() {
	Base::PostSpawn();
}

//
//===============
// TriggerDelayedUse::Think
//
//===============
//
void TriggerDelayedUse::Think() {
	Base::Think();
}

//
//===============
// TriggerDelayedUse::SpawnKey
//
//===============
//
void TriggerDelayedUse::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	// We don't want it to reposition this fucker.?
	Base::SpawnKey(key, value);
}

//
//===============
// TriggerDelayedUse::TriggerDelayedUseThink
//
// 'Think' callback handling for delayed use triggering.
//===============
//
void TriggerDelayedUse::TriggerDelayedUseThink() {
	// Use the 'set' activator.
	UseTargets(GetActivator());

	// Free this entity.
	GetGameworld()->FreeServerEntity(GetPODEntity());
}
