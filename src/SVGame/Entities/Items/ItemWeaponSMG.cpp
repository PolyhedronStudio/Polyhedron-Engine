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

#include <SharedGame/SkeletalAnimation.h>

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBaseItemWeapon.h"
#include "../Base/SVGBasePlayer.h"

// Game mode.
#include "../../Gamemodes/DefaultGamemode.h"
// Game world.
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
    SVG_PrecacheSound("weapons/holster_weapon1.wav");
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
*   @brief  The mother of all instance weapon callbacks. Calls upon the others depending on state.
**/
void ItemWeaponSMG::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponThink(player, weapon, client);

    // Ensure all pointers are valid before proceeding.
    if (!player || !client || !weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

}

/**
*   @brief  Callback used when an instance weapon is switching state.
**/
void ItemWeaponSMG::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon *weapon, ServerClient *client, int32_t newState, int32_t oldState) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponOnSwitchState(player, weapon, client, newState, oldState);

    // Ensure it is of type ItemWeaponSMG
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

    // Revert time to uint32_t.
    int64_t startTime = level.timeStamp;

    // Set animations here.
    switch (newState) {
        case WeaponState::Draw:
            // Let the player entity play the 'draw SMG' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/smg45/ready2.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/smg45/ready2.wav"), 1.f, ATTN_NORM, 0.f);

            // Call upon the SMG instance weapon's SetAnimation for this client.
            weaponSMG->InstanceWeaponSetAnimation(player, weaponSMG, client, startTime, 112, 142);
        break;
        case WeaponState::Idle: 
        break;
        case WeaponState::Holster:
            // Let the player entity play the 'holster SMG' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/holster_weapon1.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/holster_weapon1.wav"), 1.f, ATTN_NORM, 0.f);

            // Call upon the SMG instance weapon's SetAnimation for this client.
            weaponSMG->InstanceWeaponSetAnimation(player, weaponSMG, client, startTime, 104, 112);
        break;
        default:
            break;
    }
}

/**
*   @brief Called when an animation has finished. Usually used to then switch states.
**/
void ItemWeaponSMG::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponOnAnimationFinished(player, weapon, client);

    // Ensure it is of type ItemWeaponSMG
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponSMG>()) {
        return;
    }

    // Cast it.
    ItemWeaponSMG *weaponSMG = dynamic_cast<ItemWeaponSMG*>(weapon);

    // Set animations here.
    switch (client->weaponState.current) {
        case WeaponState::Draw:
                // Remove IsHolstered flag.
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsHolstered;

                // Remove state processing flag.
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

                // Queue 'Idle' State.
                weaponSMG->InstanceWeaponQueueNextState(player, weaponSMG, client, WeaponState::Idle);

                // Debug Print.
                gi.DPrintf("SMG State::Draw(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
        case WeaponState::Idle:               
                // Debug print.
                gi.DPrintf("SMG State::Idle(started: %i) current time: %i\n", client->weaponState.timeStamp, level.timeStamp);
            break;
        case WeaponState::Holster:
                // Add IsHolstered flag.
                client->weaponState.flags |= ServerClient::WeaponState::Flags::IsHolstered;
                
                // Remove state processing flag.
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

                // Queue 'None' state.
                weaponSMG->InstanceWeaponQueueNextState(player, weaponSMG, client, WeaponState::None);

                // Debug Print.
                gi.DPrintf("SMG State::Holster(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
        default:
//                gi.DPrintf("SMG State::Default(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
    }
}

/**
*   @brief  Called each frame the weapon is in Draw state.
**/
void ItemWeaponSMG::InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t drawStartFrame = 110;
    //static constexpr uint32_t drawEndFrame = 142;
     
    // Call base class method.
    Base::InstanceWeaponProcessDrawState(player, weapon, client);

    // Process animation.
    InstanceWeaponProcessAnimation(player, weapon, client);
}
    
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void ItemWeaponSMG::InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t holsterStartFrame = 104;
    //static constexpr uint32_t holsterEndFrame = 112;
     
    // Call base class method.
    Base::InstanceWeaponProcessHolsterState(player, weapon, client);

    // Process animation.
    InstanceWeaponProcessAnimation(player, weapon, client);
}
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void ItemWeaponSMG::InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t idleStartFrame = 142;
    //static constexpr uint32_t idleEndFrame = 172;

    // Call base class method.
    Base::InstanceWeaponProcessIdleState(player, weapon, client);

    // Remove state processing flag.
    client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

    // Process animation.
    //InstanceWeaponProcessAnimation(player, weapon, client);
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

    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("weapons/pickup1.wav"), 1, ATTN_NORM, 0);

    // Give the player the SMG weapon if he didn't have one yet.
    if ( !player->HasItem(GetIdentifier()) ) {
        player->GiveWeapon(GetIdentifier(), 1);
    }

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem)) {
    	// TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
        player->GiveAmmo(GetPrimaryAmmoIdentifier(), 54); // Give it 1.5 clips of ammo to go along with.
    }
    
    // Change weapon. Assuming he has the item.
    if ( player->HasItem(GetIdentifier()) >= 1 ) {
        player->ChangeWeapon(GetIdentifier(), true);
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