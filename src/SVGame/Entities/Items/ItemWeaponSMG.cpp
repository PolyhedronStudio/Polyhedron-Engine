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

    // Switch based on weapon state.
    switch (client->weaponState.currentState) { 
        case WeaponState::Idle:
            gi.DPrintf("SMG WeaponState: Idle   startTimestamp=%i   levelTime=%i    startFrame=%i  endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponIdle(player, weapon, client);
        break;
        case WeaponState::Draw:
            gi.DPrintf("SMG WeaponState: Draw   startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponDraw(player, weapon, client);
        break;
        case WeaponState::Holster:
            gi.DPrintf("SMG WeaponState: Holster    startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            InstanceWeaponHolster(player, weapon, client);
        break;
        case WeaponState::Reload:
            gi.DPrintf("SMG WeaponState: Reload     startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            //InstanceWeaponReload(player, weapon, client);
        break;
        case WeaponState::PrimaryFire:
            gi.DPrintf("SMG WeaponState: PrimaryFire    startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
            //InstanceWeaponPrimaryFire(player, weapon, client);
        break;
        case WeaponState::SecondaryFire:
            gi.DPrintf("SMG WeaponState: SecondaryFire  startTimestamp=%i   levelTime=%i    startFrame=%i   endFrame=%i\n", client->playerState.gunAnimationStartTime,(int32_t)(level.time * 1000.0f), client->playerState.gunAnimationStartFrame, client->playerState.gunAnimationEndFrame);
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
void ItemWeaponSMG::InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Animation start and end frame.
    static constexpr uint32_t idleStartFrame = 141;
    static constexpr uint32_t idleEndFrame = 171;

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
void ItemWeaponSMG::InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr uint32_t drawStartFrame = 111;
    static constexpr uint32_t drawEndFrame = 141;

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
    if (weaponFrame == -1) {
        InstanceWeaponQueueNextState(player, client, WeaponState::Idle);
    }
}

/**
*   @brief  Holster weapon callback.
**/
void ItemWeaponSMG::InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Animation start and end frame.
    static constexpr uint32_t holsterStartFrame = 104;
    static constexpr uint32_t holsterEndFrame = 112;

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
    if (weaponFrame == -1) {
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