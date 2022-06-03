/***
*
*	License here.
*
*	@file
*
*	MonsterTestDummy implementation.
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
#include "../Base/SVGBaseSlideMonster.h"

// World.
#include "../../World/ServerGameWorld.h"

// Misc Server Model Entity.
#include "MonsterTestDummy.h"


//
// Constructor/Deconstructor.
//
MonsterTestDummy::MonsterTestDummy(PODEntity *svEntity) : Base(svEntity) { 
	//const char *mapClass = GetTypeInfo()->mapClass; // typeinfo->classname = C++ classname.
	//uint32_t hashedMapClass = GetTypeInfo()->hashedMapClass; // hashed mapClass.

	//gi.DPrintf("SVG Spawned: svNumber=#%i mapClass=%s hashedMapClass=#%i\n", svEntity->state.number, mapClass, hashedMapClass);
}

MonsterTestDummy::~MonsterTestDummy() { }

/**
*
*   TestDummy Temporary WIP Area.
*
**/
/**
*	@brief	Switches the animation by blending from the current animation into the next.
*	@return	
**/
bool SwitchAnimation(const std::string& name) {
	return true;
}


//
// Interface functions.
//
//
//===============
// MonsterTestDummy::Precache
//
//===============
//
void MonsterTestDummy::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache the model for clients. (Gets passed to the config string.)
    modelHandle = SVG_PrecacheModel("models/monsters/slidedummy/slidedummy.iqm");

	// Precache the model for the server: Required to be able to process animations properly.
	qhandle_t serverModelHandle = gi.PrecacheSkeletalModelData("models/monsters/slidedummy/slidedummy.iqm");

	// Get the model data pointer and generate game friendly model data using it.
	model_t *model = gi.GetModelByHandle(serverModelHandle);
	skm = SG_SKM_GenerateModelData(model);
}

//
//===============
// MonsterTestDummy::Spawn
//
//===============
//
void MonsterTestDummy::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the barrel model, and model index.
    SetModel("models/monsters/slidedummy/slidedummy.iqm");

    // Set the bounding box.
    //SetBoundingBox({ -16, -16, -41 }, { 16, 16, 43 });

    // Setup our MonsterTestDummy callbacks.
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyStartAnimation);
    SetDieCallback(&MonsterTestDummy::MonsterTestDummyDie);

    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MonsterTestDummy::Respawn
//
//===============
//
void MonsterTestDummy::Respawn() { Base::Respawn(); }

//
//===============
// MonsterTestDummy::PostSpawn
//
//===============
//
void MonsterTestDummy::PostSpawn() {
    // Always call parent class method.
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
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyStartAnimation);
    SetNextThinkTime(level.time + FRAMETIME);
}

//===============
// MonsterTestDummy::Think
//
//===============
void MonsterTestDummy::Think() {
    // Always call parent class method.
    Base::Think();
}

//===============
// MonsterTestDummy::SpawnKey
//
//===============
void MonsterTestDummy::SpawnKey(const std::string& key, const std::string& value) {

	// We're using this for testing atm.
	if ( key == "goalentity" ) {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		strGoalEntity = parsedString;
		gi.DPrintf("MonsterTestDummy(GoalEntity: %s)\n", value.c_str());
	} else {
		Base::SpawnKey(key, value);
	}
}

/////
// Starts the animation.
// 
static uint64_t startz = 0;
void MonsterTestDummy::MonsterTestDummyStartAnimation(void) { 
	// Set the animation.
	EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
	animationState->animationIndex = 1;
	animationState->startFrame = 0;
	animationState->endFrame = 71;
	animationState->frameTime = ANIMATION_FRAMETIME;
	animationState->startTime = startz = level.time.count() + FRAMETIME.count();
	animationState->loopCount = 0;
	animationState->forceLoop = true;

    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);
}

//===============
// MonsterTestDummy::MonsterTestDummyThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
void MonsterTestDummy::MonsterTestDummyThink(void) {

    // Advance the dummy animation for a frame.
    // Set here how fast you want the tick rate to be.
    // Set here how fast you want the tick rate to be.
    static constexpr uint32_t ANIM_HZ = 30.0;

    // Calclate all related values we need to make it work smoothly even if we have
    // a nice 250fps, the game must run at 50fps.
    //static constexpr uint32_t ANIM_FRAMERATE = ANIM_HZ;
    //static constexpr double   ANIM_FRAMETIME = 1000.0 / ANIM_FRAMERATE;
    //static constexpr double   ANIM_1_FRAMETIME = 1.0 / ANIM_FRAMETIME;
    //static constexpr double   ANIM_FRAMETIME_1000 = ANIM_FRAMETIME / 1000.0;
    //float nextFrame = GetAnimationFrame();
    //nextFrame += (32.f * ANIM_1_FRAMETIME);
    //if (nextFrame > 33) {
	   // nextFrame = 2;
    //}
    //SetAnimationFrame(nextFrame);

    //
    // Move if alive.
    //
    if (GetHealth() > 0) {


		// Set the animation.
		EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
		
		//// Get animation data.
		//const int32_t animationFrame = animationState->frame;
		//if (animationFrame >= 0 && skm.boundingBoxes.size() > animationFrame) {
		//	vec3_t mins = skm.boundingBoxes[animationState->frame].mins;
		//	vec3_t maxs = skm.boundingBoxes[animationState->frame].maxs;
		//	//mins = { mins.z, mins.y, mins.x };
		//	//maxs = { maxs.z, maxs.y, maxs.x };
		//	float depth = fabs(maxs.x) + fabs(mins.x);
		//	depth /= 2.f;
		//	mins.x = - depth;
		//	maxs.x = depth;
		//	float width = fabs(maxs.y) + fabs(mins.y);
		//	width /= 2.f;
		//	mins.y = - width;
		//	maxs.y = width;

		//	vec3_t oldMins = GetMins();
		//	vec3_t oldMaxs = GetMaxs();

		//	static GameTime lastTime = GameTime::zero();
		//	if (lastTime == GameTime::zero()) {
		//		lastTime = level.time;
		//	}
		//	mins = vec3_mix(oldMins, mins, ( (float)(( level.time - lastTime ).count()) ) * FRAMETIME.count());
		//	maxs = vec3_mix(oldMaxs, maxs, ( (float)(( level.time - lastTime ).count()) ) * FRAMETIME.count());
		//	if (lastTime != GameTime::zero()) {
		//		lastTime = level.time;
		//	}
		//	gi.DPrintf("%f %f %f, %f %f %f\n",
		//		mins.x,
		//		mins.y,
		//		mins.z,
		//		maxs.x,
		//		maxs.y,
		//		maxs.z);

		//	SetMins(mins);
		//	SetMaxs(maxs);
		//	LinkEntity();
		//}

		// Navigate to goal.
		Move_NavigateToTarget( );
	}

    // Check for ground.
    //SVG_StepMove_CheckGround(this);
//	SG_CheckGround(this);
    // Link entity back in.
    LinkEntity();

	// Setup next think callback.
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);
}

//===============
// MonsterTestDummy::MonsterTestDummyDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MonsterTestDummy::MonsterTestDummyDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
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
    SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}