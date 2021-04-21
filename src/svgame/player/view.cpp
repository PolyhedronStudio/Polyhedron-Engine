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
#include "hud.h"
#include "animations.h"



static  entity_t     *current_player;
static  gclient_t   *current_client;

static  vec3_t  forward, right, up;
static float   xyspeed;

static float   bobmove;
static int     bobcycle;       // odd cycles are right foot going forward
static float   bobfracsin;     // sin(bobfrac*M_PI)

//
//===============
// SV_CalcRoll
// 
//
//===============
//
static float SV_CalcRoll(vec3_t angles, vec3_t velocity)
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
// P_ApplyDamageFeedback
// 
// Handles color blends and view kicks
//===============
//
static void P_ApplyDamageFeedback(entity_t *player)
{
    gclient_t   *client;
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
    if (client->damage_blood)
        client->playerState.stats[STAT_FLASHES] |= 1;
    if (client->damage_armor && !(player->flags & FL_GODMODE))
        client->playerState.stats[STAT_FLASHES] |= 2;

    // total points of damage shot at the player this frame
    count = (client->damage_blood + client->damage_armor + client->damage_parmor);
    if (count == 0)
        return;     // didn't take any damage

    // start a pain animation if still in the player model
    if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255) {
        static int      i;

        client->anim_priority = ANIM_PAIN;
        if (client->playerState.pmove.flags & PMF_DUCKED) {
            player->s.frame = FRAME_crpain1 - 1;
            client->anim_end = FRAME_crpain4;
        } else {
            i = (i + 1) % 3;
            switch (i) {
            case 0:
                player->s.frame = FRAME_pain101 - 1;
                client->anim_end = FRAME_pain104;
                break;
            case 1:
                player->s.frame = FRAME_pain201 - 1;
                client->anim_end = FRAME_pain204;
                break;
            case 2:
                player->s.frame = FRAME_pain301 - 1;
                client->anim_end = FRAME_pain304;
                break;
            }
        }
    }

    realcount = count;
    if (count < 10)
        count = 10; // always make a visible effect

    // Play an apropriate pain sound
    if ((level.time > player->debouncePainTime) && !(player->flags & FL_GODMODE)) {
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
    if (client->damage_alpha < 0.f)
        client->damage_alpha = 0.f;
    client->damage_alpha += count * 0.01f;
    if (client->damage_alpha < 0.2f)
        client->damage_alpha = 0.2f;
    if (client->damage_alpha > 0.6f)
        client->damage_alpha = 0.6f;     // don't go too saturated

    // The color of the blend will vary based on how much was absorbed
    // by different armors.
    vec3_t blendColor = vec3_zero();
    if (client->damage_parmor)
        blendColor = vec3_fmaf(blendColor, (float)client->damage_parmor / realcount, power_color);
    if (client->damage_armor)
        blendColor = vec3_fmaf(blendColor, (float)client->damage_armor / realcount, acolor);
    if (client->damage_blood)
        blendColor = vec3_fmaf(blendColor, (float)client->damage_blood / realcount, bcolor);
    client->damage_blend = blendColor;


    //
    // Calculate view angle kicks
    //
    kick = abs(client->damage_knockback);
    if (kick && player->health > 0) { // kick of 0 means no view adjust at all
        kick = kick * 100 / player->health;

        if (kick < count * 0.5f)
            kick = count * 0.5f;
        if (kick > 50)
            kick = 50;

        vec3_t kickVec = client->damage_from - player->s.origin;
        kickVec = vec3_normalize(kickVec);

        side = DotProduct(kickVec, right);
        client->v_dmg_roll = kick * side * 0.3f;

        side = -DotProduct(kickVec, forward);
        client->v_dmg_pitch = kick * side * 0.3f;

        client->v_dmg_time = level.time + DAMAGE_TIME;
    }

    //
    // clear totals
    //
    client->damage_blood = 0;
    client->damage_armor = 0;
    client->damage_parmor = 0;
    client->damage_knockback = 0;
}

