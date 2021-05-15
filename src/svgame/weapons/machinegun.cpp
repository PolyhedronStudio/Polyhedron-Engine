// LICENSE HERE.

//
// svgame/weapons/machinegun.c
//
//
// Machinegun/Chaingun weapon code.
//

// Include local game header.
#include "../g_local.h"

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

void Machinegun_Fire(Entity* ent)
{
    int i;
    vec3_t      start;
    vec3_t      forward, right;
    vec3_t      angles;
    int         damage = 8;
    int         kick = 2;
    vec3_t      offset;

    if (!(ent->client->buttons & BUTTON_ATTACK)) {
        ent->client->machinegunShots = 0;
        ent->client->playerState.gunFrame++;
        return;
    }

    if (ent->client->playerState.gunFrame == 5)
        ent->client->playerState.gunFrame = 4;
    else
        ent->client->playerState.gunFrame = 5;

    if (ent->client->persistent.inventory[ent->client->ammoIndex] < 1) {
        ent->client->playerState.gunFrame = 6;
        if (level.time >= ent->debouncePainTime) {
            gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
            ent->debouncePainTime = level.time + 1;
        }
        NoAmmoWeaponChange(ent);
        return;
    }

    if (is_quad) {
        damage *= 4;
        kick *= 4;
    }

    for (i = 1; i < 3; i++) {
        ent->client->kickOrigin[i] = crandom() * 0.35;
        ent->client->kickAngles[i] = crandom() * 0.7;
    }
    ent->client->kickOrigin[0] = crandom() * 0.35;
    ent->client->kickAngles[0] = ent->client->machinegunShots * -1.5;

    // raise the gun as it is firing
    if (!deathmatch->value) {
        ent->client->machinegunShots++;
        if (ent->client->machinegunShots > 9)
            ent->client->machinegunShots = 9;
    }

    // get start / end positions
    VectorAdd(ent->client->aimAngles, ent->client->kickAngles, angles);
    AngleVectors(angles, &forward, &right, NULL);
    VectorSet(offset, 0, 8, ent->viewHeight - 8);
    start = SVG_PlayerProjectSource(ent->client, ent->state.origin, offset, forward, right);
    SVG_FireBullet(ent, start, forward, damage, kick, DEFAULT_MACHINEGUN_BULLET_HSPREAD, DEFAULT_MACHINEGUN_BULLET_VSPREAD, MeansOfDeath::Machinegun);

    gi.WriteByte(SVG_CMD_MUZZLEFLASH);
    gi.WriteShort(ent - g_entities);
    gi.WriteByte(MuzzleFlashType::MachineGun | is_silenced);
    gi.Multicast(&ent->state.origin, MultiCast::PVS);

    SVG_PlayerNoise(ent, start, PNOISE_WEAPON);

    if (!((int)dmflags->value & DeathMatchFlags::InfiniteAmmo))
        ent->client->persistent.inventory[ent->client->ammoIndex]--;

    ent->client->animation.priorityAnimation = PlayerAnimation::Attack;
    if (ent->client->playerState.pmove.flags & PMF_DUCKED) {
        ent->state.frame = FRAME_crattak1 - (int)(random() + 0.25);
        ent->client->animation.endFrame = FRAME_crattak9;
    }
    else {
        ent->state.frame = FRAME_attack1 - (int)(random() + 0.25);
        ent->client->animation.endFrame = FRAME_attack8;
    }
}

void Weapon_Machinegun(Entity* ent)
{
    static int  pause_frames[] = { 23, 45, 0 };
    static int  fire_frames[] = { 4, 5, 0 };

    Weapon_Generic(ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}