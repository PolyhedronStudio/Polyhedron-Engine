/***
*
*	License here.
*
*	@file
*
*	Basic 9mm ammo clip item. Used for most short range weapons.
*   
*
***/
#include "../../ServerGameLocals.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Deathmatch Game Mode.
#include "../../Gamemodes/DeathMatchGamemode.h"

// Base class entities.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBaseItemAmmo.h"
#include "../Base/SVGBasePlayer.h"

// Ammo 9mm.
#include "ItemAmmo9mm.h"


//! Constructor/Deconstructor.
ItemAmmo9mm::ItemAmmo9mm(PODEntity *svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}


/**
* 
*   Interface implementation functions.
*
***/
/**
*   @brief
**/
void ItemAmmo9mm::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache sounds & model.
    SVG_PrecacheModel("models/ammo/9mmclip/tris.iqm");
    SVG_PrecacheSound("weapons/pickup1.wav");
}

/**
*   @brief
**/
void ItemAmmo9mm::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the health model.
    SetModel("models/ammo/9mmclip/tris.iqm");

    // Set effects to rotate.
    SetEffects(GetEffects() | EntityEffectType::Rotate);

    // Set render effects to be glowy.
    //SetRenderEffects(GetRenderEffects() | RenderEffects::Glow | RenderEffects::DebugBoundingBox);

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }
    
    // Set the count for the amount of ammo this item will give.
    if (!GetCount()) {
	    SetCount(36);   // I read that 50 is typical for 9mm ammo wpns, lol.
    }

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::No);

    // Ensure it can be respawned.
    SetFlags(GetFlags() | EntityFlags::Respawn);

    // Setup our ItemAmmo9mm callbacks.
    SetPickupCallback(&ItemAmmo9mm::Ammo9mmPickup);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

/**
*   @brief
**/
qboolean ItemAmmo9mm::Ammo9mmPickup(IServerGameEntity *other) {
    // Ensure other is a valid pointer.
    if (!other || !other->IsSubclassOf<SVGBasePlayer>()) {
        return false;
    }

    // Cast to SVGBasePlayer.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(other);
    // Get client.
    ServerClient* client = player->GetClient();

    // Last sanity check.
    if (!client) {
        return false;
    }

    // If the player can't add ammo, return false so this item won't get picked up.
    if (!player->GiveAmmo(GetIdentifier(), 36)) {
	    SVG_CenterPrint(player, "You're already carring too much 9mm ammo.");
        return false;
    }

    // Play sound.
    SVG_Sound(other, SoundChannel::Item, SVG_PrecacheSound("weapons/pickup1.wav"), 1, Attenuation::Normal, 0);

    // Let it be known we picked the fucker up.
    SVG_CenterPrint(other, std::string("Picked up item: ") + GetClassname());

    // Set a respawn think for after 2 seconds.
    if (!GetGameMode()->IsClass<DefaultGameMode>()) {
        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
        SetNextThinkTime(level.time + 2s);
    }

    return true;
}