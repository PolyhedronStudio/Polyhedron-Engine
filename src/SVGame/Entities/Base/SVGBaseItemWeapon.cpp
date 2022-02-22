/*
// LICENSE HERE.

//
// SVGBaseItemWeapon.cpp
//
// Base class to create item entities from.
//
// Gives the following functionalities:
// TODO: Explain what.
//
*/
#include "../../ServerGameLocal.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseItemWeapon.h"

// Game world.
#include "../../World/Gameworld.h"

//
// Constructor/Deconstructor.
//
SVGBaseItemWeapon::SVGBaseItemWeapon(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}
SVGBaseItemWeapon::~SVGBaseItemWeapon() {

}



//
// Interface functions. 
//
//
//===============
// SVGBaseItemWeapon::Precache
//
//===============
//
void SVGBaseItemWeapon::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache view and world models for the given weapon.
    SVG_PrecacheModel(GetViewModel());
    SVG_PrecacheModel(GetWorldModel());
}

//
//===============
// SVGBaseItemWeapon::Spawn
//
//===============
//
void SVGBaseItemWeapon::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    
    // Set the weapon item world model.
    SetModel(GetWorldModel());

    // Set the config string for this item.
    SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);
}

//
//===============
// SVGBaseItemWeapon::Respawn
//===============
//
void SVGBaseItemWeapon::Respawn() {
    Base::Respawn();
}

//
//===============
// SVGBaseItemWeapon::PostSpawn
//===============
//
void SVGBaseItemWeapon::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// SVGBaseItemWeapon::Think
//===============
//
void SVGBaseItemWeapon::Think() {
    // Always call parent class method.
    Base::Think();
}




//
// Entity functions.
//
void SVGBaseItemWeapon::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Huuueeeee
    if (!player || !client) {
        return;
    }

    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponThink : %f\n", level.time);

    //// Do we need to switch weapons now?
    //SVGBaseItemWeapon* activeWeapon = client->persistent.activeWeapon;
    //SVGBaseItemWeapon* newWeapon = client->newWeapon;

    //// When a weapon's state is down, we can exchange the active weapon with the new weapon.
    //if (newWeapon && (client->weaponState == WeaponState::Finished || client->weaponState == WeaponState::Down)) {
    //    // Current active weapon moves into last weapon slot.
	   // client->persistent.lastWeapon = activeWeapon;

    //    // Active weapon gets set to new weapon.
	   // client->persistent.activeWeapon = newWeapon;

    //    // Reset new weapon to nullptr since we're not needing it anymore.
	   // client->newWeapon = nullptr;

	   // // Set weapon state: Draw.
	   // client->weaponState = WeaponState::Draw;

    //    // Set visible model
    //    if (player->GetModelIndex() == 255) {
    //        int32_t i = 0;
	   //     if (client->persistent.activeWeapon) {
	   //         i = ((client->persistent.activeWeapon->GetViewModelIndex() & 0xff) << 8);
	   //     } else {
		  //      i = 0;
	   //     }
    //
    //        player->SetSkinNumber((player->GetServerEntity() - game.GetGameworld()->GetServerEntities() - 1) | i);
    //    }

    //    // Update the player state: gunIndex & ammoIndex to 0, meaning it won't display.
    //    if (!client->persistent.activeWeapon) {
	   //     // dead
	   //     client->playerState.gunIndex = 0;
	   //     client->ammoIndex = 0;
	   //     return;
    //    }

    //    // Update the client's ammo index.
    //    if (client->persistent.activeWeapon) {
    //	    client->ammoIndex = client->persistent.activeWeapon->GetPrimaryAmmoIdentifier();
    //    } else {
    //        client->ammoIndex = 0;
    //    }


    //    // Update playerstate: gunFrame and gunIndex.
    //    client->playerState.gunIndex = client->persistent.activeWeapon->GetViewModelIndex();
    //}

    //// Fire weapon callback.
    //if (HasWeaponThinkCallback()) { 
    //    (this->*weaponThinkFunction)(player, weapon, client);
    //}
}

//===============
// SVGBaseItemWeapon::SetRespawn
//===============
void SVGBaseItemWeapon::SetRespawn(const float delay) {
    Base::SetRespawn(delay);
}
