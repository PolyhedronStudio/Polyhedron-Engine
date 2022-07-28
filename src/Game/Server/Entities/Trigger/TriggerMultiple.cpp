/*
// LICENSE HERE.

//
// TriggerMultiple.cpp
//
// This trigger will always fire.  It is activated by the world.
//
*/
#include "../../ServerGameLocals.h"     // SVGame.
#include "../../Effects.h"     // Effects.
#include "../../Utilities.h"       // Util funcs.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "TriggerMultiple.h"

//
// Spawn Flags.
// 

// Constructor/Deconstructor.
TriggerMultiple::TriggerMultiple(PODEntity *svEntity) : SVGBaseTrigger(svEntity) {
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
		SetNoiseIndexA(SVG_PrecacheSound("misc/secret.wav")); // Similar to gi.PrecacheSound.
	else if (sound == 2)
		SetNoiseIndexA(SVG_PrecacheSound("misc/talk.wav")); // Similar to gi.PrecacheSound.
	else if (sound == 3)
		SetNoiseIndexA(SVG_PrecacheSound("misc/trigger1.wav")); // Similar to gi.PrecacheSound.

	if (GetWaitTime() == Frametime::zero()) {
		SetWaitTime(0.2s);
	}

	//self->noiseIndexA = gi.PrecacheSound("world/electro.wav");
	SetTouchCallback(&TriggerMultiple::TriggerMultipleTouch);

	// In case the entity can be "used", set it to hurt those who use it as well.
	SetUseCallback(&TriggerMultiple::TriggerMultipleUse);

	// Default to 0.
	SetNextThinkTime(GameTime::zero());

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

void TriggerMultiple::Trigger(IServerGameEntity *activator) {
	// We've already been triggered.
	if (GetNextThinkTime() != Frametime::zero()) {
		return;
	}

	// Execute UseTargets.
	SetActivator(activator);
	UseTargets(activator);

	if (GetWaitTime() != Frametime::zero()) {
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
	SetNextThinkTime(GameTime::zero());
}

//
//===============
// TriggerMultiple::TriggerMultipleTouch
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void TriggerMultiple::TriggerMultipleTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
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
void TriggerMultiple::TriggerMultipleUse(IServerGameEntity* other, IServerGameEntity* activator) {
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
void TriggerMultiple::TriggerMultipleEnable(IServerGameEntity* other, IServerGameEntity* activator) {
	// Set the new solid, since it wasn't Solid::Trigger when disabled.
	SetSolid(Solid::Trigger);

	// Set the new use function, to be its default.
	SetUseCallback(&TriggerMultiple::TriggerMultipleUse);

	// Since we changed the solid, relink the entity.
	LinkEntity();
}