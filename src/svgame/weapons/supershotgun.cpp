// LICENSE HERE.

//
// svgame/weapons/supershotgun.c
//
//
// Super Shotgun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include super shotgun weapon header.
#include "supershotgun.h"

//
//======================================================================
//
// SUPERSHOTGUN
//
//======================================================================
//
void weapon_supershotgun_fire(entity_t* ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      offset;
    vec3_t      v;
    int         damage = 6;
    int         kick = 12;

    vec3_vectors(ent->client->v_angle, &forward, &right, NULL);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -2;

    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    v[vec3_t::Pitch] = ent->client->v_angle[vec3_t::Pitch];
    v[vec3_t::Yaw] = ent->client->v_angle[vec3_t::Yaw] - 5;
    v[vec3_t::Roll] = ent->client->v_angle[vec3_t::Roll];
    vec3_vectors(v, &forward, NULL, NULL);
    Fire_Shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);
    v[vec3_t::Yaw] = ent->client->v_angle[vec3_t::Yaw] + 5;
    vec3_vectors(v, &forward, NULL, NULL);
    Fire_Shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MZ_SSHOTGUN | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    ent->client->playerState.gunframe++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DF_INFINITE_AMMO))
        ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun(entity_t* ent)
{
    static int  pause_frames[] = { 29, 42, 57, 0 };
    static int  fire_frames[] = { 7, 0 };

    Weapon_Generic(ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}