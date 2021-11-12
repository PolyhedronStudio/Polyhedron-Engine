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
// g_weapon.c

#include "../g_local.h"
#include "animations.h"

#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

#include "sharedgame/sharedgame.h" // Include SG Base.

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

    if (deathmatch->value)
        return;

    if (who->GetFlags() & EntityFlags::NoTarget)
        return;


    if (!who->GetServerEntity()->myNoisePtr) {
        noise = SVG_Spawn();
        noise->className = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who->GetServerEntity();
        noise->serverFlags = EntityServerFlags::NoClient;
        who->GetServerEntity()->myNoisePtr = noise;

        noise = SVG_Spawn();
        noise->className = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who->GetServerEntity();
        noise->serverFlags = EntityServerFlags::NoClient;
        who->GetServerEntity()->myNoise2Ptr = noise;
    }

    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
        noise = who->GetServerEntity()->myNoisePtr;
        level.soundEntity = noise;
        level.soundEntityFrameNumber = level.frameNumber;
    } else { // type == PNOISE_IMPACT
        noise = who->GetServerEntity()->myNoise2Ptr;
        level.sound2Entity = noise;
        level.sound2EntityFrameNumber = level.frameNumber;
    }

    VectorCopy(where, noise->state.origin);
    VectorSubtract(where, noise->maxs, noise->absMin);
    VectorAdd(where, noise->maxs, noise->absMax);
    noise->teleportTime = level.time;
    gi.LinkEntity(noise);
}


qboolean Pickup_Weapon(SVGBaseEntity *ent, PlayerClient *other)
{
    //int         index;
    //gitem_t     *ammo;

    //index = ITEM_INDEX(ent->item);

    //if ((((int)(gamemodeflags->value) & GameModeFlags::WeaponsStay) || coop->value)
    //    && other->client->persistent.inventory[index]) {
    //    if (!(ent->spawnFlags & (ItemSpawnFlags::DroppedItem | ItemSpawnFlags::DroppedPlayerItem)))
    //        return false;   // leave the weapon for others to pickup
    //}

    //other->client->persistent.inventory[index]++;

    //if (!(ent->spawnFlags & ItemSpawnFlags::DroppedItem)) {
    //    // give them some ammo with it
    //    ammo = SVG_FindItemByPickupName(ent->item->ammo);
    //    if ((int)gamemodeflags->value & GameModeFlags::InfiniteAmmo)
    //        SVG_AddAmmo(other, ammo, 1000);
    //    else
    //        SVG_AddAmmo(other, ammo, ammo->quantity);

    //    if (!(ent->spawnFlags & ItemSpawnFlags::DroppedPlayerItem)) {
    //        if (deathmatch->value) {
    //            if ((int)(gamemodeflags->value) & GameModeFlags::WeaponsStay)
    //                ent->flags |= EntityFlags::Respawn;
    //            else
    //                SVG_SetRespawn(ent, 30);
    //        }
    //        if (coop->value)
    //            ent->flags |= EntityFlags::Respawn;
    //    }
    //}

    //if (other->client->persistent.activeWeapon != ent->item &&
    //    (other->client->persistent.inventory[index] == 1) &&
    //    (!deathmatch->value || other->client->persistent.activeWeapon == SVG_FindItemByPickupName("blaster")))
    //    other->client->newWeapon = ent->item;

    //return true;
    return false;
}


