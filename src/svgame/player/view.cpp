/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "../g_local.h"
#include "../entities.h"
#include "../entities/base/SVGBaseEntity.h"
#include "hud.h"
#include "animations.h"



static  Entity     *current_player;
static  GameClient   *current_client;

static  vec3_t  forward, right, up;
static float   xyspeed;

static float   bobmove;
static int     bobcycle;       // odd cycles are right foot going forward
static float   bobfracsin;     // sin(bobfrac*M_PI)

//
//===============
// SVG_CalcRoll
// 
//
//===============
//
static float SVG_CalcRoll(vec3_t angles, vec3_t velocity)
{
    float   sign;
    float   side;
    float   value;

    side = vec3_dot(velocity, right);
    sign = side < 0 ? -1 : 1;
    side = fabs(side);

    value = sv_rollangle->value;

    if (side < sv_rollspeed->value)
        side = side * value / sv_rollspeed->value;
    else
        side = value;

    return side * sign;

}

//
//===============
// SVG_Player_ApplyDamageFeedback
// 
// Handles color blends and view kicks
//===============
//
static void SVG_Player_ApplyDamageFeedback(Entity *player)
{
    GameClient   *client;
    float   side;
    float   realcount, count, kick;
    vec3_t  v;
    int     r, l;
    static  vec3_t  power_color = {0.0f, 1.0f, 0.0f};
    static  vec3_t  acolor = {1.0f, 1.0f, 1.0f};
    static  vec3_t  bcolor = {1.0f, 0.0f, 0.0f};

    client = player->client;

    // flash the backgrounds behind the status numbers
    client->playerState.stats[STAT_FLASHES] = 0;
    if (client->damages.blood)
        client->playerState.stats[STAT_FLASHES] |= 1;
    if (client->damages.armor && !(player->flags & EntityFlags::GodMode))
        client->playerState.stats[STAT_FLASHES] |= 2;

    // total points of damage shot at the player this frame
    count = (client->damages.blood + client->damages.armor + client->damages.powerArmor);
    if (count == 0)
        return;     // didn't take any damage

    // start a pain animation if still in the player model
    if (client->animation.priorityAnimation < PlayerAnimation::Pain && player->state.modelIndex == 255) {
        static int      i;

        client->animation.priorityAnimation = PlayerAnimation::Pain;
        if (client->playerState.pmove.flags & PMF_DUCKED) {
            player->state.frame = FRAME_crpain1 - 1;
            client->animation.endFrame = FRAME_crpain4;
        } else {
            i = (i + 1) % 3;
            switch (i) {
            case 0:
                player->state.frame = FRAME_pain101 - 1;
                client->animation.endFrame = FRAME_pain104;
                break;
            case 1:
                player->state.frame = FRAME_pain201 - 1;
                client->animation.endFrame = FRAME_pain204;
                break;
            case 2:
                player->state.frame = FRAME_pain301 - 1;
                client->animation.endFrame = FRAME_pain304;
                break;
            }
        }
    }

    realcount = count;
    if (count < 10)
        count = 10; // always make a visible effect

    // Play an apropriate pain sound
    if ((level.time > player->debouncePainTime) && !(player->flags & EntityFlags::GodMode)) {
        r = 1 + (rand() & 1);
        player->debouncePainTime = level.time + 0.7f;
        if (player->health < 25)
            l = 25;
        else if (player->health < 50)
            l = 50;
        else if (player->health < 75)
            l = 75;
        else
            l = 100;
        gi.Sound(player, CHAN_VOICE, gi.SoundIndex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
    }

    // The total alpha of the blend is always proportional to count.
    if (client->damageAlpha < 0.f)
        client->damageAlpha = 0.f;
    client->damageAlpha += count * 0.01f;
    if (client->damageAlpha < 0.2f)
        client->damageAlpha = 0.2f;
    if (client->damageAlpha > 0.6f)
        client->damageAlpha = 0.6f;     // don't go too saturated

    // The color of the blend will vary based on how much was absorbed
    // by different armors.
    vec3_t blendColor = vec3_zero();
    if (client->damages.powerArmor)
        blendColor = vec3_fmaf(blendColor, (float)client->damages.powerArmor / realcount, power_color);
    if (client->damages.armor)
        blendColor = vec3_fmaf(blendColor, (float)client->damages.armor / realcount, acolor);
    if (client->damages.blood)
        blendColor = vec3_fmaf(blendColor, (float)client->damages.blood / realcount, bcolor);
    client->damageBlend = blendColor;


    //
    // Calculate view angle kicks
    //
    kick = abs(client->damages.knockBack);
    if (kick && player->health > 0) { // kick of 0 means no view adjust at all
        kick = kick * 100 / player->health;

        if (kick < count * 0.5f)
            kick = count * 0.5f;
        if (kick > 50)
            kick = 50;

        vec3_t kickVec = client->damages.from - player->state.origin;
        kickVec = vec3_normalize(kickVec);

        side = DotProduct(kickVec, right);
        client->viewDamage.roll = kick * side * 0.3f;

        side = -DotProduct(kickVec, forward);
        client->viewDamage.pitch = kick * side * 0.3f;

        client->viewDamage.time = level.time + DAMAGE_TIME;
    }

    //
    // clear totals
    //
    client->damages.blood = 0;
    client->damages.armor = 0;
    client->damages.powerArmor = 0;
    client->damages.knockBack = 0;
}

//
//===============
// SVG_CalculateViewOffset
// 
// Calculates t
//
// fall from 128 : 400 = 160000
// fall from 256 : 580 = 336400
// fall from 384 : 720 = 518400
// fall from 512 : 800 = 640000
// fall from 640 : 960 =
//
// damage = deltavelocity * deltavelocity * 0.0001
// 
//===============
//
static void SVG_CalculateViewOffset(Entity *ent)
{
    float       bob;
    float       ratio;
    float       delta;

    //
    // Calculate new kick angle vales. (
    // 
    // If dead, set a fixed angle and don't add any kick
    if (ent->deadFlag) {
        ent->client->playerState.kickAngles = vec3_zero();

        ent->client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
        ent->client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
        ent->client->playerState.pmove.viewAngles[vec3_t::Yaw] = ent->client->killerYaw;
    } else {
        // Fetch client kick angles.
        vec3_t newKickAngles = ent->client->playerState.kickAngles = ent->client->kickAngles; //ent->client->playerState.kickAngles;

        // Add pitch(X) and roll(Z) angles based on damage kick
        ratio = (ent->client->viewDamage.time - level.time) / DAMAGE_TIME;
        if (ratio < 0) {
            ratio = ent->client->viewDamage.pitch = ent->client->viewDamage.roll = 0;
        }
        newKickAngles[vec3_t::Pitch] += ratio * ent->client->viewDamage.pitch;
        newKickAngles[vec3_t::Roll] += ratio * ent->client->viewDamage.roll;

        // Add pitch based on fall kick
        ratio = (ent->client->fallTime - level.time) / FALL_TIME;
        if (ratio < 0)
            ratio = 0;
        newKickAngles[vec3_t::Pitch] += ratio * ent->client->fallValue;

        // Add angles based on velocity
        delta = vec3_dot(ent->velocity, forward);
        newKickAngles[vec3_t::Pitch] += delta * run_pitch->value;

        delta = vec3_dot(ent->velocity, right);
        newKickAngles[vec3_t::Roll] += delta * run_roll->value;

        // Add angles based on bob
        delta = bobfracsin * bob_pitch->value * xyspeed;
        if (ent->client->playerState.pmove.flags & PMF_DUCKED)
            delta *= 6;     // crouching
        newKickAngles[vec3_t::Pitch] += delta;
        delta = bobfracsin * bob_roll->value * xyspeed;
        if (ent->client->playerState.pmove.flags & PMF_DUCKED)
            delta *= 6;     // crouching
        if (bobcycle & 1)
            delta = -delta;
        newKickAngles[vec3_t::Roll] += delta;

        // Last but not least, assign new kickangles to player state.
        ent->client->playerState.kickAngles = newKickAngles;
    }

    //
    // Calculate new view offset.
    //
    // Start off with the base entity viewheight. (Set by Player Move code.)
    vec3_t newViewOffset = {
        0.f,
        0.f,
        (float)ent->viewHeight
    };
        
    // Add fall impact view punch height.
    ratio = (ent->client->fallTime - level.time) / FALL_TIME;
    if (ratio < 0)
        ratio = 0;
    newViewOffset.z -= ratio * ent->client->fallValue * 0.4f;

    // Add bob height.
    bob = bobfracsin * xyspeed * bob_up->value;
    if (bob > 6)
        bob = 6;
    newViewOffset.z += bob;

    // Add kick offset
    newViewOffset += ent->client->kickOrigin;

    // Clamp the new view offsets, and finally assign them to the player state.
    // Clamping ensures that they never exceed the non visible, but physically 
    // there, player bounding box.
    ent->client->playerState.pmove.viewOffset = vec3_clamp(newViewOffset,
        //{ -14, -14, -22 },
        //{ 14,  14, 30 }
        ent->mins,
        ent->maxs
    );
}

//
//===============
// SVG_CalculateGunOffset
// 
//===============
//
static void SVG_CalculateGunOffset(Entity *ent)
{
    int     i;
    float   delta;

    // gun angles from bobbing
    ent->client->playerState.gunAngles[vec3_t::Roll] = xyspeed * bobfracsin * 0.005;
    ent->client->playerState.gunAngles[vec3_t::Yaw]  = xyspeed * bobfracsin * 0.01;
    if (bobcycle & 1) {
        ent->client->playerState.gunAngles[vec3_t::Roll] = -ent->client->playerState.gunAngles[vec3_t::Roll];
        ent->client->playerState.gunAngles[vec3_t::Yaw]  = -ent->client->playerState.gunAngles[vec3_t::Yaw];
    }

    ent->client->playerState.gunAngles[vec3_t::Pitch] = xyspeed * bobfracsin * 0.005;

    // gun angles from delta movement
    for (i = 0 ; i < 3 ; i++) {
        delta = ent->client->oldViewAngles[i] - ent->client->playerState.pmove.viewAngles[i];
        if (delta > 180)
            delta -= 360;
        if (delta < -180)
            delta += 360;
        if (delta > 45)
            delta = 45;
        if (delta < -45)
            delta = -45;
        if (i == vec3_t::Yaw)
            ent->client->playerState.gunAngles[vec3_t::Roll] += 0.1 * delta;
        ent->client->playerState.gunAngles[i] += 0.2 * delta;
    }

    // gun height
    VectorClear(ent->client->playerState.gunOffset);
//  ent->playerState->gunorigin[2] += bob;

    // gun_x / gun_y / gun_z are development tools
    for (i = 0 ; i < 3 ; i++) {
        ent->client->playerState.gunOffset[i] += forward[i] * (gun_y->value);
        ent->client->playerState.gunOffset[i] += right[i] * gun_x->value;
        ent->client->playerState.gunOffset[i] += up[i] * (-gun_z->value);
    }
}

//
//===============
// SV_AddBlend
// 
//===============
//
static void SV_AddBlend(float r, float g, float b, float a, float *v_blend)
{
    float   a2, a3;

    if (a <= 0)
        return;
    a2 = v_blend[3] + (1 - v_blend[3]) * a; // new total alpha
    a3 = v_blend[3] / a2;   // fraction of color from old

    v_blend[0] = v_blend[0] * a3 + r * (1 - a3);
    v_blend[1] = v_blend[1] * a3 + g * (1 - a3);
    v_blend[2] = v_blend[2] * a3 + b * (1 - a3);
    v_blend[3] = a2;
}

//
//===============
// SVG_CalculateBlend
// 
//===============
//
static void SVG_CalculateBlend(Entity *ent)
{
    int     contents;
    vec3_t  vieworg;

    ent->client->playerState.blend[0] = ent->client->playerState.blend[1] =
                                   ent->client->playerState.blend[2] = ent->client->playerState.blend[3] = 0;

    // add for contents
    VectorAdd(ent->state.origin, ent->client->playerState.pmove.viewOffset, vieworg);
    contents = gi.PointContents(vieworg);

	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
        ent->client->playerState.rdflags |= RDF_UNDERWATER;
    else
        ent->client->playerState.rdflags &= ~RDF_UNDERWATER;

    if (contents & (CONTENTS_SOLID | CONTENTS_LAVA))
        SV_AddBlend(1.0, 0.3, 0.0, 0.6, ent->client->playerState.blend);
    else if (contents & CONTENTS_SLIME)
        SV_AddBlend(0.0, 0.1, 0.05, 0.6, ent->client->playerState.blend);
    else if (contents & CONTENTS_WATER)
        SV_AddBlend(0.5, 0.3, 0.2, 0.4, ent->client->playerState.blend);

    // add for damage
    if (ent->client->damageAlpha > 0)
        SV_AddBlend(ent->client->damageBlend[0], ent->client->damageBlend[1]
                    , ent->client->damageBlend[2], ent->client->damageAlpha, ent->client->playerState.blend);

    if (ent->client->bonusAlpha > 0)
        SV_AddBlend(0.85, 0.7, 0.3, ent->client->bonusAlpha, ent->client->playerState.blend);

    // drop the damage value
    ent->client->damageAlpha -= 0.06;
    if (ent->client->damageAlpha < 0)
        ent->client->damageAlpha = 0;

    // drop the bonus value
    ent->client->bonusAlpha -= 0.1;
    if (ent->client->bonusAlpha < 0)
        ent->client->bonusAlpha = 0;
}

//
//===============
// SVG_Player_CheckFallingDamage
// 
//===============
//
static void SVG_Player_CheckFallingDamage(Entity *ent)
{
    float   delta;
    int     damage;
    vec3_t  dir;

    if (ent->state.modelIndex != 255)
        return;     // not in the player model

    if (ent->classEntity->GetMoveType() == MoveType::NoClip || ent->classEntity->GetMoveType() == MoveType::Spectator)
        return;

    if ((ent->client->oldVelocity[2] < 0) && (ent->velocity[2] > ent->client->oldVelocity[2]) && (!ent->groundEntityPtr)) {
        delta = ent->client->oldVelocity[2];
    } else {
        if (!ent->groundEntityPtr)
            return;
        delta = ent->velocity[2] - ent->client->oldVelocity[2];
    }
    delta = delta * delta * 0.0001;

    // never take falling damage if completely underwater
    if (ent->waterLevel == 3)
        return;
    if (ent->waterLevel == 2)
        delta *= 0.25;
    if (ent->waterLevel == 1)
        delta *= 0.5;

    if (delta < 1)
        return;

    if (delta < 15) {
        ent->state.event = EntityEvent::Footstep;
        return;
    }

    ent->client->fallValue = delta * 0.5;
    if (ent->client->fallValue > 40)
        ent->client->fallValue = 40;
    ent->client->fallTime = level.time + FALL_TIME;

    if (delta > 30) {
        if (ent->health > 0) {
            if (delta >= 55)
                ent->state.event = EntityEvent::FallFar;
            else
                ent->state.event = EntityEvent::Fall;
        }
        ent->debouncePainTime = level.time;   // no normal pain sound
        damage = (delta - 30) / 2;
        if (damage < 1)
            damage = 1;
        VectorSet(dir, 0, 0, 1);

        if (!deathmatch->value || !((int)dmflags->value & DeathMatchFlags::NoFalling))
            SVG_Damage(ent->classEntity, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), dir, ent->state.origin, vec3_origin, damage, 0, 0, MeansOfDeath::Falling);
    } else {
        ent->state.event = EntityEvent::FallShort;
        return;
    }
}

