// LICENSE HERE.

//
// svgame/weapons/machinegun.c
//
//
// Machinegun/Chaingun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// This is actually declared in grenade.c
void weapon_grenade_fire(edict_t* ent, qboolean held);

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire(edict_t* ent)
{
    int i;
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      angles;
    int         damage = 8;
    int         kick = 2;
    vec3_t      offset;

    if (!(ent->client->buttons & BUTTON_ATTACK)) {
        ent->client->machinegun_shots = 0;
        ent->client->ps.gunframe++;
        return;
    }

    if (ent->client->ps.gunframe == 5)
        ent->client->ps.gunframe = 4;
    else
        ent->client->ps.gunframe = 5;

    if (ent->client->pers.inventory[ent->client->ammo_index] < 1) {
        ent->client->ps.gunframe = 6;
        if (level.time >= ent->pain_debounce_time) {
            gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
            ent->pain_debounce_time = level.time + 1;
        }
        NoAmmoWeaponChange(ent);
        return;
    }

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    for (i = 1; i < 3; i++) {
        ent->client->kick_origin[i] = crandom() * 0.35;
        ent->client->kick_angles[i] = crandom() * 0.7;
    }
    ent->client->kick_origin[0] = crandom() * 0.35;
    ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

    // raise the gun as it is firing
    if (!deathmatch->value) {
        ent->client->machinegun_shots++;
        if (ent->client->machinegun_shots > 9)
            ent->client->machinegun_shots = 9;
    }

    // get start / end positions
    VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
    AngleVectors(angles, forward, right, NULL);
    VectorSet(offset, 0, 8, ent->viewheight - 8);
    P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
    fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_MACHINEGUN | is_silenced);
    gi.multicast(ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;

    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
        ent->s.frame = FRAME_crattak1 - (int)(random() + 0.25);
        ent->client->anim_end = FRAME_crattak9;
    }
    else {
        ent->s.frame = FRAME_attack1 - (int)(random() + 0.25);
        ent->client->anim_end = FRAME_attack8;
    }
}

void Weapon_Machinegun(edict_t* ent)
{
    static int  pause_frames[] = { 23, 45, 0 };
    static int  fire_frames[] = { 4, 5, 0 };

    Weapon_Generic(ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire(edict_t* ent)
{
    int         i;
    int         shots;
    vec3_t      start;
    vec3_t      forward, right, up;
    float       r, u;
    vec3_t      offset;
    int         damage;
    int         kick = 2;

    if (deathmatch->value)
        damage = 6;
    else
        damage = 8;

    if (ent->client->ps.gunframe == 5)
        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

    if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK)) {
        ent->client->ps.gunframe = 32;
        ent->client->weapon_sound = 0;
        return;
    }
    else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
        && ent->client->pers.inventory[ent->client->ammo_index]) {
        ent->client->ps.gunframe = 15;
    }
    else {
        ent->client->ps.gunframe++;
    }

    if (ent->client->ps.gunframe == 22) {
        ent->client->weapon_sound = 0;
        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
    }
    else {
        ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
    }

    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
        ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
        ent->client->anim_end = FRAME_crattak9;
    }
    else {
        ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
        ent->client->anim_end = FRAME_attack8;
    }

    if (ent->client->ps.gunframe <= 9)
        shots = 1;
    else if (ent->client->ps.gunframe <= 14) {
        if (ent->client->buttons & BUTTON_ATTACK)
            shots = 2;
        else
            shots = 1;
    }
    else
        shots = 3;

    if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
        shots = ent->client->pers.inventory[ent->client->ammo_index];

    if (!shots) {
        if (level.time >= ent->pain_debounce_time) {
            gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
            ent->pain_debounce_time = level.time + 1;
        }
        NoAmmoWeaponChange(ent);
        return;
    }

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    for (i = 0; i < 3; i++) {
        ent->client->kick_origin[i] = crandom() * 0.35;
        ent->client->kick_angles[i] = crandom() * 0.7;
    }

    for (i = 0; i < shots; i++) {
        // get start / end positions
        AngleVectors(ent->client->v_angle, forward, right, up);
        r = 7 + crandom() * 4;
        u = crandom() * 4;
        VectorSet(offset, 0, r, u + ent->viewheight - 8);
        P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

        fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
    }

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte((MZ_CHAINGUN1 + shots - 1) | is_silenced);
    gi.multicast(ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun(edict_t* ent)
{
    static int  pause_frames[] = { 38, 43, 51, 61, 0 };
    static int  fire_frames[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0 };

    Weapon_Generic(ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}