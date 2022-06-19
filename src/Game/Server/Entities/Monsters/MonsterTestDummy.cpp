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
#include "../Base/SVGBaseRootMotionMonster.h"

// GameMode.
#include "../../GameModes/IGameMode.h"

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
	qhandle_t serverModelHandle = gi.PrecacheServerModel("models/monsters/slidedummy/slidedummy.iqm");
	//skm = SKM_GenerateModelData(gi.GetServerModelByHandle(skeletalModelHandle));
	skm = gi.GetSkeletalModelDataByHandle(serverModelHandle);

	// Zero out Z Axis for Run Stairs Up animation.
	skm->animationMap["run_stairs_up"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;// | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
	skm->animationMap["walk_standard"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
	skm->animationMap["walk_stairs_down"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
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
    SetModel( "models/monsters/slidedummy/slidedummy.iqm" );
	
    // Set the bounding box.
    SetBoundingBox( { -16, -16, 0 }, { 16, 16, 90 } );

    // Setup a die callback, this test dummy can die? Yeah bruh, it fo'sho can.
    SetDieCallback( &MonsterTestDummy::MonsterTestDummyDie );

	// Make it so that the player can toggle '+use' this monster.
	SetUseEntityFlags( UseEntityFlags::Toggle );
	SetUseCallback( &MonsterTestDummy::MonsterTestDummyUse );

	// Setup thinking.
    SetThinkCallback( &MonsterTestDummy::MonsterTestDummyThink );
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
	for (auto* geGoal : GetGameWorld()->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES)
		| cef::IsValidPointer
		| cef::HasServerEntity
		| cef::HasKeyValue("targetname", strMonsterGoalTarget)) {
			SetGoalEntity(geGoal);
			SetEnemy(geGoal);

			gi.DPrintf("Set Goal Entity for StepDummy: %s\n", geGoal->GetTargetName().c_str());
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

	/**
	*	The idea is to write hard think logic here.
	*	Use think callbacks to setup logic changing operations.
	*
	*	This should work considering our system navigates to origins.
	**/
	// First try our Goal Entity.
	GameEntity *geMoveGoal = GetGoalEntity();

	// There's no specific move goal set.
	if (!geMoveGoal) {
		// See if we got enemy.
		geMoveGoal = GetEnemy();

		if (!geMoveGoal) {
			// Y U HEF NO ENEMY?
			
			// TEMPORARY:
			/*SetVelocity(vec3_zero());
			RootMotionMove();
			return;*/
		}
	}

	// Set our Yaw Speed.
	SetYawSpeed(20.f);

	// Set our Move Speed. (It is the actual sum of distance traversed in units.)
	//SetMoveSpeed(64.015f);
}

//===============
// MonsterTestDummy::SpawnKey
//
//===============
void MonsterTestDummy::SpawnKey(const std::string& key, const std::string& value) {

	// We're using this for testing atm.
	if ( key == "monstergoaltarget" ) {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		strMonsterGoalTarget = parsedString;
		gi.DPrintf("MonsterTestDummy(GoalEntity: %s)\n", value.c_str());
	} else {
		Base::SpawnKey(key, value);
	}
}


/**
*	@brief	Toggles whether to follow its activator or stay put.
**/
void MonsterTestDummy::MonsterTestDummyUse(IServerGameEntity *other, IServerGameEntity* activator) {
	//////// Get Goal Entity number.
	//GameEntity *geGoal = GetGoalEntity();
	//const int32_t goalEntityNumber = (geGoal ? geGoal->GetNumber() : -1);

	//// Get Activator Entity number.
	//const int32_t activatorEntityNumber = (activator ? activator->GetNumber() : -1);
	int animationIndexA = skm->animationMap["walk_standard"].index;
	int animationIndexB = skm->animationMap["walk_stairs_down"].index;
	int animationIndexC = skm->animationMap["run_stairs_up"].index;

	//if (activatorEntityNumber != -1 && goalEntityNumber != activatorEntityNumber) {
		// Get current animation, switch to next.
		int32_t animationIndex = podEntity->currentState.currentAnimation.animationIndex;

		const char *animName;
		if (animationIndex == animationIndexA) {
			animName = "walk_stairs_down";
			SwitchAnimation("walk_stairs_down");
			animationToSwitchTo = animationIndexB;
		}
		else if (animationIndex == animationIndexB) {

			//SetOrigin(GetOrigin() + vec3_t{0.f, 0.f, 45.f});
			//SetMins({ -16.f, -16.f, -45.f });
			//SetMaxs({ 16.f, 16.f, 45.f});

			animName = "run_stairs_up";
			SwitchAnimation("run_stairs_up");
			animationToSwitchTo = animationIndexC;
		}
		else if (animationIndex == animationIndexC) {
			//SetOrigin(GetOrigin() + vec3_t{0.f, 0.f, -45.f});
			//SetMins({ -16.f, -16.f, 0.f });
			//SetMaxs({ 16.f, 16.f, 90.f});
			animName = "walk_standard";
			SwitchAnimation("walk_standard");
			animationToSwitchTo = animationIndexA;
		} else {
			animName = "walk_standard";
			animationToSwitchTo = SwitchAnimation("walk_standard");
			//SetMins({ -16.f, -16.f, 0.f });
			//SetMaxs({ 16.f, 16.f, 90.f});
		}

		//// See if the index incremented has valid animation data.
		//const int32_t nextAnimationIndex = (animationIndex + 1 < skm->animations.size() ? animationIndex + 1 : 0);

		//const char *animName;
		//if ( nextAnimationIndex < skm->animations.size() ) {
		//	if ( skm->animations[nextAnimationIndex] ) {
		//		// Get pointer to it.
		//		auto *animationData = skm->animations[nextAnimationIndex];
		//		animName = animationData->name.c_str();
		//		// Switch.
		//		SwitchAnimation(animationData->name);
		//	}
		//}
		//gi.DPrintf("UseEntity(#%i, \"monster_testdummy\"): Animation(\"%s\"#%i) set by client dispatched 'Use' callback.\n",
		//	GetNumber(),
		//	animName,
		//	animationIndex);

		// UseEntity(#%i): 'Use' Dispatched by Client(#%i).\n
	//	gi.DPrintf("UseEntity(#%i, \"monster_testdummy\"): New GoalEntity(#%i) and Animation(\"%s\"#%i) set by client dispatched 'Use' callback.\n",
	//		GetNumber(),
	//		activatorEntityNumber,
	//		animName,
	//		animationIndex);

	//	SetGoalEntity(activator);
	//	SetEnemy(activator);
	//} else {
	//	SetGoalEntity( nullptr );
	//	SetEnemy( nullptr );
	//}
}

/////
// Starts the animation.
// 
void MonsterTestDummy::MonsterTestDummyStartAnimation(void) { 
	//SwitchAnimation("walk_standard");
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
	// Only do logic if alive.
	if (!GetGameMode()->IsDeadEntity(this)) {
		/**
		*	#0: Very Cheap, WIP, Debug, pick the goal entity. lol.
		**/
		GameEntity *geMoveGoal= GetGoalEntity();

		if (!geMoveGoal) {
			geMoveGoal = GetEnemy();
		}

		// Now get the move origin to head into.
		const vec3_t navigationOrigin = ( geMoveGoal ? geMoveGoal->GetOrigin() : vec3_zero() );

		// Navigate to goal.
		NavigateToOrigin( navigationOrigin );

		// Get our current animation state.
		const EntityAnimationState *animationState = GetCurrentAnimationState();

		// If we got a new animation to switch to, ensure we are allowed to switch before doing so.
		if (CanSwitchAnimation(animationState, animationToSwitchTo)) {
			const std::string animName = skm->animations[animationToSwitchTo]->name;
			SwitchAnimation(animName);
		//.Otherwise keep processing the current animation frame for time.
		} else {
			ProcessSkeletalAnimationForTime(level.time);
		}

		// Link us back in.
		LinkEntity();

		// Refresh Monster Animation State.
		//RefreshAnimationState();

		// Setup next think callback.
		SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
		SetNextThinkTime(level.time + FRAMETIME);
	}
}

//===============
// MonsterTestDummy::MonsterTestDummyDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MonsterTestDummy::MonsterTestDummyDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
	// Get Gameworld.
	ServerGameWorld* gameWorld = GetGameWorld();

	// Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

    // Set the dead body to tossslide, no solid collision, and link to world for collision.
    SetMoveType(MoveType::TossSlide);
    SetSolid(Solid::Not);
    LinkEntity();

    // Play a nasty gib sound, yughh :)
    SVG_Sound(this, SoundChannel::Body, gi.PrecacheSound("misc/udeath.wav"), 1, Attenuation::Normal, 0);

    // Throw some gibs around, true horror oh boy.
    gameWorld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + FRAMETIME);
	SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}