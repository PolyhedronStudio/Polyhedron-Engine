/***
*
*	License here.
*
*	@file
*
*	MonsterTestDummy implementation.
*
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "Game/Server/Effects.h"
#include "Game/Server/Utilities.h"

#include "Game/Shared/Physics/Physics.h"
#include "Game/Shared/Physics/RootMotionMove.h"

// Server Game Base Entity.
#include "Game/Server/Entities/Base/SVGBaseEntity.h"
#include "Game/Server/Entities/Base/SVGBaseTrigger.h"
#include "Game/Server/Entities/Base/SVGBaseSkeletalAnimator.h"
#include "Game/Server/Entities/Base/SVGBaseRootMotionMonster.h"

// GameMode.
#include "Game/Server/Gamemodes/IGamemode.h"

// World.
#include "Game/Server/World/ServerGameWorld.h"

// Misc Server Model Entity.
#include "MonsterTestDummy.h"


/**
*	@brief	Constructor
**/
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
*
*   Interface functions.
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void MonsterTestDummy::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache the model for clients. (Gets passed to the config string.)
    modelHandle = SVG_PrecacheModel("models/monsters/testdummy/testdummy.iqm");

	// Precache the model for the server: Required to be able to process animations properly.
	qhandle_t serverModelHandle = gi.RegisterModel("models/monsters/testdummy/testdummy.iqm");
	
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
	model_t *modelt = gi.GetModelByHandle( serverModelHandle );
	gi.ES_CreateFromModel( modelt, &entitySkeleton );

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

/**
*   @brief  Called when it is time to spawn this entity.
**/
void MonsterTestDummy::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the barrel model, and model index.
    SetModel( "models/monsters/testdummy/testdummy.iqm" );
	SetModelIndex3( SVG_PrecacheModel("models/weapons/smg45/c_smg45.iqm") );
    // Set the bounding box.
    SetBoundingBox( { -16, -16, 0 }, { 16, 16, 88 } );

    // Setup a die callback, this test dummy can die? Yeah bruh, it fo'sho can.
    SetDieCallback( &MonsterTestDummy::DieCallback_FallDead );

	// Make it so that the player can toggle '+use' this monster.
	SetUseEntityFlags( UseEntityFlags::Toggle );
	SetUseCallback( &MonsterTestDummy::UseCallback_EngageGoal );

	// Setup thinking.
    SetThinkCallback( &MonsterTestDummy::ThinkCallback_General );
	SetNextThinkTime(level.time + FRAMETIME_S);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

/**
*   @brief  Called when it is time to respawn this entity.
**/
void MonsterTestDummy::Respawn() { Base::Respawn(); }

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
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
    SetThinkCallback(&MonsterTestDummy::Callback_DetermineSpawnAnimation);
    SetNextThinkTime(level.time + FRAMETIME_S);
}

/**
*   @brief  General entity thinking routine.
**/
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
		}
	}

	// Set our Yaw Speed.
	SetYawSpeed(20.f);

	// Set our Move Speed. (It is the actual sum of distance traversed in units.)
	//SetMoveSpeed(64.015f);
}

/**
*   @brief  Act upon the parsed key and value.
**/
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
*
*   TestDummy Temporary WIP Area.
*
**/
/**
*   @brief
**/



/**
*
*   Callback functions.
*
**/  
/**
*	@brief	'Use' callback: Engages the test dummy to follow its 'User'.
**/
void MonsterTestDummy::UseCallback_EngageGoal(IServerGameEntity *other, IServerGameEntity* activator) {
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
void MonsterTestDummy::Callback_DetermineSpawnAnimation(void) { 
	// If we did not find our goal, switch to idle animation.
	if ( !GetGoalEntity() ) {
		PrepareAnimation( "Idle" );
	// We did find it, so switch to walkforward.
	} else {
		PrepareAnimation( "Walk" );
	}

	//SwitchAnimation("WalkForward");
    SetThinkCallback(&MonsterTestDummy::ThinkCallback_General);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME_S);
}

/**
*	@brief	'Think' callback: Check for animation updates, navigate to movegoal and process animations.
**/
void MonsterTestDummy::ThinkCallback_General(void) {

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
		
		// Link us back in.
		LinkEntity();

	}

	// Setup next think callback.
	SetThinkCallback(&MonsterTestDummy::ThinkCallback_General);
	SetNextThinkTime(level.time + FRAMETIME_S);
}

/**
*	@brief	'Die' callback: Switch animation to 'WalkingToDying' and leave the body until damaged enough to gib.
**/
void MonsterTestDummy::DieCallback_FallDead(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
	// Get Gameworld.
	ServerGameWorld* gameWorld = GetGameWorld();

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

	// Switch to animation: 'WalkingToDying' so it looks like he's dropping dead.
	PrepareAnimation( "WalkingToDying", true );
	//SwitchAnimation( "WalkingToDying" );
	//PrepareAnimation( "WalkingToDying" );
	//.Otherwise keep processing the current animation frame for time.
	//ProcessSkeletalAnimationForTime(level.time);

	// Set the dead body to tossslide.
    SetMoveType(MoveType::Toss);

	// Reset health so it can gib out if it dies "again".
	SetMaxHealth(120);
	SetHealth(120);
	SetDeadFlag(DeadFlags::Dead); // 'Fake' being alive, so we can gib out.

    // Change server flags, we're a dead monster now.
    SetServerFlags( GetServerFlags() | EntityServerFlags::DeadMonster );

	// Get old mins maxs to use for x/y coordinates.
	//const vec3_t oldMaxs = GetMaxs();
	//const vec3_t oldMins = GetMins();
    // Set the bounding box.
    SetBoundingBox( { -16, 0, 0 }, { 16, 88, 16 } );

    LinkEntity();

    // Play a nasty gib sound, yughh :)
//    SVG_Sound( this, SoundChannel::Body, SVG_PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0 );
	//// Throw some gibs around, true horror oh boy.
 //   gameWorld->ThrowGib( this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic );

    // Setup the next think and think time.
    SetNextThinkTime(level.time + FRAMETIME_S);
	SetDieCallback( &MonsterTestDummy::Callback_MorphToClientGibs );
}

/**
*	@brief	Prepares the entity for removement after spawning various client gib events.
**/
void MonsterTestDummy::Callback_MorphToClientGibs(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

	// Set dead flag, disable damage taking.
	SetTakeDamage(TakeDamage::No);
	SetDeadFlag(DeadFlags::Dead);

	// Unset movetype and solid.
    SetMoveType(MoveType::None);
    SetSolid(Solid::Not);
	// Relink it in for collision that way.
    LinkEntity();

	// Trigger spawn client side gib effects and play a nasty gib sound.
	SVG_Sound( this, SoundChannel::Body, SVG_PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0);
	ServerGameWorld* gameWorld = GetGameWorld();
	gameWorld->ThrowGib( this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

	// Prepare for removing entity on next game frame.
	SetNextThinkTime(level.time + FRAMETIME_S);
	SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}