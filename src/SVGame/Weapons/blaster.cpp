// LICENSE HERE.

//
// svgame/weapons/blaster.c
//
//
// Blaster weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include class entities.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

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

void Blaster_Fire(PlayerClient* ent, const vec3_t &g_offset, int damage, qboolean hyper, int effect)
{
    vec3_t  forward, right;
    vec3_t  start;

    ServersClient* client = ent->GetClient();
    if (is_quad)
        damage *= 4;
    AngleVectors(client->aimAngles, &forward, &right, NULL);
    vec3_t offset = { 
        24.f, 8.f, (float)ent->GetViewHeight() + 8.f
    };
    VectorAdd(offset, g_offset, offset);
    start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);

    client->kickOrigin = vec3_scale(forward, -2);
    client->kickAngles[0] = -1;

    SVG_FireBlaster(ent, start, forward, damage, 1000, effect, hyper);

    // send muzzle flash
    gi.WriteByte(SVG_CMD_MUZZLEFLASH);
    gi.WriteShort(ent->GetServerEntity() - g_entities);
    gi.WriteByte(MuzzleFlashType::Blaster | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, MultiCast::PVS);

    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire(PlayerClient *ent)
{
    int     damage;

    if (deathmatch->value)
        damage = 15;
    else
        damage = 10;
    Blaster_Fire(ent, vec3_zero(), damage, false, EntityEffectType::Blaster);
    ent->GetClient()->playerState.gunFrame++;
}

void Weapon_Blaster(PlayerClient* ent)
{
    static int  pause_frames[] = { 19, 32, 0 };
    static int  fire_frames[] = { 5, 0 };

    Weapon_Generic(ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}