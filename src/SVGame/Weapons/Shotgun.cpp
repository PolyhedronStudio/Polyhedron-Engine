// LICENSE HERE.

//
// svgame/weapons/shotgun.c
//
//
// Shotgun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include class entities.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

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
static constexpr int32_t SHOTGUN_BULLET_COUNT_DEATHMATCH = 12;
static constexpr int32_t SHOTGUN_BULLET_COUNT_DEFAULT = 12;

static constexpr int32_t SHOTGUN_HSPREAD = 500;
static constexpr int32_t SHOTGUN_VSPREAD = 500;

void weapon_shotgun_fire(PlayerClient * ent)
{
    vec3_t      forward, right;
    int         damage = 4;
    int         kick = 8;

    ServersClient* client = ent->GetClient();

    if (client->playerState.gunFrame == 9) {
        client->playerState.gunFrame++;
        return;
    }

    vec3_vectors(client->aimAngles, &forward, &right, NULL);

    client->kickOrigin = vec3_scale(forward, -2);
    client->kickAngles[0] = -2;

    vec3_t offset = {
        0.f, 8.f, ent->GetViewHeight() - 8.f
    };
    vec3_t start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    if (deathmatch->value)
        SVG_FireShotgun(ent, start, forward, damage, kick, SHOTGUN_HSPREAD, SHOTGUN_VSPREAD, SHOTGUN_BULLET_COUNT_DEATHMATCH, MeansOfDeath::Shotgun);
    else
        SVG_FireShotgun(ent, start, forward, damage, kick, SHOTGUN_HSPREAD, SHOTGUN_VSPREAD, SHOTGUN_BULLET_COUNT_DEFAULT, MeansOfDeath::Shotgun);

    // send muzzle flash
    gi.WriteByte(SVG_CMD_MUZZLEFLASH);
    gi.WriteShort(ent->GetServerEntity() - g_entities);
    gi.WriteByte(MuzzleFlashType::Shotgun | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, MultiCast::PVS);

    client->playerState.gunFrame++;
    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)gamemodeflags->value & GameModeFlags::InfiniteAmmo))
        client->persistent.inventory[client->ammoIndex]--;
}

void Weapon_Shotgun(PlayerClient* ent)
{
    static int  pause_frames[] = { 22, 28, 34, 0 };
    static int  fire_frames[] = { 8, 9, 0 };

    Weapon_Generic(ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}