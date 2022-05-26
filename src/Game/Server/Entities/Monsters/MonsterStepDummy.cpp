/***
*
*	License here.
*
*	@file
*
*	MonsterStepDummy implementation.
*
***/
#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseSkeletalAnimator.h"
#include "../Base/SVGBaseMonster.h"

// World.
#include "../../World/ServerGameWorld.h"

// Misc Server Model Entity.
#include "MonsterStepDummy.h"


//
// Constructor/Deconstructor.
//
MonsterStepDummy::MonsterStepDummy(PODEntity *svEntity) : Base(svEntity) { 
	//const char *mapClass = GetTypeInfo()->mapClass; // typeinfo->classname = C++ classname.
	//uint32_t hashedMapClass = GetTypeInfo()->hashedMapClass; // hashed mapClass.

	//gi.DPrintf("SVG Spawned: svNumber=#%i mapClass=%s hashedMapClass=#%i\n", svEntity->state.number, mapClass, hashedMapClass);
}

MonsterStepDummy::~MonsterStepDummy() { }


//
// Interface functions.
//
//
//===============
// MonsterStepDummy::Precache
//
//===============
//
void MonsterStepDummy::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache test dummy model.
    SVG_PrecacheModel("models/monsters/stepdummy/stepdummy.iqm");
}

//
//===============
// MonsterStepDummy::Spawn
//
//===============
//
void MonsterStepDummy::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set solid.
    SetSolid(Solid::OctagonBox);
	// Set move type.
    SetMoveType(MoveType::Step);
	// Since this is a "monster", after all...
	//SetFlags(EntityFlags::Fly);
    SetServerFlags(EntityServerFlags::Monster);
	// Set clip mask.
    SetClipMask(BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid);

    // Set the barrel model, and model index.
    SetModel("models/monsters/stepdummy/stepdummy.iqm");
	// Set the bounding box.
    SetBoundingBox({ -16, -16, 0 }, { 16, 16, 52 });

	// Entity is alive.
	SetDeadFlag(DeadFlags::Alive);
    // Set entity to allow taking damage.
    SetTakeDamage(TakeDamage::Yes);

    // Set default values in case we have none.
    if (!GetMass()) { SetMass(200); }
    if (!GetHealth()) { SetHealth(200); }
    
	// Setup callbacks.
    SetDieCallback(&MonsterStepDummy::MonsterStepDummyDie);

	// Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MonsterStepDummy::Respawn
//
//===============
//
void MonsterStepDummy::Respawn() { Base::Respawn(); }

//
//===============
// MonsterStepDummy::PostSpawn
//
//===============
//
void MonsterStepDummy::PostSpawn() { 
	Base::PostSpawn(); 
	
	// TEMP CODE: SET THE GOAL AND ENEMY ENTITY.
	ServerGameWorld *gw = GetGameWorld();
	for (auto* geGoalEntity : GetGameWorld()->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES)
		| cef::IsValidPointer
		| cef::HasServerEntity
		| cef::InUse
		| cef::HasKeyValue("goalentity", strGoalEntity)) {
			SetGoalEntity(geGoalEntity);
			SetEnemy(geGoalEntity);

			gi.DPrintf("Set Goal Entity for StepDummy: %s\n", geGoalEntity->GetTargetName().c_str());
	}

    // Setup our MonsterStepDummy callbacks.
    SetThinkCallback(&MonsterStepDummy::MonsterStepDummyStartAnimation);
    SetNextThinkTime(level.time + FRAMETIME);
}
//===============
// MonsterStepDummy::Think
//
//===============
void MonsterStepDummy::Think() { Base::Think(); }


//===============
// MonsterStepDummy::SpawnKey
//
//===============
void MonsterStepDummy::SpawnKey(const std::string& key, const std::string& value) {
	// We're using this for testing atm.
	if ( key == "goalentity" ) {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		strGoalEntity = parsedString;
		gi.DPrintf("MonsterStepDummy(GoalEntity: %s)\n", value.c_str());
	} else {
		Base::SpawnKey(key, value);
	}
}

