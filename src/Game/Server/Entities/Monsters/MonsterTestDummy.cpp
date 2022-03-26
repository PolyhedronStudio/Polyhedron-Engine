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
#include "../Base/SVGBaseMonster.h"

// World.
#include "../../World/Gameworld.h"

// Misc Server Model Entity.
#include "MonsterTestDummy.h"


//
// Constructor/Deconstructor.
//
MonsterTestDummy::MonsterTestDummy(Entity* svEntity) : Base(svEntity) { 
	const char *mapClass = GetTypeInfo()->mapClass; // typeinfo->classname = C++ classname.
	uint32_t hashedMapClass = GetTypeInfo()->hashedMapClass; // hashed mapClass.

	//gi.DPrintf("SVG Spawned: svNumber=#%i mapClass=%s hashedMapClass=#%i\n", svEntity->state.number, mapClass, hashedMapClass);
}

MonsterTestDummy::~MonsterTestDummy() { }


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
    SVG_PrecacheModel("models/monsters/testdummy/60fps.iqm");
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

    // Set solid.
    SetSolid(Solid::OctagonBox);

    // Set move type.
    SetMoveType(MoveType::Step);

    // Since this is a "monster", after all...
    SetServerFlags(EntityServerFlags::Monster);

    // Set clip mask.
    SetClipMask(BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid);

    // Set the barrel model, and model index.
    SetModel("models/monsters/testdummy/testdummy.iqm");

    // Set the bounding box.
    SetBoundingBox({ -16, -16, 0 }, { 16, 16, 52 });

    // Set default values in case we have none.
    if (!GetMass()) {
	    SetMass(200);
    }
    if (!GetHealth()) {
	    SetHealth(200);
    }
    
    // Setup the start frame to animate from.
    SetAnimationFrame(0);
    
    // Set entity to allow taking damage.
    SetTakeDamage(TakeDamage::Yes);

    //// Setup our MonsterTestDummy callbacks.
    SetThinkCallback(&MonsterTestDummy::MonsterTestDummyStartAnimation);
    SetDieCallback(&MonsterTestDummy::MonsterTestDummyDie);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2.f * FRAMETIME);

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

    GetPODEntity()->state.animationStartTime = level.time * 1000;
    //GetPODEntity()->state.animationFramerate = 60;

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
    EntityState* state = &GetPODEntity()->state;

    if (key == "startframe") { 
        // This is a lame hack so I can debug this from TB, set an animation index should always be done using SetAnimation
        int32_t parsedInt = 0;
	    ParseIntegerKeyValue(key, value, parsedInt);
        state->animationStartFrame = parsedInt;
    } else if (key == "endframe") {
	// This is a lame hack so I can debug this from TB, set an animation index should always be done using SetAnimation
	    int32_t parsedInt = 0;
	    ParseIntegerKeyValue(key, value, parsedInt);
	    state->animationEndFrame = parsedInt;
    } else if (key == "framerate") {
	    float parsedFloat = 0.f;
	    ParseFloatKeyValue(key, value, parsedFloat);
	    state->animationFramerate = parsedFloat;        
    } else if (key == "animindex") {
    } else if (key == "skin") {
    } else if (key == "health") {
        int32_t parsedInt = 0;
        ParseIntegerKeyValue(key, value, parsedInt);
        SetHealth(parsedInt);
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
    // Calculate direction.
    //
    //if (GetHealth() > 0) {
	   // vec3_t currentMoveAngles = GetAngles();

	   // // Direction vector between player and other entity.
	   // vec3_t wishMoveAngles = GetGameworld()->GetClassEntities()[1]->GetOrigin() - GetOrigin();

	   // // Teehee
	   // vec3_t newModelAngles = vec3_euler(wishMoveAngles);
	   // newModelAngles.x = 0;

	   // SetAngles(newModelAngles);

	   // // Calculate yaw to use based on direction.
	   // float yaw = vec3_to_yaw(wishMoveAngles);

	   // // Last but not least, move a step ahead.
	   // SVG_StepMove_Walk(this, yaw, 90 * FRAMETIME);
    //}

    // Link entity back in.
    LinkEntity();

    // Check for ground.
    SVG_StepMove_CheckGround(this);

    // Setup its next think time, for a frame ahead.
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
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
    Gameworld* gameworld = GetGameworld();
    gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + 1 * FRAMETIME);

    // Set think function.
    SetThinkCallback(&MonsterTestDummy::SVGBaseEntityThinkFree);
}