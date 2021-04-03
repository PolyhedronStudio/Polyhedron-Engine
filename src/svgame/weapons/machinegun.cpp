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

// Include machinegun weapon header.
#include "machinegun.h"

//
//======================================================================
//
// MACHINEGUN
//
//======================================================================
//
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
        ent->client->kickOrigin[i] = crandom() * 0.35;
        ent->client->kickAngles[i] = crandom() * 0.7;
    }
    ent->client->kickOrigin[0] = crandom() * 0.35;
    ent->client->kickAngles[0] = ent->client->machinegun_shots * -1.5;

    // raise the gun as it is firing
    if (!deathmatch->value) {
        ent->client->machinegun_shots++;
        if (ent->client->machinegun_shots > 9)
            ent->client->machinegun_shots = 9;
    }

    // get start / end positions
    VectorAdd(ent->client->v_angle, ent->client->kickAngles, angles);
    AngleVectors(angles, &forward, &right, NULL);
    VectorSet(offset, 0, 8, ent->viewheight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);
    fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_MACHINEGUN | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;

    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.flags & PMF_DUCKED) {
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