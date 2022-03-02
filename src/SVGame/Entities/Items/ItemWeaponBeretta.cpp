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

#include <SharedGame/SkeletalAnimation.h>

// World.
#include "../../World/Gameworld.h"
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
    // Switch based on weapon state.
    switch (client->weaponState.currentState) { 
        case WeaponState::Idle:
            gi.DPrintf("Beretta WeaponState: Idle   startTimestamp=%i   levelTime=%i    startFrame=%i  endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponIdle(player, weapon, client);
        break;
        case WeaponState::Draw:
            gi.DPrintf("Beretta WeaponState: Draw   startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponDraw(player, weapon, client);
        break;
        case WeaponState::Holster:
            gi.DPrintf("Beretta WeaponState: Holster    startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponHolster(player, weapon, client);
        break;
        case WeaponState::Reload:
            gi.DPrintf("Beretta WeaponState: Reload     startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            //InstanceWeaponReload(player, weapon, client);
        break;
        case WeaponState::PrimaryFire:
            gi.DPrintf("Beretta WeaponState: PrimaryFire    startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            //InstanceWeaponPrimaryFire(player, weapon, client);
        break;
        case WeaponState::SecondaryFire:
            gi.DPrintf("Beretta WeaponState: SecondaryFire  startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            //InstanceWeaponSecondaryFire(player, weapon, client);
        break;
        default:
            // Do an idle anyway.
 //           InstanceWeaponIdle(player, weapon, client);
        break;
    }
}

/**
*   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
**/
void ItemWeaponBeretta::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr uint32_t idleStartFrame = 187;
    static constexpr uint32_t idleEndFrame = 210;

    client->playerState.gunAnimationFrametime = 20.f;
    client->playerState.gunAnimationStartFrame = idleStartFrame;
    client->playerState.gunAnimationEndFrame = idleEndFrame;
    client->playerState.gunAnimationLoopCount = 0;
    client->playerState.gunAnimationForceLoop = true;

    // The current frame of the state's animation.
    int32_t weaponFrame = 0;

    // Calculate current frame for time since stateTimeStamp was set.
    SG_FrameForTime(&weaponFrame, 
        (uint32_t)(level.time * 1000.f), 
        client->playerState.gunAnimationStartTime, 
        client->playerState.gunAnimationFrametime, 
        client->playerState.gunAnimationStartFrame,
        client->playerState.gunAnimationEndFrame, 
        client->playerState.gunAnimationLoopCount, 
        client->playerState.gunAnimationForceLoop
    );

    // If the animation has ended...
    if ( weaponFrame == -1) {
        InstanceWeaponQueueNextState(player, client, WeaponState::Idle);
    }
}

/**
*   @brief  Draw weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr uint32_t drawStartFrame = 151;
    static constexpr uint32_t drawEndFrame = 186;

    client->playerState.gunAnimationFrametime = 20.f;
    client->playerState.gunAnimationStartFrame = drawStartFrame;
    client->playerState.gunAnimationEndFrame = drawEndFrame;
    client->playerState.gunAnimationLoopCount = 1;
    client->playerState.gunAnimationForceLoop = false;


    // The current frame of the state's animation.
    int32_t weaponFrame = 0;

    // Calculate current frame for time since stateTimeStamp was set.
    SG_FrameForTime(&weaponFrame, 
        (uint32_t)(level.time * 1000.f), 
        client->playerState.gunAnimationStartTime, 
        client->playerState.gunAnimationFrametime, 
        client->playerState.gunAnimationStartFrame,
        client->playerState.gunAnimationEndFrame, 
        client->playerState.gunAnimationLoopCount, 
        client->playerState.gunAnimationForceLoop
    );

    // If the animation has ended...
    if ( weaponFrame == -1) {
        InstanceWeaponQueueNextState(player, client, WeaponState::Idle);
    }
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponBeretta::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr uint32_t holsterStartFrame = 140;
    static constexpr uint32_t holsterEndFrame = 150;

    client->playerState.gunAnimationFrametime = 20.f;
    client->playerState.gunAnimationStartFrame = holsterStartFrame;
    client->playerState.gunAnimationEndFrame = holsterEndFrame;
    client->playerState.gunAnimationLoopCount = 1;
    client->playerState.gunAnimationForceLoop = false;

    // The current frame of the state's animation.
    int32_t weaponFrame = 0;

    // Calculate current frame for time since stateTimeStamp was set.
    SG_FrameForTime(&weaponFrame, 
        (uint32_t)(level.time * 1000.f), 
        client->playerState.gunAnimationStartTime, 
        client->playerState.gunAnimationFrametime, 
        client->playerState.gunAnimationStartFrame,
        client->playerState.gunAnimationEndFrame, 
        client->playerState.gunAnimationLoopCount, 
        client->playerState.gunAnimationForceLoop
    );

    // If the animation has ended...
    if ( weaponFrame == -1) {
        InstanceWeaponQueueNextState(player, client, WeaponState::Down);
    }
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
        gi.DPrintf("Warning: InstanceWeaponSMGUse called without a valid SVGBasePlayer pointer.\n");
        return false;
    }

    // Save to cast now.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(validEntity);

    // Save to fetch client now.
    ServerClient *client = player->GetClient();

    //// TODO HERE: Check whether game mode allows for picking up this tiem.
    // Check whether the player already had an SMG or not.
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
*   @brief Changes the player's weapon to the Beretta if it has one that is.
**/
void ItemWeaponBeretta::InstanceWeaponBerettaUse(SVGBaseEntity* user, SVGBaseItem* item) {
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
    if (!item || !item->IsClass<ItemWeaponBeretta>()) {
        return;
    }

    // Let player change weapon.
    player->ChangeWeapon(GetIdentifier(), true);

    //// Set it as our new to switch to.
    //client->newWeapon = berettaItem;

    //// Set state to holster if active weapon, otherwise to draw.
    //if (client->persistent.inventory.activeWeaponID) {
	   // client->weaponState.queuedState = WeaponState::Holster;
    //} else {
	   // client->weaponState.queuedState = WeaponState::Draw;
    //}

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