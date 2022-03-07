/***
*
*	License here.
*
*	@file
*
*	Beretta weapon implementation.
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

// Beretta.
#include "ItemWeaponBeretta.h"


//! Constructor/Deconstructor.
ItemWeaponBeretta::ItemWeaponBeretta(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) { }
ItemWeaponBeretta::~ItemWeaponBeretta() { }


/**
*
*   Interface Functions.
*
**/
/**
*   @brief 
**/
void ItemWeaponBeretta::Precache() {
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
    SVG_PrecacheSound("weapons/beretta/fire2.wav");
    SVG_PrecacheSound("weapons/beretta/ready1.wav");

    SVG_PrecacheSound("weapons/beretta/reload1.wav");
    SVG_PrecacheSound("weapons/beretta/reload2.wav");
}

/**
*   @brief 
**/
void ItemWeaponBeretta::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set render effects to be glowy.
    SetEffects(GetEffects() | EntityEffectType::Rotate);
    SetRenderEffects(GetRenderEffects() | RenderEffects::Glow);

    // Set the count for the amount of ammo this weapon will give by default.
    SetCount(36);

    // Ensure it can be respawned.
    SetFlags(GetFlags() | EntityFlags::Respawn);

    // Setup our ItemWeaponBeretta callbacks.
    SetPickupCallback(&ItemWeaponBeretta::WeaponBerettaPickup);
    SetUseInstanceCallback(&ItemWeaponBeretta::InstanceWeaponBerettaUse);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

/**
*   @brief 
**/
void ItemWeaponBeretta::Respawn() { 
    Base::Respawn(); 
}

/**
*   @brief 
**/
void ItemWeaponBeretta::PostSpawn() {
   Base::PostSpawn();
}

/**
*   @brief 
**/
void ItemWeaponBeretta::Think() {
    Base::Think();
}



/**
*
*   Instance Interface implementation functions.
*
**/
void ItemWeaponBeretta::InstanceSpawn() {
    // Setup the instance use callback.
    SetUseInstanceCallback(&ItemWeaponBeretta::InstanceWeaponBerettaUse);
}


/**
*
*   Weapon Instance functionality.
*
**/
/**
*   @brief
**/
//void ItemWeaponBeretta::InstanceWeaponBerettaIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) { 
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
void ItemWeaponBeretta::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponThink(player, weapon, client);

    // Ensure it is of type ItemWeaponBeretta
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponBeretta>()) {
        return;
    }

    // Cast it.
    ItemWeaponBeretta *weaponBeretta = dynamic_cast<ItemWeaponBeretta*>(weapon);

    // Call base InstanceWeaponThink, this will check whether we have newWeapon set and engage a switch.
    Base::InstanceWeaponThink(player, weaponBeretta, client);

    // Switch based on weapon state.
    switch (client->weaponState.current) { 
        case WeaponState::Idle:
            weaponBeretta->InstanceWeaponIdle(player, weaponBeretta, client);
        break;
        case WeaponState::Draw:
            weaponBeretta->InstanceWeaponDraw(player, weaponBeretta, client);
        break;
        case WeaponState::Holster:
            weaponBeretta->InstanceWeaponHolster(player, weaponBeretta, client);
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
void ItemWeaponBeretta::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon *weapon, ServerClient *client, int32_t newState, int32_t oldState) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponOnSwitchState(player, weapon, client, newState, oldState);

    // Ensure it is of type ItemWeaponBeretta
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponBeretta>()) {
        return;
    }

    // Cast it.
    ItemWeaponBeretta *weaponBeretta = dynamic_cast<ItemWeaponBeretta*>(weapon);

    // Revert time to uint32_t.
    int64_t startTime = level.timeStamp;

    // Set animations here.
    switch (newState) {
        case WeaponState::Draw:
            // Let the player entity play the 'draw Beretta' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/beretta/ready1.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/beretta/ready1.wav"), 1.f, ATTN_NORM, 0.f);

            // Call upon the Beretta instance weapon's SetAnimation for this client.
            weaponBeretta->InstanceWeaponSetAnimation(player, weaponBeretta, client, startTime, 151, 186);
        break;
        case WeaponState::Idle: 

        break;
        case WeaponState::Holster:
            // Let the player entity play the 'holster Beretta' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/holster_weapon1.wav");
            SVG_Sound(player, CHAN_WEAPON, SVG_PrecacheSound("weapons/holster_weapon1.wav"), 1.f, ATTN_NORM, 0.f);

            // Call upon the Beretta instance weapon's SetAnimation for this client.
            weaponBeretta->InstanceWeaponSetAnimation(player, weaponBeretta, client, startTime, 140, 150);
        break;
        default:
            break;
    }
}

