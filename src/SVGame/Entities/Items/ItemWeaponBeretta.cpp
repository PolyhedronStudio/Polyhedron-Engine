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

// Misc Explosion Box Entity.
#include "ItemWeaponBeretta.h"


//! Constructor/Deconstructor.
ItemWeaponBeretta::ItemWeaponBeretta(Entity* svEntity, const std::string& displayString, uint32_t identifier) : Base(svEntity, displayString, identifier) { }
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
    SVG_PrecacheSound("weapons/hidedefault.wav");
    SVG_PrecacheSound("weapons/pickup1.wav");
    SVG_PrecacheSound("weapons/readygeneric.wav");
    // TODO: The above precache sound section of this code must move to player sound precache code.

    // Precache sounds.
    SVG_PrecacheSound("weapons/Beretta/fire1.wav");
    SVG_PrecacheSound("weapons/Beretta/fire2.wav");
    SVG_PrecacheSound("weapons/Beretta/ready1.wav");
    SVG_PrecacheSound("weapons/Beretta/ready2.wav");

    SVG_PrecacheSound("weapons/Beretta/reload1.wav");
    SVG_PrecacheSound("weapons/Beretta/reload2.wav");

    SVG_PrecacheSound("weapons/Beretta/reloadclip1.wav");
    SVG_PrecacheSound("weapons/Beretta/reloadclip2.wav");
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
void ItemWeaponBeretta::Respawn() { Base::Respawn(); }

/**
*   @brief 
**/
void ItemWeaponBeretta::PostSpawn() { Base::PostSpawn(); }

/**
*   @brief 
**/
void ItemWeaponBeretta::Think() { Base::Think(); }


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
    // Call base InstanceWeaponThink, this will check whether we have newWeapon set and engage a switch.
    Base::InstanceWeaponThink(player, weapon, client);

   // gi.DPrintf("ItemWeaponBeretta::InstanceWeaponThink : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);


    switch (client->weaponState.currentState) {
	case WeaponState::Idle:
	    InstanceWeaponIdle(player, weapon, client);
	    break;
	case WeaponState::Draw:
	    InstanceWeaponDraw(player, weapon, client);
	    break;
	case WeaponState::Holster:
	    InstanceWeaponHolster(player, weapon, client);
	    break;
	case WeaponState::Reload:
	    //InstanceWeaponReload(player, weapon, client);
	    break;
	case WeaponState::PrimaryFire:
	    //InstanceWeaponPrimaryFire(player, weapon, client);
	    break;
	case WeaponState::SecondaryFire:
	    //InstanceWeaponSecondaryFire(player, weapon, client);
	    break;
	default:
	    // Do an idle anyway.
	    //InstanceWeaponIdle(player, weapon, client);
	    break;
    }
}

/**
*   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
**/
void ItemWeaponBeretta::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    static constexpr float idleStartFrame = 187.f;
    static constexpr float idleEndFrame = 210.f;

    //// Get current gunAnimationFrame and Clamp it between start and end just to be sure.
    //float currentFrame = Clampf(client->playerState.gunAnimationFrame, idleStartFrame, idleEndFrame);

    //// Calculate the next frame to head on forward to.
    //float nextFrame = currentFrame + BASE_1_FRAMETIME;

    //// Move on to the next frame until we've ended.
    //if (nextFrame < idleEndFrame) {
    //	client->playerState.gunAnimationFrame = nextFrame;
    //} else {
    //	client->playerState.gunAnimationFrame = idleStartFrame;
    //}
    gi.DPrintf("ItemWeaponBeretta::InstanceWeaponIdle : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
}

