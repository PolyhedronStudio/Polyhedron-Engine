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

// Misc Explosion Box Entity.
#include "ItemWeaponSMG.h"


//! Constructor/Deconstructor.
ItemWeaponSMG::ItemWeaponSMG(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) { 
}
ItemWeaponSMG::~ItemWeaponSMG() { 
}


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
    SVG_PrecacheSound("weapons/smg/fire1.wav");
    SVG_PrecacheSound("weapons/smg/fire2.wav");
    SVG_PrecacheSound("weapons/smg/ready1.wav");
    SVG_PrecacheSound("weapons/smg/ready2.wav");

    SVG_PrecacheSound("weapons/smg/reload1.wav");
    SVG_PrecacheSound("weapons/smg/reload2.wav");

    SVG_PrecacheSound("weapons/smg/reloadclip1.wav");
    SVG_PrecacheSound("weapons/smg/reloadclip2.wav");
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
    //// Call base InstanceWeaponThink, this will check whether we have newWeapon set and engage a switch.
    //Base::InstanceWeaponThink(player, weapon, client);

    //// Switch based on weapon state.
    //switch (client->weaponState.currentState) { 
    //    case WeaponState::Idle:
    //        InstanceWeaponIdle(player, weapon, client);
    //    break;
    //    case WeaponState::Draw:
	   //     InstanceWeaponDraw(player, weapon, client);
    //    break;
    //    case WeaponState::Holster:
	   //     InstanceWeaponHolster(player, weapon, client);
    //    break;
    //    case WeaponState::Reload:
	   //     //InstanceWeaponReload(player, weapon, client);
    //    break;
    //    case WeaponState::PrimaryFire:
	   //     //InstanceWeaponPrimaryFire(player, weapon, client);
    //    break;
    //    case WeaponState::SecondaryFire:
	   //     //InstanceWeaponSecondaryFire(player, weapon, client);
    //    break;
    //    default:
    //        // Do an idle anyway.
    //	    //InstanceWeaponIdle(player, weapon, client);
    //    break;
    //}
}

/**
*   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
**/
void ItemWeaponSMG::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    static constexpr float idleStartFrame = 141.f;
    static constexpr float idleEndFrame = 171.f;

    //// Get current gunFrame and Clamp it between start and end just to be sure.
    //float currentFrame = Clampf(client->playerState.gunAnimationFrame, idleStartFrame, idleEndFrame);

    //// Calculate the next frame to head on forward to.
    //float nextFrame = currentFrame + 1.f; //(idleEndFrame - idleStartFrame) * ANIM_1_FRAMETIME;

    //// Move on to the next frame until we've ended.
    //if (nextFrame < idleEndFrame) {
    //	client->playerState.gunAnimationFrame = nextFrame;
    //} else {
    //	client->playerState.gunAnimationFrame = idleStartFrame;
    //}
    //gi.DPrintf("ItemWeaponSMG::InstanceWeaponIdle : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
}

