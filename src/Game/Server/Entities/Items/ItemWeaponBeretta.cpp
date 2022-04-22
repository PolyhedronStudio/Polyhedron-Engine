/***
*
*	License here.
*
*	@file
*
*	Beretta weapon implementation.
*
***/
#include "../../ServerGameLocals.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Deathmatch Game Mode.
#include "../../GameModes/DeathMatchGameMode.h"

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBaseItemWeapon.h"
#include "../Base/SVGBasePlayer.h"

// World.
#include "../../World/ServerGameWorld.h"

// Beretta.
#include "ItemWeaponBeretta.h"


//! Constructor/Deconstructor.
ItemWeaponBeretta::ItemWeaponBeretta(PODEntity *svEntity, const std::string& displayString, uint32_t identifier) 
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
    // NOTE: There are none to precache right here. SVGBaseItem does so by using
    // GetViewModel and GetWorldModel to acquire the path for precaching.

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
*   @brief  The mother of all instance weapon callbacks. Calls upon the others depending on state.
**/
void ItemWeaponBeretta::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponThink(player, weapon, client);

    // Ensure all pointers are valid before proceeding.
    if (!player || !client || !weapon || !weapon->IsSubclassOf<ItemWeaponBeretta>()) {
        return;
    }

    // Cast it.
    ItemWeaponBeretta *weaponBeretta = dynamic_cast<ItemWeaponBeretta*>(weapon);
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
    GameTime startTime = level.time;

    // Set animations here.
    switch (newState) {
        case WeaponState::Draw:
            // Let the player entity play the 'draw Beretta' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/beretta/ready1.wav");
            SVG_Sound(player, SoundChannel::Weapon, SVG_PrecacheSound("weapons/beretta/ready1.wav"), 1.f, Attenuation::Normal, 0.f);

            // Call upon the Beretta instance weapon's SetAnimation for this client.
            weaponBeretta->InstanceWeaponSetAnimation(player, weaponBeretta, client, startTime, 151, 186);
        break;
        case WeaponState::Idle: 
            // DEBUG: Remove state processing flag, see if it alleviates our complaints.
            client->weaponState.flags &= ServerClient::WeaponState::Flags::IsProcessingState;
        break;
        case WeaponState::Holster:
            // Let the player entity play the 'holster Beretta' sound.
            client->weaponSound = SVG_PrecacheSound("weapons/holster_weapon1.wav");
            SVG_Sound(player, SoundChannel::Weapon, SVG_PrecacheSound("weapons/holster_weapon1.wav"), 1.f, Attenuation::Normal, 0.f);

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
    // Execute base class function regardless of sanity checks in this override.
    Base::InstanceWeaponOnAnimationFinished(player, weapon, client);

    // Ensure it is of type ItemWeaponSMG
    if (!client || !weapon || !weapon->IsSubclassOf<ItemWeaponBeretta>()) {
        return;
    }

    // Cast it.
    ItemWeaponBeretta *weaponBeretta = dynamic_cast<ItemWeaponBeretta*>(weapon);

    // Set animations here.
    switch (client->weaponState.current) {
        case WeaponState::Draw:
                // Remove IsHolstered flag.
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsHolstered;

                // Remove state processing flag because we'll queue idle state next. .
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

                // Queue 'Idle' State.
                weaponBeretta->InstanceWeaponQueueNextState(player, weaponBeretta, client, WeaponState::Idle);

                // Debug Print.
                gi.DPrintf("Beretta State::Draw(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.time);
            break;
        case WeaponState::Idle:                
                // Debug print.
                gi.DPrintf("Beretta State::Idle(started: %i) current time: %i\n", client->weaponState.timeStamp, level.time);
            break;
        case WeaponState::Holster:
                // Add IsHolstered flag.
                client->weaponState.flags |= ServerClient::WeaponState::Flags::IsHolstered;
                
                // Remove state processing flag.
                client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

                // Queue 'None' state.
                weaponBeretta->InstanceWeaponQueueNextState(player, weaponBeretta, client, WeaponState::None);

                // Debug Print.
                gi.DPrintf("Beretta State::Holster(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.time);
            break;
        default:
//                gi.DPrintf("SMG State::Default(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
            break;
    }
}

/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void ItemWeaponBeretta::InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t drawStartFrame = 151;
    //static constexpr uint32_t drawEndFrame = 186;

    // Call base class method.
    Base::InstanceWeaponProcessIdleState(player, weapon, client);

    // Process animation.
    InstanceWeaponProcessAnimation(player, weapon, client);
}

/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void ItemWeaponBeretta::InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t holsterStartFrame = 140;
    //static constexpr uint32_t holsterEndFrame = 150;

    // Call base class method.
    Base::InstanceWeaponProcessIdleState(player, weapon, client);

    // Process animation.
    InstanceWeaponProcessAnimation(player, weapon, client);
}

/**
*   @brief  Called each frame the weapon is in Draw state.
**/
void ItemWeaponBeretta::InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    //static constexpr uint32_t idleStartFrame = 187;
    //static constexpr uint32_t idleEndFrame = 210;

    // Call base class method.
    Base::InstanceWeaponProcessIdleState(player, weapon, client);

    // Remove state processing flag.
    client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;

    // Process animation.
    InstanceWeaponProcessAnimation(player, weapon, client);
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
qboolean ItemWeaponBeretta::WeaponBerettaPickup(IServerGameEntity *other) {
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = ServerGameWorld::ValidateEntity(other, true, true);

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
    SVG_Sound(other, SoundChannel::Item, SVG_PrecacheSound("weapons/pickup1.wav"), 1, Attenuation::Normal, 0);

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
    if (!GetGameMode()->IsClass<DefaultGameMode>()) {
        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
        SetNextThinkTime(level.time + 2s);
    }

    return true;
}

/**
*   @brief  If a player has the Beretta in its inventory try and change weapon.
**/
void ItemWeaponBeretta::InstanceWeaponBerettaUse(SVGBaseEntity* user, SVGBaseItem* item) { 
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = ServerGameWorld::ValidateEntity(user, true, true);

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