//
//===============
// SVG_Player_CheckWorldEffects
// 
//===============
//
static void SVG_Player_CheckWorldEffects(void)
{
    int         waterlevel, oldWaterLevel;

    if (!current_player->classEntity)
        return;

    if (current_player->classEntity->GetMoveType() == MoveType::NoClip || current_player->classEntity->GetMoveType() == MoveType::Spectator) {
        current_player->airFinishedTime = level.time + 12; // don't need air
        return;
    }

    waterlevel = current_player->waterLevel;
    oldWaterLevel = current_client->oldWaterLevel;
    current_client->oldWaterLevel = waterlevel;

    //
    // if just entered a water volume, play a sound
    //
    if (!oldWaterLevel && waterlevel) {
        SVG_PlayerNoise(current_player->classEntity, current_player->state.origin, PNOISE_SELF);
        if (current_player->waterType & CONTENTS_LAVA)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/lava_in.wav"), 1, ATTN_NORM, 0);
        else if (current_player->waterType & CONTENTS_SLIME)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        else if (current_player->waterType & CONTENTS_WATER)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        current_player->flags |= EntityFlags::InWater;

        // clear damage_debounce, so the pain sound will play immediately
        current_player->debounceDamageTime = level.time - 1;
    }

    //
    // if just completely exited a water volume, play a sound
    //
    if (oldWaterLevel && ! waterlevel) {
        SVG_PlayerNoise(current_player->classEntity, current_player->state.origin, PNOISE_SELF);
        gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
        current_player->flags &= ~EntityFlags::InWater;
    }

    //
    // check for head just going under water
    //
    if (oldWaterLevel != 3 && waterlevel == 3) {
        gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_un.wav"), 1, ATTN_NORM, 0);
    }

    //
    // check for head just coming out of water
    //
    if (oldWaterLevel == 3 && waterlevel != 3) {
        if (current_player->airFinishedTime < level.time) {
            // gasp for air
            gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/gasp1.wav"), 1, ATTN_NORM, 0);
            SVG_PlayerNoise(current_player->classEntity, current_player->state.origin, PNOISE_SELF);
        } else  if (current_player->airFinishedTime < level.time + 11) {
            // just break surface
            gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/gasp2.wav"), 1, ATTN_NORM, 0);
        }
    }

    //
    // check for drowning
    //
    if (waterlevel == 3) {
        // if out of air, start drowning
        if (current_player->airFinishedTime < level.time) {
            // drown!
            if (current_player->client->nextDrownTime < level.time
                && current_player->health > 0) {
                current_player->client->nextDrownTime = level.time + 1;

                // take more damage the longer underwater
                current_player->damage += 2;
                if (current_player->damage > 15)
                    current_player->damage = 15;

                // play a gurp sound instead of a normal pain sound
                if (current_player->health <= current_player->damage)
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/drown1.wav"), 1, ATTN_NORM, 0);
                else if (rand() & 1)
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("*gurp1.wav"), 1, ATTN_NORM, 0);
                else
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("*gurp2.wav"), 1, ATTN_NORM, 0);

                current_player->debouncePainTime = level.time;

                SVG_Damage(current_player->classEntity, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_origin, current_player->state.origin, vec3_origin, current_player->damage, 0, DamageFlags::NoArmorProtection, MeansOfDeath::Water);
            }
        }
    } else {
        current_player->airFinishedTime = level.time + 12;
        current_player->damage = 2;
    }

    //
    // check for sizzle damage
    //
    if (waterlevel && (current_player->waterType & (CONTENTS_LAVA | CONTENTS_SLIME))) {
        if (current_player->waterType & CONTENTS_LAVA) {
            if (current_player->health > 0
                && current_player->debouncePainTime <= level.time) {
                if (rand() & 1)
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/burn1.wav"), 1, ATTN_NORM, 0);
                else
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/burn2.wav"), 1, ATTN_NORM, 0);
                current_player->debouncePainTime = level.time + 1;
            }

            SVG_Damage(current_player->classEntity, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_origin, current_player->state.origin, vec3_origin, 3 * waterlevel, 0, 0, MeansOfDeath::Lava);
        }

        if (current_player->waterType & CONTENTS_SLIME) {
            SVG_Damage(current_player->classEntity, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_origin, current_player->state.origin, vec3_origin, 1 * waterlevel, 0, 0, MeansOfDeath::Slime);
        }
    }
}

