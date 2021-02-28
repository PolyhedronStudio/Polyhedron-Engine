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

//
//======================================================================
//
// SHOTGUN
//
//======================================================================
//

void weapon_shotgun_fire(edict_t* ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    int         damage = 4;
    int         kick = 8;

    if (ent->client->ps.gunframe == 9) {
        ent->client->ps.gunframe++;
        return;
    }

    AngleVectors(ent->client->v_angle, forward, right, NULL);

    VectorScale(forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -2;

    VectorSet(offset, 0, 8, ent->viewheight - 8);
    P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

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
    gi.multicast(ent->s.origin, MULTICAST_PVS);

    ent->client->ps.gunframe++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun(edict_t* ent)
{
    static int  pause_frames[] = { 22, 28, 34, 0 };
    static int  fire_frames[] = { 8, 9, 0 };

    Weapon_Generic(ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}