//
//===============
// SV_CalculateViewOffset
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
static void SV_CalculateViewOffset(entity_t *ent)
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

        ent->client->playerState.viewAngles[vec3_t::Roll] = 40;
        ent->client->playerState.viewAngles[vec3_t::Pitch] = -15;
        ent->client->playerState.viewAngles[vec3_t::Yaw] = ent->client->killer_yaw;
    } else {
        // Fetch client kick angles.
        vec3_t newKickAngles = ent->client->playerState.kickAngles = ent->client->kickAngles; //ent->client->playerState.kickAngles;

        // Add pitch(X) and roll(Z) angles based on damage kick
        ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
        if (ratio < 0) {
            ratio = ent->client->v_dmg_pitch = ent->client->v_dmg_roll = 0;
        }
        newKickAngles[vec3_t::Pitch] += ratio * ent->client->v_dmg_pitch;
        newKickAngles[vec3_t::Roll] += ratio * ent->client->v_dmg_roll;

        // Add pitch based on fall kick
        ratio = (ent->client->fall_time - level.time) / FALL_TIME;
        if (ratio < 0)
            ratio = 0;
        newKickAngles[vec3_t::Pitch] += ratio * ent->client->fall_value;

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
    ratio = (ent->client->fall_time - level.time) / FALL_TIME;
    if (ratio < 0)
        ratio = 0;
    newViewOffset.z -= ratio * ent->client->fall_value * 0.4f;

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
    ent->client->playerState.viewoffset = vec3_clamp(newViewOffset,
        //{ -14, -14, -22 },
        //{ 14,  14, 30 }
        ent->mins,
        ent->maxs
    );
}

