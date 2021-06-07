/*
// LICENSE HERE.

//
// TriggerDelayedUse.cpp
//
// This trigger will always fire.  It is activated by the world.
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../entities.h"    // Entities.
#include "../../utils.h"       // Util funcs.

// Class entities.
#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"

// Delayed use.
#include "TriggerDelayedUse.h"

//
// Spawn Flags.
// 

// Constructor/Deconstructor.
TriggerDelayedUse::TriggerDelayedUse(Entity* svEntity) : SVGBaseTrigger(svEntity) {
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
	SVGBaseTrigger::Precache();
}

//
//===============
// TriggerDelayedUse::Spawn
//
//===============
//
void TriggerDelayedUse::Spawn() {
	// Spawn base trigger.
	SVGBaseTrigger::Spawn();

	// Initialize Brush Trigger.
	InitPointTrigger();

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
	SVGBaseTrigger::Respawn();
}

//
//===============
// TriggerDelayedUse::PostSpawn
// 
//===============
//
void TriggerDelayedUse::PostSpawn() {
	SVGBaseTrigger::PostSpawn();
}

//
//===============
// TriggerDelayedUse::Think
//
//===============
//
void TriggerDelayedUse::Think() {
	SVGBaseTrigger::Think();
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
	SVGBaseTrigger::SpawnKey(key, value);
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
	SVG_FreeEntity(GetServerEntity());
}