/////
// Starts the animation.
// 
static uint64_t startz = 0;
void MonsterStepDummy::MonsterStepDummyStartAnimation(void) { 
	// Set the animation.
	EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
	animationState->animationIndex = 1;
	animationState->startFrame = 1;
	animationState->endFrame = 62;
	animationState->frameTime = ANIMATION_FRAMETIME;
	animationState->startTime = startz = level.time.count() + FRAMETIME.count();
	animationState->loopCount = 0;
	animationState->forceLoop = true;

    // First, ensure our origin is +1 off the floor.
    vec3_t newOrigin = GetOrigin() + vec3_t{
        0.f, 0.f, 1.f
    };

    SetOrigin(newOrigin);
    
    // Calculate the end origin to use for tracing.
    vec3_t end = newOrigin + vec3_t{
        0, 0, -2048.f
    };
        
    // Exceute the trace.
    SGTraceResult trace = SG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, BrushContentsMask::MonsterSolid);
    
    // Return in case we hit anything.
	if (trace.fraction == 1 || trace.allSolid) {
		Com_DPrintf("(%s): Bailed out for Floor Fall thing\n", __func__);
        return;
	}
    
	// Set new entity origin.
    SetOrigin(trace.endPosition);
	LinkEntity();

    SetThinkCallback(&MonsterStepDummy::MonsterStepDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);

}

//===============
// MonsterStepDummy::MonsterStepDummyThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
void MonsterStepDummy::MonsterStepDummyThink(void) {
    //
    // Calculate direction.
    //
    if (GetDeadFlag() != DeadFlags::Dead) {
		// Store old origin so we can reset it in case we aren't turned into a wished for yaw angle yet.
		const vec3_t oldOrigin = GetOrigin();
		const vec3_t oldAngles = GetAngles();

		// Yaw Speed.
		SetYawSpeed(20.f);

		// Setup Client as our enemy.
		//GameEntity *geClientEnemy = GetGameWorld()->GetGameEntities()[1]; // Client.
		//SetEnemy(geClientEnemy);
		//SetGoalEntity(geClientEnemy);

		// Our Goal entity is either...:
		// 1: Goal
		// 2: Enemy
		// 3: None.
		GameEntity *geGoal = GetGoalEntity();

		if (!geGoal) {
			geGoal = GetEnemy();

			if (!geGoal) {
				geGoal = GetGameWorld()->GetGameEntities()[1];

				// if !geGoal .. geGoal = ... ?
			}
		}

		//
        // Direction vector between player and other entity.
		//
        vec3_t wishDir = geGoal->GetOrigin() - GetOrigin();
		wishDir.z = 0;

		//
		// Calculate yaw to use based on direction.
		//
	    float yaw = vec3_to_yaw(wishDir);
		// Set ideal Yaw Angle and turn to it.
		SetIdealYawAngle(yaw);
		// Turn to angles.
		float deltaYawAngle = TurnToIdealYawAngle();

		//
		// (Step-)Move into direction.
		//
		if ( StepMove_WalkDirection(yaw, 90.f) ) {
			if ( deltaYawAngle > 45 && deltaYawAngle < 315 ) {
				// Reset to old position, so it can turn some more.
				SetOrigin(oldOrigin);
			}

			if ( !StepMove_CheckBottom() ) {
				StepMove_FixCheckBottom();
				// Reset to old position, so it can turn some more.
				SetOrigin(oldOrigin);
			}
		}

		// Link entity.
		LinkEntity();

		// Touch Triggers.
		SG_TouchTriggers( this );
	}    
	


	CategorizePosition();

    // Check for ground.   
	SG_CheckGround( this );

    // Setup its next think time, for a frame ahead.
    SetThinkCallback(&MonsterStepDummy::MonsterStepDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);
}

//===============
// MonsterStepDummy::MonsterStepDummyDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MonsterStepDummy::MonsterStepDummyDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

    // Set movetype to dead, solid dead.
    SetMoveType(MoveType::TossSlide);
    SetSolid(Solid::Not);
    LinkEntity();
    // Play a nasty gib sound, yughh :)
    SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("misc/udeath.wav"), 1, Attenuation::Normal, 0);

    // Throw some gibs around, true horror oh boy.
    ServerGameWorld* gameWorld = GetGameWorld();
    gameWorld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + FRAMETIME);

    // Set think function.
    SetThinkCallback(&MonsterStepDummy::SVGBaseEntityThinkFree);
}