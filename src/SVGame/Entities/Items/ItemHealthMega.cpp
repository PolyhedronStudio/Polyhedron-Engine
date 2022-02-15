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
#include "../Base/SVGBasePlayer.h"

// Misc Explosion Box Entity.
#include "ItemHealthMega.h"

//Added in respawn flag support for items. ( I still gotta do it by the time I read this in the morning lol )
//
//Create SVGBaseItem::CreateDroppedItem(...) and do similar functionality for other entities that need it.
//
//Investigate client side weaponry by creating them in sharedgame. Perhaps same for trigger entities, or perhaps just start creating a client entity system as a whole...
//
//Investigate PhysX over Bullet 3D, look at the bookmarked articles about them and character controllers. Perhaps look for some collision library out there or physics and use that instead.

//
// Constructor/Deconstructor.
//
ItemHealthMega::ItemHealthMega(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

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
    SVG_PrecacheModel("models/items/healing/large/tris.md3");
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

    // Set the health model.
    SetModel("models/items/healing/large/tris.md3");
    //SetModelIndex(SVG_PrecacheModel("models/items/healing/large/tris.md3"));

    // Set render effects to be glowy.
    SetRenderEffects(GetRenderEffects() | RenderEffects::Glow | RenderEffects::DebugBoundingBox);

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }
    
    // Set the count for the amount of health this item will give.
    SetCount(100);

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::No);

    // Ensure it can be respawned.
    SetFlags(GetFlags() | EntityFlags::Respawn);

    // Setup our ItemHealthMega callbacks.
    SetPickupCallback(&ItemHealthMega::HealthMegaPickup);

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

    // Ensure other is a subclass of SVGBasePlayer.
    if (!other->IsSubclassOf<SVGBasePlayer>()) {
        return false;
    }

    // Cast to SVGBasePlayer.
    SVGBasePlayer* playerClient = dynamic_cast<SVGBasePlayer*>(other);

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

    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("items/m_health.wav"), 1, ATTN_NORM, 0);

    // Let it be known we picked the fucker up.
    SVG_CenterPrint(other, std::string("Picked up item: ") + GetClassname());

    // Set a respawn think for after 2 seconds.
    if (!game.GetCurrentGamemode()->IsClass<DefaultGamemode>()) {
        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
        SetNextThinkTime(level.time + 2);
    }

    return true;
}

//===============
// ItemHealthMega::HealthMegaThink
//
// 
//===============
void ItemHealthMega::HealthMegaThink(void) {

}

//===============
// ItemHealthMega::HealthMegaDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//void ItemHealthMega::HealthMegaDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
//
//}
//

////===============
//// ItemHealthMega::HealthMegaTouch
////
//// 'Touch' callback, to calculate the direction to move into.
////===============
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
