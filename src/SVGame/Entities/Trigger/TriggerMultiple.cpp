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
	Base::Precache();
}

//
//===============
// TriggerMultiple::Spawn
//
//===============
//
void TriggerMultiple::Spawn() {
	// Spawn base trigger.
	Base::Spawn();

	// Initialize Brush Trigger.
	InitBrushTrigger();

	// Set default sounds.
	int32_t sound = GetSound();
	if (sound == 1)
		SetNoiseIndex(SVG_PrecacheSound("misc/secret.wav")); // Similar to gi.SoundIndex.
	else if (sound == 2)
		SetNoiseIndex(SVG_PrecacheSound("misc/talk.wav")); // Similar to gi.SoundIndex.
	else if (sound == 3)
		SetNoiseIndex(SVG_PrecacheSound("misc/trigger1.wav")); // Similar to gi.SoundIndex.

	if (!GetWaitTime())
		SetWaitTime(0.2f);

	//self->noiseIndex = gi.SoundIndex("world/electro.wav");
	SetTouchCallback(&TriggerMultiple::TriggerMultipleTouch);

	// In case the entity can be "used", set it to hurt those who use it as well.
	SetUseCallback(&TriggerMultiple::TriggerMultipleUse);

	// Default to 0.
	SetNextThinkTime(0);

	LinkEntity();
}

//
//===============
// TriggerMultiple::Respawn
// 
//===============
//
void TriggerMultiple::Respawn() {
	Base::Respawn();
}

//
//===============
// TriggerMultiple::PostSpawn
// 
//===============
//
void TriggerMultiple::PostSpawn() {
	Base::PostSpawn();
}

//
//===============
// TriggerMultiple::Think
//
//===============
//
void TriggerMultiple::Think() {
	Base::Think();
}

//
//===============
// TriggerMultiple::SpawnKey
//
//===============
//
void TriggerMultiple::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	// We don't want it to reposition this fucker.?
	Base::SpawnKey(key, value);
}

void TriggerMultiple::Trigger(SVGBaseEntity *activator) {
	// We've already been triggered.
	if (GetNextThinkTime())
		return;

	// Execute UseTargets.
	SetActivator(activator);
	UseTargets(activator);

	if (GetWaitTime() > 0) {
		// Set our think callback to be "waiting".
		SetThinkCallback(&TriggerMultiple::TriggerMultipleThinkWait);

		// Update the next think callback.
		SetNextThinkTime(level.time + GetWaitTime());
	} else {
		// We can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		SetTouchCallback(nullptr);
		SetNextThinkTime(level.time + FRAMETIME);
		SetThinkCallback(&TriggerMultiple::SVGBaseEntityThinkFree);
	}
}

//
//===============
// TriggerMultiple::TriggerMultipleThinkWait
//
// 'Think' callback, to wait out.
//===============
//
void TriggerMultiple::TriggerMultipleThinkWait() {
	SetNextThinkTime(0.f);
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

	//gi.DPrintf("#1 Touched trigger multiple");
	if (other->GetClient()) {
	    if (GetSpawnFlags() & 2)
	        return;
	} else if (other->GetServerFlags() & EntityServerFlags::Monster) {
	    if (!(GetSpawnFlags() & 1))
	        return;
	} else {
		return;
	}
	//gi.DPrintf("#2 Touched trigger multiple");

	//if (!vec3_equal(self->moveDirection, vec3_zero())) {
	//	vec3_t  forward;

	//	AngleVectors(other->state.angles, &forward, NULL, NULL);
	//	if (DotProduct(forward, self->moveDirection) < 0)
	//		return;
	//}

	//self->activator = other;
	SetActivator(other);
	Trigger(other);
}

//
//===============
// TriggerMultiple::TriggerMultipleUse
//
// 'Use' callback, whenever the trigger is activated.
//===============
//
void TriggerMultiple::TriggerMultipleUse(SVGBaseEntity* other, SVGBaseEntity* activator) {
	// Trigger itself.
	Trigger(activator);
}

//
//===============
// TriggerMultiple::TriggerMultipleEnable
//
// 'Use' callback, whenever the trigger wasn't, but still has to be activated.
//===============
//
void TriggerMultiple::TriggerMultipleEnable(SVGBaseEntity* other, SVGBaseEntity* activator) {
	// Set the new solid, since it wasn't Solid::Trigger when disabled.
	SetSolid(Solid::Trigger);

	// Set the new use function, to be its default.
	SetUseCallback(&TriggerMultiple::TriggerMultipleUse);

	// Since we changed the solid, relink the entity.
	LinkEntity();
}