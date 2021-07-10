/*
// LICENSE HERE.

//
// BodyCorpse.cpp
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
#include "BodyCorpse.h"

// Constructor/Deconstructor.
BodyCorpse::BodyCorpse(Entity* svEntity)
    : SVGBaseEntity(svEntity) {

}
BodyCorpse::~BodyCorpse() {

}

//
//===============
// PlayerClient::Precache
//
//===============
//
void BodyCorpse::Precache() {
    Base::Precache();
}

//
//===============
// PlayerClient::Spawn
//
//===============
//
void BodyCorpse::Spawn() {
    // Spawn.
    Base::Spawn();
}

//
//===============
// BodyCorpse::Respawn
//
//===============
//
void BodyCorpse::Respawn() {
    Base::Respawn();
}

//
//===============
// BodyCorpse::PostSpawn
//
//===============
//
void BodyCorpse::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// BodyCorpse::Think
//
//===============
//
void BodyCorpse::Think() {
    // Parent class Think.
    Base::Think();
}

//
//===============
// BodyCorpse::SpawnKey
//
// BodyCorpse spawn key handling.
//===============
//
void BodyCorpse::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//
//===============
// BodyCorpse::BodyCorpseTouch
//
// 'Touch' callback, I am unsure what to use it for but I can imagine it being...
// like picking up their items or something? I suppose we could do that...
//===============
//
void BodyCorpse::BodyCorpseTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {

}