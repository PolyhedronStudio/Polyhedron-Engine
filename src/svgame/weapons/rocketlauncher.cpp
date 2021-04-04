// LICENSE HERE.

//
// svgame/weapons/rocketlauncher.c
//
//
// Rocket Launcher weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include rocket launcher weapon header.
#include "rocketlauncher.h"


//
//======================================================================
//
//ROCKET
//
//======================================================================
//

void Weapon_RocketLauncher_Fire(entity_t* ent)
{
    vec3_t  offset, start;
    vec3_t  forward, right;
    int     damage;
    float   damage_radius;
    int     radius_damage;

    damage = 100 + (int)(random() * 20.0);
    radius_damage = 120;
    damage_radius = 120;
    if (is_quad) {
        damage *= 4;
        radius_damage *= 4;
    }

    vec3_vectors(ent->client->v_angle, &forward, &right, NULL);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -1;

    VectorSet(offset, 8, 8, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);
    Fire_Rocket(ent, start, forward, damage, 650, damage_radius, radius_damage);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_ROCKET | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    ent->client->playerState.gunframe++;

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher(entity_t* ent)
{
    static int  pause_frames[] = { 25, 33, 42, 50, 0 };
    static int  fire_frames[] = { 5, 0 };

    Weapon_Generic(ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}
