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

#include "sharedgame/sharedgame.h" // Include SG Base.

qboolean is_quad;
byte     is_silenced;

/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(entity_t *who, vec3_t where, int type)
{
    entity_t     *noise;

    if (deathmatch->value)
        return;

    if (who->flags & FL_NOTARGET)
        return;


    if (!who->myNoise) {
        noise = G_Spawn();
        noise->classname = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who;
        noise->svFlags = SVF_NOCLIENT;
        who->myNoise = noise;

        noise = G_Spawn();
        noise->classname = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who;
        noise->svFlags = SVF_NOCLIENT;
        who->myNoise2 = noise;
    }

    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
        noise = who->myNoise;
        level.sound_entity = noise;
        level.sound_entity_framenum = level.framenum;
    } else { // type == PNOISE_IMPACT
        noise = who->myNoise2;
        level.sound2_entity = noise;
        level.sound2_entity_framenum = level.framenum;
    }

    VectorCopy(where, noise->s.origin);
    VectorSubtract(where, noise->maxs, noise->absMin);
    VectorAdd(where, noise->maxs, noise->absMax);
    noise->teleportTime = level.time;
    gi.LinkEntity(noise);
}


qboolean Pickup_Weapon(entity_t *ent, entity_t *other)
{
    int         index;
    gitem_t     *ammo;

    index = ITEM_INDEX(ent->item);

    if ((((int)(dmflags->value) & DeathMatchFlags::WeaponsStay) || coop->value)
        && other->client->pers.inventory[index]) {
        if (!(ent->spawnFlags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
            return false;   // leave the weapon for others to pickup
    }

    other->client->pers.inventory[index]++;

    if (!(ent->spawnFlags & DROPPED_ITEM)) {
        // give them some ammo with it
        ammo = FindItem(ent->item->ammo);
        if ((int)dmflags->value & DeathMatchFlags::InfiniteAmmo)
            Add_Ammo(other, ammo, 1000);
        else
            Add_Ammo(other, ammo, ammo->quantity);

        if (!(ent->spawnFlags & DROPPED_PLAYER_ITEM)) {
            if (deathmatch->value) {
                if ((int)(dmflags->value) & DeathMatchFlags::WeaponsStay)
                    ent->flags |= FL_RESPAWN;
                else
                    SetRespawn(ent, 30);
            }
            if (coop->value)
                ent->flags |= FL_RESPAWN;
        }
    }

    if (other->client->pers.weapon != ent->item &&
        (other->client->pers.inventory[index] == 1) &&
        (!deathmatch->value || other->client->pers.weapon == FindItem("blaster")))
        other->client->newweapon = ent->item;

    return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon(entity_t *ent)
{
    int i;

    ent->client->pers.lastweapon = ent->client->pers.weapon;
    ent->client->pers.weapon = ent->client->newweapon;
    ent->client->newweapon = NULL;
    ent->client->machinegun_shots = 0;

    // set visible model
    if (ent->s.modelindex == 255) {
        if (ent->client->pers.weapon)
            i = ((ent->client->pers.weapon->weaponModel & 0xff) << 8);
        else
            i = 0;
        ent->s.skinnum = (ent - g_edicts - 1) | i;
    }

    if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
        ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
    else
        ent->client->ammo_index = 0;

    if (!ent->client->pers.weapon) {
        // dead
        ent->client->playerState.gunindex = 0;
        return;
    }

    ent->client->weaponstate = WEAPON_ACTIVATING;
    ent->client->playerState.gunframe = 0;
    ent->client->playerState.gunindex = gi.ModelIndex(ent->client->pers.weapon->viewModel);

    ent->client->anim_priority = ANIM_PAIN;
    if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
        ent->s.frame = FRAME_crpain1;
        ent->client->anim_end = FRAME_crpain4;
    } else {
        ent->s.frame = FRAME_pain301;
        ent->client->anim_end = FRAME_pain304;

    }
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange(entity_t *ent)
{
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))]) {
        ent->client->newweapon = FindItem("machinegun");
        return;
    }
    ent->client->newweapon = FindItem("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon(entity_t *ent)
{
    // if just died, put the weapon away
    if (ent->health < 1) {
        ent->client->newweapon = NULL;
        ChangeWeapon(ent);
    }

    // call active weapon Think routine
    if (ent->client->pers.weapon && ent->client->pers.weapon->WeaponThink) {
        ent->client->pers.weapon->WeaponThink(ent);
    }
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(entity_t *ent, gitem_t *item)
{
    int         ammo_index;
    gitem_t     *ammo_item;

    // see if we're already using it
    if (item == ent->client->pers.weapon)
        return;

    if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO)) {
        ammo_item = FindItem(item->ammo);
        ammo_index = ITEM_INDEX(ammo_item);

        if (!ent->client->pers.inventory[ammo_index]) {
            gi.CPrintf(ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickupName, item->pickupName);
            return;
        }

        if (ent->client->pers.inventory[ammo_index] < item->quantity) {
            gi.CPrintf(ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickupName, item->pickupName);
            return;
        }
    }

    // change to this weapon when down
    ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon(entity_t *ent, gitem_t *item)
{
    int     index;

    if ((int)(dmflags->value) & DeathMatchFlags::WeaponsStay)
        return;

    index = ITEM_INDEX(item);
    // see if we're already using it
    if (((item == ent->client->pers.weapon) || (item == ent->client->newweapon)) && (ent->client->pers.inventory[index] == 1)) {
        gi.CPrintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
        return;
    }

    Drop_Item(ent, item);
    ent->client->pers.inventory[index]--;
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

void Weapon_Generic(entity_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(entity_t *ent))
{
    int     n;

    if (ent->deadFlag || ent->s.modelindex != 255) { // VWep animations screw up corpses
        return;
    }

    if (ent->client->weaponstate == WEAPON_DROPPING) {
        if (ent->client->playerState.gunframe == FRAME_DEACTIVATE_LAST) {
            ChangeWeapon(ent);
            return;
        } else if ((FRAME_DEACTIVATE_LAST - ent->client->playerState.gunframe) == 4) {
            ent->client->anim_priority = ANIM_REVERSE;
            if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
                ent->s.frame = FRAME_crpain4 + 1;
                ent->client->anim_end = FRAME_crpain1;
            } else {
                ent->s.frame = FRAME_pain304 + 1;
                ent->client->anim_end = FRAME_pain301;

            }
        }

        ent->client->playerState.gunframe++;
        return;
    }

    if (ent->client->weaponstate == WEAPON_ACTIVATING) {
        if (ent->client->playerState.gunframe == FRAME_ACTIVATE_LAST) {
            ent->client->weaponstate = WEAPON_READY;
            ent->client->playerState.gunframe = FRAME_IDLE_FIRST;
            return;
        }

        ent->client->playerState.gunframe++;
        return;
    }

    if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING)) {
        ent->client->weaponstate = WEAPON_DROPPING;
        ent->client->playerState.gunframe = FRAME_DEACTIVATE_FIRST;

        if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4) {
            ent->client->anim_priority = ANIM_REVERSE;
            if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
                ent->s.frame = FRAME_crpain4 + 1;
                ent->client->anim_end = FRAME_crpain1;
            } else {
                ent->s.frame = FRAME_pain304 + 1;
                ent->client->anim_end = FRAME_pain301;

            }
        }
        return;
    }

    if (ent->client->weaponstate == WEAPON_READY) {
        if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK)) {
            ent->client->latched_buttons &= ~BUTTON_ATTACK;
            if ((!ent->client->ammo_index) ||
                (ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity)) {
                ent->client->playerState.gunframe = FRAME_FIRE_FIRST;
                ent->client->weaponstate = WEAPON_FIRING;

                // start the animation
                ent->client->anim_priority = ANIM_ATTACK;
                if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
                    ent->s.frame = FRAME_crattak1 - 1;
                    ent->client->anim_end = FRAME_crattak9;
                } else {
                    ent->s.frame = FRAME_attack1 - 1;
                    ent->client->anim_end = FRAME_attack8;
                }
            } else {
                if (level.time >= ent->debouncePainTime) {
                    gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                    ent->debouncePainTime = level.time + 1;
                }
                NoAmmoWeaponChange(ent);
            }
        } else {
            if (ent->client->playerState.gunframe == FRAME_IDLE_LAST) {
                ent->client->playerState.gunframe = FRAME_IDLE_FIRST;
                return;
            }

            if (pause_frames) {
                for (n = 0; pause_frames[n]; n++) {
                    if (ent->client->playerState.gunframe == pause_frames[n]) {
                        if (rand() & 15)
                            return;
                    }
                }
            }

            ent->client->playerState.gunframe++;
            return;
        }
    }

    if (ent->client->weaponstate == WEAPON_FIRING) {
        for (n = 0; fire_frames[n]; n++) {
            if (ent->client->playerState.gunframe == fire_frames[n]) {
                fire(ent);
                break;
            }
        }

        if (!fire_frames[n])
            ent->client->playerState.gunframe++;

        if (ent->client->playerState.gunframe == FRAME_IDLE_FIRST + 1)
            ent->client->weaponstate = WEAPON_READY;
    }
}