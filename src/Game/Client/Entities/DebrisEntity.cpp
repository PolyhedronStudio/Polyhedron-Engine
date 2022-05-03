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
DebrisEntity::~DebrisEntity() {

}

/**
*	@brief	Die callback.
**/
void DebrisEntity::DebrisEntityDie(GameEntity* inflictor, GameEntity* attacker, int32_t damage, const vec3_t& point) {
    // Save to queue for removal.
    Remove();
}


/**
*   @brief  Used by game modes to spawn server side gibs.
*   @param  debrisser The entity that is about to spawn debris.
**/
DebrisEntity* DebrisEntity::Create(GameEntity* debrisser, const std::string& debrisModel, const vec3_t& origin, float speed) {
	PODEntity *localDebrisEntity = GetGameWorld()->GetUnusedPODEntity(false);

	// Create a gib entity.
    DebrisEntity *debrisEntity = GetGameWorld()->CreateGameEntity<DebrisEntity>(localDebrisEntity, false, false);


	//// Chunk Entity.
    //DebrisEntity* debrisEntity = GetGameWorld()->CreateGameEntity<DebrisEntity>();

    // Set the origin.
    debrisEntity->SetOrigin(origin);

    // Set the model.
    debrisEntity->SetModel(debrisModel);

    // Calculate and set the velocity.
    vec3_t velocity = { 100.f * crandom(), 100.f * crandom(), 100.f + 100.f * crandom() };
    debrisEntity->SetVelocity(vec3_fmaf(debrisser->GetVelocity(), speed, velocity));

    // Set Movetype and Solid.
    debrisEntity->SetMoveType(MoveType::Bounce);
    debrisEntity->SetSolid(Solid::Not);

    // Calculate and set angular velocity.
    vec3_t angularVelocity = { random() * 600, random() * 600, random() * 600 };
    debrisEntity->SetAngularVelocity(angularVelocity);

    // Set up the thinking machine.
    debrisEntity->SetThinkCallback(&CLGBaseLocalEntity::CLGBaseLocalEntityThinkFree);
    debrisEntity->SetNextThinkTime(level.time + 5s + Frametime(random() * 5));

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