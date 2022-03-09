/***
*
*	License here.
*
*	@file
*
*	None weapon implementation.
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

// None.
#include "ItemWeaponNone.h"


//! Constructor/Deconstructor.
ItemWeaponNone::ItemWeaponNone(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) { }
ItemWeaponNone::~ItemWeaponNone() { }


/**
*
*   Interface Functions.
*
**/
/**
*   @brief 
**/
void ItemWeaponNone::Precache() {
    // Always call parent class method.
    Base::Precache();
}

/**
*   @brief 
**/
void ItemWeaponNone::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Not visible to clients at all.
    //SetServerFlags(EntityServerFlags::NoClient);

    // Set Instance callback.
    //SetUseInstanceCallback(&ItemWeaponNone::InstanceWeaponNoneUse);

    // Link Entity.
    LinkEntity();
}

/**
*   @brief 
**/
void ItemWeaponNone::Respawn() { 
    Base::Respawn(); 
}

/**
*   @brief 
**/
void ItemWeaponNone::PostSpawn() {
   Base::PostSpawn();
}

/**
*   @brief 
**/
void ItemWeaponNone::Think() {
    Base::Think();
}



/**
*
*   Instance Interface implementation functions.
*
**/
void ItemWeaponNone::InstanceSpawn() {
    // Setup the instance use callback.
    SetUseInstanceCallback(&ItemWeaponNone::InstanceWeaponNoneUse);
}


/**
*
*   Weapon Instance functionality.
*
**/
/**
*   @brief
**/
//void ItemWeaponNone::InstanceWeaponNoneIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) { 
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
void ItemWeaponNone::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Ensure it is of type ItemWeaponNone
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponNone>()) {
        return;
    }

    // Cast it.
    ItemWeaponNone *weaponNone = dynamic_cast<ItemWeaponNone*>(weapon);

    // Call base InstanceWeaponThink, this will check whether we have newWeapon set and engage a switch.
    Base::InstanceWeaponThink(player, weaponNone, client);
}

/**
*   @brief  Callback used when an instance weapon is switching state.
**/
void ItemWeaponNone::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon *weapon, ServerClient *client, int32_t newState, int32_t oldState) {
    // Ensure it is of type ItemWeaponNone
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponNone>()) {
        return;
    }

}

/**
*   @brief Called when an animation has finished. Usually used to then switch states.
**/
void ItemWeaponNone::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    using WeaponState = ServerClient::WeaponState;

    // Ensure it is of type ItemWeaponNone
    if (!weapon || !weapon->IsSubclassOf<ItemWeaponNone>()) {
        return;
    }
        
    // Cast it.
    ItemWeaponNone *weaponNone = dynamic_cast<ItemWeaponNone*>(weapon);

    //// Set animations here.
    //switch (client->weaponState.current) {
    //    default:
    //            gi.DPrintf("WeaponState::Default(started: %i) finished animating at time: %i\n", client->playerState.gunAnimationStartTime, level.timeStamp);
    //        break;
    //}
}


/**
*
*   Callback Functions.
*
**/
/**
*   @brief  If a player has the None in its inventory try and change weapon.
**/
void ItemWeaponNone::InstanceWeaponNoneUse(SVGBaseEntity* user, SVGBaseItem* item) { 
    // Acquire player entity pointer.
    SVGBaseEntity *validEntity = Gameworld::ValidateEntity(user, true, true);

    // Sanity check.
    if (!validEntity || !validEntity->IsSubclassOf<SVGBasePlayer>()) {
        gi.DPrintf("Warning: InstanceWeaponNoneUse called without a valid SVGBasePlayer pointer.\n");
        return;
    }

    // Save to cast now.
    SVGBasePlayer *player = dynamic_cast<SVGBasePlayer*>(validEntity);

    // Make sure it is a valid None item.
    if (!item || !item->IsClass<ItemWeaponNone>()) {
        return;
    }

    // Let player change weapon.
    player->ChangeWeapon(GetIdentifier(), true);
}