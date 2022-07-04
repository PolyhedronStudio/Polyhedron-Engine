/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../../ClientGameLocals.h"

// Base Entity.
#include "../Base/CLGBasePacketEntity.h"

// MonsterTestDummy
#include "MonsterTestDummy.h"


//
// Constructor/Deconstructor.
//
MonsterTestDummy::MonsterTestDummy(PODEntity *svEntity) : Base(svEntity) { 

	//gi.DPrintf("SVG Spawned: svNumber=#%i mapClass=%s hashedMapClass=#%i\n", svEntity->state.number, mapClass, hashedMapClass);
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

    // Precache test dummy model.

}

//
//===============
// MonsterTestDummy::Spawn
//
//===============
//
void MonsterTestDummy::Spawn() {

	//clgi.R_RegisterModel("models/monsters/slidedummy/slidedummy.iqm");
	// Always call parent class method.
    Base::Spawn();
		
    // Set solid.
    SetSolid(Solid::OctagonBox);

    //// Set move type.
    SetMoveType(MoveType::RootMotionMove);

    // Since this is a "monster", after all...
    SetServerFlags(EntityServerFlags::Monster);

    // Set clip mask.
    SetClipMask(BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid);

    //// Set the barrel model, and model index.
    SetModel("models/monsters/slidedummy/slidedummy.iqm");

    // Set the bounding box.
    //SetBoundingBox({ -16, -16, -41 }, { 16, 16, 43 });

    // Set default values in case we have none.
    if (!GetMass()) {
	    SetMass(200);
    }
    if (!GetHealth()) {
	    SetHealth(200);
    }
	
	// Set entity to allow taking damage.
    SetTakeDamage(TakeDamage::Yes);

    // Setup our MonsterTestDummy callbacks.
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyStartAnimation);
    //SetDieCallback(&MonsterTestDummy::MonsterTestDummyDie);

    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);

    // Link the entity to world, for collision testing.
    SetInUse(true);
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
 //   EntityState* state = &GetPODEntity()->currentState;

    if (key == "startframe") { 
    } else if (key == "endframe") {
    } else if (key == "framerate") {
    } else if (key == "animindex") {
    } else if (key == "skin") {
    } else if (key == "health") {
    } else {
	    Base::SpawnKey(key, value);
	}

}

/////
// Starts the animation.
// 
void MonsterTestDummy::MonsterTestDummyStartAnimation(void) { 
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
}

//===============
// MonsterTestDummy::MonsterTestDummyThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
void MonsterTestDummy::MonsterTestDummyThink(void) {
	EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
	const int32_t animationFrame = animationState->frame;
	if (animationFrame >= 0 && skm->boundingBoxes.size() > animationFrame) {
			//vec3_t mins = skm->boundingBoxes[animationState->frame].mins;
			//vec3_t maxs = skm->boundingBoxes[animationState->frame].maxs;
			////mins = { mins.z, mins.y, mins.x };
			////maxs = { maxs.z, maxs.y, maxs.x };
			//float depth = fabs(maxs.x) + fabs(mins.x);
			//depth /= 2.f;
			//mins.x = - depth;
			//maxs.x = depth;
			//float width = fabs(maxs.y) + fabs(mins.y);
			//width /= 2.f;
			//mins.y = - width;
			//maxs.y = width;
			//float height = fabs(maxs.z) + fabs(mins.z);
			//height /= 2.f;
			//maxs.z = Minf(0.f, -height);
			////mins.z = Minsf() - height;
			//maxs.z = height;

			//vec3_t oldMins = GetMins();
			//vec3_t oldMaxs = GetMaxs();

			//static GameTime lastTime = GameTime::zero();
			//if (lastTime == GameTime::zero()) {
			//	lastTime = level.time;
			//}
			//mins = vec3_mix(oldMins, mins, ( (float)(( level.time - lastTime ).count()) ) * FRAMETIME.count());
			//maxs = vec3_mix(oldMaxs, maxs, ( (float)(( level.time - lastTime ).count()) ) * FRAMETIME.count());
			//if (lastTime != GameTime::zero()) {
			//	lastTime = level.time;
			//}


			//SetMins(mins);
			//SetMaxs(maxs);
			//LinkEntity();
	}
    //SG_StepMove_CheckGround(this);
	//SG_CheckGround(this);
    // Link entity back in.
    LinkEntity();

    // Setup its next think time, for a frame ahead.
    SetNextThinkTime(level.time + FRAMETIME);
}

//===============
// MonsterTestDummy::MonsterTestDummyDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MonsterTestDummy::MonsterTestDummyDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

    // Set movetype to dead, solid dead.
    SetMoveType(MoveType::TossSlide);
    SetSolid(Solid::Not);
    LinkEntity();
    // Play a nasty gib sound, yughh :)
    //SVG_Sound(this, SoundChannel::Body, gi.PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0);

    // Throw some gibs around, true horror oh boy.
    //ClientGameWorld* gameWorld = GetGameWorld();
    //gameWorld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", 12, damage, GibType::Organic);

    // Setup the next think and think time.
    //SetNextThinkTime(level.time + 1 * FRAMETIME);

    // Set think function.
    //SetThinkCallback(&MonsterTestDummy::CLGBasePacketEntityThinkFree);
}

void MonsterTestDummy::OnDeallocate() {

}