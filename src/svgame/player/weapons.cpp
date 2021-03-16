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


qboolean is_quad;
byte     is_silenced;


void weapon_grenade_fire(edict_t *ent, qboolean held);


void P_ProjectSource(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
    vec3_t  _distance;

    VectorCopy(distance, _distance);
    if (client->pers.hand == LEFT_HANDED)
        _distance[1] *= -1;
    else if (client->pers.hand == CENTER_HANDED)
        _distance[1] = 0;
    G_ProjectSource(point, _distance, forward, right, result);
}


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
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
    edict_t     *noise;

    if (type == PNOISE_WEAPON) {
        if (who->client->silencer_shots) {
            who->client->silencer_shots--;
            return;
        }
    }

    if (deathmatch->value)
        return;

    if (who->flags & FL_NOTARGET)
        return;


    if (!who->mynoise) {
        noise = G_Spawn();
        noise->classname = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who;
        noise->svflags = SVF_NOCLIENT;
        who->mynoise = noise;

        noise = G_Spawn();
        noise->classname = "player_noise";
        VectorSet(noise->mins, -8, -8, -8);
        VectorSet(noise->maxs, 8, 8, 8);
        noise->owner = who;
        noise->svflags = SVF_NOCLIENT;
        who->mynoise2 = noise;
    }

    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
        noise = who->mynoise;
        level.sound_entity = noise;
        level.sound_entity_framenum = level.framenum;
    } else { // type == PNOISE_IMPACT
        noise = who->mynoise2;
        level.sound2_entity = noise;
        level.sound2_entity_framenum = level.framenum;
    }

    VectorCopy(where, noise->s.origin);
    VectorSubtract(where, noise->maxs, noise->absmin);
    VectorAdd(where, noise->maxs, noise->absmax);
    noise->teleport_time = level.time;
    gi.linkentity(noise);
}


