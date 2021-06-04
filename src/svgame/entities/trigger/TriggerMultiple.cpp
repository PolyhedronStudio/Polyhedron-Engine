/*
// LICENSE HERE.

//
// TriggerMultiple.cpp
//
// This trigger will always fire.  It is activated by the world.
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../utils.h"       // Util funcs.
#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"
#include "TriggerMultiple.h"

//
// Spawn Flags.
// 

// Constructor/Deconstructor.
TriggerMultiple::TriggerMultiple(Entity* svEntity) : SVGBaseTrigger(svEntity) {
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
TriggerMultiple::~TriggerMultiple() {

}

// Interface functions. 
//
//===============
// TriggerMultiple::Precache
//
//===============
//
void TriggerMultiple::Precache() {
	SVGBaseTrigger::Precache();
}

//
//===============
// TriggerMultiple::Spawn
//
//===============
//
void TriggerMultiple::Spawn() {
	// Spawn base trigger.
	SVGBaseTrigger::Spawn();

	// Initialize Brush Trigger.
	InitPointTrigger();


	//self->noiseIndex = gi.SoundIndex("world/electro.wav");
	SetTouchCallback(&TriggerMultiple::TriggerMultipleTouch);

	// In case the entity can be "used", set it to hurt those who use it as well.
	SetUseCallback(&TriggerMultiple::TriggerMultipleUse);

	gi.DPrintf("TriggerHurt::Spawn!\n");
	LinkEntity();
}

//
//===============
// TriggerMultiple::Respawn
// 
//===============
//
void TriggerMultiple::Respawn() {
	SVGBaseTrigger::Respawn();
}

//
//===============
// TriggerMultiple::PostSpawn
// 
//===============
//
void TriggerMultiple::PostSpawn() {
	SVGBaseTrigger::PostSpawn();
}

//
//===============
// TriggerMultiple::Think
//
//===============
//
void TriggerMultiple::Think() {
	SVGBaseTrigger::Think();
}

void TriggerMultiple::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	// We don't want it to reposition this fucker.?
	SVGBaseTrigger::SpawnKey(key, value);
}

//
//===============
// TriggerMultiple::TriggerMultipleTouch
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void TriggerMultiple::TriggerMultipleTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
	if (this == other)
		return;

	gi.DPrintf("TriggerMultipleTouch!\n");
}

//
//===============
// TriggerMultiple::TriggerMultipleUse
//
// 'Use' callback, to trigger it on/off.
//===============
//
void TriggerMultiple::TriggerMultipleUse(SVGBaseEntity* other, SVGBaseEntity* activator) {

}