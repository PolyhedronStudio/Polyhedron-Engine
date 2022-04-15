/*
// LICENSE HERE.

//
// SVGBaseEntity.cpp
//
//
*/

#include "../../ServerGameLocals.h"		// SVGame.

//
// EntityBridge.
//
#include "../../Effects.h"		// Effects.
#include "../../Entities.h"		// Entities.
#include "../../Utilities.h"		// Util funcs.

// Entities.
#include "SVGBaseTrigger.h"

// Delayed Use Trigger.
#include "../Trigger/TriggerDelayedUse.h"

// World.
#include "../../World/Gameworld.h"



//! Used for returning vectors from a const vec3_t & reference.
vec3_t SVGBaseEntity::ZeroVec3 = vec3_zero();

// Constructor/Deconstructor.
SVGBaseEntity::SVGBaseEntity(Entity* svEntity) : IServerGameEntity() {
	podEntity = svEntity;
}

// Interface functions. 
/**
*	This function is used to load all entity data with.
**/
void SVGBaseEntity::Precache() {
}

/**
*	This function can be overrided, to allow for entity spawning.
*	Setup the basic entity properties here.
**/
void SVGBaseEntity::Spawn() {
}

/**
*	This function can be overrided, to allow for entity respawning.
*	Setup the basic entity properties here.
**/
void SVGBaseEntity::Respawn() {
}

/**
*	This function can be overrided, to allow for entity post spawning.
*	An example of that could be finding targetnames for certain target
*	trigger settings, etc.
**/	
void SVGBaseEntity::PostSpawn() {
}

/**
*	This function can be overrided, to allow for custom entity thinking.
*	By default it only executes the 'Think' callback in case we have any set.
**/
void SVGBaseEntity::Think() {
	// Update current state of server entity's hash classname??
	if (podEntity) {
		podEntity->state.hashedClassname = GetTypeInfo()->hashedMapClass;
	}

	// Safety check.
	if (thinkFunction == nullptr)
		return;

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

/**
*	@brief	This function can be overrided, it should always call upon its Base::SpawnKey function
*			in order to make sure that spawnkeys from the inherited base class get parsed
*			and set accordingly as well.
**/
void SVGBaseEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Deal with classname, set it anyway.
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
		ParseKeyValue(key, value, hackedAngles.y);

		// Set angle.
		SetAngles( hackedAngles );
	}
	// Angles.
	else if (key == "angles") {
		// Parse angles.
		vec3_t parsedAngles = vec3_zero();
		ParseKeyValue(key, value, parsedAngles);

		// Set origin.
		SetAngles(parsedAngles);
	}
	// Damage(dmg)
	else if (key == "dmg") {
		// Parse damage.
		int32_t parsedDamage = 0;
		ParseKeyValue(key, value, parsedDamage);

		// Set Damage.
		SetDamage(parsedDamage);
	}
	// Delay.
	else if (key == "delay") {
		// Parsed float.
		Frametime parsedTime = Frametime::zero();
		ParseKeyValue(key, value, parsedTime);

		// Assign.
		SetDelayTime(parsedTime);
	}
	// KillTarget.
	else if (key == "killtarget") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetKillTarget(parsedString);
	}
	// Mass.
	else if (key == "mass") {
	    // Parsed string.
	    int32_t parsedInteger = 0;
	    ParseKeyValue(key, value, parsedInteger);

	    // Assign.
	    SetMass(parsedInteger);
	} 
	// Message.
	else if (key == "message") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetMessage(parsedString);
	} 
	// Model.
	else if (key == "model") {
		// Parse model.
		std::string parsedModel = "";
		ParseKeyValue(key, value, parsedModel);

		// Set model.
		SetModel(parsedModel);
	}
	// Origin.
	else if (key == "origin") {
		// Parse origin.
		vec3_t parsedOrigin = vec3_zero();
		ParseKeyValue(key, value, parsedOrigin);

		// Set origin.
		SetOrigin(parsedOrigin);
	// Target.
	} else if (key == "target") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetTarget(parsedString);
	// TargetName.
	} else 	if (key == "targetname") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetTargetName(parsedString);
	}
	// Spawnflags.
	else if (key == "spawnflags") {
		// Parse damage.
		int32_t parsedSpawnFlags = 0;
		ParseKeyValue(key, value, parsedSpawnFlags);

		// Set SpawnFlags.
		SetSpawnFlags(parsedSpawnFlags);
	} else {
	    SVG_DPrint("Warning: Entity[#" + std::to_string(GetNumber()) + ":" + GetClassname() + "] has unknown Key/Value['" + key + "','" + value + "']\n");
	}
}