qboolean Pickup_Weapon(edict_t *ent, edict_t *other)
{
    int         index;
    gitem_t     *ammo;

    index = ITEM_INDEX(ent->item);

    if ((((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
        && other->client->pers.inventory[index]) {
        if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
            return qfalse;   // leave the weapon for others to pickup
    }

    other->client->pers.inventory[index]++;

    if (!(ent->spawnflags & DROPPED_ITEM)) {
        // give them some ammo with it
        ammo = FindItem(ent->item->ammo);
        if ((int)dmflags->value & DF_INFINITE_AMMO)
            Add_Ammo(other, ammo, 1000);
        else
            Add_Ammo(other, ammo, ammo->quantity);

        if (!(ent->spawnflags & DROPPED_PLAYER_ITEM)) {
            if (deathmatch->value) {
                if ((int)(dmflags->value) & DF_WEAPONS_STAY)
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

    return qtrue;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon(edict_t *ent)
{
    int i;

    if (ent->client->grenade_time) {
        ent->client->grenade_time = level.time;
        ent->client->weapon_sound = 0;
        weapon_grenade_fire(ent, qfalse);
        ent->client->grenade_time = 0;
    }

    ent->client->pers.lastweapon = ent->client->pers.weapon;
    ent->client->pers.weapon = ent->client->newweapon;
    ent->client->newweapon = NULL;
    ent->client->machinegun_shots = 0;

    // set visible model
    if (ent->s.modelindex == 255) {
        if (ent->client->pers.weapon)
            i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
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
        ent->client->ps.gunindex = 0;
        return;
    }

    ent->client->weaponstate = WEAPON_ACTIVATING;
    ent->client->ps.gunframe = 0;
    ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

    ent->client->anim_priority = ANIM_PAIN;
    if (ent->client->ps.pmove.flags & PMF_DUCKED) {
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
void NoAmmoWeaponChange(edict_t *ent)
{
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))]) {
        ent->client->newweapon = FindItem("railgun");
        return;
    }
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))]) {
        ent->client->newweapon = FindItem("hyperblaster");
        return;
    }
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))]) {
        ent->client->newweapon = FindItem("chaingun");
        return;
    }
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))]) {
        ent->client->newweapon = FindItem("machinegun");
        return;
    }
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))]) {
        ent->client->newweapon = FindItem("super shotgun");
        return;
    }
    if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
        &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))]) {
        ent->client->newweapon = FindItem("shotgun");
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
void Think_Weapon(edict_t *ent)
{
    // if just died, put the weapon away
    if (ent->health < 1) {
        ent->client->newweapon = NULL;
        ChangeWeapon(ent);
    }

    // call active weapon think routine
    if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink) {
        is_quad = (ent->client->quad_framenum > level.framenum);
        if (ent->client->silencer_shots)
            is_silenced = MZ_SILENCED;
        else
            is_silenced = 0;
        ent->client->pers.weapon->weaponthink(ent);
    }
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(edict_t *ent, gitem_t *item)
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
            gi.cprintf(ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
            return;
        }

        if (ent->client->pers.inventory[ammo_index] < item->quantity) {
            gi.cprintf(ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
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
void Drop_Weapon(edict_t *ent, gitem_t *item)
{
    int     index;

    if ((int)(dmflags->value) & DF_WEAPONS_STAY)
        return;

    index = ITEM_INDEX(item);
    // see if we're already using it
    if (((item == ent->client->pers.weapon) || (item == ent->client->newweapon)) && (ent->client->pers.inventory[index] == 1)) {
        gi.cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
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

void Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
    int     n;

    if (ent->deadflag || ent->s.modelindex != 255) { // VWep animations screw up corpses
        return;
    }

    if (ent->client->weaponstate == WEAPON_DROPPING) {
        if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST) {
            ChangeWeapon(ent);
            return;
        } else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4) {
            ent->client->anim_priority = ANIM_REVERSE;
            if (ent->client->ps.pmove.flags & PMF_DUCKED) {
                ent->s.frame = FRAME_crpain4 + 1;
                ent->client->anim_end = FRAME_crpain1;
            } else {
                ent->s.frame = FRAME_pain304 + 1;
                ent->client->anim_end = FRAME_pain301;

            }
        }

        ent->client->ps.gunframe++;
        return;
    }

    if (ent->client->weaponstate == WEAPON_ACTIVATING) {
        if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST) {
            ent->client->weaponstate = WEAPON_READY;
            ent->client->ps.gunframe = FRAME_IDLE_FIRST;
            return;
        }

        ent->client->ps.gunframe++;
        return;
    }

    if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING)) {
        ent->client->weaponstate = WEAPON_DROPPING;
        ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

        if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4) {
            ent->client->anim_priority = ANIM_REVERSE;
            if (ent->client->ps.pmove.flags & PMF_DUCKED) {
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
                ent->client->ps.gunframe = FRAME_FIRE_FIRST;
                ent->client->weaponstate = WEAPON_FIRING;

                // start the animation
                ent->client->anim_priority = ANIM_ATTACK;
                if (ent->client->ps.pmove.flags & PMF_DUCKED) {
                    ent->s.frame = FRAME_crattak1 - 1;
                    ent->client->anim_end = FRAME_crattak9;
                } else {
                    ent->s.frame = FRAME_attack1 - 1;
                    ent->client->anim_end = FRAME_attack8;
                }
            } else {
                if (level.time >= ent->pain_debounce_time) {
                    gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                    ent->pain_debounce_time = level.time + 1;
                }
                NoAmmoWeaponChange(ent);
            }
        } else {
            if (ent->client->ps.gunframe == FRAME_IDLE_LAST) {
                ent->client->ps.gunframe = FRAME_IDLE_FIRST;
                return;
            }

            if (pause_frames) {
                for (n = 0; pause_frames[n]; n++) {
                    if (ent->client->ps.gunframe == pause_frames[n]) {
                        if (rand() & 15)
                            return;
                    }
                }
            }

            ent->client->ps.gunframe++;
            return;
        }
    }

    if (ent->client->weaponstate == WEAPON_FIRING) {
        for (n = 0; fire_frames[n]; n++) {
            if (ent->client->ps.gunframe == fire_frames[n]) {
                if (ent->client->quad_framenum > level.framenum)
                    gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

                fire(ent);
                break;
            }
        }

        if (!fire_frames[n])
            ent->client->ps.gunframe++;

        if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
            ent->client->weaponstate = WEAPON_READY;
    }
}