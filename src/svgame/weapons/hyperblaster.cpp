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
void Weapon_HyperBlaster_Fire(entity_t* ent)
{
    float   rotation;
    vec3_t  offset;
    int     effect;
    int     damage;

    ent->client->weapon_sound = gi.SoundIndex("weapons/hyprbl1a.wav");

    if (!(ent->client->buttons & BUTTON_ATTACK)) {
        ent->client->playerState.gunframe++;
    }
    else {
        if (!ent->client->pers.inventory[ent->client->ammo_index]) {
            if (level.time >= ent->debouncePainTime) {
                gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                ent->debouncePainTime = level.time + 1;
            }
            NoAmmoWeaponChange(ent);
        }
        else {
            rotation = (ent->client->playerState.gunframe - 5) * 2 * M_PI / 6;
            offset[0] = -4 * std::sinf(rotation);
            offset[1] = 0;
            offset[2] = 4 * std::cosf(rotation);

            if ((ent->client->playerState.gunframe == 6) || (ent->client->playerState.gunframe == 9))
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
            if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
                ent->s.frame = FRAME_crattak1 - 1;
                ent->client->anim_end = FRAME_crattak9;
            }
            else {
                ent->s.frame = FRAME_attack1 - 1;
                ent->client->anim_end = FRAME_attack8;
            }
        }

        ent->client->playerState.gunframe++;
        if (ent->client->playerState.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
            ent->client->playerState.gunframe = 6;
    }

    if (ent->client->playerState.gunframe == 12) {
        gi.Sound(ent, CHAN_AUTO, gi.SoundIndex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
        ent->client->weapon_sound = 0;
    }

}

void Weapon_HyperBlaster(entity_t* ent)
{
    static int  pause_frames[] = { 0 };
    static int  fire_frames[] = { 6, 7, 8, 9, 10, 11, 0 };

    Weapon_Generic(ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}