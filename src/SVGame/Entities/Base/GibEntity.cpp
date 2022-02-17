/*
// LICENSE HERE.

//
// GibEntity.cpp
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
#include "GibEntity.h"

// Gamemode.
#include "../../Gamemodes/IGamemode.h"

// World.
#include "../../World/Gameworld.h"


/**
*   @brief  Used by game modes to spawn server side gibs.
**/
GibEntity* GibEntity::Create(SVGBaseEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType) {
    // Create a gib entity.
    GibEntity* gibEntity = GetGameworld()->CreateClassEntity<GibEntity>();

    // Set size.
    vec3_t size = vec3_scale(gibber->GetSize(), 0.5f);
    gibEntity->SetSize(size);

    // Generate the origin to start from.
    vec3_t origin = gibber->GetAbsoluteMin() + gibber->GetSize();

    // Add some random values to it, so they all differ.
    origin.x += crandom() * size.x;
    origin.y += crandom() * size.y;
    origin.z += crandom() * size.z;

    // Set the origin.
    gibEntity->SetOrigin(origin);

    // Set the model.
    gibEntity->SetModel(gibModel);

    // Set solid and other properties.
    gibEntity->SetSolid(Solid::Not);
    gibEntity->SetEffects(gibEntity->GetEffects() | EntityEffectType::Gib);
    gibEntity->SetFlags(gibEntity->GetFlags() | EntityFlags::NoKnockBack);
    gibEntity->SetTakeDamage(TakeDamage::Yes);
    gibEntity->SetDieCallback(&GibEntity::GibEntityDie);

    // Default velocity scale for non organic materials.
    float velocityScale = 1.f;

    // Is it an organic gib type?
    if (gibType == GibType::Organic) {
    	// Different move type for organic gibs.
	    gibEntity->SetMoveType(MoveType::Toss);

	    // Most of all, we setup a touch callback too ofc.
	    gibEntity->SetTouchCallback(&GibEntity::GibEntityTouch);

	    // Adjust the velocity scale.
	    velocityScale = 0.5f;
    } else {
	    // Pick a different movetype, bouncing. No touch callback :)
	    gibEntity->SetMoveType(MoveType::Bounce);
    }

    // Comment later...
    vec3_t velocityDamage = game.GetGamemode()->CalculateDamageVelocity(damage);

    // Reassign 'velocityDamage' and multiply 'self->GetVelocity' to scale, and then
    // adding it on to 'velocityDamage' its old value.
    vec3_t gibVelocity = vec3_fmaf(gibber->GetVelocity(), velocityScale, velocityDamage);

    // Be sure to clip our velocity, just in case.
    gibEntity->ClipGibVelocity(velocityDamage);

    // Last but not least, set our velocity.
    gibEntity->SetVelocity(velocityDamage);
    
    // Generate angular velocity.
    vec3_t angularVelocity = { Randomui() * 600.f, Randomui() * 600.f, Randomui() * 600.f };

    // Set angular velocity.
    gibEntity->SetAngularVelocity(angularVelocity);

    // Setup the Gib think function and its think time.
    gibEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    gibEntity->SetNextThinkTime(level.time + 10 + Randomui() * 10);

    // Link entity into the world.
    gibEntity->LinkEntity();

    return gibEntity;
}


// Constructor/Deconstructor.
GibEntity::GibEntity(Entity* svEntity) : SVGBaseEntity(svEntity) { }
GibEntity::~GibEntity() { }

//
//===============
// SVGBasePlayer::Precache
//
//===============
//
void GibEntity::Precache() {
    Base::Precache();
}

//
//===============
// SVGBasePlayer::Spawn
//
//===============
//
void GibEntity::Spawn() {
    // Spawn.
    Base::Spawn();
}

//
//===============
// GibEntity::Respawn
//
//===============
//
void GibEntity::Respawn() {
    Base::Respawn();
}

//
//===============
// GibEntity::PostSpawn
//
//===============
//
void GibEntity::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// GibEntity::Think
//
//===============
//
void GibEntity::Think() {
    // Parent class Think.
    Base::Think();
}

//===============
// GibEntity::SpawnKey
//
// GibEntity spawn key handling.
//===============
void GibEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//===============
// GibEntity::ClipGibVelocity
//
// Clips gib velocity, in case it goes out of control that is.
//===============
void GibEntity::ClipGibVelocity(vec3_t &velocity) {
    // Y Axis.
    if (velocity.x < -300)
        velocity.x = -300;
    else if (velocity.x > 300)
        velocity.x = 300;
    
    // Y Axis.
    if (velocity.y < -300)
        velocity.y = -300;
    else if (velocity.y > 300)
        velocity.y = 300;

    // Z Axis.
    if (velocity.z < 200)
        velocity.z = 200;
    else if (velocity.z > 500)
        velocity.y = 500;
}

//===============
// GibEntity::CalculateVelocityForDamage
//
// 'other' is the actual entity that is spawning these gibs.
//===============
//float GibEntity::CalculateVelocityForDamage(SVGBaseEntity *other, const int32_t damage, vec3_t &velocity) {
//    // Calculate the velocity based on the damage passed over.
//    velocity = {
//        100.f * crandom(),
//        100.f * crandom(),
//        200.f + 100.f * random()
//    };
//
//    // Set velocity scale value based on the intensity of the amount of 'damage'.
//    float velocityScale = 1.2f;
//    if (damage < 50)
//        velocityScale = 0.7f;
//    
//    // Calculate velocity for damage.
//    velocity = vec3_scale(velocity, 0.7);
//
//    // Return.
//    return velocityScale;
//}

//===============
// GibEntity::GibEntityThink
//
//===============
void GibEntity::GibEntityThink() {
    // Increase frame and set a new think time.
    SetFrame(GetFrame() + 1);
    SetNextThinkTime(level.time + FRAMETIME);

    // Play frames for these meshes, cut the crap at frame 10.
    if (GetFrame() == 10) {
        SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
        SetNextThinkTime(level.time + 8 + Randomui() * 10);
    }
}

//===============
// GibEntity::GibEntityTouch
//
//===============
void GibEntity::GibEntityTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    vec3_t  right;

    if (!GetGroundEntity())
        return;

    // Reset touch callback to nullptr.
    SetTouchCallback(nullptr);

    // Did we get a plane passed?
    if (plane) {
        SVG_Sound(this, CHAN_VOICE, gi.SoundIndex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

        vec3_t normalAngles = vec3_euler(plane->normal);
        vec3_vectors(normalAngles, NULL, &right, NULL);
        vec3_t right = vec3_euler(GetState().angles);

        if (GetModelIndex() == sm_meat_index) {
            SetFrame(GetFrame() + 1);
            SetThinkCallback(&GibEntity::GibEntityThink);
            SetNextThinkTime(level.time + FRAMETIME);
        }
    }
}

//===============
// GibEntity::GibEntityDie
//
// Savely call Remove so it queues up for removal without causing 
// serverEntity/classEntity conflicts.
//===============
void GibEntity::GibEntityDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Time to queue it for removal.
    Remove();
}