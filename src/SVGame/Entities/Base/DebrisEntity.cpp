/*
// LICENSE HERE.

//
// DebrisEntity.cpp
//
//
*/
#include "../../ServerGameLocal.h"              // SVGame.
#include "../../Effects.h"              // Effects.
#include "../../Entities.h"             // Entities.
#include "../../Player/Client.h"        // Player Client functions.
#include "../../Player/Animations.h"    // Include Player Client Animations.
#include "../../Player/View.h"          // Include Player View functions..
#include "../../Utilities.h"                // Util funcs.

// Class Entities.
#include "DebrisEntity.h"

// World.
#include "../../World/Gameworld.h"

/**
*   @brief  Used by game modes to spawn server side gibs.
*   @param  debrisser The entity that is about to spawn debris.
**/
DebrisEntity* DebrisEntity::Create(SVGBaseEntity* debrisser, const std::string& debrisModel, const vec3_t& origin, float speed) {
    // Chunk Entity.
    DebrisEntity* debrisEntity = GetGameworld()->CreateClassEntity<DebrisEntity>();

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
    debrisEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    debrisEntity->SetNextThinkTime(level.time + 5 + random() * 5);

    // Setup the other properties.
    debrisEntity->SetAnimationFrame(0);
    debrisEntity->SetFlags(0);
    debrisEntity->SetTakeDamage(TakeDamage::Yes);
    debrisEntity->SetDieCallback(&DebrisEntity::DebrisEntityDie);

    // Link it up.
    debrisEntity->LinkEntity();

    // Return the debris entity.
    return debrisEntity;
}

// Constructor/Deconstructor.
DebrisEntity::DebrisEntity(Entity* svEntity)
    : SVGBaseEntity(svEntity) {

}
DebrisEntity::~DebrisEntity() {

}

//
//===============
// SVGBasePlayer::Precache
//
//===============
//
void DebrisEntity::Precache() {
    Base::Precache();
}

//
//===============
// SVGBasePlayer::Spawn
//
//===============
//
void DebrisEntity::Spawn() {
    // Spawn.
    Base::Spawn();
}

//
//===============
// DebrisEntity::Respawn
//
//===============
//
void DebrisEntity::Respawn() {
    Base::Respawn();
}

//
//===============
// DebrisEntity::PostSpawn
//
//===============
//
void DebrisEntity::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// DebrisEntity::Think
//
//===============
//
void DebrisEntity::Think() {
    // Parent class Think.
    Base::Think();
}

//===============
// DebrisEntity::SpawnKey
//
// DebrisEntity spawn key handling.
//===============
void DebrisEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//===============
// DebrisEntity::DebrisEntityDie
//
// Spawn gibs to make things gore like :P
//===============
void DebrisEntity::DebrisEntityDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Save to queue for removal.
    Remove();
}