/*
===============
SVG_ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void SVG_ChangeWeapon(PlayerClient*ent)
{
    int i;

    if (!ent)
        return;

    ServersClient* client = ent->GetClient();

    client->persistent.lastWeapon = client->persistent.activeWeapon;
    client->persistent.activeWeapon = client->newWeapon;
    client->newWeapon = NULL;
    client->machinegunShots = 0;

    // set visible model
    if (ent->GetModelIndex() == 255) {
        if (client->persistent.activeWeapon)
            i = ((client->persistent.activeWeapon->weaponModelIndex & 0xff) << 8);
        else
            i = 0;
        ent->SetSkinNumber((ent->GetServerEntity() - g_entities - 1) | i);
    }

    if (client->persistent.activeWeapon && client->persistent.activeWeapon->ammo)
        client->ammoIndex = ITEM_INDEX(SVG_FindItemByPickupName(client->persistent.activeWeapon->ammo));
    else
        client->ammoIndex = 0;

    if (!client->persistent.activeWeapon) {
        // dead
        client->playerState.gunIndex = 0;
        return;
    }

    client->weaponState = WeaponState::Activating;
    client->playerState.gunFrame = 0;
    client->playerState.gunIndex = gi.ModelIndex(client->persistent.activeWeapon->viewModel);

    client->animation.priorityAnimation = PlayerAnimation::Pain;
    if (client->playerState.pmove.flags & PMF_DUCKED) {
        ent->SetFrame(FRAME_crpain1);
        client->animation.endFrame = FRAME_crpain4;
    } else {
        ent->SetFrame(FRAME_pain301);
        client->animation.endFrame = FRAME_pain304;

    }
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange(PlayerClient *ent)
{
    ServersClient* client = ent->GetClient();

    if (client->persistent.inventory[ITEM_INDEX(SVG_FindItemByPickupName("bullets"))]
        &&  client->persistent.inventory[ITEM_INDEX(SVG_FindItemByPickupName("machinegun"))]) {
        client->newWeapon = SVG_FindItemByPickupName("machinegun");
        return;
    }
    client->newWeapon = SVG_FindItemByPickupName("blaster");
}

/*
=================
SVG_ThinkWeapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void SVG_ThinkWeapon(PlayerClient *ent)
{
    if (!ent)
        return;

    ServersClient* client = ent->GetClient();

    if (!client)
        return;

    // if just died, put the weapon away
    if (ent->GetHealth() < 1) {
        client->newWeapon = NULL;
        SVG_ChangeWeapon(ent);
    }

    // call active weapon Think routine
    if (client->persistent.activeWeapon && client->persistent.activeWeapon->WeaponThink) {
        client->persistent.activeWeapon->WeaponThink(ent);
    }
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(PlayerClient *ent, gitem_t* item)
{
    int         ammoIndex;
    gitem_t     *ammo_item;
    ServersClient* client = ent->GetClient();

    // see if we're already using it
    if (item == client->persistent.activeWeapon)
        return;

    if (item->ammo && !g_select_empty->value && !(item->flags & ItemFlags::IsAmmo)) {
        ammo_item = SVG_FindItemByPickupName(item->ammo);
        ammoIndex = ITEM_INDEX(ammo_item);

        if (!client->persistent.inventory[ammoIndex]) {
            gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "No %s for %s.\n", ammo_item->pickupName, item->pickupName);
            return;
        }

        if (client->persistent.inventory[ammoIndex] < item->quantity) {
            gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickupName, item->pickupName);
            return;
        }
    }

    // change to this weapon when down
    client->newWeapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon(PlayerClient *ent, gitem_t *item)
{
    int     index;

    if ((int)(gamemodeflags->value) & GameModeFlags::WeaponsStay)
        return;

    ServersClient* client = ent->GetClient();
    index = ITEM_INDEX(item);
    // see if we're already using it
    if (((item == client->persistent.activeWeapon) || (item == client->newWeapon)) && (client->persistent.inventory[index] == 1)) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Can't drop current weapon\n");
        return;
    }

    SVG_DropItem(ent->GetServerEntity(), item);
    client->persistent.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST        (FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST        (FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST  (FRAME_IDLE_LAST + 1)

void Weapon_Generic(PlayerClient *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(PlayerClient *ent))
{
    int     n;

    if (ent->GetDeadFlag() || ent->GetModelIndex() != 255) { // VWep animations screw up corpses
        return;
    }

    ServersClient* client = ent->GetClient();

    if (client->weaponState == WeaponState::Dropping) {
        if (client->playerState.gunFrame == FRAME_DEACTIVATE_LAST) {
            SVG_ChangeWeapon(ent);
            return;
        } else if ((FRAME_DEACTIVATE_LAST - client->playerState.gunFrame) == 4) {
            client->animation.priorityAnimation = PlayerAnimation::Reverse;
            if (client->playerState.pmove.flags & PMF_DUCKED) {
                ent->SetFrame(FRAME_crpain4 + 1);
                client->animation.endFrame = FRAME_crpain1;
            } else {
                ent->SetFrame(FRAME_pain304 + 1);
                client->animation.endFrame = FRAME_pain301;

            }
        }

        client->playerState.gunFrame++;
        return;
    }

    if (client->weaponState == WeaponState::Activating) {
        if (client->playerState.gunFrame == FRAME_ACTIVATE_LAST) {
            client->weaponState = WeaponState::Ready;
            client->playerState.gunFrame = FRAME_IDLE_FIRST;
            return;
        }

        client->playerState.gunFrame++;
        return;
    }

    if ((client->newWeapon) && (client->weaponState != WeaponState::Firing)) {
        client->weaponState = WeaponState::Dropping;
        client->playerState.gunFrame = FRAME_DEACTIVATE_FIRST;

        if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4) {
            client->animation.priorityAnimation = PlayerAnimation::Reverse;
            if (client->playerState.pmove.flags & PMF_DUCKED) {
                ent->SetFrame(FRAME_crpain4 + 1);
                client->animation.endFrame = FRAME_crpain1;
            } else {
                ent->SetFrame(FRAME_pain304 + 1);
                client->animation.endFrame = FRAME_pain301;

            }
        }
        return;
    }

    if (client->weaponState == WeaponState::Ready) {
        if (((client->latchedButtons | client->buttons) & BUTTON_ATTACK)) {
            client->latchedButtons &= ~BUTTON_ATTACK;
            if ((!client->ammoIndex) ||
                (client->persistent.inventory[client->ammoIndex] >= client->persistent.activeWeapon->quantity)) {
                client->playerState.gunFrame = FRAME_FIRE_FIRST;
                client->weaponState = WeaponState::Firing;

                // start the animation
                client->animation.priorityAnimation = PlayerAnimation::Attack;
                if (client->playerState.pmove.flags & PMF_DUCKED) {
                    ent->SetFrame(FRAME_crattak1 - 1);
                    client->animation.endFrame = FRAME_crattak9;
                } else {
                    ent->SetFrame(FRAME_attack1 - 1);
                    client->animation.endFrame = FRAME_attack8;
                }
            } else {
                if (level.time >= ent->GetDebouncePainTime()) {
                    gi.Sound(ent->GetServerEntity(), CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                    ent->SetDebouncePainTime(level.time + 1);
                }
                NoAmmoWeaponChange(ent);
            }
        } else {
            if (client->playerState.gunFrame == FRAME_IDLE_LAST) {
                client->playerState.gunFrame = FRAME_IDLE_FIRST;
                return;
            }

            if (pause_frames) {
                for (n = 0; pause_frames[n]; n++) {
                    if (client->playerState.gunFrame == pause_frames[n]) {
                        if (rand() & 15)
                            return;
                    }
                }
            }

            client->playerState.gunFrame++;
            return;
        }
    }

    if (client->weaponState == WeaponState::Firing) {
        for (n = 0; fire_frames[n]; n++) {
            if (client->playerState.gunFrame == fire_frames[n]) {
                fire(ent);
                break;
            }
        }

        if (!fire_frames[n])
            client->playerState.gunFrame++;

        if (client->playerState.gunFrame == FRAME_IDLE_FIRST + 1)
            client->weaponState = WeaponState::Ready;
    }
}