//
//===============
// SV_CalculateGunOffset
// 
//===============
//
static void SV_CalculateGunOffset(entity_t *ent)
{
    int     i;
    float   delta;

    // gun angles from bobbing
    ent->client->playerState.gunangles[vec3_t::Roll] = xyspeed * bobfracsin * 0.005;
    ent->client->playerState.gunangles[vec3_t::Yaw]  = xyspeed * bobfracsin * 0.01;
    if (bobcycle & 1) {
        ent->client->playerState.gunangles[vec3_t::Roll] = -ent->client->playerState.gunangles[vec3_t::Roll];
        ent->client->playerState.gunangles[vec3_t::Yaw]  = -ent->client->playerState.gunangles[vec3_t::Yaw];
    }

    ent->client->playerState.gunangles[vec3_t::Pitch] = xyspeed * bobfracsin * 0.005;

    // gun angles from delta movement
    for (i = 0 ; i < 3 ; i++) {
        delta = ent->client->oldViewAngles[i] - ent->client->playerState.viewAngles[i];
        if (delta > 180)
            delta -= 360;
        if (delta < -180)
            delta += 360;
        if (delta > 45)
            delta = 45;
        if (delta < -45)
            delta = -45;
        if (i == vec3_t::Yaw)
            ent->client->playerState.gunangles[vec3_t::Roll] += 0.1 * delta;
        ent->client->playerState.gunangles[i] += 0.2 * delta;
    }

    // gun height
    VectorClear(ent->client->playerState.gunoffset);
//  ent->playerState->gunorigin[2] += bob;

    // gun_x / gun_y / gun_z are development tools
    for (i = 0 ; i < 3 ; i++) {
        ent->client->playerState.gunoffset[i] += forward[i] * (gun_y->value);
        ent->client->playerState.gunoffset[i] += right[i] * gun_x->value;
        ent->client->playerState.gunoffset[i] += up[i] * (-gun_z->value);
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
// SV_CalculateBlend
// 
//===============
//
static void SV_CalculateBlend(entity_t *ent)
{
    int     contents;
    vec3_t  vieworg;

    ent->client->playerState.blend[0] = ent->client->playerState.blend[1] =
                                   ent->client->playerState.blend[2] = ent->client->playerState.blend[3] = 0;

    // add for contents
    VectorAdd(ent->s.origin, ent->client->playerState.viewoffset, vieworg);
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
    if (ent->client->damage_alpha > 0)
        SV_AddBlend(ent->client->damage_blend[0], ent->client->damage_blend[1]
                    , ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->playerState.blend);

    if (ent->client->bonus_alpha > 0)
        SV_AddBlend(0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->playerState.blend);

    // drop the damage value
    ent->client->damage_alpha -= 0.06;
    if (ent->client->damage_alpha < 0)
        ent->client->damage_alpha = 0;

    // drop the bonus value
    ent->client->bonus_alpha -= 0.1;
    if (ent->client->bonus_alpha < 0)
        ent->client->bonus_alpha = 0;
}

//
//===============
// P_CheckFallingDamage
// 
//===============
//
static void P_CheckFallingDamage(entity_t *ent)
{
    float   delta;
    int     damage;
    vec3_t  dir;

    if (ent->s.modelindex != 255)
        return;     // not in the player model

    if (ent->moveType == MOVETYPE_NOCLIP)
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
        ent->s.event = EV_FOOTSTEP;
        return;
    }

    ent->client->fall_value = delta * 0.5;
    if (ent->client->fall_value > 40)
        ent->client->fall_value = 40;
    ent->client->fall_time = level.time + FALL_TIME;

    if (delta > 30) {
        if (ent->health > 0) {
            if (delta >= 55)
                ent->s.event = EV_FALLFAR;
            else
                ent->s.event = EV_FALL;
        }
        ent->debouncePainTime = level.time;   // no normal pain sound
        damage = (delta - 30) / 2;
        if (damage < 1)
            damage = 1;
        VectorSet(dir, 0, 0, 1);

        if (!deathmatch->value || !((int)dmflags->value & DeathMatchFlags::NoFalling))
            T_Damage(ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
    } else {
        ent->s.event = EV_FALLSHORT;
        return;
    }
}

//
//===============
// P_CheckWorldEffects
// 
//===============
//
static void P_CheckWorldEffects(void)
{
    int         waterlevel, old_waterlevel;

    if (current_player->moveType == MOVETYPE_NOCLIP) {
        current_player->air_finished = level.time + 12; // don't need air
        return;
    }

    waterlevel = current_player->waterLevel;
    old_waterlevel = current_client->old_waterlevel;
    current_client->old_waterlevel = waterlevel;

    //
    // if just entered a water volume, play a sound
    //
    if (!old_waterlevel && waterlevel) {
        PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
        if (current_player->waterType & CONTENTS_LAVA)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/lava_in.wav"), 1, ATTN_NORM, 0);
        else if (current_player->waterType & CONTENTS_SLIME)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        else if (current_player->waterType & CONTENTS_WATER)
            gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        current_player->flags |= FL_INWATER;

        // clear damage_debounce, so the pain sound will play immediately
        current_player->debounceDamageTime = level.time - 1;
    }

    //
    // if just completely exited a water volume, play a sound
    //
    if (old_waterlevel && ! waterlevel) {
        PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
        gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
        current_player->flags &= ~FL_INWATER;
    }

    //
    // check for head just going under water
    //
    if (old_waterlevel != 3 && waterlevel == 3) {
        gi.Sound(current_player, CHAN_BODY, gi.SoundIndex("player/watr_un.wav"), 1, ATTN_NORM, 0);
    }

    //
    // check for head just coming out of water
    //
    if (old_waterlevel == 3 && waterlevel != 3) {
        if (current_player->air_finished < level.time) {
            // gasp for air
            gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/gasp1.wav"), 1, ATTN_NORM, 0);
            PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
        } else  if (current_player->air_finished < level.time + 11) {
            // just break surface
            gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/gasp2.wav"), 1, ATTN_NORM, 0);
        }
    }

    //
    // check for drowning
    //
    if (waterlevel == 3) {
        // if out of air, start drowning
        if (current_player->air_finished < level.time) {
            // drown!
            if (current_player->client->next_drown_time < level.time
                && current_player->health > 0) {
                current_player->client->next_drown_time = level.time + 1;

                // take more damage the longer underwater
                current_player->dmg += 2;
                if (current_player->dmg > 15)
                    current_player->dmg = 15;

                // play a gurp sound instead of a normal pain sound
                if (current_player->health <= current_player->dmg)
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("player/drown1.wav"), 1, ATTN_NORM, 0);
                else if (rand() & 1)
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("*gurp1.wav"), 1, ATTN_NORM, 0);
                else
                    gi.Sound(current_player, CHAN_VOICE, gi.SoundIndex("*gurp2.wav"), 1, ATTN_NORM, 0);

                current_player->debouncePainTime = level.time;

                T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
            }
        }
    } else {
        current_player->air_finished = level.time + 12;
        current_player->dmg = 2;
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

            T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3 * waterlevel, 0, 0, MOD_LAVA);
        }

        if (current_player->waterType & CONTENTS_SLIME) {
            T_Damage(current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1 * waterlevel, 0, 0, MOD_SLIME);
        }
    }
}

