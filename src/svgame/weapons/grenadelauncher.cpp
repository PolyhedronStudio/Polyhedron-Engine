// LICENSE HERE.

//
// svgame/weapons/grenadelauncher.c
//
//
// Grenade Launcher weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include grenade and grenadelauncher header.
#include "grenade.h"
#include "grenadelauncher.h"

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire(edict_t* ent)
{
    vec3_t  offset;
    vec3_t  forward, right;
    vec3_t  start;
    int     damage = 120;
    float   radius;

    radius = damage + 40;
    if (is_quad)
        damage *= 4;

    VectorSet(offset, 8, 8, ent->viewheight - 8);
    AngleVectors(ent->client->v_angle, &forward, &right, NULL);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -1;

    fire_grenade(ent, start, forward, damage, 600, 2.5, radius);

    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_GRENADE | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    ent->client->ps.gunframe++;

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher(edict_t* ent)
{
    static int  pause_frames[] = { 34, 51, 59, 0 };
    static int  fire_frames[] = { 6, 0 };

    Weapon_Generic(ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}