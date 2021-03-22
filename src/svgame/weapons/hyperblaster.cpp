// LICENSE HERE.

//
// svgame/weapons/hyperblaster.c
//
//
// Hyperblaster weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include blaster and hyperblaster header.
#include "blaster.h"
#include "hyperblaster.h"

//
//======================================================================
//
//HYPERBLASTER
//
//======================================================================
//
void Weapon_HyperBlaster_Fire(edict_t* ent)
{
    float   rotation;
    vec3_t  offset;
    int     effect;
    int     damage;

    ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

    if (!(ent->client->buttons & BUTTON_ATTACK)) {
        ent->client->ps.gunframe++;
    }
    else {
        if (!ent->client->pers.inventory[ent->client->ammo_index]) {
            if (level.time >= ent->pain_debounce_time) {
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                ent->pain_debounce_time = level.time + 1;
            }
            NoAmmoWeaponChange(ent);
        }
        else {
            rotation = (ent->client->ps.gunframe - 5) * 2 * M_PI / 6;
            offset[0] = -4 * sin(rotation);
            offset[1] = 0;
            offset[2] = 4 * cos(rotation);

            if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
                effect = EF_HYPERBLASTER;
            else
                effect = 0;
            if (deathmatch->value)
                damage = 15;
            else
                damage = 20;
            Blaster_Fire(ent, offset, damage, true, effect);
            if (!((int)dmflags->value & DF_INFINITE_AMMO))
                ent->client->pers.inventory[ent->client->ammo_index]--;

            ent->client->anim_priority = ANIM_ATTACK;
            if (ent->client->ps.pmove.flags & PMF_DUCKED) {
                ent->s.frame = FRAME_crattak1 - 1;
                ent->client->anim_end = FRAME_crattak9;
            }
            else {
                ent->s.frame = FRAME_attack1 - 1;
                ent->client->anim_end = FRAME_attack8;
            }
        }

        ent->client->ps.gunframe++;
        if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
            ent->client->ps.gunframe = 6;
    }

    if (ent->client->ps.gunframe == 12) {
        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
        ent->client->weapon_sound = 0;
    }

}

void Weapon_HyperBlaster(edict_t* ent)
{
    static int  pause_frames[] = { 0 };
    static int  fire_frames[] = { 6, 7, 8, 9, 10, 11, 0 };

    Weapon_Generic(ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}