/**
*   @brief Called when an animation has finished. Usually used to then switch states.
**/
void ItemWeaponBeretta::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    using WeaponState = ServerClient::WeaponState;

    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponOnAnimationFinished(player, weapon, client);

    // Ensure it is of type ItemWeaponBeretta
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponBeretta>()) {
        return;
    }

    // Cast it.
    ItemWeaponBeretta *weaponBeretta = dynamic_cast<ItemWeaponBeretta*>(weapon);

    // Set animations here.
    switch (client->weaponState.current) {
        case ::WeaponState::Draw:
                // Remove IsHolstered flag.
                client->weaponState.flags &= ~WeaponState::Flags::IsHolstered;

                // Queue 'Idle' State.
                weaponBeretta->InstanceWeaponQueueNextState(player, weaponBeretta, client, ::WeaponState::Idle);

                // Debug Print.
                gi.DPrintf("Beretta State::Draw(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
        case ::WeaponState::Idle:

                // Debug print.
                gi.DPrintf("Beretta State::Idle(started: %i) current time: %i\n", client->weaponState.timeStamp, level.timeStamp);
            break;
        case ::WeaponState::Holster:
                // Add IsHolstered flag.
                client->weaponState.flags |= WeaponState::Flags::IsHolstered;

                // Queue 'None' state.
                weaponBeretta->InstanceWeaponQueueNextState(player, weaponBeretta, client, ::WeaponState::None);

                // Debug Print.
                gi.DPrintf("Beretta State::Holster(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
        default:
//                gi.DPrintf("Beretta State::Default(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
    }
}

/**
*   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
**/
void ItemWeaponBeretta::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    ////// Animation start and end frame.
    //static constexpr uint32_t idleStartFrame = 187;
    //static constexpr uint32_t idleEndFrame = 210;
}

/**
*   @brief  Draw weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    //static constexpr uint32_t drawStartFrame = 151;
    //static constexpr uint32_t drawEndFrame = 186;
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    //static constexpr uint32_t holsterStartFrame = 140;
    //static constexpr uint32_t holsterEndFrame = 150;
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
qboolean ItemWeaponBeretta::WeaponBerettaPickup(SVGBaseEntity* other) {
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = Gameworld::ValidateEntity(other, true, true);

    // Sanity check.
    if (!validEntity || !validEntity->IsSubclassOf<SVGBasePlayer>()) {
        gi.DPrintf("Warning: InstanceWeaponBerettaUse called without a valid SVGBasePlayer pointer.\n");
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

    //// TODO HERE: Check whether game mode allows for picking up this tiem.
    // Check whether the player already had an Beretta or not.
    player->GiveWeapon(GetIdentifier(), 1);

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem)) {
    	// TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
        player->GiveAmmo(GetPrimaryAmmoIdentifier(), 54); // Give it 1.5 clips of ammo to go along with.
    }

    // Execute a player change weapon in case he isn't holding it already.
    if ( player->HasItem(GetIdentifier()) >= 1 && client->persistent.inventory.activeWeaponID != GetIdentifier() ) {
        player->ChangeWeapon(GetIdentifier());
    }

    // Play sound.
    SVG_Sound(other, CHAN_ITEM, SVG_PrecacheSound("weapons/pickup1.wav"), 1, ATTN_NORM, 0);

    // Set a respawn think for after 2 seconds.
    if (!game.GetGamemode()->IsClass<DefaultGamemode>()) {
        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
        SetNextThinkTime(level.time + 2);
    }

    return true;
}

/**
*   @brief  If a player has the Beretta in its inventory try and change weapon.
**/
void ItemWeaponBeretta::InstanceWeaponBerettaUse(SVGBaseEntity* user, SVGBaseItem* item) { 
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = Gameworld::ValidateEntity(user, true, true);

    // Sanity check.
    if (!validEntity || !validEntity->IsSubclassOf<SVGBasePlayer>()) {
        gi.DPrintf("Warning: InstanceWeaponBerettaUse called without a valid SVGBasePlayer pointer.\n");
        return;
    }

    // Save to cast now.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(validEntity);

    // Make sure it is a valid Beretta item.
    if (!item || !item->IsClass<ItemWeaponBeretta>()) {
        return;
    }

    // Let player change weapon.
    player->ChangeWeapon(GetIdentifier(), true);
}