//
//===============
// G_SetClientEffects
// 
//===============
//
static void G_SetClientEffects(entity_t *ent)
{
    ent->s.effects = 0;
    ent->s.renderfx = 0;

    if (ent->health <= 0 || level.intermissiontime)
        return;

    // show cheaters!!!
    if (ent->flags & FL_GODMODE) {
        ent->s.renderfx |= (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell);
    }
}

//
//===============
// G_SetClientEvent
// 
//===============
//
static void G_SetClientEvent(entity_t *ent)
{
    if (ent->s.event)
        return;

    if (ent->groundEntityPtr && xyspeed > 225) {
        if ((int)(current_client->bobtime + bobmove) != bobcycle)
            ent->s.event = EV_FOOTSTEP;
    }
}

//
//===============
// G_SetClientSound
// 
//===============
//
static void G_SetClientSound(entity_t *ent)
{
    const char    *weap; // C++20: STRING: Added const to char*

    if (ent->client->pers.weapon)
        weap = ent->client->pers.weapon->classname;
    else
        weap = "";

    if (ent->waterLevel && (ent->waterType & (CONTENTS_LAVA | CONTENTS_SLIME)))
        ent->s.sound = snd_fry;
    else if (strcmp(weap, "weapon_railgun") == 0)
        ent->s.sound = gi.SoundIndex("weapons/rg_hum.wav");
    else if (strcmp(weap, "weapon_bfg") == 0)
        ent->s.sound = gi.SoundIndex("weapons/bfg_hum.wav");
    else if (ent->client->weapon_sound)
        ent->s.sound = ent->client->weapon_sound;
    else
        ent->s.sound = 0;
}

//
//===============
// G_SetClientFrame
// 
//===============
//
static void G_SetClientFrame(entity_t *ent)
{
    gclient_t   *client;
    qboolean    duck, run;

    if (ent->s.modelindex != 255)
        return;     // not in the player model

    client = ent->client;

    if (client->playerState.pmove.flags & PMF_DUCKED)
        duck = true;
    else
        duck = false;
    if (xyspeed)
        run = true;
    else
        run = false;

    // check for stand/duck and stop/go transitions
    if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
        goto newanim;
    if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
        goto newanim;
    if (!ent->groundEntityPtr && client->anim_priority <= ANIM_WAVE)
        goto newanim;

    if (client->anim_priority == ANIM_REVERSE) {
        if (ent->s.frame > client->anim_end) {
            ent->s.frame--;
            return;
        }
    } else if (ent->s.frame < client->anim_end) {
        // continue an animation
        ent->s.frame++;
        return;
    }

    if (client->anim_priority == ANIM_DEATH)
        return;     // stay there
    if (client->anim_priority == ANIM_JUMP) {
        if (!ent->groundEntityPtr)
            return;     // stay there
        ent->client->anim_priority = ANIM_WAVE;
        ent->s.frame = FRAME_jump3;
        ent->client->anim_end = FRAME_jump6;
        return;
    }

newanim:
    // return to either a running or standing frame
    client->anim_priority = ANIM_BASIC;
    client->anim_duck = duck;
    client->anim_run = run;

    if (!ent->groundEntityPtr) {
        client->anim_priority = ANIM_JUMP;
        if (ent->s.frame != FRAME_jump2)
            ent->s.frame = FRAME_jump1;
        client->anim_end = FRAME_jump2;
    } else if (run) {
        // running
        if (duck) {
            ent->s.frame = FRAME_crwalk1;
            client->anim_end = FRAME_crwalk6;
        } else {
            ent->s.frame = FRAME_run1;
            client->anim_end = FRAME_run6;
        }
    } else {
        // standing
        if (duck) {
            ent->s.frame = FRAME_crstnd01;
            client->anim_end = FRAME_crstnd19;
        } else {
            ent->s.frame = FRAME_stand01;
            client->anim_end = FRAME_stand40;
        }
    }
}