//
//===============
// SVG_SetClientEffects
// 
//===============
//
static void SVG_SetClientEffects(Entity *ent)
{
    ent->state.effects = 0;
    ent->state.renderfx = 0;

    if (ent->health <= 0 || level.intermission.time)
        return;

    // show cheaters!!!
    if (ent->flags & EntityFlags::GodMode) {
        ent->state.renderfx |= (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell);
    }
}

//
//===============
// SVG_SetClientEvent
// 
//===============
//
static void SVG_SetClientEvent(Entity *ent)
{
    if (ent->state.event)
        return;

    if (ent->groundEntityPtr && xyspeed > 225) {
        if ((int)(current_client->bobTime + bobmove) != bobcycle)
            ent->state.event = EntityEvent::Footstep;
    }
}

//
//===============
// SVG_SetClientSound
// 
//===============
//
static void SVG_SetClientSound(Entity *ent)
{
    const char    *weap; // C++20: STRING: Added const to char*

    if (ent->client->persistent.activeWeapon)
        weap = ent->client->persistent.activeWeapon->className;
    else
        weap = "";

    if (ent->waterLevel && (ent->waterType & (CONTENTS_LAVA | CONTENTS_SLIME)))
        ent->state.sound = snd_fry;
    else if (strcmp(weap, "weapon_railgun") == 0)
        ent->state.sound = gi.SoundIndex("weapons/rg_hum.wav");
    else if (strcmp(weap, "weapon_bfg") == 0)
        ent->state.sound = gi.SoundIndex("weapons/bfg_hum.wav");
    else if (ent->client->weaponSound)
        ent->state.sound = ent->client->weaponSound;
    else
        ent->state.sound = 0;
}

