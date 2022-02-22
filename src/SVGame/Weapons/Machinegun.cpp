// LICENSE HERE.

//
// svgame/weapons/machinegun.c
//
//
// Machinegun/Chaingun weapon code.
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

// Include machinegun weapon header.
#include "Machinegun.h"

//
//======================================================================
//
// MACHINEGUN
//
//======================================================================
//
static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_HSPREAD = 300;
static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_VSPREAD = 500;

void Machinegun_Fire(SVGBasePlayer* ent)
{
    int32_t i;
    vec3_t start;
    vec3_t forward, right;

    int32_t damage = 8;
    int32_t kick = 2;


    // Get the client.
    ServerClient* client = ent->GetClient();

    if (!(client->buttons & ButtonBits::Attack)) {
        client->machinegunShots = 0;
        client->playerState.gunAnimationFrame++;
        return;
    }

    if (client->playerState.gunAnimationFrame == 5)
        client->playerState.gunAnimationFrame = 4;
    else
        client->playerState.gunAnimationFrame = 5;

    if (client->persistent.inventory[client->ammoIndex] < 1) {
        client->playerState.gunAnimationFrame = 6;
        if (level.time >= ent->GetDebouncePainTime()) {
            gi.Sound(ent->GetServerEntity(), CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
            ent->SetDebouncePainTime(level.time + 1);
        }
        NoAmmoWeaponChange(ent);
        return;
    }

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    for (i = 1; i < 3; i++) {
        client->kickOrigin[i] = crandom() * 0.35;
        client->kickAngles[i] = crandom() * 0.7;
    }
    client->kickOrigin[0] = crandom() * 0.35;
    client->kickAngles[0] = client->machinegunShots * -1.5;

    // raise the gun as it is firing if not in deathmatch mode.
    if (!game.GetGamemode()->IsClass<DeathmatchGamemode>()) {
        client->machinegunShots++;
        if (client->machinegunShots > 9)
            client->machinegunShots = 9;
    }

    // get start / end positions
    vec3_t angles = client->aimAngles + client->kickAngles;
    vec3_vectors(angles, &forward, &right, NULL);
    vec3_t offset = {
        0.f, 8, (float)ent->GetViewHeight() - 8.f
    };
    start = SVG_PlayerProjectSource(client, ent->GetOrigin(), offset, forward, right);
    SVG_FireBullet(ent, start, forward, damage, kick, DEFAULT_MACHINEGUN_BULLET_HSPREAD, DEFAULT_MACHINEGUN_BULLET_VSPREAD, MeansOfDeath::Machinegun);

    gi.WriteByte(ServerGameCommands::MuzzleFlash);
    gi.WriteShort(ent->GetServerEntity() - game.world->GetServerEntities());
    gi.WriteByte(MuzzleFlashType::MachineGun | is_silenced);
    vec3_t origin = ent->GetOrigin();
    gi.Multicast(origin, Multicast::PVS);

    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)gamemodeflags->value & GamemodeFlags::InfiniteAmmo))
        client->persistent.inventory[client->ammoIndex]--;

    client->animation.priorityAnimation = PlayerAnimation::Attack;
    if (client->playerState.pmove.flags & PMF_DUCKED) {
        ent->SetAnimationFrame(FRAME_crattak1 - (int)(random() + 0.25));
        client->animation.endFrame = FRAME_crattak9;
    }
    else {
        ent->SetAnimationFrame(FRAME_attack1 - (int)(random() + 0.25));
        client->animation.endFrame = FRAME_attack8;
    }
}

void Weapon_Machinegun(SVGBasePlayer* ent)
{
    static int  pause_frames[] = { 23, 45, 0 };
    static int  fire_frames[] = { 4, 5, 0 };

//    Weapon_Generic(ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}