/**
*   @brief  Draw weapon callback.
**/
void ItemWeaponSMG::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr float drawStartFrame = 112.f;
    static constexpr float drawEndFrame = 141.f;

 //   // Get current gunFrame and Clamp it between start and end just to be sure.
 //   float currentFrame = Clampf(client->playerState.gunAnimationFrame, drawStartFrame, drawEndFrame);

 //   // Calculate the next frame to head on forward to.
 //   float nextFrame = currentFrame + 30.f * (1.0f / (level.time - drawStart));  //+ (drawEndFrame - drawStartFrame) * ANIM_FRAMETIME_1000;

 //   if (newDraw == true) {
 //       drawStart = level.time;
 //       newDraw = false;
 //   }

 //   // Move on to the next frame until we've ended.
 //   if (nextFrame < drawEndFrame) {
 //       client->playerState.gunAnimationFrame = nextFrame;
 //   } else {
 //       totalTimeTaken = level.time - drawStart;
 //       newDraw = true;
	//gi.DPrintf("totalTimeTaken = %f\n", totalTimeTaken);

	//    client->weaponState.currentState = WeaponState::Finished;
 //       client->weaponState.queuedState = WeaponState::Idle;
 //   }

   // gi.DPrintf("ItemWeaponSMG::InstanceWeaponDraw : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponSMG::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr float holsterStartFrame = 104.f;
    static constexpr float holsterEndFrame = 112.f;

    //// Get current gunFrame and Clamp it between start and end just to be sure.
    //float currentFrame = Clampf(client->playerState.gunAnimationFrame, holsterStartFrame, holsterEndFrame);

    //// Calculate the next frame to head on forward to.
    //float nextFrame = currentFrame + 1.f; //    (holsterEndFrame - holsterStartFrame) * ANIM_1_FRAMETIME;

    //// Move on to the next frame until we've ended.
    //if (nextFrame < holsterEndFrame) {
    //    client->playerState.gunAnimationFrame = nextFrame;
    //} else {
	   // //client->weaponState.currentState = WeaponState::Down;
	   // client->weaponState.queuedState = WeaponState::Down;
    //}

   // gi.DPrintf("ItemWeaponSMG::InstanceWeaponHolster : %f - currentState: %i - queuedState: %i\n", level.time, client->weaponState.currentState, client->weaponState.queuedState);
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
    // Sanity check.
    if (!other || !other->IsSubclassOf<SVGBasePlayer>()) {
	    return false;
    }

    // Cast player, get client, sanity check, and let's go.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(other);
    ServerClient *client = player->GetClient();
    if (!client) {
        return false;
    }

    // Do nothing if player already has this weapon.
    if (client->persistent.inventory.items[GetIdentifier()] >= 1) {
        return false;
    }

    //// TODO HERE: Check whether game mode allows for picking up this tiem.
    // Check whether the player already had an SMG or not.
    player->GiveWeapon(GetIdentifier(), 1);

    // If this item wasn't dropped by an other player, give them some ammo to go along.
    if (!(GetSpawnFlags() & ItemSpawnFlags::DroppedItem)) {
    	// TODO HERE: Check spawnflag for dropped or not, and possibly set a respawn action.
        player->GiveAmmo(GetPrimaryAmmoIdentifier(), 54); // Give it 1.5 clips of ammo to go along with.
    }

    // Execute a player change weapon in case he isn't holding it already.
    if (client->persistent.inventory.activeWeaponID != GetIdentifier() && client->persistent.inventory.items[GetIdentifier()] >= 1) {
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

    // Check if the caller is a player.
    /*if (!user || !user->GetClient() || !user->IsSubclassOf<SVGBasePlayer>()) {
        return;
    }*/

    //// Check if the activator is a valid weapon.
    //if (!item->IsClass<ItemWeaponSMG>()) {
    //    return;
    //}

    //// Cast to player.
    //SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(user);
    //// Cast to weapon.
    //ItemWeaponSMG* smgItem = dynamic_cast<ItemWeaponSMG*>(item);
    //// Get its identifier.
    //uint32_t smgItemID = smgItem->GetIdentifier();
    //// Get client.
    //ServerClient* client = player->GetClient();

    //// Set the client's new weapon before calling upon change weapon.
    //client->newWeapon = smgItem;

    //// Set state to holster if active weapon, otherwise to draw.
    //if (client->persistent.inventory.activeWeaponID) {
	   // client->weaponState.queuedState = WeaponState::Holster;
    //} else {
	   // client->weaponState.queuedState = WeaponState::Draw;
    //}

    ////SVG_ChangeWeapon(player);

    //// Is the client already having an active weapon? Queue up a holster state.
    ////if (client->persistent.activeWeapon) {
    ////    client->weaponState.queuedState = WeaponState::Holster;
    ////// Otherwise, queue up a draw weapon state.
    ////} else {
    ////    client->weaponState.queuedState = WeaponState::Draw;
    ////}
}