/*
// LICENSE HERE.

//
// DebrisEntity.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../../effects.h"              // Effects.
#include "../../entities.h"             // Entities.
#include "../../player/client.h"        // Player Client functions.
#include "../../player/animations.h"    // Include Player Client Animations.
#include "../../player/view.h"          // Include Player View functions..
#include "../../utils.h"                // Util funcs.

// Class Entities.
#include "../base/SVGBaseEntity.h"
#include "DebrisEntity.h"

// Constructor/Deconstructor.
DebrisEntity::DebrisEntity(Entity* svEntity)
    : SVGBaseEntity(svEntity) {

}
DebrisEntity::~DebrisEntity() {

}

//
//===============
// PlayerClient::Precache
//
//===============
//
void DebrisEntity::Precache() {
    Base::Precache();
}

//
//===============
// PlayerClient::Spawn
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