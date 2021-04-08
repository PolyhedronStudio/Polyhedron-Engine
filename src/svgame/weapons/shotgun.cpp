// LICENSE HERE.

//
// svgame/weapons/shotgun.c
//
//
// Shotgun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include shotgun weapon header.
#include "shotgun.h"


//
//======================================================================
//
// SHOTGUN
//
//======================================================================
//

void weapon_shotgun_fire(entity_t* ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    int         damage = 4;
    int         kick = 8;

    if (ent->client->playerState.gunframe == 9) {
        ent->client->playerState.gunframe++;
        return;
    }

    AngleVectors(ent->client->v_angle, &forward, &right, NULL);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -2;

    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    if (deathmatch->value)
        fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
    else
        fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_SHOTGUN | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    ent->client->playerState.gunframe++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun(entity_t* ent)
{
    static int  pause_frames[] = { 22, 28, 34, 0 };
    static int  fire_frames[] = { 8, 9, 0 };

    Weapon_Generic(ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}