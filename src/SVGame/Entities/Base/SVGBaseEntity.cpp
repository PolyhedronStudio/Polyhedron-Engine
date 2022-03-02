/*
// LICENSE HERE.

//
// SVGBaseEntity.cpp
//
//
*/

#include "../../ServerGameLocal.h"		// SVGame.

//
// EntityBridge.
//
#include "../../Effects.h"		// Effects.
#include "../../Entities.h"		// Entities.
#include "../../Utilities.h"		// Util funcs.

// Entities.
#include "SVGBaseTrigger.h"

// Delayed Use Trigger.
#include "../trigger/TriggerDelayedUse.h"

// World.
#include "../../World/Gameworld.h"

// Constructor/Deconstructor.
SVGBaseEntity::SVGBaseEntity(Entity* svEntity) : serverEntity(svEntity) {

}
SVGBaseEntity::~SVGBaseEntity() {

}

// Interface functions. 
//
//===============
// SVGBaseEntity::Precache
//
// This function is used to load all entity data with.
//===============
//
void SVGBaseEntity::Precache() {
}

//
//===============
// SVGBaseEntity::Spawn
//
// This function can be overrided, to allow for entity spawning.
// Setup the basic entity properties here.
//===============
//
void SVGBaseEntity::Spawn() {
}

//
//===============
// SVGBaseEntity::Respawn
//
// This function can be overrided, to allow for entity respawning.
// Setup the basic entity properties here.
//===============
//
void SVGBaseEntity::Respawn() {
}

//
//===============
// SVGBaseEntity::PostSpawn
//
// This function can be overrided, to allow for entity post spawning.
// An example of that could be finding targetnames for certain target
// trigger settings, etc.
//===============
//
void SVGBaseEntity::PostSpawn() {
}

//
//===============
// SVGBaseEntity::Think
//
// This function can be overrided, to allow for custom entity thinking.
// By default it only executes the 'Think' callback in case we have any set.
//===============
//
void SVGBaseEntity::Think() {
	// Safety check.
	if (thinkFunction == nullptr)
		return;

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

//
//===============
// SVGBaseEntity::ParseFloatKeyValue
//
// PROTECTED function to help parsing float key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseFloatKeyValue(const std::string& key, const std::string& value, float &floatNumber) {
	floatNumber = std::stof(value);

	return true;
}

//
//===============
// SVGBaseEntity::ParseIntegerKeyValue
//
// PROTECTED function to help parsing int32_t key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseIntegerKeyValue(const std::string& key, const std::string& value, int32_t &integerNumber) {
	integerNumber = std::stoi(value);

	return true;
}

//
//===============
// SVGBaseEntity::ParseUnsignedIntegerKeyValue
//
// PROTECTED function to help parsing uint32_t key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseUnsignedIntegerKeyValue(const std::string& key, const std::string& value, uint32_t& unsignedIntegerNumber) {
	unsignedIntegerNumber = std::stoul(value);

	return true;
}

//
//===============
// SVGBaseEntity::ParseStringKeyValue
//
// PROTECTED function to help parsing string key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseStringKeyValue(const std::string& key, const std::string& value, std::string& stringValue) {
	stringValue = value;

	return true;
}

//
//===============
// SVGBaseEntity::ParseVector3KeyValue
//
// PROTECTED function to help parsing vector key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseVector3KeyValue(const std::string& key, const std::string &value, vec3_t &vectorValue) {
	// Stores vector fields fetched from string. (Might be corrupted, so we're parsing this nicely.)
	std::vector<std::string> vectorFields;

	// We split it based on the space delimiter. Empties are okay, how can they be empty then? Good question...
	STR_Split(vectorFields, value, " ");

	// Zero out our vector.
	vectorValue = vec3_zero();
	int32_t i = 0;
	for (auto& str : vectorFields) {
		vectorValue[i] = std::stof(str);
		i++;

		if (i > 2)
			break;
	}

	return true;
}

