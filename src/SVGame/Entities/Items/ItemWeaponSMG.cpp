/***
*
*	License here.
*
*	@file
*
*	SMG weapon implementation.
*
***/
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
#include "../Base/SVGBaseItemWeapon.h"
#include "../Base/SVGBasePlayer.h"

// Misc Explosion Box Entity.
#include "ItemWeaponSMG.h"

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
ItemWeaponSMG::ItemWeaponSMG(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) { 
}
ItemWeaponSMG::~ItemWeaponSMG() { 
}


//
// Interface functions.
//
//
//===============
// ItemWeaponSMG::Precache
//
//===============
//
void ItemWeaponSMG::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache sounds. // TODO: First precache sound section of this code must move to player sound precache code.
    SVG_PrecacheSound("weapons/bulletdrop1.wav");
    SVG_PrecacheSound("weapons/bulletdrop2.wav");
    SVG_PrecacheSound("weapons/bulletdrop3.wav");

    SVG_PrecacheSound("weapons/dryfire.wav");
    SVG_PrecacheSound("weapons/hidedefault.wav");
    SVG_PrecacheSound("weapons/pickup1.wav");
    SVG_PrecacheSound("weapons/readygeneric.wav");

    // Precache sounds.
    SVG_PrecacheSound("weapons/smg/fire1.wav");
    SVG_PrecacheSound("weapons/smg/fire2.wav");
    SVG_PrecacheSound("weapons/smg/ready1.wav");
    SVG_PrecacheSound("weapons/smg/ready2.wav");

    SVG_PrecacheSound("weapons/smg/reload1.wav");
    SVG_PrecacheSound("weapons/smg/reload2.wav");

    SVG_PrecacheSound("weapons/smg/reloadclip1.wav");
    SVG_PrecacheSound("weapons/smg/reloadclip2.wav");

    // Precache world and view model.
    SVG_PrecacheModel("models/weapons/v_smg/tris.iqm");
    SVG_PrecacheModel("models/weapons/w_smg/tris.iqm");
}

//
//===============
// ItemWeaponSMG::Spawn
//
//===============
//
void ItemWeaponSMG::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the weapon world model.
    SetModel("models/weapons/w_smg/tris.iqm");

    // Set render effects to be glowy.
    SetEffects(GetEffects() | EntityEffectType::Rotate);
    SetRenderEffects(GetRenderEffects() | RenderEffects::Glow);

    // Set the count for the amount of ammo this weapon will give by default.
    SetCount(36);

    // Ensure it can be respawned.
    SetFlags(GetFlags() | EntityFlags::Respawn);

    // Setup our ItemWeaponSMG callbacks.
    SetPickupCallback(&ItemWeaponSMG::WeaponSMGPickup);
    SetUseInstanceCallback(&ItemWeaponSMG::WeaponSMGUseInstance);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// ItemWeaponSMG::Respawn
//
//===============
//
void ItemWeaponSMG::Respawn() { 
    Base::Respawn(); 
}

//
//===============
// ItemWeaponSMG::PostSpawn
//
//===============
//
void ItemWeaponSMG::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// ItemWeaponSMG::Think
//
//===============
//
void ItemWeaponSMG::Think() {
    // Always call parent class method.
    Base::Think();
}


/**
*
*   Instance Interface implementation functions.
*
**/
void ItemWeaponSMG::InstanceSpawn() { 
    SetUseInstanceCallback(&ItemWeaponSMG::WeaponSMGUseInstance);
}

