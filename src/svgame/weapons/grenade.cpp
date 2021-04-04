// LICENSE HERE.

//
// svgame/weapons/grenade.c
//
//
// Grenade weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include weapon header.
#include "grenade.h"

/*
======================================================================

GRENADE

======================================================================
*/

void weapon_grenade_fire(entity_t* ent, qboolean held)
{
    vec3_t  offset;
    vec3_t  forward, right;
    vec3_t  start;
    int     damage = 125;
    float   timer;
    int     speed;
    float   radius;

    radius = damage + 40;
    if (is_quad)
        damage *= 4;

    VectorSet(offset, 8, 8, ent->viewHeight - 8);
    vec3_vectors(ent->client->v_angle, &forward, &right, NULL);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    timer = ent->client->grenade_time - level.time;
    speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
    Fire_Grenade2(ent, start, forward, damage, speed, timer, radius, held);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;

    ent->client->grenade_time = level.time + 1.0;

    if (ent->deadFlag || ent->s.modelindex != 255) { // VWep animations screw up corpses
        return;
    }

    if (ent->health <= 0)
        return;

    if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
        ent->client->anim_priority = ANIM_ATTACK;
        ent->s.frame = FRAME_crattak1 - 1;
        ent->client->anim_end = FRAME_crattak3;
    }
    else {
        ent->client->anim_priority = ANIM_REVERSE;
        ent->s.frame = FRAME_wave08;
        ent->client->anim_end = FRAME_wave01;
    }
}

void Weapon_Grenade(entity_t* ent)
{
    if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY)) {
        ChangeWeapon(ent);
        return;
    }

    if (ent->client->weaponstate == WEAPON_ACTIVATING) {
        ent->client->weaponstate = WEAPON_READY;
        ent->client->playerState.gunframe = 16;
        return;
    }

    if (ent->client->weaponstate == WEAPON_READY) {
        if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK)) {
            ent->client->latched_buttons &= ~BUTTON_ATTACK;
            if (ent->client->pers.inventory[ent->client->ammo_index]) {
                ent->client->playerState.gunframe = 1;
                ent->client->weaponstate = WEAPON_FIRING;
                ent->client->grenade_time = 0;
            }
            else {
                if (level.time >= ent->debouncePainTime) {
                    gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                    ent->debouncePainTime = level.time + 1;
                }
                NoAmmoWeaponChange(ent);
            }
            return;
        }

        if ((ent->client->playerState.gunframe == 29) || (ent->client->playerState.gunframe == 34) || (ent->client->playerState.gunframe == 39) || (ent->client->playerState.gunframe == 48)) {
            if (rand() & 15)
                return;
        }

        if (++ent->client->playerState.gunframe > 48)
            ent->client->playerState.gunframe = 16;
        return;
    }

    if (ent->client->weaponstate == WEAPON_FIRING) {
        if (ent->client->playerState.gunframe == 5)
            gi.Sound(ent, CHAN_WEAPON, gi.SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

        if (ent->client->playerState.gunframe == 11) {
            if (!ent->client->grenade_time) {
                ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
                ent->client->weapon_sound = gi.SoundIndex("weapons/hgrenc1b.wav");
            }

            // they waited too long, detonate it in their hand
            if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time) {
                ent->client->weapon_sound = 0;
                weapon_grenade_fire(ent, true);
                ent->client->grenade_blew_up = true;
            }

            if (ent->client->buttons & BUTTON_ATTACK)
                return;

            if (ent->client->grenade_blew_up) {
                if (level.time >= ent->client->grenade_time) {
                    ent->client->playerState.gunframe = 15;
                    ent->client->grenade_blew_up = false;
                }
                else {
                    return;
                }
            }
        }

        if (ent->client->playerState.gunframe == 12) {
            ent->client->weapon_sound = 0;
            weapon_grenade_fire(ent, false);
        }

        if ((ent->client->playerState.gunframe == 15) && (level.time < ent->client->grenade_time))
            return;

        ent->client->playerState.gunframe++;

        if (ent->client->playerState.gunframe == 16) {
            ent->client->grenade_time = 0;
            ent->client->weaponstate = WEAPON_READY;
        }
    }
}