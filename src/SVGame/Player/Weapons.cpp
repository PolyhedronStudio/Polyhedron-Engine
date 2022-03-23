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
//    Entity     *noise;

    //if (deathmatch->value)
    //    return;
    //if (GetGamemode()->IsClass<DeathmatchGamemode>()) {
    //    return;
    //}

    //if (who->GetFlags() & EntityFlags::NoTarget)
    //    return;


//    if (!who->GetPODEntity()->myNoisePtr) {
//        noise = SVG_Spawn();
////        noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetPODEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetPODEntity()->myNoisePtr = noise;
//
//        noise = SVG_Spawn();
//     //   noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetPODEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetPODEntity()->myNoise2Ptr = noise;
//    }
//
//    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
//        noise = who->GetPODEntity()->myNoisePtr;
//        level.soundEntity = noise;
//        level.soundEntityFrameNumber = level.frameNumber;
//    } else { // type == PNOISE_IMPACT
//        noise = who->GetPODEntity()->myNoise2Ptr;
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