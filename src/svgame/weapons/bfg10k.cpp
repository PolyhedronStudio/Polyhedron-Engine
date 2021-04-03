// LICENSE HERE.

//
// svgame/weapons/bfg10k.c
//
//
// BFG10K weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include weapon header.
#include "bfg10k.h"

// This is actually declared in grenade.c
void weapon_grenade_fire(edict_t* ent, qboolean held);

//
//======================================================================
//
// BFG10K
//
//======================================================================
//

void weapon_bfg_fire(edict_t* ent)
{
    vec3_t  offset, start;
    vec3_t  forward, right;
    int     damage;
    float   damage_radius = 1000;

    if (deathmatch->value)
        damage = 200;
    else
        damage = 500;

    if (ent->client->ps.gunframe == 9) {
        // send muzzle flash
        gi.WriteByte(svg_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MZ_BFG | is_silenced);
        gi.Multicast(&ent->s.origin, MULTICAST_PVS);

        ent->client->ps.gunframe++;

        PlayerNoise(ent, start, PNOISE_WEAPON);
        return;
    }

    // cells can go down during windup (from power armor hits), so
    // check again and abort firing if we don't have enough now
    if (ent->client->pers.inventory[ent->client->ammo_index] < 50) {
        ent->client->ps.gunframe++;
        return;
    }

    if (is_quad)
        damage *= 4;

    AngleVectors(ent->client->v_angle, &forward, &right, NULL);

    VectorScale(forward, -2, ent->client->kickOrigin);

    // make a big pitch kick with an inverse fall
    ent->client->v_dmg_pitch = -40;
    ent->client->v_dmg_roll = crandom() * 8;
    ent->client->v_dmg_time = level.time + DAMAGE_TIME;

    VectorSet(offset, 8, 8, ent->viewheight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);
    fire_bfg(ent, start, forward, damage, 400, damage_radius);

    ent->client->ps.gunframe++;

    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG(edict_t* ent)
{
    static int  pause_frames[] = { 39, 45, 50, 55, 0 };
    static int  fire_frames[] = { 9, 17, 0 };

    Weapon_Generic(ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}