//
//===============
// SVGBaseEntity::SpawnKey
//
// This function can be overrided, to allow for custom entity key:value parsing.
//===============
//
void SVGBaseEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Stop mapversion from causing warnings.
	if ( key == "classname" ) {
		SetClassname( value );
	}
	// Stop mapversion from causing warnings.
	else if (key == "mapversion") {
		
	}
	// Angle.
	else if (key == "angle") {
		// Parse angle.
		vec3_t hackedAngles = vec3_zero();
		ParseFloatKeyValue(key, value, hackedAngles.y);

		// Set angle.
		SetAngles( hackedAngles );
	}
	// Angles.
	else if (key == "angles") {
		// Parse angles.
		vec3_t parsedAngles = vec3_zero();
		ParseVector3KeyValue(key, value, parsedAngles);

		// Set origin.
		SetAngles(parsedAngles);
	}
	// Damage(dmg)
	else if (key == "dmg") {
		// Parse damage.
		int32_t parsedDamage = 0;
		ParseIntegerKeyValue(key, value, parsedDamage);

		// Set Damage.
		SetDamage(parsedDamage);
	}
	// Delay.
	else if (key == "delay") {
		// Parsed float.
		float parsedFloat = 0.f;
		ParseFloatKeyValue(key, value, parsedFloat);

		// Assign.
		SetDelayTime(parsedFloat);
	}
	// KillTarget.
	else if (key == "killtarget") {
		// Parsed string.
		std::string parsedString = "";
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		SetKillTarget(parsedString);
	}
	// Mass.
	else if (key == "mass") {
	    // Parsed string.
	    int32_t parsedInteger = 0;
	    ParseIntegerKeyValue(key, value, parsedInteger);

	    // Assign.
	    SetMass(parsedInteger);
	} 
	// Message.
	else if (key == "message") {
		// Parsed string.
		std::string parsedString = "";
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		SetMessage(parsedString);
	} 
	// Model.
	else if (key == "model") {
		// Parse model.
		std::string parsedModel = "";
		ParseStringKeyValue(key, value, parsedModel);

		// Set model.
		SetModel(parsedModel);
	}
	// Origin.
	else if (key == "origin") {
		// Parse origin.
		vec3_t parsedOrigin = vec3_zero();
		ParseVector3KeyValue(key, value, parsedOrigin);

		// Set origin.
		SetOrigin(parsedOrigin);
	// Target.
	} else if (key == "target") {
		// Parsed string.
		std::string parsedString = "";
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		SetTarget(parsedString);
	// TargetName.
	} else 	if (key == "targetname") {
		// Parsed string.
		std::string parsedString = "";
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		SetTargetName(parsedString);
	}
	// Spawnflags.
	else if (key == "spawnflags") {
		// Parse damage.
		int32_t parsedSpawnFlags = 0;
		ParseIntegerKeyValue(key, value, parsedSpawnFlags);

		// Set SpawnFlags.
		SetSpawnFlags(parsedSpawnFlags);
	} else {
	    SVG_DPrint("Warning: Entity[#" + std::to_string(GetNumber()) + ":" + GetClassname() + "] has unknown Key/Value['" + key + "','" + value + "']\n");
	}
}

//
//===============
// SVGBaseEntity::Use
//
// Execute the 'Use' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Use(SVGBaseEntity* other, SVGBaseEntity* activator) {
	// Safety check.
	if (useFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*useFunction)(other, activator);
}

//
//===============
// SVGBaseEntity::Blocked
//
// Execute the 'Blocked' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Blocked(SVGBaseEntity* other) {
	// Safety check.
	if (blockedFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*blockedFunction)(other);
}

//
//===============
// SVGBaseEntity::TakeDamage
//
// Execute the 'TakeDamage' callback in case we ran into any.
//===============
//
void SVGBaseEntity::TakeDamage(SVGBaseEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
}

//
//===============
// SVGBaseEntity::Die
//
// Execute the 'Die' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
	// Safety check.
	if (dieFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*dieFunction)(inflictor, attacker, damage, point);
}

//
//===============
// SVGBaseEntity::Touch
//
// Execute the 'Touch' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
	// Safety check.
	if (touchFunction == nullptr)
		return;

	// Execute 'Touch' callback function.
	(this->*touchFunction)(self, other, plane, surf);
}

