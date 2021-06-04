/*
// LICENSE HERE.

//
// SVGBaseTrigger.cpp
//
//
*/
#include "../../g_local.h"		// SVGame.
#include "../../effects.h"		// Effects.
#include "../../entities.h"		// Entities.
#include "../../utils.h"		// Util funcs.

// Class Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"

// Constructor/Deconstructor.
SVGBaseTrigger::SVGBaseTrigger(Entity* svEntity) : SVGBaseEntity(svEntity) {
	//
	// All callback functions best be nullptr.
	//
	//thinkFunction = nullptr;


	//
	// Set all entity pointer references to nullptr.
	//
	activatorEntity = nullptr;
	//activatorEntity = nullptr;
	//enemyEntity = nullptr;
	//groundEntity = nullptr;
	//oldEnemyEntity = nullptr;
	//teamChainEntity = nullptr;
	//teamMasterEntity = nullptr;

	//
	// Default values for members.
	//
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
SVGBaseTrigger::~SVGBaseTrigger() {

}

// Interface functions. 
//
//===============
// SVGBaseTrigger::Precache
//
//===============
//
void SVGBaseTrigger::Precache() {
	SVGBaseEntity::Precache();
}

//
//===============
// SVGBaseTrigger::Spawn
//
//===============
//
void SVGBaseTrigger::Spawn() {
	SVGBaseEntity::Spawn();


}

//
//===============
// SVGBaseTrigger::Respawn
// 
//===============
//
void SVGBaseTrigger::Respawn() {
	SVGBaseEntity::Respawn();
}

//
//===============
// SVGBaseTrigger::PostSpawn
// 
//===============
//
void SVGBaseTrigger::PostSpawn() {
	SVGBaseEntity::PostSpawn();
}

//
//===============
// SVGBaseTrigger::Think
//
//===============
//
void SVGBaseTrigger::Think() {
	SVGBaseEntity::Think();
}

//
//===============
// SVGBaseTrigger::InitBrushTrigger
//
//===============
//
void SVGBaseTrigger::InitBrushTrigger() {
	SetModel(GetModel());
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);
	SetInUse(true);

	//SetServerFlags(EntityServerFlags::NoClient);
}

//
//===============
// SVGBaseTrigger::InitPointTrigger
//
//===============
//
void SVGBaseTrigger::InitPointTrigger() {
	const vec3_t HULL_MINS = { -16.f, -16.f, -36.f };
	const vec3_t HULL_MAXS = { 16.f,  16.f,  36.f };

	SetSize(HULL_MINS + HULL_MAXS);
	SetMoveType(MoveType::None);
	SetSolid(Solid::Trigger);

	// Ensure we got the proper no client flags.
	//SetServerFlags(EntityServerFlags::NoClient);
}

//
//===============
// SVGBaseTrigger::InitPointTrigger
//
//===============
//
void SVGBaseTrigger::SpawnKey(const std::string& key, const std::string& value) {
	if (key == "killtarget") {
		// Parsed string.
		std::string parsedString = "";

		// Parse.
		ParseStringKeyValue(key, value, parsedString);

		// Assign.
		killTargetStr = value;
	} else {
		// Parent class spawnkey.
		SVGBaseEntity::SpawnKey(key, value);
	}
}

//
//===============
// SVGBaseTrigger::UseTargets
//
// The activator is the entity who is initiating the firing. If not set as
// a function argument, it will use whichever is set in the entity itself.
//
// If self.delay is set, a DelayedUse entity will be created that will actually
// do the SUB_UseTargets after that many seconds have passed.
//
// Centerprints any self.message to the activator.
//
// Search for (string)targetName in all entities that (string)target and
// calls their Use function.
//===============
//
void SVGBaseTrigger::UseTargets(SVGBaseEntity* activator) {
	//
	// Check for a delay
	//
    if (GetDelay()) {
		// Create a temporary DelayedTrigger entity, to fire at a latter time.
	//        t = SVG_Spawn();
	//        t->className = "DelayedUse";
	////        t->nextThinkTime = level.time + ent->GetDelay();
	//        //t->Think = Think_Delay;
	////        t->activator = activator;
	//        if (!activator)
	//            gi.DPrintf("Think_Delay with no activator\n");
	//        t->message = ent->GetMessage();
	//        t->target = ent->GetTarget();
	//        t->killTarget = ent->GetKillTarget();
	//        return;
	}
	
	//
	// Print the "message"
	//
	if (GetMessage() && !(activator->GetServerFlags() & EntityServerFlags::Monster)) {
		// Fetch noise index.
		int32_t noiseIndex = GetNoiseIndex();

		// Print the message.
		SVG_CenterPrint(activator, GetMessage());

		// Play specific noise sound, in case one is set. Default talk1.wav otherwise.
		if (noiseIndex) {
			SVG_Sound(activator, CHAN_AUTO, noiseIndex, 1, ATTN_NORM, 0);
		} else {
			SVG_Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// Kill killtargets
	//
	if (GetKillTarget()) {
		SVGBaseEntity* triggerEntity = nullptr;

		while (triggerEntity != SVG_FindEntityByKeyValue("targetname", GetKillTarget(), triggerEntity))
			// It is going to die, free it.
			SVG_FreeClassEntity(triggerEntity->GetServerEntity());

			if (!IsInUse()) {
                gi.DPrintf("entity was removed while using killtargets\n");
                return;
			}
	}
	
	//
	// Fire targets
	//
	if (GetTarget().length()) {
		SVGBaseEntity* triggerEntity = nullptr;

		while (triggerEntity != SVG_FindEntityByKeyValue("targetname", GetTarget(), triggerEntity)) {

		}
	//        t = NULL;
	//        while ((t = SVG_Find(t, FOFS(targetName), ent->GetTarget()))) {
	//            // doors fire area portals in a specific way
	//            if (!Q_stricmp(t->className, "func_areaportal") &&
	//                (!Q_stricmp(ent->GetClassName(), "func_door") || !Q_stricmp(ent->GetClassName(), "func_door_rotating")))
	//                continue;
	//
	//            if (t == ent->GetServerEntity()) {
	//                gi.DPrintf("WARNING: Entity used itself.\n");
	//            } else {
	//                SVGBaseEntity* targetClassEntity = t->classEntity;
	//
	//                // Only continue if there is a classentity.
	//                if (targetClassEntity) {
	//                    targetClassEntity->Use(ent, activator);
	//                }
	//            }
	//            if (!ent->IsInUse()) {
	//                gi.DPrintf("entity was removed while using targets\n");
	//                return;
	//            }
	//	}
	}
}