/**
*   @brief  Dispatches 'Use' callback.
*   @param  other:      
*   @param  activator:  
**/
void SVGBaseEntity::DispatchUseCallback(IServerGameEntity* other, IServerGameEntity* activator) {
	// Safety check.
	if (useFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*useFunction)(other, activator);
}

/**
*   @brief  Dispatches 'Die' callback.
*   @param  inflictor:  
*   @param  attacker:   
*   @param  damage:     
*   @param  pointer:    
**/
void SVGBaseEntity::DispatchDieCallback(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
	// Safety check.
	if (dieFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*dieFunction)(inflictor, attacker, damage, point);
}


/**
*   @brief  Dispatches 'Blocked' callback.
*   @param  other:  
**/
void SVGBaseEntity::DispatchBlockedCallback(IServerGameEntity* other) {
	// Safety check.
	if (blockedFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*blockedFunction)(other);
}

/**
*   @brief  Dispatches 'Touch' callback.
*   @param  self:   
*   @param  other:  
*   @param  plane:  
*   @param  surf:   
**/
void SVGBaseEntity::DispatchTouchCallback(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// Safety check.
	if (touchFunction == nullptr)
		return;

	// Execute 'Touch' callback function.
	(this->*touchFunction)(self, other, plane, surf);
}

/**
*   @brief  Dispatches 'TakeDamage' callback.
*   @param  other:
*   @param  kick:
*   @param  damage:
**/
void SVGBaseEntity::DispatchTakeDamageCallback(IServerGameEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
}




/**
*	@brief Calls Use on this entity's targets, as well as killtargets
**/
void SVGBaseEntity::UseTargets( IServerGameEntity* activatorOverride )
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
	if ( GetDelayTime() != Frametime::zero() ) {
		// This is all very lengthy. I'd rather have a static method in TriggerDelayedUse that
		// allocates one such entity and accepts activator, message, target etc. as parameters
		// Something like 'TriggerDelayedUse::Schedule( GetTarget(), GetKillTarget(), activatorOverride, GetMessage(), GetDelayTime() );'
	    SVGBaseTrigger* triggerDelay = GetGameworld()->CreateGameEntity<TriggerDelayedUse>();
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
			SVG_Sound( activatorOverride, SoundChannel::Auto, messageSound, 1, Attenuation::Normal, 0 );
		} else {
			SVG_Sound( activatorOverride, SoundChannel::Auto, SVG_PrecacheSound( "misc/talk1.wav" ), 1, Attenuation::Normal, 0 );
		}
	}

	// Remove all entities that qualify as our killtargets
	if ( !GetKillTarget().empty() ) {
		qboolean foundKillTarget = false;

		for (auto* killtargetEntity : GetGameworld()->GetClassEntityRange(0, MAX_EDICTS)
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
				triggerEntity->DispatchUseCallback(this, activatorOverride);
			}
		}
	}
}

/**
*   @brief  Link entity to world for collision testing using gi.LinkEntity.
**/
void SVGBaseEntity::LinkEntity() {
	gi.LinkEntity(podEntity);
}

/**
*   @brief  Unlink the entity from the world for collision testing.
**/
void SVGBaseEntity::UnlinkEntity() {
	gi.UnlinkEntity(podEntity);
}

/**
*   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
*           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
*           on us during the current server game frame we're processing.
**/
void SVGBaseEntity::Remove()
{
	podEntity->serverFlags |= EntityServerFlags::Remove;
}

/**
*   @brief  Callback method to use for freeing this entity. It calls upon Remove()
**/
void SVGBaseEntity::SVGBaseEntityThinkFree(void) {
	//SVG_FreeEntity(podEntity);
	Remove();
}