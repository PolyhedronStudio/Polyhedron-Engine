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

    vec3_vectors(ent->client->aimAngles, &forward, &right, NULL);

    ent->client->kickOrigin = vec3_scale(forward, -2);
    ent->client->kickAngles[0] = -2;

    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->state.origin, offset, forward, right);

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    v[vec3_t::PYR::Pitch] = ent->client->aimAngles[vec3_t::PYR::Pitch];
    v[vec3_t::PYR::Yaw] = ent->client->aimAngles[vec3_t::PYR::Yaw] - 5;
    v[vec3_t::PYR::Roll] = ent->client->aimAngles[vec3_t::PYR::Roll];
    vec3_vectors(v, &forward, NULL, NULL);
    fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);
    v[vec3_t::PYR::Yaw] = ent->client->aimAngles[vec3_t::PYR::Yaw] + 5;
    vec3_vectors(v, &forward, NULL, NULL);
    fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

    // send muzzle flash
    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MuzzleFlashType::SuperShotgun | is_silenced);
    gi.Multicast(&ent->state.origin, MultiCast::PVS);

    ent->client->playerState.gunFrame++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DeathMatchFlags::InfiniteAmmo))
        ent->client->persistent.inventory[ent->client->ammoIndex] -= 2;
}

void Weapon_SuperShotgun(entity_t* ent)
{
    static int  pause_frames[] = { 29, 42, 57, 0 };
    static int  fire_frames[] = { 7, 0 };

    Weapon_Generic(ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}