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

//===============
// BodyCorpse::SpawnKey
//
// BodyCorpse spawn key handling.
//===============
void BodyCorpse::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//===============
// BodyCorpse::BodyCorpseTouch
//
// 'Touch' callback, I am unsure what to use it for but I can imagine it being...
// like picking up their items or something? I suppose we could do that...
//===============
void BodyCorpse::BodyCorpseTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {

}

//===============
// BodyCorpse::BodyCorpseDie
//
// Spawn gibs to make things gore like :P
//===============
void BodyCorpse::BodyCorpseDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    //int n;

    // In case health is low enough...
    if (GetHealth() < -40) {
        // Play sound.
        SVG_Sound(this, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);

        // Toss gibs.
        for (int32_t i = 0; i < 4; i++)
            SVG_ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

        // Ensure its origin goes -48, it is a lame hack but hey...
        vec3_t origin = GetOrigin();
        origin.z -= 48;
        SetOrigin(origin);
    
        // Toss head around.
        //SVG_ThrowClientHead(self, damage);

        // Ensure we take no damage no more.
        SetTakeDamage(TakeDamage::No);
    }


    // Set the ehrm.. think free thing.
    //SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    //SetNextThinkTime(level.time + FRAMETIME);
    // Remove body.
    SetModelIndex(0);
    //Remove();
}