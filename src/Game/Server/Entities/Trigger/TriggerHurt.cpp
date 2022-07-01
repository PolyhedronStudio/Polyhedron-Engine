/*
// LICENSE HERE.

//
// SVGBaseTrigger.cpp
//
//
*/
#include "../../ServerGameLocals.h"     // SVGame.
#include "../../Effects.h"     // Effects.
#include "../../Utilities.h"       // Util funcs.

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"

#include "../../GameModes/IGameMode.h"

#include "TriggerHurt.h"

//
// Spawn Flags.
// 
static constexpr int32_t SPAWNFLAG_START_OFF		= 1;
static constexpr int32_t SPAWNFLAG_TOGGLE			= 2;
static constexpr int32_t SPAWNFLAG_SILENT			= 4;
static constexpr int32_t SPAWNFLAG_NO_PROTECTION	= 8;
static constexpr int32_t SPAWNFLAG_SLOW_HURT		= 16;

// Constructor/Deconstructor.
TriggerHurt::TriggerHurt(PODEntity *svEntity) : SVGBaseTrigger(svEntity) {

}
TriggerHurt::~TriggerHurt() {

}

// Interface functions. 
//
//===============
// TriggerHurt::Precache
//
//===============
//
void TriggerHurt::Precache() {
	Base::Precache();
}

//
//===============
// TriggerHurt::Spawn
//
//===============
//
void TriggerHurt::Spawn() {
	// Spawn base trigger.
	Base::Spawn();

	// Initialize Brush Trigger.
	InitBrushTrigger();

	//self->noiseIndexA = gi.PrecacheSound("world/electro.wav");
	//self->Touch = hurt_touch;
	SetTouchCallback(&TriggerHurt::TriggerHurtTouch);

	// Check for default values (from TB, otherwise... set it ourselves to a default.)
	if (!GetDamage()) {
		SetDamage(5);
	}

	// In case it wasn't set, we set it.
	if (GetDelayTime() == Frametime::zero()) {
		SetDelayTime(Frametime(300ms));
	}
	// Make it solid trigger, or start off.
	if (GetSpawnFlags() & SPAWNFLAG_START_OFF)
		SetSolid(Solid::Not);	// Make it solid::not, meaning it can't be triggered.
	else
		SetSolid(Solid::Trigger); // Make it triggerable :)

	// In case the entity can be "used", set it to hurt those who use it as well.
	if (GetSpawnFlags() & SPAWNFLAG_TOGGLE)
		SetUseCallback(&TriggerHurt::TriggerHurtUse);

	gi.DPrintf("TriggerHurt::Spawn!\n");
	LinkEntity();
}

//
//===============
// TriggerHurt::Respawn
// 
//===============
//
void TriggerHurt::Respawn() {
	Base::Respawn();
}

//
//===============
// TriggerHurt::PostSpawn
// 
//===============
//
void TriggerHurt::PostSpawn() {
	Base::PostSpawn();
}

//
//===============
// TriggerHurt::Think
//
//===============
//
void TriggerHurt::Think() {
	Base::Think();
}

void TriggerHurt::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	// We don't want it to reposition this fucker.?
	
	if (key == "origin") {

	} else {
		Base::SpawnKey(key, value);
	}
}

//
//===============
// TriggerHurt::TriggerHurtTouch
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void TriggerHurt::TriggerHurtTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	if (this == other)
		return;

	if (!other->GetTakeDamage())
		return;

	if (lastHurtTime > level.time)
		return;

	// Get Duration in GameTime.
	GameTime gtDuration = duration_cast< GameTime >( GetDelayTime() );
	if (GetSpawnFlags() & SPAWNFLAG_SLOW_HURT) {
		lastHurtTime = level.time + 1s;
	} else {
		// Convert delayTime (FrameTime, good old 3.0 = 3 secs) to GameTime(ms).
		lastHurtTime = level.time + gtDuration;
		//GameTime((int64_t)(1000.0 * GetDelayTime().count()));
	}

	if (!(GetSpawnFlags()& SPAWNFLAG_SILENT)) {
		if ((level.time % gtDuration) == GameTime::zero()) {
			SVG_Sound(other, SoundChannel::Auto, GetNoiseIndexA(), 1, Attenuation::Normal, 0);
		}
	}

	int32_t damageFlags = 0;
	if (GetSpawnFlags() & SPAWNFLAG_NO_PROTECTION)
		damageFlags = DamageFlags::IgnoreProtection;
	else
		damageFlags = 0;

	GetGameMode()->InflictDamage(other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), GetDamage(), damageFlags, MeansOfDeath::TriggerHurt);
}

//
//===============
// TriggerHurt::TriggerHurtUse
//
// 'Use' callback, to trigger it on/off.
//===============
//
void TriggerHurt::TriggerHurtUse(IServerGameEntity* other, IServerGameEntity* activator) {
	gi.DPrintf("TriggerHurtUse!\n");

	// Switch states.
	if (GetSolid() == Solid::Not)
		SetSolid(Solid::Trigger);
	else
		SetSolid(Solid::Not);
	
	// Link entity back in for collision use.
	LinkEntity();

	// Ensure that it can only be used ONCE.
	if (!(GetSpawnFlags() & 2))
		SetTouchCallback(nullptr);
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
// Base::Use
//
// Execute the 'Use' callback in case we ran into any.
//===============
//
//void Base::Use(IServerGameEntity* other, IServerGameEntity* activator) {
//	// Safety check.
//	if (useFunction == nullptr)
//		return;
//
//	// Execute 'Die' callback function.
//	(this->*useFunction)(other, activator);
//}
