// LICENSE HERE.

//
// svgame/weapons/supershotgun.c
//
//
// Super Shotgun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include class entities.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

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
static constexpr int32_t DEFAULT_SUPERSHOTGUN_HSPREAD = 1000;
static constexpr int32_t DEFAULT_SUPERSHOTGUN_VSPREAD = 500;

static constexpr int32_t DEFAULT_SUPERSHOTGUN_COUNT = 20;

void weapon_supershotgun_fire(PlayerClient * ent)
{
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      v;
    int         damage = 6;
    int         kick = 12;

    ServersClient* client = ent->GetClient();

    vec3_vectors(client->aimAngles, &forward, &right, NULL);

    client->kickOrigin = vec3_scale(forward, -2);
    client->kickAngles[0] = -2;

    vec3_t offset = {
        0.f, 8.f, (float)ent->GetViewHeight() - 8.f
    };
    start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    v[vec3_t::PYR::Pitch] = client->aimAngles[vec3_t::PYR::Pitch];
    v[vec3_t::PYR::Yaw] = client->aimAngles[vec3_t::PYR::Yaw] - 5;
    v[vec3_t::PYR::Roll] = client->aimAngles[vec3_t::PYR::Roll];
    vec3_vectors(v, &forward, NULL, NULL);
    SVG_FireShotgun(ent, start, forward, damage, kick, DEFAULT_SUPERSHOTGUN_HSPREAD, DEFAULT_SUPERSHOTGUN_VSPREAD, DEFAULT_SUPERSHOTGUN_COUNT / 2, MeansOfDeath::SuperShotgun);
    v[vec3_t::PYR::Yaw] = client->aimAngles[vec3_t::PYR::Yaw] + 5;
    vec3_vectors(v, &forward, NULL, NULL);
    SVG_FireShotgun(ent, start, forward, damage, kick, DEFAULT_SUPERSHOTGUN_HSPREAD, DEFAULT_SUPERSHOTGUN_VSPREAD, DEFAULT_SUPERSHOTGUN_COUNT / 2, MeansOfDeath::SuperShotgun);

    // send muzzle flash
    gi.WriteByte(SVG_CMD_MUZZLEFLASH);
    gi.WriteShort(ent->GetServerEntity() - g_entities);
    gi.WriteByte(MuzzleFlashType::SuperShotgun | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, MultiCast::PVS);

    client->playerState.gunFrame++;
    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)gamemodeflags->value & GameModeFlags::InfiniteAmmo))
        client->persistent.inventory[client->ammoIndex] -= 2;
}

void Weapon_SuperShotgun(PlayerClient* ent)
{
    static int  pause_frames[] = { 29, 42, 57, 0 };
    static int  fire_frames[] = { 7, 0 };

    Weapon_Generic(ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}