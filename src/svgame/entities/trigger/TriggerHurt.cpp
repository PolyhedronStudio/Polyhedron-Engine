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
#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"
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
TriggerHurt::TriggerHurt(Entity* svEntity) : SVGBaseTrigger(svEntity) {
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
	lastHurtTime = 0.f;
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
	SVGBaseTrigger::Precache();
}

//
//===============
// TriggerHurt::Spawn
//
//===============
//
void TriggerHurt::Spawn() {
	// Spawn base trigger.
	SVGBaseTrigger::Spawn();

	// Initialize Brush Trigger.
	InitBrushTrigger();

	//self->noiseIndex = gi.SoundIndex("world/electro.wav");
	//self->Touch = hurt_touch;
	SetTouchCallback(&TriggerHurt::TriggerHurtTouch);

	if (!GetDamage()) {
		SetDamage(5);
	}

	// Make it solid trigger, or start off.
	if (GetSpawnFlags() & SPAWNFLAG_START_OFF)
		SetSolid(Solid::Not);
	else
		SetSolid(Solid::Trigger);

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
	SVGBaseTrigger::Respawn();
}

//
//===============
// TriggerHurt::PostSpawn
// 
//===============
//
void TriggerHurt::PostSpawn() {
	SVGBaseTrigger::PostSpawn();
}

//
//===============
// TriggerHurt::Think
//
//===============
//
void TriggerHurt::Think() {
	SVGBaseTrigger::Think();
}

void TriggerHurt::SpawnKey(const std::string& key, const std::string& value) {
	// Parent class spawnkey.
	// We don't want it to reposition this fucker.?
	
	if (key == "origin") {

	} else {
		SVGBaseTrigger::SpawnKey(key, value);
	}
}

//
//===============
// TriggerHurt::TriggerHurtTouch
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void TriggerHurt::TriggerHurtTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
	gi.DPrintf("TriggerHurtTouch!\n");

	if (this == other)
		return;

	if (!other->GetTakeDamage())
		return;

	if (lastHurtTime > level.time)
		return;

	if (GetSpawnFlags() & SPAWNFLAG_SLOW_HURT)
		lastHurtTime = level.time + 1;
	else
		lastHurtTime = level.time + FRAMETIME;

	if (!(GetSpawnFlags()& SPAWNFLAG_SILENT)) {
		if ((level.frameNumber % 10) == 0)
			SVG_Sound(other, CHAN_AUTO, GetNoiseIndex(), 1, ATTN_NORM, 0);
	}

	int32_t damageFlags = 0;
	if (GetSpawnFlags() & SPAWNFLAG_NO_PROTECTION)
		damageFlags = DamageFlags::IgnoreProtection;
	else
		damageFlags = 0;

	SVG_Damage(other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), GetDamage(), damageFlags, MeansOfDeath::TriggerHurt);
}

//
//===============
// TriggerHurt::TriggerHurtUse
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void TriggerHurt::TriggerHurtUse(SVGBaseEntity* other, SVGBaseEntity* activator) {
	gi.DPrintf("TriggerHurtUse!\n");

	if (GetSolid() == Solid::Not)
		SetSolid(Solid::Trigger);
	else
		SetSolid(Solid::Not);
	
	LinkEntity();

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
