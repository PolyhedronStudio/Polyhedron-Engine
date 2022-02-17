// LICENSE HERE.

//
// svgame/weapons/blaster.c
//
//
// Blaster weapon code.
//

// Include local game header.
#include "../ServerGameLocal.h"

// Include class entities.
#include "../Entities/Base/SVGBaseEntity.h"
#include "../Entities/Base/SVGBasePlayer.h"

// Include player headers.
#include "../Player/Animations.h"
#include "../Player/Weapons.h"

// Game modes.
#include "../Gamemodes/IGameMode.h"
#include "../Gamemodes/DeathMatchGamemode.h"

// World.
#include "../World/Gameworld.h"

// Include weapon header.
#include "Blaster.h"

//
//======================================================================
//
// BLASTER
//
//======================================================================
//

void Blaster_Fire(SVGBasePlayer* ent, const vec3_t &g_offset, int damage, qboolean hyper, int effect)
{
    //vec3_t  forward, right;
    //vec3_t  start;

    //ServerClient* client = ent->GetClient();
    //if (is_quad)
    //    damage *= 4;
    //AngleVectors(client->aimAngles, &forward, &right, NULL);
    //vec3_t offset = { 
    //    24.f, 8.f, (float)ent->GetViewHeight() + 8.f
    //};
    //VectorAdd(offset, g_offset, offset);
    //start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);

    //client->kickOrigin = vec3_scale(forward, -2);
    //client->kickAngles[0] = -1;

    //SVG_FireBlaster(ent, start, forward, damage, 1000, effect, hyper);

    //// send muzzle flash
    //gi.WriteByte(ServerGameCommands::MuzzleFlash);
    //gi.WriteShort(ent->GetServerEntity() - g_entities);
    //gi.WriteByte(MuzzleFlashType::Blaster | is_silenced);
    //vec3_t origin = ent->GetOrigin();
    //gi.Multicast(origin, MultiCast::PVS);
    static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_HSPREAD = 300;
    static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_VSPREAD = 500;

    vec3_t  forward, right;
    vec3_t  start;

    ServerClient* client = ent->GetClient();

    if (!client) {
        return;
    }

    if (is_quad)
        damage *= 4;
    int32_t kick = 2;
    //// get start / end positions
    vec3_t angles = client->aimAngles + client->kickAngles;
    vec3_vectors(angles, &forward, &right, NULL);
    vec3_t offset = { 0.f, 0, (float)ent->GetViewHeight() - 8.f };
    start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);

    client->kickOrigin = vec3_scale(forward, -2);
    client->kickAngles[0] = -1;


    //SVG_FireBlaster(ent, start, forward, damage, 1000, effect, hyper);
    
    SVG_FireBullet(ent, start, forward, damage, kick, DEFAULT_MACHINEGUN_BULLET_HSPREAD, DEFAULT_MACHINEGUN_BULLET_VSPREAD, MeansOfDeath::Machinegun);

    // send muzzle flash
    gi.WriteByte(ServerGameCommands::MuzzleFlash);
    gi.WriteShort(ent->GetServerEntity() - game.world->GetServerEntities());
    gi.WriteByte(MuzzleFlashType::Blaster | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, MultiCast::PVS);


    //int32_t i;
    //vec3_t start;
    //vec3_t forward, right;

    //int32_t damage = 8;
    //int32_t kick = 2;


    //// Get the client.
    //if (!ent) {
    //    return;
    //}

    //ServerClient* client = ent->GetClient();

    //if (!(client->buttons & ButtonBits::Attack)) {
    //    client->machinegunShots = 0;
    //    client->playerState.gunFrame++;
    //    return;
    //}

    //if (client->playerState.gunFrame == 5)
    //    client->playerState.gunFrame = 4;
    //else
    //    client->playerState.gunFrame = 5;

    //if (client->persistent.inventory[client->ammoIndex] < 1) {
    //    client->playerState.gunFrame = 6;
    //    if (level.time >= ent->GetDebouncePainTime()) {
    //        gi.Sound(ent->GetServerEntity(), CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
    //        ent->SetDebouncePainTime(level.time + 1);
    //    }
    //    NoAmmoWeaponChange(ent);
    //    return;
    //}

    //if (is_quad) {
    //    damage *= 4;
    //    kick *= 4;
    //}

    //for (i = 1; i < 3; i++) {
    //    client->kickOrigin[i] = crandom() * 0.35;
    //    client->kickAngles[i] = crandom() * 0.7;
    //}
    //client->kickOrigin[0] = crandom() * 0.35;
    //client->kickAngles[0] = client->machinegunShots * -1.5;

    //// raise the gun as it is firing if not in deathmatch mode.
    //if (!game.GetGamemode()->IsClass<DeathmatchGamemode>()) {
    //    client->machinegunShots++;
    //    if (client->machinegunShots > 9)
    //        client->machinegunShots = 9;
    //}

    //// get start / end positions
    //vec3_t angles = client->aimAngles + client->kickAngles;
    //vec3_vectors(angles, &forward, &right, NULL);
    //vec3_t offset = {
    //    0.f, 8, (float)ent->GetViewHeight() - 8.f
    //};
    //start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);
    //SVG_FireBullet(ent, start, forward, damage, kick, DEFAULT_MACHINEGUN_BULLET_HSPREAD, DEFAULT_MACHINEGUN_BULLET_VSPREAD, MeansOfDeath::Machinegun);

    //SVG_PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire(SVGBasePlayer *ent)
{
    int     damage;

    if (game.GetGamemode()->IsClass<DeathmatchGamemode>())
        damage = 15;
    else
        damage = 10;
    Blaster_Fire(ent, vec3_zero(), damage, false, EntityEffectType::Blaster);
    ent->GetClient()->playerState.gunFrame++;
}

void Weapon_Blaster(SVGBasePlayer* ent)
{
    static int  pause_frames[] = { 160 };
    static int  fire_frames[] = { 119 };

    //Weapon_Generic(ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
    Weapon_Generic(ent, 136, 160, 114, 124, 160, 160, 124, 134, pause_frames, fire_frames, Weapon_Blaster_Fire);
}