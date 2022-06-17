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
    SetBoundingBox( { -16, -16, -49 }, { 16, 16, 41 } );

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
		| cef::HasKeyValue("targetname", strGoalEntity)) {
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
	SetMoveSpeed(64.015f);
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


/**
*	@brief	Toggles whether to follow its activator or stay put.
**/
void MonsterTestDummy::MonsterTestDummyUse(IServerGameEntity *other, IServerGameEntity* activator) {
	////// Get Goal Entity number.
	GameEntity *geGoal = GetGoalEntity();
	const int32_t goalEntityNumber = (geGoal ? geGoal->GetNumber() : -1);

	// Get Activator Entity number.
	const int32_t activatorEntityNumber = (activator ? activator->GetNumber() : -1);

	if (activatorEntityNumber != -1 && goalEntityNumber != activatorEntityNumber) {
		// UseEntity(#%i): 'Use' Dispatched by Client(#%i).\n
		gi.DPrintf("UseEntity(#%i, \"monster_testdummy\"): New GoalEntity(#%i) set by client dispatched 'Use' callback.\n",
			GetNumber(),
			activatorEntityNumber);

		SetGoalEntity(activator);
		SetEnemy(activator);
	} else {
		SetGoalEntity( nullptr );
		SetEnemy( nullptr );
	}
}

/////
// Starts the animation.
// 
void MonsterTestDummy::MonsterTestDummyStartAnimation(void) { 
	SwitchAnimation("walk_standard");

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

		// Refresh Monster Animation State.
		RefreshAnimationState();

		// Link entity back in.
		LinkEntity();

		// Setup next think callback.
		SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
		// Setup the next think time.
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