// LICENSE HERE.

//
// svgame/weapons/railgun.c
//
//
// Railgun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include railgun weapon header.
#include "railgun.h"


//
//======================================================================
//
//RAILGUN
//
//======================================================================
//

void weapon_railgun_fire(entity_t* ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    int         damage;
    int         kick;

    if (deathmatch->value) {
        // normal damage is too extreme in dm
        damage = 100;
        kick = 200;
    }
    else {
        damage = 150;
        kick = 250;
    }

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    AngleVectors(ent->client->v_angle, &forward, &right, NULL);

    VectorScale(forward, -3, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -3;

    VectorSet(offset, 0, 7, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);
    fire_rail(ent, start, forward, damage, kick);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_RAILGUN | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    ent->client->playerState.gunframe++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun(entity_t* ent)
{
    static int  pause_frames[] = { 56, 0 };
    static int  fire_frames[] = { 4, 0 };

    Weapon_Generic(ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}