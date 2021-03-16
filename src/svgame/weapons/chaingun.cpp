// LICENSE HERE.

//
// svgame/weapons/chaingun.c
//
//
// Chaingun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include weapon header.
#include "chaingun.h"

//
//======================================================================
//
// CHAINGUN
//
//======================================================================
//
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
    if (ent->client->ps.pmove.flags & PMF_DUCKED) {
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