/**
*   @brief  Draw weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr float drawStartFrame = 149.f;
    static constexpr float drawEndFrame = 181.f;

    //// Get current gunAnimationFrame and Clamp it between start and end just to be sure.
    //float currentFrame = Clampf(client->playerState.gunAnimationFrame, drawStartFrame, drawEndFrame);

    //// Calculate the next frame to head on forward to.
    //float nextFrame = currentFrame + BASE_1_FRAMETIME;

    //// Move on to the next frame until we've ended.
    //if (nextFrame < drawEndFrame) {
    //    client->playerState.gunAnimationFrame = nextFrame;
    //} else {
	   // client->weaponState.currentState = WeaponState::Finished;
	   // client->weaponState.queuedState = WeaponState::Idle;
    //}

    gi.DPrintf("ItemWeaponBeretta::InstanceWeaponDraw : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr float holsterStartFrame = 140.f;
    static constexpr float holsterEndFrame = 150.f;

    //// Get current gunAnimationFrame and Clamp it between start and end just to be sure.
    //float currentFrame = Clampf(client->playerState.gunAnimationFrame, holsterStartFrame, holsterEndFrame);

    //// Calculate the next frame to head on forward to.
    //float nextFrame = currentFrame + BASE_1_FRAMETIME;

    //// Move on to the next frame until we've ended.
    //if (nextFrame < holsterEndFrame) {
    //    client->playerState.gunAnimationFrame = nextFrame;
    //} else {
    //    client->weaponState.queuedState = WeaponState::Down;
    //}

    gi.DPrintf("ItemWeaponBeretta::InstanceWeaponHolster : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
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
    // Sanity check.
    if (!other || !other->GetClient() || !other->IsSubclassOf<SVGBasePlayer>()) {
	    return false;
    }

    // Cast to SVGBasePlayer.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(other);
    // Acquire client.
    ServerClient* client = player->GetClient();
    // Acquire the player's active weapon instance. (If any.)
    SVGBaseItemWeapon* activeWeapon = player->GetActiveWeapon();

    // TODO HERE: Check whether game mode allows for picking up this tiem.
    // Check whether the player already had an Beretta or not.
    player->GiveWeapon(ItemIdentifier::Beretta, 1);

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem)) {
	    // TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
	    player->GiveAmmo(GetPrimaryAmmoIdentifier(), 54);  // Give it 1.5 clips of ammo to go along with.
    }

    // Do an auto change weapon pick up in this case.
    if (!activeWeapon || (activeWeapon->GetIdentifier() != ItemIdentifier::Beretta && client->persistent.inventory[ItemIdentifier::Beretta] >= 1)) {
	    // Fetch weapon instance so we can call upon UseInstance
	    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(GetIdentifier());

        // Take care of switching player weapon here.
        weaponInstance->UseInstance(player, weaponInstance);
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
*   @brief Changes the player's weapon to the Beretta if it has one that is.
**/
void ItemWeaponBeretta::InstanceWeaponBerettaUse(SVGBaseEntity* user, SVGBaseItem* item) {
    // Check if the caller is a player.
    if (!user || !user->GetClient() || !user->IsSubclassOf<SVGBasePlayer>()) {
	    return;
    }

    // Check if the activator is a valid weapon.
    if (!item->IsClass<ItemWeaponBeretta>()) {
	    return;
    }

    // Cast to player.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(user);
    // Cast to weapon.
    ItemWeaponBeretta* berettaItem = dynamic_cast<ItemWeaponBeretta*>(item);
    // Get client.
    ServerClient* client = player->GetClient();

    // Set it as our new to switch to.
    client->newWeapon = berettaItem;

    // Set state to holster if active weapon, otherwise to draw.
    if (client->persistent.activeWeapon) {
	    client->weaponState.queuedState = WeaponState::Holster;
    } else {
	    client->weaponState.queuedState = WeaponState::Draw;
    }

    // Is the client already having an active weapon? Queue up a holster state.
 //   if (!client->persistent.activeWeapon) {
	//    client->weaponState.queuedState = WeaponState::Holster;
	//// Otherwise, queue up a draw weapon state.
 //   } else {
	//    client->weaponState.queuedState = WeaponState::Draw;
 //   }

    // Change player's weapon.
    //SVG_ChangeWeapon(player);
}