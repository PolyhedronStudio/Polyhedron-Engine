// LICENSE HERE.

//
// svgame/weapons/shotgun.c
//
//
// Shotgun weapon code.
//

// Include local game header.
#include "../ServerGameLocal.h"

// Include class entities.
#include "../Entities/Base/SVGBaseEntity.h"
#include "../Entities/Base/SVGBasePlayer.h"

// Include player headers.
#include "../Player/Animations.h"
#include "../Player/Weapons.h"

// Gamemodes.
#include "../Gamemodes/IGamemode.h"
//#include "../Gamemodes/DefaultGamemode.h"
//#include "../Gamemodes/CoopGamemode.h"
#include "../Gamemodes/DeathmatchGamemode.h"

// World.
#include "../World/Gameworld.h"

// Include shotgun weapon header.
#include "Shotgun.h"


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

void weapon_shotgun_fire(SVGBasePlayer * ent)
{
    vec3_t      forward, right;
    int         damage = 4;
    int         kick = 8;

    ServerClient* client = ent->GetClient();

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

    // Use a different count for shotgun mode.
    if (game.GetGamemode()->IsClass<DeathmatchGamemode>()) {
        SVG_FireShotgun(ent, start, forward, damage, kick, SHOTGUN_HSPREAD, SHOTGUN_VSPREAD, SHOTGUN_BULLET_COUNT_DEATHMATCH, MeansOfDeath::Shotgun);
    } else {
        SVG_FireShotgun(ent, start, forward, damage, kick, SHOTGUN_HSPREAD, SHOTGUN_VSPREAD, SHOTGUN_BULLET_COUNT_DEFAULT, MeansOfDeath::Shotgun);
    }

    // send muzzle flash
    gi.WriteByte(ServerGameCommands::MuzzleFlash);
    gi.WriteShort(ent->GetServerEntity() - game.world->GetServerEntities());
    gi.WriteByte(MuzzleFlashType::Shotgun | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, MultiCast::PVS);

    client->playerState.gunFrame++;
    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)gamemodeflags->value & GamemodeFlags::InfiniteAmmo))
        client->persistent.inventory[client->ammoIndex]--;
}

void Weapon_Shotgun(SVGBasePlayer* ent)
{
    static int  pause_frames[] = { 22, 28, 34, 0 };
    static int  fire_frames[] = { 8, 9, 0 };

    _Weapon_Generic(ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}