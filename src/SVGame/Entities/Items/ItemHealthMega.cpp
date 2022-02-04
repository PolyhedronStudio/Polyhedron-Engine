/*
// LICENSE HERE.

//
// ItemHealthMega.cpp
//
// Base class to create item entities from.
//
// Gives the following functionalities:
// TODO: Explain what.
//
*/
#include "../../ServerGameLocal.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Deathmatch Game Mode.
#include "../../Gamemodes/DeathmatchGamemode.h"

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/PlayerClient.h"

// Misc Explosion Box Entity.
#include "ItemHealthMega.h"


//
// Constructor/Deconstructor.
//
ItemHealthMega::ItemHealthMega(Entity* svEntity) 
    : Base(svEntity) {

}
ItemHealthMega::~ItemHealthMega() {

}



//
// Interface functions. 
//
//
//===============
// ItemHealthMega::Precache
//
//===============
//
void ItemHealthMega::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache sounds & model.
    SVG_PrecacheSound("items/pkup.wav");
    SVG_PrecacheSound("items/m_health.wav");
    SVG_PrecacheModel("models/items/healing/medium/tris.md2");
}

//
//===============
// ItemHealthMega::Spawn
//
//===============
//
void ItemHealthMega::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set solid.
    SetSolid(Solid::Trigger);

    // Set move type.
    SetMoveType(MoveType::Toss);

    // Set the health model.
    SetModel("models/objects/debris1/tris.md2");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, -16.f },
        // Maxs.
        { 16.f, 16.f, 16.f }
    );

    // Set render effects to be glowy.
    //SetRenderEffects(GetRenderEffects() | RenderEffects::Glow);
    //SetFlags(EntityFlags::PowerArmor);
    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }

    // Set the count for the amount of health this item will give.
    SetCount(100);

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::No);

    // Setup our ItemHealthMega callbacks.
    //SetUseCallback(&ItemHealthMega::HealthMegaUse);
    //SetThinkCallback(&ItemHealthMega::HealthMegaThink);
    //SetDieCallback(&ItemHealthMega::HealthMegaDie);
    //SetTouchCallback(&ItemHealthMega::HealthMegaTouch);
    SetPickupCallback(&ItemHealthMega::HealthMegaPickup);

    // Start thinking after other entities have spawned. This allows for items to safely
    // drop on platforms etc.
    SetNextThinkTime(level.time + 2.1f * FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// ItemHealthMega::Respawn
//
//===============
//
void ItemHealthMega::Respawn() {
    Base::Respawn();
}

//
//===============
// ItemHealthMega::PostSpawn
//
//===============
//
void ItemHealthMega::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// ItemHealthMega::Think
//
//===============
//
void ItemHealthMega::Think() {
    // Always call parent class method.
    Base::Think();

    // Prevent the owner from picking up the item

}


//
// Callback Functions.
//

//===============
// ItemHealthMega::HealthMegaPickup
// 
// 
//===============
qboolean ItemHealthMega::HealthMegaPickup(SVGBaseEntity* other) {
    // Ensure other is a valid pointer.
    if (!other) {
        return false;
    }

    // Ensure other is a subclass of PlayerClient.
    if (!other->IsSubclassOf<PlayerClient>()) {
        return false;
    }

    // Cast to PlayerClient.
    PlayerClient* playerClient = dynamic_cast<PlayerClient*>(other);

    // Increase health.
    const int32_t playerHealth = playerClient->GetHealth();
    other->SetHealth(playerHealth + GetCount());

    // Reset health to maximum health in case IgnoreMaxHealth style is set.
    if (!(GetStyle() & ItemHealthMega::IF_IgnoreMaxHealth)) {
        const int32_t maxHealth = other->GetMaxHealth();
        if (other->GetHealth() > maxHealth) {
            other->SetHealth(maxHealth);
        }
    }

    // If health is timed...
    if (GetStyle() & ItemHealthMega::IF_TimedHealth) {
        //SetThinkCallback(&ItemHealthMega::HealthMegaThink);
        SetNextThinkTime(level.time + 5);
        SetOwner(other);
        SetFlags(GetFlags() | EntityFlags::Respawn);
        SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);
        SetSolid(Solid::Not);
    } else {
        if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem) && game.gameMode->IsSubclassOf<DeathmatchGamemode>()) {
        //SVG_SetRespawn();
        }
    }

    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("items/m_health.wav"), 1, ATTN_NORM, 0);

    // Let it be known we picked the fucker up.
    SVG_CenterPrint(other, std::string("Picked up: %s") + GetClassname());

    return true;
}

//
//===============
// ItemHealthMega::HealthMegaThink
//
// 
//===============
//
//void ItemHealthMega::HealthMegaThink(void) {
//
//
//    // Setup its next think time, for a frame ahead.
//    SetThinkCallback(&ItemHealthMega::HealthMegaThink);
//    SetNextThinkTime(level.time + 1.f * FRAMETIME);
//}

//
//===============
// ItemHealthMega::HealthMegaDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
//void ItemHealthMega::HealthMegaDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
//
//}
//
////
////===============
//// ItemHealthMega::HealthMegaTouch
////
//// 'Touch' callback, to calculate the direction to move into.
////===============
////
//void ItemHealthMega::HealthMegaTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
//    // Safety checks.
//    if (!self)
//        return;
//    if (!other)
//        return;
//    // TODO: Move elsewhere in baseentity, I guess?
//    // Prevent this entity from touching itself.
//    if (self == other)
//        return;
//
//    // Ground entity checks.
//    if ((!other->GetGroundEntity()) || (other->GetGroundEntity() == self))
//        return;
//
//
//}