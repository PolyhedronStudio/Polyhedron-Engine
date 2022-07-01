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
	qhandle_t serverModelHandle = gi.PrecacheSkeletalModelData("models/monsters/slidedummy/slidedummy.iqm");
	
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
		for (int32_t i = 0; i < skm->animations.size(); i++) {
			skm->animations[i]->rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;//SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;//SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;// | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		}

		//TPose,Idle,IdleAiming,RifleAim,RifleFire,WalkForward,WalkForwardLeft,WalkForwardRight,WalkingToDying,WalkLeft,WalkRight
		const int32_t ZeroAllAxis = ( SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation | SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation );
		skm->animationMap["TPose"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;// |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;//SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["Idle"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["IdleAiming"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["RifleAim"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["RifleFire"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["Waving"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
		skm->animationMap["Reloading"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;

		skm->animationMap["RunForward"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
		skm->animationMap["WalkForward"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
		skm->animationMap["WalkForwardLeft"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		skm->animationMap["WalkForwardRight"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		skm->animationMap["WalkingToDying"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
		skm->animationMap["WalkLeft"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
		skm->animationMap["WalkRight"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
																											   //skm->animationMap["TPose"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;//SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;
	//skm->animationMap["TPose"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;//SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask;

	//skm->animationMap["PistolIdleTense"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
	//// | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
	//skm->animationMap["WalkLeft"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
	//skm->animationMap["WalkRight"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation;
	//
	//skm->animationMap["PistolWalkBackward"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
	//skm->animationMap["WalkForward"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;

	//skm->animationMap["PistolWhip"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
	//skm->animationMap["Reload"].rootBoneAxisFlags |= SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;

		// Pistol Walk Forward: ZeroY. - We only want movement into "depth", meaning forward/backward, to be accounted
		// for by physics.
		//if ( skm->animationMap.contains( "WalkForward" ) ) {
		//	skm->animationMap["WalkForward"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
		//}

		//// Pistol Walk Backward: ZeroY. - We only want movement into "depth", meaning forward/backward, to be accounted
		//// for by physics.
		//if ( skm->animationMap.contains( "PistolWalkBackward") ) {
		//	skm->animationMap["PistolWalkBackward"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
		//}

		//// Pistol Run Forward: ZeroY. - We only want movement into "depth", meaning forward/backward, to be accounted
		//// for by physics.
		//if ( skm->animationMap.contains( "PistolRunForward" ) ) {
		//	skm->animationMap["PistolRunForward"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
		//}
		//// Pistol Run Backward: ZeroY. - We only want movement into "depth", meaning forward/backward, to be accounted
		//// for by physics.
		//if ( skm->animationMap.contains( "PistolRunBackward" ) ) {
		//	skm->animationMap["PistolRunBackward"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation;
		//}

		//// Pistol Strafe Left: ZeroX. - We only want sideways, meaning left/right, to be accounted for by physics.
		//if ( skm->animationMap.contains( "WalkLeft" ) ) {
		//	skm->animationMap["WalkLeft"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		//}
		//// Pistol Strafe Right: ZeroX. - We only want sideways, meaning left/right, to be accounted for by physics.
		//if ( skm->animationMap.contains( "WalkRight" ) ) {
		//	skm->animationMap["WalkRight"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		//}

		//// We do NOT want idle to move at all.
		//if ( skm->animationMap.contains( "PistolIdleTense" ) ) {
		//	skm->animationMap["PistolIdleTense"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::DefaultTranslationMask | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
		//}
	}
	
	//skm->animationMap["TPose"].rootBoneAxisFlags = SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation | SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;// | SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation;
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
    SetBoundingBox( { -16, -16, 0 }, { 16, 16, 88 } );

    // Setup a die callback, this test dummy can die? Yeah bruh, it fo'sho can.
    SetDieCallback( &MonsterTestDummy::MonsterTestDummyDie );

	// Make it so that the player can toggle '+use' this monster.
	SetUseEntityFlags( UseEntityFlags::Toggle );
	SetUseCallback( &MonsterTestDummy::MonsterTestDummyUse );

	// Setup thinking.
    SetThinkCallback( &MonsterTestDummy::MonsterTestDummyThink );
	SetNextThinkTime(level.time + FRAMETIME);
	//SwitchAnimation("PistolIdleTense");
	//MonsterTestDummyUse(this, this);

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
	// Get Goal Entity number.
	GameEntity *geGoal = GetGoalEntity();

	const int32_t animationIndex = podEntity->currentState.currentAnimation.animationIndex;

	if (animationIndex == skm->animationMap["WalkForward"].index) {
		SwitchAnimation("RunForward");
	} else 	if (animationIndex == skm->animationMap["RunForward"].index) {
		SwitchAnimation("Idle");
	} else 	if (animationIndex == skm->animationMap["Idle"].index) {
		SwitchAnimation("WalkForwardLeft");
	} else 	if (animationIndex == skm->animationMap["WalkForwardLeft"].index) {
		SwitchAnimation("WalkForwardRight");
	} else 	if (animationIndex == skm->animationMap["WalkForwardRight"].index) {
		SwitchAnimation("WalkRight");
	} else 	if (animationIndex == skm->animationMap["WalkRight"].index) {
		SwitchAnimation("WalkLeft");
	} else 	{ //if (animationIndex == skm->animationMap["WalkForward"].index) {
		SwitchAnimation("WalkForward");
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
			SetEnemy(activator);
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
				SetEnemy(geGoal);

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
		SwitchAnimation( "WalkForward" );
	}

	//SwitchAnimation("WalkForward");
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
    SVG_Sound(this, SoundChannel::Body, SVG_PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0);

    // Throw some gibs around, true horror oh boy.
    gameWorld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + FRAMETIME);
	SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}