//
//===============
// ClientEndServerFrame
//
// Called for each player at the end of the server frame and right 
// after spawning.
//===============
//
void ClientEndServerFrame(entity_t *ent)
{
    float   bobtime;
    int     i;

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
    for (i = 0 ; i < 3 ; i++) {
        // N&C: FF Precision.
        VectorCopy(ent->s.origin, current_client->playerState.pmove.origin);
        VectorCopy(ent->velocity, current_client->playerState.pmove.velocity);
    }

    //
    // If the end of unit layout is displayed, don't give
    // the player any normal movement attributes
    //
    if (level.intermissiontime) {
        // FIXME: add view drifting here?
        current_client->playerState.blend[3] = 0;
        current_client->playerState.fov = 90;
        HUD_SetClientStats(ent);
        return;
    }

    vec3_vectors(ent->client->v_angle, &forward, &right, &up);

    // burn from lava, etc
    P_CheckWorldEffects();

    //
    // set model angles from view angles so other things in
    // the world can tell which direction you are looking
    //
    if (ent->client->v_angle[vec3_t::Pitch] > 180)
        ent->s.angles[vec3_t::Pitch] = (-360 + ent->client->v_angle[vec3_t::Pitch]) / 3;
    else
        ent->s.angles[vec3_t::Pitch] = ent->client->v_angle[vec3_t::Pitch] / 3;
    ent->s.angles[vec3_t::Yaw] = ent->client->v_angle[vec3_t::Yaw];
    ent->s.angles[vec3_t::Roll] = 0;
    ent->s.angles[vec3_t::Roll] = SV_CalcRoll(ent->s.angles, ent->velocity) * 4;

    //
    // calculate speed and cycle to be used for
    // all cyclic walking effects
    //
    xyspeed = std::sqrtf(ent->velocity[0] * ent->velocity[0] + ent->velocity[1] * ent->velocity[1]);

    if (xyspeed < 5) {
        bobmove = 0;
        current_client->bobtime = 0;    // start at beginning of cycle again
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
            bobmove = 0.0625;
        else if (!ent->groundEntityPtr && ent->waterLevel == 2)
            bobmove = 0.1625;
        else
            bobmove = 0.03125;
    }

    bobtime = (current_client->bobtime += bobmove);

    if (current_client->playerState.pmove.flags & PMF_DUCKED)
        bobtime *= 2;   // N&C: Footstep tweak.

    bobcycle = (int)bobtime;
    bobfracsin = std::fabsf(std::sinf(bobtime * M_PI));

    // Detect hitting the floor, and apply damage appropriately.
    P_CheckFallingDamage(ent);

    // Apply all other the damage taken this frame
    P_ApplyDamageFeedback(ent);

    // Determine the new frame's view offsets
    SV_CalculateViewOffset(ent);

    // Determine the gun offsets
    SV_CalculateGunOffset(ent);

    // Determine the full screen color blend
    // must be after viewoffset, so eye contents can be
    // accurately determined
    // FIXME: with client prediction, the contents
    // should be determined by the client
    SV_CalculateBlend(ent);

    // Set the stats to display for this client (one of the chase spectator stats or...)
    if (ent->client->resp.spectator)
        HUD_SetSpectatorStats(ent);
    else
        HUD_SetClientStats(ent);

    HUD_CheckChaseStats(ent);

    G_SetClientEvent(ent);

    G_SetClientEffects(ent);

    G_SetClientSound(ent);

    G_SetClientFrame(ent);

    // Store velocity and view angles.
    ent->client->oldVelocity = ent->velocity;
    ent->client->oldViewAngles = ent->client->playerState.viewAngles;

    // Reset weapon kicks to zer0.
    ent->client->kickOrigin = vec3_zero();
    ent->client->kickAngles = vec3_zero();

    // if the scoreboard is up, update it
    if (ent->client->showscores && !(level.framenum & 31)) {
        HUD_GenerateDMScoreboardLayout(ent, ent->enemy);
        gi.Unicast(ent, false);
    }
}