//
//===============
// SVG_SetClientFrame
// 
//===============
//
static void SVG_SetClientFrame(Entity *ent)
{
    GameClient *client = NULL;
    qboolean isDucking = false;
    qboolean isRunning = false;

    if (!ent)
        return;

    if (ent->state.modelIndex != 255)
        return;     // not in the player model

    client = ent->client;

    if (client->playerState.pmove.flags & PMF_DUCKED)
        isDucking = true;
    else
        isDucking = false;
    if (xyspeed)
        isRunning = true;
    else
        isRunning = false;

    // check for stand/duck and stop/go transitions
    if (isDucking != client->animation.isDucking && client->animation.priorityAnimation < PlayerAnimation::Death)
        goto newanim;
    if (isRunning != client->animation.isRunning && client->animation.priorityAnimation == PlayerAnimation::Basic)
        goto newanim;
    if (!ent->groundEntityPtr && client->animation.priorityAnimation <= PlayerAnimation::Wave)
        goto newanim;

    if (client->animation.priorityAnimation == PlayerAnimation::Reverse) {
        if (ent->state.frame > client->animation.endFrame) {
            ent->state.frame--;
            return;
        }
    } else if (ent->state.frame < client->animation.endFrame) {
        // continue an animation
        ent->state.frame++;
        return;
    }

    if (client->animation.priorityAnimation == PlayerAnimation::Death)
        return;     // stay there
    if (client->animation.priorityAnimation == PlayerAnimation::Jump) {
        if (!ent->groundEntityPtr)
            return;     // stay there
        ent->client->animation.priorityAnimation = PlayerAnimation::Wave;
        ent->state.frame = FRAME_jump3;
        ent->client->animation.endFrame = FRAME_jump6;
        return;
    }

newanim:
    // return to either a running or standing frame
    client->animation.priorityAnimation = PlayerAnimation::Basic;
    client->animation.isDucking = isDucking;
    client->animation.isRunning = isRunning;

    if (!ent->groundEntityPtr) {
        client->animation.priorityAnimation = PlayerAnimation::Jump;
        if (ent->state.frame != FRAME_jump2)
            ent->state.frame = FRAME_jump1;
        client->animation.endFrame = FRAME_jump2;
    } else if (isRunning) {
        // running
        if (isDucking) {
            ent->state.frame = FRAME_crwalk1;
            client->animation.endFrame = FRAME_crwalk6;
        } else {
            ent->state.frame = FRAME_run1;
            client->animation.endFrame = FRAME_run6;
        }
    } else {
        // standing
        if (isDucking) {
            ent->state.frame = FRAME_crstnd01;
            client->animation.endFrame = FRAME_crstnd19;
        } else {
            ent->state.frame = FRAME_stand01;
            client->animation.endFrame = FRAME_stand40;
        }
    }
}

