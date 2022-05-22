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

    // Set default values in case we have none.
    if (!GetMass()) {
	    SetMass(200);
    }
    if (!GetHealth()) {
	    SetHealth(200);
    }
    
    // Set entity to allow taking damage.
    SetTakeDamage(TakeDamage::Yes);

    // Setup our MonsterStepDummy callbacks.
    SetThinkCallback(&MonsterStepDummy::MonsterStepDummyStartAnimation);
    SetDieCallback(&MonsterStepDummy::MonsterStepDummyDie);

    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);

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
void MonsterStepDummy::PostSpawn() { Base::PostSpawn(); }
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
	Base::SpawnKey(key, value);
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
    // First, ensure our origin is +1 off the floor.
    //vec3_t newOrigin = GetOrigin() + vec3_t{
    //    0.f, 0.f, 1.f
    //};

    //SetOrigin(newOrigin);
    
    //// Calculate the end origin to use for tracing.
    //vec3_t end = newOrigin + vec3_t{
    //    0, 0, -256.f
    //};
    //
    //
    //// Exceute the trace.
    //SVGTraceResult trace = SVG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, BrushContentsMask::MonsterSolid);
    //
    //// Return in case we hit anything.
    //if (trace.fraction == 1 || trace.allSolid)
    //    return;
    //
    //// Set new entity origin.
	

	//gi.DPrintf("YawSpeed: %f\n", GetYawSpeed());

    //
    // Calculate direction.
    //
    if (GetHealth() > 0) {
		// Yaw Speed.
		SetYawSpeed(10.f / BASE_FRAMEDIVIDER);

		// Setup Client as our enemy.
		GameEntity *geClientEnemy = GetGameWorld()->GetGameEntities()[1]; // Client.
		SetEnemy(geClientEnemy);
		SetGoalEntity(geClientEnemy);

        // Direction vector between player and other entity.
        vec3_t wishDir = GetGameWorld()->GetGameEntities()[1]->GetOrigin() - GetOrigin();
		wishDir.z = 0;
		
		// Set Model Angles.
		SetAngles(vec3_euler(wishDir));
		gi.DPrintf("wishDir: %f %f %f\n", wishDir.x, wishDir.y, wishDir.z);
		
		// Calculate yaw to use based on direction.
	    float yaw = vec3_to_yaw(wishDir);
		
        // Last but not least, move a step ahead.
        StepMove_WalkDirection(yaw, 90.f );//(0.1f / BASE_FRAMEDIVIDER) * (200.f / 200.f));
	}    

	LinkEntity();
	
	if ( !StepMove_CheckBottom() ) {
		StepMove_FixCheckBottom();
	}

    // Check for ground.   
	SG_CheckGround( this );

    // Setup its next think time, for a frame ahead.
    SetThinkCallback(&MonsterStepDummy::MonsterStepDummyThink);
    // Setup the next think time.
    SetNextThinkTime(level.time + FRAMETIME);

 //   // Advance the dummy animation for a frame.
 //   // Set here how fast you want the tick rate to be.
 //   // Set here how fast you want the tick rate to be.
 //   static constexpr uint32_t ANIM_HZ = 30.0;

 //   // Calclate all related values we need to make it work smoothly even if we have
 //   // a nice 250fps, the game must run at 50fps.
 //   //static constexpr uint32_t ANIM_FRAMERATE = ANIM_HZ;
 //   //static constexpr double   ANIM_FRAMETIME = 1000.0 / ANIM_FRAMERATE;
 //   //static constexpr double   ANIM_1_FRAMETIME = 1.0 / ANIM_FRAMETIME;
 //   //static constexpr double   ANIM_FRAMETIME_1000 = ANIM_FRAMETIME / 1000.0;
 //   //float nextFrame = GetAnimationFrame();
 //   //nextFrame += (32.f * ANIM_1_FRAMETIME);
 //   //if (nextFrame > 33) {
	//   // nextFrame = 2;
 //   //}
 //   //SetAnimationFrame(nextFrame);

 //   //
 //   // Calculate direction.
 //   //
 //   if (GetHealth() > 0) {
	//	// Get direction vector.
	//	vec3_t direction = GetGameWorld()->GetGameEntities()[1]->GetOrigin() - GetOrigin();
	//	
	//	// Cancel uit the Z direction.
	//	direction.z = 0;

	//	// Set model angles to euler converted direction.
	//	SetAngles(vec3_euler(direction));

	//	// Set velocity to head into direction.
	//	const vec3_t normalizedDir = vec3_normalize(direction);
	//	const vec3_t oldVelocity = GetVelocity();
	//	const vec3_t wishVelocity = vec3_t {
	//		92.f * normalizedDir.x,
	//		92.f * normalizedDir.y,
	//		oldVelocity.z
	//	};
	//	SetVelocity(wishVelocity);

	//	// Set the animation.
	//	EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
	//	animationState->animationIndex = 1;
	//	animationState->startFrame = 1;
	//	animationState->endFrame = 62;
	//	animationState->frameTime = ANIMATION_FRAMETIME;
	//	animationState->startTime = startz = level.time.count() + FRAMETIME.count();
	//	animationState->loopCount = 0;
	//	animationState->forceLoop = true;
	//}

 //   // Check for ground.
 //   //SVG_StepMove_CheckGround(this);
	//SG_CheckGround(this);
 //   // Link entity back in.
 //   LinkEntity();

	//// Setup next think callback.
 //   SetThinkCallback(&MonsterStepDummy::MonsterStepDummyThink);
 //   // Setup the next think time.
 //   SetNextThinkTime(level.time + FRAMETIME);
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