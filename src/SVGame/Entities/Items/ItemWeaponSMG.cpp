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

// World.
#include "../../World/Gameworld.h"

// SMG.
#include "ItemWeaponSMG.h"


//! Constructor/Deconstructor.
ItemWeaponSMG::ItemWeaponSMG(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) { }
ItemWeaponSMG::~ItemWeaponSMG() { }


/**
*
*   Interface Functions.
*
**/
/**
*   @brief 
**/
void ItemWeaponSMG::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache models.
    // NOTE: There are none to precache as of yet, SVGBaseItem does so by using
    // GetViewModel and GetWorldModel to acquire the path for precaching.


    // Precache sounds.
    // TODO: First precache sound section of this code must move to player sound precache code.
    SVG_PrecacheSound("weapons/bulletdrop1.wav");
    SVG_PrecacheSound("weapons/bulletdrop2.wav");
    SVG_PrecacheSound("weapons/bulletdrop3.wav");

    SVG_PrecacheSound("weapons/dryfire.wav");
    SVG_PrecacheSound("weapons/hidedefault.wav");
    SVG_PrecacheSound("weapons/pickup1.wav");
    SVG_PrecacheSound("weapons/readygeneric.wav");
    // TODO: The above precache sound section of this code must move to player sound precache code.

    // Precache sounds.
    SVG_PrecacheSound("weapons/smg45/fire1.wav");
    SVG_PrecacheSound("weapons/smg45/fire2.wav");
    SVG_PrecacheSound("weapons/smg45/ready1.wav");
    SVG_PrecacheSound("weapons/smg45/ready2.wav");

    SVG_PrecacheSound("weapons/smg45/reload1.wav");
    SVG_PrecacheSound("weapons/smg45/reload2.wav");

    SVG_PrecacheSound("weapons/smg45/reloadclip1.wav");
    SVG_PrecacheSound("weapons/smg45/reloadclip2.wav");
}

/**
*   @brief 
**/
void ItemWeaponSMG::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set render effects to be glowy.
    SetEffects(GetEffects() | EntityEffectType::Rotate);
    SetRenderEffects(GetRenderEffects() | RenderEffects::Glow);

    // Set the count for the amount of ammo this weapon will give by default.
    SetCount(36);

    // Ensure it can be respawned.
    SetFlags(GetFlags() | EntityFlags::Respawn);

    // Setup our ItemWeaponSMG callbacks.
    SetPickupCallback(&ItemWeaponSMG::WeaponSMGPickup);
    SetUseInstanceCallback(&ItemWeaponSMG::InstanceWeaponSMGUse);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

/**
*   @brief 
**/
void ItemWeaponSMG::Respawn() { 
    Base::Respawn(); 
}

/**
*   @brief 
**/
void ItemWeaponSMG::PostSpawn() {
   Base::PostSpawn();
}

/**
*   @brief 
**/
void ItemWeaponSMG::Think() {
    Base::Think();
}



/**
*
*   Instance Interface implementation functions.
*
**/
void ItemWeaponSMG::InstanceSpawn() {
    // Setup the instance use callback.
    SetUseInstanceCallback(&ItemWeaponSMG::InstanceWeaponSMGUse);
}


/**
*
*   Weapon Instance functionality.
*
**/
/**
*   @brief
**/
//void ItemWeaponSMG::InstanceWeaponSMGIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) { 
//    // Default think callback.
//
//    // Presumably base weapon item WEaponThink-> calls whichever think callback is set.
//
//    // This one should thus show an idle animation.
//
//    // Primary fire does a fire animation, it'll keep setting itself to nextthink until all
//    // frames are done playing.
//}

/**
*   @brief  The mother of all instance weapon callbacks. Calls upon the others depending on state.
**/
void ItemWeaponSMG::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Ensure it is of type ItemWeaponSMG
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

    // Call base InstanceWeaponThink, this will check whether we have newWeapon set and engage a switch.
    Base::InstanceWeaponThink(player, weaponSMG, client);

    // Switch based on weapon state.
    switch (client->weaponState.current) { 
        case WeaponState::Idle:
            weaponSMG->InstanceWeaponIdle(player, weaponSMG, client);
        break;
        case WeaponState::Draw:
            weaponSMG->InstanceWeaponDraw(player, weaponSMG, client);
        break;
        case WeaponState::Holster:
            weaponSMG->InstanceWeaponHolster(player, weaponSMG, client);
        break;
        case WeaponState::Reload:
            //weapon->InstanceWeaponReload(player, weapon, client);
        break;
        case WeaponState::PrimaryFire:
            //weapon->InstanceWeaponPrimaryFire(player, weapon, client);
        break;
        case WeaponState::SecondaryFire:
            //weapon->InstanceWeaponSecondaryFire(player, weapon, client);
        break;
        default:
 //           weapon->InstanceWeaponIdle(player, weapon, client);
        break;
    }

}