//===============
// SVGBaseEntity::UseTargets
// 
// Calls Use on this entity's targets, as well as killtargets
//===============
void SVGBaseEntity::UseTargets( SVGBaseEntity* activatorOverride )
{
	// If activatorOverride is null, use our default activator.
	if (activatorOverride == nullptr) {
		activatorOverride = activatorEntityPtr;
	}

	// If we have no activator at all, then it is this entity itself doing it.
	if (GetActivator() == nullptr) {
		activatorOverride = this;
	}

	// Create a temporary DelayedUse entity in case this entity has a trigger delay
	if ( GetDelayTime() ) {
		// This is all very lengthy. I'd rather have a static method in TriggerDelayedUse that
		// allocates one such entity and accepts activator, message, target etc. as parameters
		// Something like 'TriggerDelayedUse::Schedule( GetTarget(), GetKillTarget(), activatorOverride, GetMessage(), GetDelayTime() );'
	    SVGBaseTrigger* triggerDelay = GetGameworld()->CreateClassEntity<TriggerDelayedUse>();
		triggerDelay->SetActivator( activatorOverride );
		triggerDelay->SetMessage( GetMessage() );
		triggerDelay->SetTarget( GetTarget() );
		triggerDelay->SetKillTarget( GetKillTarget() );
		triggerDelay->SetNextThinkTime( level.time + GetDelayTime() );
		triggerDelay->SetThinkCallback( &TriggerDelayedUse::TriggerDelayedUseThink );
		// No need to continue. The rest happens by delay.
		return;
	}

	// Print the "message"
	if ( !GetMessage().empty() && !(activatorOverride->GetServerFlags() & EntityServerFlags::Monster) ) {
		// Get the message sound
		const int32_t messageSound = GetNoiseIndexA();
		
		// Print the message.
		SVG_CenterPrint(activatorOverride, GetMessage());

		// Play the message sound
		if ( messageSound ) {
			SVG_Sound( activatorOverride, CHAN_AUTO, messageSound, 1, ATTN_NORM, 0 );
		} else {
			SVG_Sound( activatorOverride, CHAN_AUTO, SVG_PrecacheSound( "misc/talk1.wav" ), 1, ATTN_NORM, 0 );
		}
	}

	// Remove all entities that qualify as our killtargets
	if ( !GetKillTarget().empty() ) {
		qboolean foundKillTarget = false;

		for (auto* killtargetEntity : GetGameworld()->GetClassEntityRange<0, MAX_EDICTS>()
			| cef::IsValidPointer
			| cef::HasServerEntity
			| cef::InUse
			| cef::HasKeyValue("targetname", GetKillTarget())) {

			// We found a killtarget entity.
			foundKillTarget = true;

			// Remove our killtarget entity.
			killtargetEntity->Remove();
		}

		// Inform that we haven't found the killtarget entity.
		if (!foundKillTarget) {
			gi.DPrintf("Warning: killtarget entity '%s' couldn't be found.\n", GetKillTarget().c_str());
		}
	}

	// Actually fire the targets
	if ( !GetTarget().empty() ) {
		qboolean foundTarget = false;
	    for (auto* triggerEntity : GetGameworld()->GetClassEntityRange<0, MAX_EDICTS>()
			| cef::IsValidPointer
			| cef::HasServerEntity
			| cef::InUse
			| cef::HasKeyValue("targetname", GetTarget())) {

			// Make sure it is in use, if not, debug.
			if (!triggerEntity->IsInUse()) {
				gi.DPrintf("Warning: Target entity{#(%i):%s} is not in use.\n", GetState().number, GetTarget());
				continue;
			}

			// Doors fire area portals in a special way. So we skip those.
			if (triggerEntity->GetClassname() == "func_areaportal"
				&& (GetClassname() == "func_door" || GetClassname() == "func_door_rotating")) {
				continue;
			}

			// Do NOT ALLOW an entity to USE ITSELF. :)
			if (triggerEntity == this) {
				gi.DPrintf("Warning: Target entity{#(%i):%s} can't trigger itself.\n", GetState().number, GetTarget().c_str());
			} else {
				triggerEntity->Use(this, activatorOverride);
			}
		}
	}
}

//
//===============
// SVGBaseEntity::LinkEntity
//
// Link entity to world for collision testing using gi.LinkEntity.
//===============
//
void SVGBaseEntity::LinkEntity() {
	gi.LinkEntity(serverEntity);
}

//===============
// SVGBaseEntity::UnlinkEntity
//
// 
//===============
void SVGBaseEntity::UnlinkEntity() {
	gi.UnlinkEntity(serverEntity);
}

//===============
// SVGBaseEntity::Remove
//===============
void SVGBaseEntity::Remove()
{
	serverEntity->serverFlags |= EntityServerFlags::Remove;
}

//
//
//
void SVGBaseEntity::SVGBaseEntityThinkFree(void) {
	//SVG_FreeEntity(serverEntity);
	Remove();
}