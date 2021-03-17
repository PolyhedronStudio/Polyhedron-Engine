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

void weapon_grenade_fire(edict_t* ent, qboolean held)
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

    Vec3_Set(offset, 8, 8, ent->viewheight - 8);
    AngleVectors(ent->client->v_angle, forward, right, NULL);
    P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

    timer = ent->client->grenade_time - level.time;
    speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
    fire_grenade2(ent, start, forward, damage, speed, timer, radius, held);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;

    ent->client->grenade_time = level.time + 1.0;

    if (ent->deadflag || ent->s.modelindex != 255) { // VWep animations screw up corpses
        return;
    }

    if (ent->health <= 0)
        return;

    if (ent->client->ps.pmove.flags & PMF_DUCKED) {
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

void Weapon_Grenade(edict_t* ent)
{
    if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY)) {
        ChangeWeapon(ent);
        return;
    }

    if (ent->client->weaponstate == WEAPON_ACTIVATING) {
        ent->client->weaponstate = WEAPON_READY;
        ent->client->ps.gunframe = 16;
        return;
    }

    if (ent->client->weaponstate == WEAPON_READY) {
        if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK)) {
            ent->client->latched_buttons &= ~BUTTON_ATTACK;
            if (ent->client->pers.inventory[ent->client->ammo_index]) {
                ent->client->ps.gunframe = 1;
                ent->client->weaponstate = WEAPON_FIRING;
                ent->client->grenade_time = 0;
            }
            else {
                if (level.time >= ent->pain_debounce_time) {
                    gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
                    ent->pain_debounce_time = level.time + 1;
                }
                NoAmmoWeaponChange(ent);
            }
            return;
        }

        if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48)) {
            if (rand() & 15)
                return;
        }

        if (++ent->client->ps.gunframe > 48)
            ent->client->ps.gunframe = 16;
        return;
    }

    if (ent->client->weaponstate == WEAPON_FIRING) {
        if (ent->client->ps.gunframe == 5)
            gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

        if (ent->client->ps.gunframe == 11) {
            if (!ent->client->grenade_time) {
                ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
                ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
            }

            // they waited too long, detonate it in their hand
            if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time) {
                ent->client->weapon_sound = 0;
                weapon_grenade_fire(ent, qtrue);
                ent->client->grenade_blew_up = qtrue;
            }

            if (ent->client->buttons & BUTTON_ATTACK)
                return;

            if (ent->client->grenade_blew_up) {
                if (level.time >= ent->client->grenade_time) {
                    ent->client->ps.gunframe = 15;
                    ent->client->grenade_blew_up = qfalse;
                }
                else {
                    return;
                }
            }
        }

        if (ent->client->ps.gunframe == 12) {
            ent->client->weapon_sound = 0;
            weapon_grenade_fire(ent, qfalse);
        }

        if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
            return;

        ent->client->ps.gunframe++;

        if (ent->client->ps.gunframe == 16) {
            ent->client->grenade_time = 0;
            ent->client->weaponstate = WEAPON_READY;
        }
    }
}