/**
*   @brief  Callback used when an instance weapon is switching state.
**/
void ItemWeaponSMG::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon *weapon, ServerClient *client, int32_t newState, int32_t oldState) {
    // Ensure it is of type ItemWeaponSMG
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

    // Revert time to uint32_t.
    uint32_t startTime = level.timeStamp;

    // Set animations here.
    switch (newState) {
    case WeaponState::Draw:
            // Let the player entity play the 'draw SMG' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/smg45/ready2.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/smg45/ready2.wav"), 1.f, ATTN_NORM, 0.f);

            // Call upon the SMG instance weapon's SetAnimation for this client.
            weaponSMG->InstanceWeaponSetAnimation(player, weaponSMG, client, startTime, 110, 142);

            // Disable client from being able to holster, and switch weapons.
            client->weaponState.canHolster = false;
        break;
    case WeaponState::Idle:
            // Play idle animation based on random number generation. Cheap, but effective.
            if (client->weaponState.animationFrame == -1) {
                int32_t animateIdleState = 100 % RandomRangeui(0, 100);

                if (animateIdleState < 10) {
                    weaponSMG->InstanceWeaponSetAnimation(player, weaponSMG, client, startTime, 141, 172);
                }
            }
    
            // Enable holstering to client, so it can switch weapons.
            client->weaponState.canHolster = true;
        break;
    case WeaponState::Holster:
            // Let the player entity play the 'holster SMG' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/hidedefault.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/hidedefault.wav"), 1.f, ATTN_NORM, 0.f);
            // Call upon the SMG instance weapon's SetAnimation for this client.
            weaponSMG->InstanceWeaponSetAnimation(player, weaponSMG, client, startTime, 104, 112);

            // Disable client from being able to holster, and switch weapons.
            client->weaponState.canHolster = false;
        break;
    default:
        break;
    }
}

/**
*   @brief Called when an animation has finished. Usually used to then switch states.
**/
void ItemWeaponSMG::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Ensure it is of type ItemWeaponSMG
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

    // Set animations here.
    switch (client->weaponState.current) {
    case WeaponState::Draw:
        weaponSMG->InstanceWeaponQueueNextState(player, weaponSMG, client, WeaponState::Idle);
        gi.DPrintf("WeaponState::Draw(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
        break;
    case WeaponState::Idle:
        gi.DPrintf("WeaponState::Idle(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
        break;
    case WeaponState::Holster:
        // Switch to down state.
        weaponSMG->InstanceWeaponQueueNextState(player, weaponSMG, client, WeaponState::Down);

        // Enable holstering to client, so it can switch weapons.
        client->weaponState.canHolster = true;

        gi.DPrintf("WeaponState::Holster(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
        break;
    default:
        gi.DPrintf("WeaponState::Default(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
        break;
    }
}

/**
*   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
**/
void ItemWeaponSMG::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    ////// Animation start and end frame.
    //static constexpr uint32_t idleStartFrame = 142;
    //static constexpr uint32_t idleEndFrame = 172;
}

/**
*   @brief  Draw weapon callback.
**/
void ItemWeaponSMG::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    //static constexpr uint32_t drawStartFrame = 110;
    //static constexpr uint32_t drawEndFrame = 142;
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponSMG::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    //static constexpr uint32_t holsterStartFrame = 104;
    //static constexpr uint32_t holsterEndFrame = 112;
}



/**
*
*   Callback Functions.
*
**/
/**
*   @brief  Checks whether to add to inventory or not. In case of adding it 
*           to the inventory it also checks whether to change weapon or not.
**/
qboolean ItemWeaponSMG::WeaponSMGPickup(SVGBaseEntity* other) {
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = Gameworld::ValidateEntity(other, true, true);

    // Sanity check.
    if (!validEntity || !validEntity->IsSubclassOf<SVGBasePlayer>()) {
        gi.DPrintf("Warning: InstanceWeaponSMGUse called without a valid SVGBasePlayer pointer.\n");
        return false;
    }

    // Save to cast now.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(validEntity);

    // Save to fetch client now.
    ServerClient *client = player->GetClient();

    //// Don't proceed is the player already owns this weapon, we want it to stay on-ground
    //// for other players to pick up.
    //if (player->HasItem[GetIdentifier()] >= 1) {
    //    return false;
    //}
    
    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("weapons/pickup1.wav"), 1, ATTN_NORM, 0);

    //// TODO HERE: Check whether game mode allows for picking up this tiem.
    // Check whether the player already had an SMG or not.
    player->GiveWeapon(GetIdentifier(), 1);

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem)) {
    	// TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
        player->GiveAmmo(GetPrimaryAmmoIdentifier(), 54); // Give it 1.5 clips of ammo to go along with.
    }

    // Change weapon. Assuming he has the item.
    if ( player->HasItem(GetIdentifier()) >= 1 && client->persistent.inventory.activeWeaponID != GetIdentifier() ) {
        player->ChangeWeapon(GetIdentifier());
    }

    // Set a respawn think for after 2 seconds.
    if (!game.GetGamemode()->IsClass<DefaultGamemode>()) {
        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
        SetNextThinkTime(level.time + 2);
    }

    return true;
}

/**
*   @brief  If a player has the SMG in its inventory try and change weapon.
**/
void ItemWeaponSMG::InstanceWeaponSMGUse(SVGBaseEntity* user, SVGBaseItem* item) { 
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = Gameworld::ValidateEntity(user, true, true);

    // Sanity check.
    if (!validEntity || !validEntity->IsSubclassOf<SVGBasePlayer>()) {
        gi.DPrintf("Warning: InstanceWeaponSMGUse called without a valid SVGBasePlayer pointer.\n");
        return;
    }

    // Save to cast now.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(validEntity);

    // Make sure it is a valid SMG item.
    if (!item || !item->IsClass<ItemWeaponSMG>()) {
        return;
    }

    // Let player change weapon.
    player->ChangeWeapon(GetIdentifier(), true);
}