//
// Callback Functions.
//
//===============
// ItemWeaponSMG::WeaponSMGPickup
//
//
//===============
qboolean ItemWeaponSMG::WeaponSMGPickup(SVGBaseEntity* other) {
    // Sanity check.
    if (!other || !other->GetClient() || !other->IsSubclassOf<SVGBasePlayer>()) {
	    return false;
    }

    // Cast to SVGBasePlayer.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(other);
    ServerClient *client = player->GetClient();

    // Acquire our identifier.
    uint32_t id = GetIdentifier();

    // TODO HERE: Check whether game mode allows for picking up this tiem.
    player->GetClient()->persistent.inventory[id]++; // If we CAN pick it up, increment inventory item index.

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    //if (!(GetSpawnFlags() & DROPPED_ITEM)) {
        // TODO HERE: Add ammo of quantity to player->GetClient()->persistent.inventory[AmmoIdentifier()];

        // TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
    //}

    // Last but not least, set the weapon as active in case the player does not have it selected.
    //gi.DPrintf("BEFORE: Identifier=%i, activeWeaponIdentifier=%i, inventory[%i]=%i\n", GetIdentifier(), client->persistent.activeWeapon->GetIdentifier(), id, client->persistent.inventory[id]);
    if (client->persistent.activeWeapon != SVGBaseItemWeapon::GetItemInstanceByID(id) && client->persistent.inventory[id] >= 1) {
	    // Ensure the item is of class smg, return false if not.
        SVGBaseItem *itemInstance= SVGBaseItem::GetItemInstanceByID(id);

        if (!itemInstance->IsClass<ItemWeaponSMG>()) {
            return false;
        }

        // Up cast our weapon to SVGItemWeaponSMG.
    	ItemWeaponSMG* weaponInstance = dynamic_cast<ItemWeaponSMG*>(itemInstance);

        // Set it as our new weapon.
        client->newWeapon = weaponInstance;
    }
    //gi.DPrintf("MID: Identifier=%i, activeWeaponIdentifier=%i, inventory[%i]=%i\n", GetIdentifier(), client->persistent.activeWeapon->GetIdentifier(), id, client->persistent.inventory[id]);
    
    // Activate freshly pickup weapon.
    //SVG_ChangeWeapon(player);

    //gi.DPrintf("AFTER: Identifier=%i, activeWeaponIdentifier=%i, inventory[%i]=%i\n", GetIdentifier(), client->persistent.activeWeapon->GetIdentifier(), id, client->persistent.inventory[id]);

    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("weapons/pickup1.wav"), 1, ATTN_NORM, 0);

    // Let it be known we picked the fucker up.
    SVG_CenterPrint(other, std::string("Picked up item: ") + GetClassname());

    // Set a respawn think for after 2 seconds.
    if (!game.GetGamemode()->IsClass<DefaultGamemode>()) {
	    SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
	    SetNextThinkTime(level.time + 2);
    }

    return true;
}

//===============
// ItemWeaponSMG::WeaponSMGThink
//
//
//===============
void ItemWeaponSMG::WeaponSMGThink(void) { 
}

void ItemWeaponSMG::WeaponSMGUseInstance(SVGBaseEntity* user, SVGBaseItem* item) { 
    // Check if the caller is a player.
    if (!user || !user->GetClient() || !user->IsSubclassOf<SVGBasePlayer>()) {
        return;
    }

    // Check if the activator is a valid weapon.
    if (!item->IsClass<ItemWeaponSMG>()) {
        return;
    }

    // Cast to player.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(user);

    // Cast to weapon.
    ItemWeaponSMG* smgItem = dynamic_cast<ItemWeaponSMG*>(item);

    // Get its identifier.
    uint32_t smgItemID = smgItem->GetIdentifier();

    // Get client.
    ServerClient* client = player->GetClient();

    // Prevent change weapon from happening if this weapon happens to be already active.
    if (client->persistent.activeWeapon && client->persistent.activeWeapon->GetIdentifier() != smgItemID) {
	    SVG_ChangeWeapon(player);
    } else if (!client->persistent.activeWeapon) {
	    SVG_ChangeWeapon(player);
    }
}
//===============
// ItemWeaponSMG::WeaponSMGDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//void ItemWeaponSMG::WeaponSMGDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
//
//}
//

////===============
//// ItemWeaponSMG::WeaponSMGTouch
////
//// 'Touch' callback, to calculate the direction to move into.
////===============
//void ItemWeaponSMG::WeaponSMGTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
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
