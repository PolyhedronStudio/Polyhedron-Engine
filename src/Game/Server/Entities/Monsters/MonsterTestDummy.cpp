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
#include "../../Gamemodes/IGamemode.h"

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
	qhandle_t serverModelHandle = gi.RegisterModel("models/monsters/slidedummy/slidedummy.iqm");
	
	SVG_PrecacheModel("models/weapons/smg45/c_smg45.iqm");

	//
	// TODO:	All skeletal functionality should go into client and server, neatly
	//			put into a "Game API".
	//
	//			Each entity needs to be able to acquire a copy of skeletal model data
	//			for its own self. Making the whole EntitySkeleton redundant to be as
	//			a separate object acting on its own. In short: EntitySkeleton gets replaced
	//			and encapsulated by SkeletalModelData.
	//
	//			The use of caching here might be, example: Let's say we have a certain animation
	//			that we want to have certain bones relocated, we want to cache this for performance
	//			as well as memory reasons. Without having unique SKM data, all entities
	//			would now suffer from those relocated bones, while unwished for.
	//
	skm = gi.GetSkeletalModelDataByHandle(serverModelHandle);


	//
	//	TODO:	Eventually, we need some animation.cfg like file because it's just simpler to work
	//			with. Not to mention it allows for a workflow where Blender Action names do not
	//			matter, but we just use the .cfg file to define these things ourselves.
	//
	if (skm) {
		for (int32_t i = 0; i < skm->actions.size(); i++) {
			skm->actions[i]->rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;//SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;//SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;// | SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;
		}

		//TPose,Idle,IdleAiming,RifleAim,RifleFire,WalkForward,WalkForwardLeft,WalkForwardRight,WalkingToDying,WalkLeft,WalkRight
		const int32_t ZeroAllAxis = ( SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation | SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation | SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation );
		skm->actionMap["TPose"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;// |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;//SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["Idle"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["IdleAiming"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["RifleAim"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["RifleFire"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["Waving"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;
		skm->actionMap["Reloading"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;

		skm->actionMap["RunForward"].rootBoneAxisFlags |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkForward"].rootBoneAxisFlags |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkForwardLeft"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;
		skm->actionMap["WalkForwardRight"].rootBoneAxisFlags = SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;
		skm->actionMap["WalkingToDying"].rootBoneAxisFlags |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkLeft"].rootBoneAxisFlags |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;
		skm->actionMap["WalkRight"].rootBoneAxisFlags |= SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;
	}
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
	SetModelIndex3( SVG_PrecacheModel("models/weapons/smg45/c_smg45.iqm") );
    // Set the bounding box.
    SetBoundingBox( { -16, -16, 0 }, { 16, 16, 88 } );

    // Setup a die callback, this test dummy can die? Yeah bruh, it fo'sho can.
    SetDieCallback( &MonsterTestDummy::MonsterTestDummyDie );

	// Make it so that the player can toggle '+use' this monster.
	SetUseEntityFlags( UseEntityFlags::Toggle );
	SetUseCallback( &MonsterTestDummy::MonsterTestDummyUse );

	// Setup thinking.
    SetThinkCallback( &MonsterTestDummy::MonsterTestDummyThink );
	SetNextThinkTime(level.time + FRAMETIME_S);

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
    SetNextThinkTime(level.time + FRAMETIME_S);
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
	// Get Goal Entity number.
	GameEntity *geGoal = GetGoalEntity();

	const int32_t animationIndex = podEntity->currentState.currentAnimation.animationIndex;

	if (animationIndex == skm->animationMap["Idle"].index) {
		SwitchAnimation("IdleReload");
	} else 	if (animationIndex == skm->animationMap["IdleReload"].index) {
		SwitchAnimation("IdleRifleAim");
	} else 	if (animationIndex == skm->animationMap["IdleRifleAim"].index) {
		SwitchAnimation("IdleRifleFire");
	} else 	if (animationIndex == skm->animationMap["IdleRifleFire"].index) {
		SwitchAnimation("Walk");
	} else 	if (animationIndex == skm->animationMap["Walk"].index) {
		SwitchAnimation("WalkReload");
	} else 	if (animationIndex == skm->animationMap["WalkReload"].index) {
		SwitchAnimation("WalkWaving");
	} else 	if (animationIndex == skm->animationMap["WalkWaving"].index) {
		SwitchAnimation("WalkRifleAim");
	} else 	if (animationIndex == skm->animationMap["WalkRifleAim"].index) {
		SwitchAnimation("WalkRifleFire");
	} else 	if (animationIndex == skm->animationMap["WalkRifleFire"].index) {
		SwitchAnimation("WalkingToDying");
	} else 	if (animationIndex == skm->animationMap["WalkingToDying"].index) {
		SwitchAnimation("RunForward");
	} else { //if (animationIndex == skm->animationMap["WalkForward"].index) {
		SwitchAnimation("Idle");
	}

//	// Get ent numbers for easy comparison.
	const int32_t goalEntityNumber = (geGoal ? geGoal->GetNumber() : -1);
	const int32_t activatorEntityNumber = (activator ? activator->GetNumber() : -1);

	if (activatorEntityNumber != -1 && goalEntityNumber != activatorEntityNumber) {
		// Get current animation, switch to next.
//		const int32_t animationIndex = podEntity->currentState.currentAnimation.animationIndex;
//
//		//// See if the index incremented has valid animation data.
//		const int32_t nextAnimationIndex = (animationIndex + 1 < skm->animations.size() ? animationIndex + 1 : 5);
//
//	if (nextAnimationIndex != )
//		const std::string animName = skm->animations[nextAnimationIndex]->name;
//		SwitchAnimation(animName);
//		
//		gi.DPrintf("UseEntity(#%i, \"monster_testdummy\"): Animation(\"%s\"#%i) set by client dispatched 'Use' callback.\n",
//			GetNumber(),
//			animName.c_str(),
//			nextAnimationIndex);
//
//		// Make activator the goal and enemy.
//		//if (! (nextAnimationIndex == skm->animationMap["WalkForwardRight"].index
//		//	|| nextAnimationIndex == skm->animationMap["WalkForwardLeft"].index 
//		//	|| nextAnimationIndex == skm->animationMap["WalkLeft"].index
//		//	|| nextAnimationIndex == skm->animationMap["WalkRight"].index) ) {
//
			SetGoalEntity(activator);
			//SetEnemy(activator);
////		}
//	//	else {
//		//	SetGoalEntity( nullptr );
//	//		SetEnemy( nullptr );
//		//}
	} else {
		// TEMP CODE: SET THE GOAL AND ENEMY ENTITY.
		ServerGameWorld *gw = GetGameWorld();
		for (auto* geGoal : GetGameWorld()->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES)
			| cef::IsValidPointer
			| cef::HasServerEntity
			| cef::HasKeyValue("targetname", strMonsterGoalTarget)) {
				SetGoalEntity(geGoal);
				//SetEnemy(geGoal);

			//gi.DPrintf("Set Goal Entity for StepDummy: %s\n", geGoal->GetTargetName().c_str());
		}

//		SetGoalEntity( nullptr );
//		SetEnemy( nullptr );
	}
}

/////
// Starts the animation.
// 
void MonsterTestDummy::MonsterTestDummyStartAnimation(void) { 
	
	// If we did not find our goal, switch to idle animation.
	if ( !GetGoalEntity() ) {
		SwitchAnimation( "Idle" );
	// We did find it, so switch to walkforward.
	} else {
		SwitchAnimation( "Walk" );
	}

	//SwitchAnimation("WalkForward");
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME_S);
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
		SetNextThinkTime(level.time + FRAMETIME_S);
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
    SVG_Sound(this, SoundChannel::Body, SVG_PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0);

    // Throw some gibs around, true horror oh boy.
    gameWorld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + FRAMETIME_S);
	SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}