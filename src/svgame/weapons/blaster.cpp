// LICENSE HERE.

//
// svgame/weapons/blaster.c
//
//
// Blaster weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include weapon header.
#include "blaster.h"

//
//======================================================================
//
// BLASTER
//
//======================================================================
//

void Blaster_Fire(entity_t* ent, const vec3_t &g_offset, int damage, qboolean hyper, int effect)
{
    vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;

    if (is_quad)
        damage *= 4;
    AngleVectors(ent->client->v_angle, &forward, &right, NULL);
    VectorSet(offset, 24, 8, ent->viewHeight - 8);
    VectorAdd(offset, g_offset, offset);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -1;

    fire_blaster(ent, start, forward, damage, 1000, effect, hyper);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    if (hyper)
        gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
    else
        gi.WriteByte(MZ_BLASTER | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire(entity_t* ent)
{
    int     damage;

    if (deathmatch->value)
        damage = 15;
    else
        damage = 10;
    Blaster_Fire(ent, vec3_origin, damage, false, EF_BLASTER);
    ent->client->playerState.gunframe++;
}

void Weapon_Blaster(entity_t* ent)
{
    static int  pause_frames[] = { 19, 32, 0 };
    static int  fire_frames[] = { 5, 0 };

    Weapon_Generic(ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}