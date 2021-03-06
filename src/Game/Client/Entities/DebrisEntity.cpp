/*
// LICENSE HERE.

//
// DebrisEntity.cpp
//
//
*/
#include "../ClientGameLocals.h"              // SVGame.
#include "../Effects/ParticleEffects.h"              // Effects.
//#include "../Entities.h"             // Entities.
//#include "../../Player/Client.h"        // Player Client functions.
//#include "../../Player/Animations.h"    // Include Player Client Animations.
#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0f * (random() - 0.5f))

// Class Entities.
#include "Base/CLGBaseLocalEntity.h"
#include "DebrisEntity.h"

// World.
#include "../World/ClientGameWorld.h"


//! Constructor/Deconstructor.
DebrisEntity::DebrisEntity(PODEntity *svEntity)
    : CLGBaseLocalEntity(svEntity) {

}

/**
*	@brief	Die callback.
**/
void DebrisEntity::DebrisEntityDie(GameEntity* inflictor, GameEntity* attacker, int32_t damage, const vec3_t& point) {
    // Save to queue for removal.
    Remove();
}

static inline const vec3_t CalculateDamageVelocity(int32_t damage) {
    // Pick random velocities.
    vec3_t velocity = {
        100.0f * crandom(),
        100.0f * crandom(),
        200.0f + 100.0f * random()
    };

    // Scale velocities.
    if (damage < 50)
        velocity = vec3_scale(velocity, 0.7f);
    else
        velocity = vec3_scale(velocity, 1.2f);

    // Return.
    return velocity;
}

/**
*   @brief  Used by game modes to spawn server side gibs.
*   @param  debrisser The entity that is about to spawn debris.
**/
DebrisEntity* DebrisEntity::Create(GameEntity* debrisser, const std::string& debrisModel, const vec3_t& origin, float speed) {
	PODEntity *localDebrisEntity = GetGameWorld()->GetUnusedPODEntity(false);

	// Create a gib entity.
    DebrisEntity *debrisEntity = GetGameWorld()->CreateGameEntity<DebrisEntity>(localDebrisEntity, false, false);


	// Set actual size of 
    vec3_t size = vec3_scale(debrisser->GetSize() , 0.5f);
    debrisEntity->SetSize(size);

    // Set the origin.
    debrisEntity->SetOrigin(origin);

    // Set the model.
    debrisEntity->SetModel(debrisModel);
	//const float velocityScale = 1.f;
 //   // Comment later...
 //   const vec3_t velocityDamage = CalculateDamageVelocity(200);

	//vec3_t velocity = vec3_t{ speed, speed, speed };
 //   // Reassign 'velocityDamage' and multiply 'self->GetVelocity' to scale, and then
 //   // adding it on to 'velocityDamage' its old value.
 //   vec3_t gibVelocity = vec3_fmaf( velocityDamage, velocityScale,  velocity );

 //   // Be sure to clip our velocity, just in case.
 //   ClipGibVelocity(gibVelocity);
	//debrisEntity->SetMoveType(MoveType::TossSlide);
 //   debrisEntity->SetVelocity(gibVelocity);

    // Calculate and set the velocity.
    const vec3_t velocity = { 500.f * crandom(), 500.f * crandom(), 500.f + 200.f * crandom() };
    debrisEntity->SetVelocity( vec3_fmaf( debrisser->GetVelocity(), speed, velocity ) );
	Com_DPrint(" THIS IS AN ENTITY I WANT TO KNOW ITS SPEED=%f\n", speed);

    // Set Movetype and Solid.
    debrisEntity->SetMoveType(MoveType::Bounce);
    debrisEntity->SetSolid(Solid::OctagonBox);

    // Calculate and set angular velocity.
    //vec3_t angularVelocity = { random() * 600, random() * 600, random() * 600 };
    //debrisEntity->SetAngularVelocity(angularVelocity);
    
    // Generate and apply a random angular velocity.
    const vec3_t angularVelocity = { Randomui() * 600.f, Randomui() * 600.f, Randomui() * 600.f };
	debrisEntity->SetAngularVelocity(angularVelocity);

    // Set up the thinking machine.
    debrisEntity->SetThinkCallback(&CLGBaseLocalEntity::CLGBaseLocalEntityThinkFree);
    debrisEntity->SetNextThinkTime(level.time + 5s + random() * 5s);

    // Setup the other properties.
    debrisEntity->SetAnimationFrame(0);
	debrisEntity->SetFlags(0);
    debrisEntity->SetTakeDamage(1);
    debrisEntity->SetDieCallback(&DebrisEntity::DebrisEntityDie);

    // Link it up.
    debrisEntity->LinkEntity();

    // Return the debris entity.
    return debrisEntity;
}