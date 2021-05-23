// LICENSE HERE.

//
// svgame/weapons/machinegun.c
//
//
// Machinegun/Chaingun weapon code.
//

// Include local game header.
#include "../g_local.h"

// ClassEntities.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include machinegun weapon header.
#include "machinegun.h"

//
//======================================================================
//
// MACHINEGUN
//
//======================================================================
//
static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_HSPREAD = 300;
static constexpr int32_t DEFAULT_MACHINEGUN_BULLET_VSPREAD = 500;

void Machinegun_Fire(PlayerClient* ent)
{
    int i;
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      angles;
    int         damage = 8;
    int         kick = 2;
    vec3_t      offset;

    GameClient* client = ent->GetClient();

    if (!(client->buttons & BUTTON_ATTACK)) {
        client->machinegunShots = 0;
        client->playerState.gunFrame++;
        return;
    }

    if (client->playerState.gunFrame == 5)
        client->playerState.gunFrame = 4;
    else
        client->playerState.gunFrame = 5;

    if (client->persistent.inventory[client->ammoIndex] < 1) {
        client->playerState.gunFrame = 6;
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

    // raise the gun as it is firing
    if (!deathmatch->value) {
        client->machinegunShots++;
        if (client->machinegunShots > 9)
            client->machinegunShots = 9;
    }

    // get start / end positions
    VectorAdd(client->aimAngles, client->kickAngles, angles);
    AngleVectors(angles, &forward, &right, NULL);
    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = SVG_PlayerProjectSource(client, ent->state.origin, offset, forward, right);
    SVG_FireBullet(ent->classEntity, start, forward, damage, kick, DEFAULT_MACHINEGUN_BULLET_HSPREAD, DEFAULT_MACHINEGUN_BULLET_VSPREAD, MeansOfDeath::Machinegun);

    gi.WriteByte(SVG_CMD_MUZZLEFLASH);
    gi.WriteShort(ent - g_entities);
    gi.WriteByte(MuzzleFlashType::MachineGun | is_silenced);
    gi.Multicast(&ent->state.origin, MultiCast::PVS);

    SVG_PlayerNoise(ent->classEntity, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DeathMatchFlags::InfiniteAmmo))
        client->persistent.inventory[client->ammoIndex]--;

    client->animation.priorityAnimation = PlayerAnimation::Attack;
    if (client->playerState.pmove.flags & PMF_DUCKED) {
        ent->state.frame = FRAME_crattak1 - (int)(random() + 0.25);
        client->animation.endFrame = FRAME_crattak9;
    }
    else {
        ent->state.frame = FRAME_attack1 - (int)(random() + 0.25);
        client->animation.endFrame = FRAME_attack8;
    }
}

void Weapon_Machinegun(Entity* ent)
{
    static int  pause_frames[] = { 23, 45, 0 };
    static int  fire_frames[] = { 4, 5, 0 };

    Weapon_Generic(ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}