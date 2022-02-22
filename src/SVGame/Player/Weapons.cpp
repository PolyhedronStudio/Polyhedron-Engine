/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// Core.
#include "../ServerGameLocal.h"

// Entities.
#include "../Entities.h"
#include "../entities/base/SVGBaseTrigger.h"
#include "../entities/base/SVGBasePlayer.h"
#include "../Entities/Base/SVGBaseItem.h"
#include "../Entities/Base/SVGBaseItemWeapon.h"

// SharedGame.
#include "SharedGame/SharedGame.h"

// Player Animations Header.
#include "Animations.h"

// Gamemodes.
#include "../Gamemodes/IGameMode.h"
#include "../Gamemodes/DeathMatchGamemode.h"

// World.
#include "../World/Gameworld.h"

qboolean is_quad;
byte     is_silenced;

/*
===============
SVG_PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void SVG_PlayerNoise(SVGBaseEntity *who, vec3_t where, int type)
{
    Entity     *noise;

    //if (deathmatch->value)
    //    return;
    if (game.GetGamemode()->IsClass<DeathmatchGamemode>()) {
        return;
    }

    if (who->GetFlags() & EntityFlags::NoTarget)
        return;


//    if (!who->GetServerEntity()->myNoisePtr) {
//        noise = SVG_Spawn();
////        noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetServerEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetServerEntity()->myNoisePtr = noise;
//
//        noise = SVG_Spawn();
//     //   noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetServerEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetServerEntity()->myNoise2Ptr = noise;
//    }
//
//    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
//        noise = who->GetServerEntity()->myNoisePtr;
//        level.soundEntity = noise;
//        level.soundEntityFrameNumber = level.frameNumber;
//    } else { // type == PNOISE_IMPACT
//        noise = who->GetServerEntity()->myNoise2Ptr;
//        level.sound2Entity = noise;
//        level.sound2EntityFrameNumber = level.frameNumber;
//    }
//
//    VectorCopy(where, noise->state.origin);
//    VectorSubtract(where, noise->maxs, noise->absMin);
//    VectorAdd(where, noise->maxs, noise->absMax);
//    noise->teleportTime = level.time;
//    gi.LinkEntity(noise);
}

/*
=================
SVG_ThinkWeapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void SVG_ThinkWeapon(SVGBasePlayer *player)
{
    if (!player) {
	    return;
    }

    ServerClient* client = player->GetClient();

    if (!client) { 
        return;
    }

    //gi.DPrintf("SVG_ThinkWeapon:%f\n", level.time);

    // if just died, put the weapon away
    if (player->GetHealth() < 1) {
        client->newWeapon = nullptr;
        SVG_ChangeWeapon(player);    
        return;
    }

    // If we're finished holstering, and thus our state is Down, we can switch weapon.
    int32_t currentState = client->weaponState.currentState;
    if (client->newWeapon && (currentState == WeaponState::Down || currentState == WeaponState::Finished)) {
	    // Change weapon, yeeeehaa.
	    SVG_ChangeWeapon(player);
    }

    // See if we got a queued state, if we do, override our current weaponstate.
    if (client->weaponState.queuedState != -1){
	    // TODO: Add a proper state machine for these animations here.
        // Reset gun frame so animations work properly.
        client->playerState.gunFrame = 0;

        // Switch to queued weaponstate.
	    client->weaponState.currentState = client->weaponState.queuedState;
	    // Reset it.
	    client->weaponState.queuedState = -1;
    }


    // call active weapon Think routine
    if (client->persistent.activeWeapon) { // && client->weaponState.shouldThink) {
        int x = 10;
        int x2 = 11;
        int x3 = 12;
	gi.DPrintf("x=%i x2=%i x3=%i\n", x, x2, x3);
	gi.DPrintf("!x=%i !x2=%i !x3=%i\n", !x, !x2, !x3);
	    client->persistent.activeWeapon->InstanceWeaponThink(player, client->persistent.activeWeapon, client);
    }
}

/*
===============
SVG_ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void SVG_ChangeWeapon(SVGBasePlayer *player) {
    // Sanity check.
    if (!player) {
	    return;
    }

    // Get client, sanity check.
    ServerClient* client = player->GetClient();
    if (!client) {
        return;
    }

    gi.DPrintf("SVG_ChangeWeapon:%f\n", level.time);

    // Do the switch.
    client->persistent.lastWeapon = client->persistent.activeWeapon;
    client->persistent.activeWeapon = client->newWeapon;
    client->newWeapon = nullptr;
    
    // Set visible model
    if (player->GetModelIndex() == 255) {
        int32_t i = 0;
        if (client->persistent.activeWeapon) {
    	    i = ((client->persistent.activeWeapon->GetViewModelIndex() & 0xff) << 8);
        } else {
    	    i = 0;
        }
    
        player->SetSkinNumber((player->GetServerEntity() - GetGameworld()->GetServerEntities() - 1) | i);
    }
    
    // Update the player state: gunIndex & ammoIndex to 0, meaning it won't display.
    if (!client->persistent.activeWeapon) {
        // dead
        client->playerState.gunIndex = 0;
        client->ammoIndex = 0;
        client->weaponState.currentState = WeaponState::Down;
        client->weaponState.queuedState = -1;
        return;
    }
    
    // Update the client's ammo index.
    if (client->persistent.activeWeapon) {
        client->ammoIndex = client->persistent.activeWeapon->GetPrimaryAmmoIdentifier();
    } else {
        client->ammoIndex = 0;
    }
    
    
    // Update playerstate: gunFrame and gunIndex.
    if (client->persistent.activeWeapon) { 
        client->playerState.gunIndex = client->persistent.activeWeapon->GetViewModelIndex();

        // Queue draw weapon state.
        client->weaponState.queuedState = WeaponState::Draw;
    }

    // Set player animation. No clue why the OG code used Pain, but we'll have to live with it for now.
    client->animation.priorityAnimation = PlayerAnimation::Pain;
    if (client->playerState.pmove.flags & PMF_DUCKED) {
        player->SetFrame(FRAME_crpain1);
        client->animation.endFrame = FRAME_crpain4;
    } else {
        player->SetFrame(FRAME_pain301);
        client->animation.endFrame = FRAME_pain304;
    }
}
//void SVG_ChangeWeapon(SVGBasePlayer *player) {
//    // Sanity check.
//    if (!player) {
//	    return;
//    }
//
//    // Get client, sanity check.
//    ServerClient* client = player->GetClient();
//    if (!client) {
//        return;
//    }
//
//    // Get pointer to active weapon.
//    SVGBaseItemWeapon *activeWeapon = client->persistent.activeWeapon;
//    // Get pointer to new weapon, could be set due to a key press or an item pick up.
//    SVGBaseItemWeapon *newWeapon = client->newWeapon;
//
//    // When a weapon's state is down, we can exchange the active weapon with the new weapon.
//    if (client->weaponState == WeaponState::Down || !activeWeapon) {
//        // Current active weapon moves into last weapon slot.
//	    client->persistent.lastWeapon = activeWeapon;
//
//        // Active weapon gets set to new weapon.
//	    client->persistent.activeWeapon = newWeapon;
//
//        // Reset new weapon to nullptr since we're not needing it anymore.
//	    client->newWeapon = nullptr;
//
//	    // Set weapon state: Draw.
//	    client->weaponState = WeaponState::Draw;
//
//        // Set visible model
//        if (player->GetModelIndex() == 255) {
//            int32_t i = 0;
//	        if (client->persistent.activeWeapon) {
//	            i = ((client->persistent.activeWeapon->GetViewModelIndex() & 0xff) << 8);
//	        } else {
//		        i = 0;
//	        }
//    
//            player->SetSkinNumber((player->GetServerEntity() - GetGameworld()->GetServerEntities() - 1) | i);
//        }
//
//        // Update the player state: gunIndex & ammoIndex to 0, meaning it won't display.
//        if (!client->persistent.activeWeapon) {
//	        // dead
//	        client->playerState.gunIndex = 0;
//	        client->ammoIndex = 0;
//	        return;
//        }
//
//        // Update the client's ammo index.
//        if (client->persistent.activeWeapon) {
//    	    client->ammoIndex = client->persistent.activeWeapon->GetPrimaryAmmoIdentifier();
//        } else {
//            client->ammoIndex = 0;
//        }
//
//
//        // Update playerstate: gunFrame and gunIndex.
//    //    client->playerState.gunFrame = 0;
//        client->playerState.gunIndex = client->persistent.activeWeapon->GetViewModelIndex();
//    }
//
//    //// When a weapon its state is higher or equal to up, we want to go and holster it.
//    //if (client->weaponState >= WeaponState::Up) {
//    //    client->weaponState = WeaponState::Holster;
//    //}
//
//
//
//    // Set player animation. No clue why the OG code used Pain, but we'll have to live with it for now.
//    client->animation.priorityAnimation = PlayerAnimation::Pain;
//    if (client->playerState.pmove.flags & PMF_DUCKED) {
//        player->SetFrame(FRAME_crpain1);
//        client->animation.endFrame = FRAME_crpain4;
//    } else {
//        player->SetFrame(FRAME_pain301);
//        client->animation.endFrame = FRAME_pain304;
//    }
//}
//===========================================================================================================
//void SVG_ChangeWeapon(SVGBasePlayer *player) {
//    // Sanity check.
//    if (!player) {
//	    return;
//    }
//
//    // Get client, sanity check.
//    ServerClient* client = player->GetClient();
//    if (!client) {
//        return;
//    }
//
//    // Acquire base item weapon.
//    SVGBaseItemWeapon *activeWeapon = client->persistent.activeWeapon;
//    SVGBaseItemWeapon *newWeapon = client->newWeapon;
//
//    // Store activeWeapon as lastWeapon, set the active weapon to newWeapon and reset newWeapon after doing so.
//    client->persistent.lastWeapon = activeWeapon;
//    client->persistent.activeWeapon = newWeapon;
//    client->newWeapon = nullptr;
//
//    // Set visible model
//    if (player->GetModelIndex() == 255) {
//        int32_t i = 0;
//	    if (client->persistent.activeWeapon) {
//	        i = ((client->persistent.activeWeapon->GetViewModelIndex() & 0xff) << 8);
//	    } else {
//		    i = 0;
//	    }
//    
//        player->SetSkinNumber((player->GetServerEntity() - GetGameworld()->GetServerEntities() - 1) | i);
//    }
//
//    // Update the player state: gunIndex & ammoIndex to 0, meaning it won't display.
//    if (!client->persistent.activeWeapon) {
//	    // dead
//	    client->playerState.gunIndex = 0;
//	    client->ammoIndex = 0;
//	    return;
//    }
//
//    // Update the client's ammo index.
//    if (client->persistent.activeWeapon) {
//    	client->ammoIndex = client->persistent.activeWeapon->GetPrimaryAmmoIdentifier();
//    } else {
//        client->ammoIndex = 0;
//    }
//
//    // Set weapon state: Drawing.
//    client->weaponState = WeaponState::Draw;
//
//    // Update playerstate: gunFrame and gunIndex.
////    client->playerState.gunFrame = 0;
//    client->playerState.gunIndex = client->persistent.activeWeapon->GetViewModelIndex();
//
//    // Set player animation. No clue why the OG code used Pain, but we'll have to live with it for now.
//    client->animation.priorityAnimation = PlayerAnimation::Pain;
//    if (client->playerState.pmove.flags & PMF_DUCKED) {
//        player->SetFrame(FRAME_crpain1);
//        client->animation.endFrame = FRAME_crpain4;
//    } else {
//        player->SetFrame(FRAME_pain301);
//        client->animation.endFrame = FRAME_pain304;
//    }
//}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange(SVGBasePlayer *ent)
{
    ServerClient* client = ent->GetClient();

    //if (client->persistent.inventory[ITEM_INDEX(SVG_FindItemByPickupName("bullets"))]
    //    &&  client->persistent.inventory[ITEM_INDEX(SVG_FindItemByPickupName("machinegun"))]) {
    //    client->newWeapon = SVG_FindItemByPickupName("machinegun");
    //    return;
    //}
    //client->newWeapon = SVG_FindItemByPickupName("blaster");
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon(SVGBasePlayer *ent, gitem_t *item)
{
    int     index;

    ServerClient* client = ent->GetClient();
    //index = ITEM_INDEX(item);
    //// see if we're already using it
    //if (((item == client->persistent.activeWeapon) || (item == client->newWeapon)) && (client->persistent.inventory[index] == 1)) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Can't drop current weapon\n");
    //    return;
    //}

    //SVG_DropItem(ent->GetServerEntity(), item);
    //client->persistent.inventory[index]--;
}