//
//===============
// SVG_ClientEndServerFrame
//
// Called for each player at the end of the server frame and right 
// after spawning.
//===============
//
void SVG_ClientEndServerFrame(Entity *ent)
{
    float   bobTime;

    if (!ent || !ent->client || !ent->classEntity) {
        return;
    }

    // Setup the current player and entity being processed.
    current_player = ent;
    current_client = ent->client;

    //
    // If the origin or velocity have changed since ClientThink(),
    // update the pmove values.  This will happen when the client
    // is pushed by a bmodel or kicked by an explosion.
    //
    // If it wasn't updated here, the view position would lag a frame
    // behind the body position when pushed -- "sinking into plats"
    //
    current_client->playerState.pmove.origin = ent->state.origin;
    current_client->playerState.pmove.velocity = ent->velocity;

    //
    // If the end of unit layout is displayed, don't give
    // the player any normal movement attributes
    //
    if (level.intermission.time) {
        // FIXME: add view drifting here?
        current_client->playerState.blend[3] = 0;
        current_client->playerState.fov = 90;
        SVG_HUD_SetClientStats(ent);
        return;
    }

    vec3_vectors(ent->client->aimAngles, &forward, &right, &up);

    // burn from lava, etc
    SVG_Player_CheckWorldEffects();

    //
    // set model angles from view angles so other things in
    // the world can tell which direction you are looking
    //
    if (ent->client->aimAngles[vec3_t::Pitch] > 180)
        ent->state.angles[vec3_t::Pitch] = (-360 + ent->client->aimAngles[vec3_t::Pitch]) / 3;
    else
        ent->state.angles[vec3_t::Pitch] = ent->client->aimAngles[vec3_t::Pitch] / 3;
    ent->state.angles[vec3_t::Yaw] = ent->client->aimAngles[vec3_t::Yaw];
    ent->state.angles[vec3_t::Roll] = 0;
    ent->state.angles[vec3_t::Roll] = SVG_CalcRoll(ent->state.angles, ent->velocity) * 4;

    //
    // calculate speed and cycle to be used for
    // all cyclic walking effects
    //
    xyspeed = std::sqrtf(ent->velocity[0] * ent->velocity[0] + ent->velocity[1] * ent->velocity[1]);

    if (xyspeed < 5 || !(current_client->playerState.pmove.flags & PMF_ON_GROUND)) {
        bobmove = 0;
        current_client->bobTime = 0;    // start at beginning of cycle again
    }

    // N&C: Footstep tweaks.
    else if (ent->groundEntityPtr || ent->waterLevel == 2)
    {	// so bobbing only cycles when on ground
        if (xyspeed > 450) // Knightmare added
            bobmove = 0.25;
        else if (xyspeed > 210)
            bobmove = 0.125;
        else if (!ent->groundEntityPtr && ent->waterLevel == 2 && xyspeed > 100)
            bobmove = 0.225;
        else if (xyspeed > 100)
            bobmove = 0.0825;
        else if (!ent->groundEntityPtr && ent->waterLevel == 2)
            bobmove = 0.1625;
        else
            bobmove = 0.03125;
    }

    // Generate bob time.
    current_client->bobTime += bobmove;
    bobTime = current_client->bobTime;

    if (current_client->playerState.pmove.flags & PMF_DUCKED)
        bobTime *= 2;   // N&C: Footstep tweak.

    bobcycle = (int)bobTime;
    bobfracsin = std::fabsf(std::sinf(bobTime * M_PI));

    // Detect hitting the floor, and apply damage appropriately.
    SVG_Player_CheckFallingDamage(ent);

    // Apply all other the damage taken this frame
    SVG_Player_ApplyDamageFeedback(ent);

    // Determine the new frame's view offsets
    SVG_CalculateViewOffset(ent);

    // Determine the gun offsets
    SVG_CalculateGunOffset(ent);

    // Determine the full screen color blend
    // must be after viewOffset, so eye contents can be
    // accurately determined
    // FIXME: with client prediction, the contents
    // should be determined by the client
    SVG_CalculateBlend(ent);

    // Set the stats to display for this client (one of the chase isSpectator stats or...)
    if (ent->client->respawn.isSpectator)
        SVG_HUD_SetSpectatorStats(ent);
    else
        SVG_HUD_SetClientStats(ent);

    SVG_HUD_CheckChaseStats(ent);

    SVG_SetClientEvent(ent);

    SVG_SetClientEffects(ent);

    SVG_SetClientSound(ent);

    SVG_SetClientFrame(ent);

    // Store velocity and view angles.
    ent->client->oldVelocity = ent->velocity;
    ent->client->oldViewAngles = ent->client->playerState.pmove.viewAngles;

    // Reset weapon kicks to zer0.
    ent->client->kickOrigin = vec3_zero();
    ent->client->kickAngles = vec3_zero();

    // if the scoreboard is up, update it
    if (ent->client->showScores && !(level.frameNumber & 31)) {
        Entity* enemyEntity = NULL;
        if (ent->classEntity) {
            if (ent->classEntity->GetEnemy())
                enemyEntity = ent->classEntity->GetEnemy()->GetServerEntity();
        }
        SVG_HUD_GenerateDMScoreboardLayout(ent, enemyEntity);
        gi.Unicast(ent, false);
    }
}

