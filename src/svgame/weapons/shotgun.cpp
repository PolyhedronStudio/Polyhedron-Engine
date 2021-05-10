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

    if (ent->client->playerState.gunFrame == 9) {
        ent->client->playerState.gunFrame++;
        return;
    }

    vec3_vectors(ent->client->aimAngles, &forward, &right, NULL);

    ent->client->kickOrigin = vec3_scale(forward, -2);
    ent->client->kickAngles[0] = -2;

    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = P_ProjectSource(ent->client, ent->state.origin, offset, forward, right);

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
    gi.WriteByte(MuzzleFlashType::Shotgun | is_silenced);
    gi.Multicast(&ent->state.origin, MultiCast::PVS);

    ent->client->playerState.gunFrame++;
    PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DeathMatchFlags::InfiniteAmmo))
        ent->client->persistent.inventory[ent->client->ammoIndex]--;
}

void Weapon_Shotgun(entity_t* ent)
{
    static int  pause_frames[] = { 22, 28, 34, 0 };
    static int  fire_frames[] = { 8, 9, 0 };

    Weapon_Generic(ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}