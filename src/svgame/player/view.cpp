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
#include "../entities/base/PlayerClient.h"
#include "hud.h"
#include "animations.h"

////
////===============
//// SVG_CalculateViewOffset
//// 
//// Calculates t
////
//// fall from 128 : 400 = 160000
//// fall from 256 : 580 = 336400
//// fall from 384 : 720 = 518400
//// fall from 512 : 800 = 640000
//// fall from 640 : 960 =
////
//// damage = deltavelocity * deltavelocity * 0.0001
//// 
////===============
////
//void SVG_Client_CalculateViewOffset(PlayerClient *ent)
//{
//    float       bob;
//    float       ratio;
//    float       delta;
//
//    // Check whether ent is valid, and a PlayerClient hooked up 
//    // to a valid client.
//    ServersClient* client = nullptr;
//
//    if (!ent || !(client = ent->GetClient())) {
//        return;
//    }
//
//    //
//    // Calculate new kick angle vales. (
//    // 
//    // If dead, set a fixed angle and don't add any kick
//    if (ent->GetDeadFlag()) {
//        client->playerState.kickAngles = vec3_zero();
//
//        client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
//        client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
//        client->playerState.pmove.viewAngles[vec3_t::Yaw] = client->killerYaw;
//    } else {
//        // Fetch client kick angles.
//        vec3_t newKickAngles = client->playerState.kickAngles = client->kickAngles; //ent->client->playerState.kickAngles;
//
//                                                                                    // Add pitch(X) and roll(Z) angles based on damage kick
//        ratio = ((client->viewDamage.time - level.time) / DAMAGE_TIME);
//        if (ratio < 0) {
//            ratio = client->viewDamage.pitch = client->viewDamage.roll = 0;
//        }
//        newKickAngles[vec3_t::Pitch] += ratio * client->viewDamage.pitch;
//        newKickAngles[vec3_t::Roll] += ratio * client->viewDamage.roll;
//
//        // Add pitch based on fall kick
//        ratio = ((client->fallTime - level.time) / FALL_TIME);
//        if (ratio < 0)
//            ratio = 0;
//        newKickAngles[vec3_t::Pitch] += ratio * client->fallValue;
//
//        // Add angles based on velocity
//        delta = vec3_dot(ent->GetVelocity(), ent->bobforward);
//        newKickAngles[vec3_t::Pitch] += delta * run_pitch->value;
//
//        delta = vec3_dot(ent->GetVelocity(), right);
//        newKickAngles[vec3_t::Roll] += delta * run_roll->value;
//
//        // Add angles based on bob
//        delta = bobFracsin * bob_pitch->value * XYSpeed;
//        if (client->playerState.pmove.flags & PMF_DUCKED)
//            delta *= 6;     // crouching
//        newKickAngles[vec3_t::Pitch] += delta;
//        delta = bobFracsin * bob_roll->value * XYSpeed;
//        if (client->playerState.pmove.flags & PMF_DUCKED)
//            delta *= 6;     // crouching
//        if (bobCycle & 1)
//            delta = -delta;
//        newKickAngles[vec3_t::Roll] += delta;
//
//        // Last but not least, assign new kickangles to player state.
//        client->playerState.kickAngles = newKickAngles;
//    }
//
//    //
//    // Calculate new view offset.
//    //
//    // Start off with the base entity viewheight. (Set by Player Move code.)
//    vec3_t newViewOffset = {
//        0.f,
//        0.f,
//        (float)ent->GetViewHeight()
//    };
//
//    // Add fall impact view punch height.
//    ratio = (client->fallTime - level.time) / FALL_TIME;
//    if (ratio < 0)
//        ratio = 0;
//    newViewOffset.z -= ratio * client->fallValue * 0.4f;
//
//    // Add bob height.
//    bob = bobFracsin * XYSpeed * bob_up->value ;
//    if (bob > 6)
//        bob = 6;
//    newViewOffset.z += bob;
//
//    // Add kick offset
//    newViewOffset += client->kickOrigin;
//
//    // Clamp the new view offsets, and finally assign them to the player state.
//    // Clamping ensures that they never exceed the non visible, but physically 
//    // there, player bounding box.
//    client->playerState.pmove.viewOffset = vec3_clamp(newViewOffset,
//        //{ -14, -14, -22 },
//        //{ 14,  14, 30 }
//        ent->GetMins(),
//        ent->GetMaxs()
//    );
//}
//
////
////===============
//// SVG_Client_CalculateGunOffset
//// 
////===============
////
//void SVG_Client_CalculateGunOffset(PlayerClient *ent)
//{
//    int     i;
//    float   delta;
//
//    // Check whether ent is valid, and a PlayerClient hooked up 
//    // to a valid client.
//    ServersClient* client = nullptr;
//
//    if (!ent || !(client = ent->GetClient()) ||
//        !ent->IsSubclassOf<PlayerClient>()) {
//        return;
//    }
//
//    // gun angles from bobbing
//    client->playerState.gunAngles[vec3_t::Roll] = XYSpeed * bobFracsin * 0.005;
//    client->playerState.gunAngles[vec3_t::Yaw]  = XYSpeed * bobFracsin * 0.01;
//    if (bobCycle & 1) {
//        client->playerState.gunAngles[vec3_t::Roll] = -client->playerState.gunAngles[vec3_t::Roll];
//        client->playerState.gunAngles[vec3_t::Yaw]  = -client->playerState.gunAngles[vec3_t::Yaw];
//    }
//
//    client->playerState.gunAngles[vec3_t::Pitch] = XYSpeed * bobFracsin * 0.005;
//
//    // gun angles from delta movement
//    for (i = 0 ; i < 3 ; i++) {
//        delta = client->oldViewAngles[i] - client->playerState.pmove.viewAngles[i];
//        if (delta > 180)
//            delta -= 360;
//        if (delta < -180)
//            delta += 360;
//        if (delta > 45)
//            delta = 45;
//        if (delta < -45)
//            delta = -45;
//        if (i == vec3_t::Yaw)
//            client->playerState.gunAngles[vec3_t::Roll] += 0.1 * delta;
//        client->playerState.gunAngles[i] += 0.2 * delta;
//    }
//
//    // gun height
//    client->playerState.gunOffset = vec3_zero();
////  ent->playerState->gunorigin[2] += bob;
//
//    // gun_x / gun_y / gun_z are development tools
//    for (i = 0 ; i < 3 ; i++) {
//        client->playerState.gunOffset[i] += forward[i] * (gun_y->value);
//        client->playerState.gunOffset[i] += right[i] * gun_x->value;
//        client->playerState.gunOffset[i] += up[i] * (-gun_z->value);
//    }
//}

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
void SVG_Client_CalculateBlend(PlayerClient *ent)
{
    // Check whether ent is valid, and a PlayerClient hooked up 
    // to a valid client.
    ServersClient* client = nullptr;

    if (!ent || !(client = ent->GetClient()) ||
        !ent->IsSubclassOf<PlayerClient>()) {
        return;
    }

    // Clear blend values.
    client->playerState.blend[0] = client->playerState.blend[1] =
        client->playerState.blend[2] = client->playerState.blend[3] = 0;

    // Calculate view origin to use for PointContents.
    vec3_t viewOrigin = ent->GetOrigin() + client->playerState.pmove.viewOffset;
    int32_t contents = gi.PointContents(viewOrigin);

	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
        client->playerState.rdflags |= RDF_UNDERWATER;
    else
        client->playerState.rdflags &= ~RDF_UNDERWATER;

    if (contents & (CONTENTS_SOLID | CONTENTS_LAVA))
        SV_AddBlend(1.0, 0.3, 0.0, 0.6, client->playerState.blend);
    else if (contents & CONTENTS_SLIME)
        SV_AddBlend(0.0, 0.1, 0.05, 0.6, client->playerState.blend);
    else if (contents & CONTENTS_WATER)
        SV_AddBlend(0.5, 0.3, 0.2, 0.4, client->playerState.blend);

    // add for damage
    if (client->damageAlpha > 0)
        SV_AddBlend(client->damageBlend[0], client->damageBlend[1]
                    , client->damageBlend[2], client->damageAlpha, client->playerState.blend);

    if (client->bonusAlpha > 0)
        SV_AddBlend(0.85, 0.7, 0.3, client->bonusAlpha, client->playerState.blend);

    // drop the damage value
    client->damageAlpha -= 0.06;
    if (client->damageAlpha < 0)
        client->damageAlpha = 0;

    // drop the bonus value
    client->bonusAlpha -= 0.1;
    if (client->bonusAlpha < 0)
        client->bonusAlpha = 0;
}

//
//===============
// SVG_Client_CheckFallingDamage
// 
//===============
//
//void SVG_Client_CheckFallingDamage(PlayerClient *ent)
//{
//    float   delta;
//    int     damage;
//    vec3_t  dir;
//
//    // Check whether ent is valid, and a PlayerClient hooked up 
//    // to a valid client.
//    ServersClient* client = nullptr;
//
//    if (!ent || !(client = ent->GetClient()) ||
//        !ent->IsSubclassOf<PlayerClient>()) {
//        return;
//    }
//
//    if (ent->GetModelIndex() != 255)
//        return;     // not in the player model
//
//    if (ent->GetMoveType() == MoveType::NoClip || ent->GetMoveType() == MoveType::Spectator)
//        return;
//
//    // Calculate delta velocity.
//    vec3_t velocity = ent->GetVelocity();
//
//    if ((client->oldVelocity[2] < 0) && (velocity[2] > client->oldVelocity[2]) && (!ent->GetGroundEntity())) {
//        delta = client->oldVelocity[2];
//    } else {
//        if (!ent->GetGroundEntity())
//            return;
//        delta = velocity[2] - client->oldVelocity[2];
//    }
//    delta = delta * delta * 0.0001;
//
//    // never take falling damage if completely underwater
//    if (ent->GetWaterLevel() == 3)
//        return;
//    if (ent->GetWaterLevel() == 2)
//        delta *= 0.25;
//    if (ent->GetWaterLevel() == 1)
//        delta *= 0.5;
//
//    if (delta < 1)
//        return;
//
//    if (delta < 15) {
//        ent->SetEventID(EntityEvent::Footstep);
//        return;
//    }
//
//    client->fallValue = delta * 0.5;
//    if (client->fallValue > 40)
//        client->fallValue = 40;
//    client->fallTime = level.time + FALL_TIME;
//
//    if (delta > 30) {
//        if (ent->GetHealth() > 0) {
//            if (delta >= 55)
//                ent->SetEventID(EntityEvent::FallFar);
//            else
//                ent->SetEventID(EntityEvent::Fall);
//        }
//        ent->SetDebouncePainTime(level.time);   // no normal pain sound
//        damage = (delta - 30) / 2;
//        if (damage < 1)
//            damage = 1;
//        dir = { 0.f, 0.f, 1.f };
//
//        if (!deathmatch->value || !((int)gamemodeflags->value & GameModeFlags::NoFalling))
//            SVG_InflictDamage(ent, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), dir, ent->GetOrigin(), vec3_zero(), damage, 0, 0, MeansOfDeath::Falling);
//    } else {
//        ent->SetEventID(EntityEvent::FallShort);
//        return;
//    }
//}

//
//===============
// SVG_Client_SetEvent
// 
//===============
//
void SVG_Client_SetEvent(PlayerClient* ent) {
    //if (!ent || !ent->GetClient()) {
    //    return;
    //}

    //if (ent->GetEventID())
    //    return;

    //if (ent->GetGroundEntity() && bobMove.XYSpeed > 225) {
    //    if ((int)(ent->bobTime + bobMove) != bobCycle)
    //        ent->SetEventID(EntityEvent::Footstep);
    //}
}


//
//===============
// SVG_Client_SetEffects
// 
//===============
//
void SVG_Client_SetEffects(PlayerClient *ent)
{
    //if (!ent || !ent->IsSubclassOf<PlayerClient>()) {
    //    return;
    //}

    //ent->SetEffects(0);
    //ent->SetRenderEffects(0);

    //if (ent->GetHealth() <= 0 || level.intermission.time)
    //    return;

    //// show cheaters!!!
    //if (ent->GetFlags() & EntityFlags::GodMode) {
    //    ent->SetRenderEffects(ent->GetRenderEffects() | (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell));
    //}
}

//
//===============
// SVG_Client_SetSound
// 
//===============
//
void SVG_Client_SetSound(PlayerClient *ent)
{
    //const char    *weap; // C++20: STRING: Added const to char*

    //// Check whether ent is valid, and a PlayerClient hooked up 
    //// to a valid client.
    //ServersClient* client = nullptr;

    //if (!ent || !(client = ent->GetClient()) ||
    //    !ent->IsSubclassOf<PlayerClient>()) {
    //    return;
    //}

    //if (client->persistent.activeWeapon)
    //    weap = client->persistent.activeWeapon->className;
    //else
    //    weap = "";

    //if (ent->GetWaterLevel() && (ent->GetWaterType() & (CONTENTS_LAVA | CONTENTS_SLIME)))
    //    ent->SetSound(snd_fry);
    //else if (strcmp(weap, "weapon_railgun") == 0)
    //    ent->SetSound(gi.SoundIndex("weapons/rg_hum.wav"));
    //else if (strcmp(weap, "weapon_bfg") == 0)
    //    ent->SetSound(gi.SoundIndex("weapons/bfg_hum.wav"));
    //else if (client->weaponSound)
    //    ent->SetSound(client->weaponSound);
    //else
    //    ent->SetSound(0);
}

//
//===============
// SVG_Client_SetAnimationFrame
// 
//===============
//
//void SVG_Client_SetAnimationFrame(PlayerClient *ent)
//{
//    qboolean isDucking = false;
//    qboolean isRunning = false;
//
//    // Check whether ent is valid, and a PlayerClient hooked up 
//    // to a valid client.
//    ServersClient* client = nullptr;
//
//    if (!ent || !(client = ent->GetClient()) ||
//        !ent->IsSubclassOf<PlayerClient>()) {
//        return;
//    }
//
//    if (ent->GetModelIndex() != 255)
//        return;     // not in the player model
//
//    //client = ent->client;
//
//    if (client->playerState.pmove.flags & PMF_DUCKED)
//        isDucking = true;
//    else
//        isDucking = false;
//    if (ent->bobMove.XYSpeed)
//        isRunning = true;
//    else
//        isRunning = false;
//
//    // check for stand/duck and stop/go transitions
//    if (isDucking != client->animation.isDucking && client->animation.priorityAnimation < PlayerAnimation::Death)
//        goto newanim;
//    if (isRunning != client->animation.isRunning && client->animation.priorityAnimation == PlayerAnimation::Basic)
//        goto newanim;
//    if (!ent->GetGroundEntity() && client->animation.priorityAnimation <= PlayerAnimation::Wave)
//        goto newanim;
//
//    if (client->animation.priorityAnimation == PlayerAnimation::Reverse) {
//        if (ent->GetFrame() > client->animation.endFrame) {
//            ent->SetFrame(ent->GetFrame() - 1);
//            return;
//        }
//    } else if (ent->GetFrame() < client->animation.endFrame) {
//        // continue an animation
//        ent->SetFrame(ent->GetFrame() + 1);
//        return;
//    }
//
//    if (client->animation.priorityAnimation == PlayerAnimation::Death)
//        return;     // stay there
//    if (client->animation.priorityAnimation == PlayerAnimation::Jump) {
//        if (!ent->GetGroundEntity())
//            return;     // stay there
//        client->animation.priorityAnimation = PlayerAnimation::Wave;
//        ent->SetFrame(FRAME_jump3);
//        client->animation.endFrame = FRAME_jump6;
//        return;
//    }
//
//newanim:
//    // return to either a running or standing frame
//    client->animation.priorityAnimation = PlayerAnimation::Basic;
//    client->animation.isDucking = isDucking;
//    client->animation.isRunning = isRunning;
//
//    if (!ent->GetGroundEntity()) {
//        client->animation.priorityAnimation = PlayerAnimation::Jump;
//        if (ent->GetFrame() != FRAME_jump2)
//            ent->SetFrame(FRAME_jump1);
//        client->animation.endFrame = FRAME_jump2;
//    } else if (isRunning) {
//        // running
//        if (isDucking) {
//            ent->SetFrame(FRAME_crwalk1);
//            client->animation.endFrame = FRAME_crwalk6;
//        } else {
//            ent->SetFrame(FRAME_run1);
//            client->animation.endFrame = FRAME_run6;
//        }
//    } else {
//        // standing
//        if (isDucking) {
//            ent->SetFrame(FRAME_crstnd01);
//            client->animation.endFrame = FRAME_crstnd19;
//        } else {
//            ent->SetFrame(FRAME_stand01);
//            client->animation.endFrame = FRAME_stand40;
//        }
//    }
//}

//
//===============
// SVG_ClientEndServerFrame
//
// Called for each player at the end of the server frame and right 
// after spawning.
// 
// Used to make ends meet, prevent us from sinking into platforms or other
// objects in case we have received a knockback.
// 
// Check for intermission, if so, act on it. 
// 
// Setup the entity player model direction settings
// so others in the world can see it that way too.
// 
// Further: Calculate bobcycle, any specific events, viewoffset additions,
// sound, effects, animations, you name it.
//===============
//
//void SVG_ClientEndServerFrame(PlayerClient *ent)
//{
//    float   bobTime;
//
//    // Check whether ent is valid, and a PlayerClient hooked up 
//    // to a valid client.
//    ServersClient* client = nullptr;
//
//    if (!ent || !(client = ent->GetClient()) ||
//        !ent->IsSubclassOf<PlayerClient>()) {
//        return;
//    }
//
//    // Setup the current player and entity being processed.
//    ent = ent;
//    client = ent->GetClient();
//
//    //
//    // If the origin or velocity have changed since ClientThink(),
//    // update the pmove values.  This will happen when the client
//    // is pushed by a bmodel or kicked by an explosion.
//    //
//    // If it wasn't updated here, the view position would lag a frame
//    // behind the body position when pushed -- "sinking into plats"
//    //
//    client->playerState.pmove.origin = ent->GetOrigin();
//    client->playerState.pmove.velocity = ent->GetVelocity();
//
//    //
//    // If the end of unit layout is displayed, don't give
//    // the player any normal movement attributes
//    //
//    if (level.intermission.time) {
//        // FIXME: add view drifting here?
//        client->playerState.blend[3] = 0;
//        client->playerState.fov = 90;
//        SVG_HUD_SetClientStats(ent->GetServerEntity());
//        return;
//    }
//
//    vec3_vectors(client->aimAngles, &forward, &right, &up);
//
//    // Burn from lava, etc
//    SVG_Client_CheckWorldEffects(ent);
//
//    //
//    // Set model angles from view angles so other things in
//    // the world can tell which direction you are looking
//    //
//    vec3_t newPlayerAngles = ent->GetAngles();
//
//    if (client->aimAngles[vec3_t::Pitch] > 180)
//        newPlayerAngles[vec3_t::Pitch] = (-360 + client->aimAngles[vec3_t::Pitch]) / 3;
//    else
//        newPlayerAngles[vec3_t::Pitch] = client->aimAngles[vec3_t::Pitch] / 3;
//    newPlayerAngles[vec3_t::Yaw] = client->aimAngles[vec3_t::Yaw];
//    newPlayerAngles[vec3_t::Roll] = 0;
//    newPlayerAngles[vec3_t::Roll] = SVG_Client_CalcRoll(newPlayerAngles, ent->GetVelocity()) * 4;
//
//    // Last but not least, after having calculated the Pitch, Yaw, and Roll, set the new angles.
//    ent->SetAngles(newPlayerAngles);
//
//    //
//    // Calculate the player its X Y axis' speed and calculate the cycle for
//    // bobbing based on that.
//    //
//    vec3_t playerVelocity = ent->GetVelocity();
//    XYSpeed = std::sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);
//
//    if (XYSpeed < 5 || !(client->playerState.pmove.flags & PMF_ON_GROUND)) {
//        // Special handling for when not on ground.
//        bobMove = 0;
//
//        // Start at beginning of cycle again (See the else if statement.)
//        client->bobTime = 0;
//    } else if (ent->GetGroundEntity() || ent->GetWaterLevel() == 2) {
//        // So bobbing only cycles when on ground.
//        if (XYSpeed > 450)
//            bobMove = 0.25;
//        else if (XYSpeed > 210)
//            bobMove = 0.125;
//        else if (!ent->GetGroundEntity() && ent->GetWaterLevel() == 2 && XYSpeed > 100)
//            bobMove = 0.225;
//        else if (XYSpeed > 100)
//            bobMove = 0.0825;
//        else if (!ent->GetGroundEntity() && ent->GetWaterLevel() == 2)
//            bobMove = 0.1625;
//        else
//            bobMove = 0.03125;
//    }
//
//    // Generate bob time.
//    client->bobTime += bobMove;
//    bobTime = client->bobTime;
//
//    bobTime /= 6;
//    if (client->playerState.pmove.flags & PMF_DUCKED)
//        bobTime *= 2;   // N&C: Footstep tweak.
//
//    bobCycle = bobTime;
//    bobFracsin = std::fabsf(std::sinf(bobTime * M_PI));
//
//    // Detect hitting the floor, and apply damage appropriately.
//    SVG_Client_CheckFallingDamage(ent);
//
//    // Apply all other the damage taken this frame
//    SVG_Client_ApplyDamageFeedback(ent);
//
//    // Determine the new frame's view offsets
//    SVG_Client_CalculateViewOffset(ent);
//
//    // Determine the gun offsets
//    SVG_Client_CalculateGunOffset(ent);
//
//    // Determine the full screen color blend
//    // must be after viewOffset, so eye contents can be
//    // accurately determined
//    // FIXME: with client prediction, the contents
//    // should be determined by the client
//    SVG_Client_CalculateBlend(ent);
//
//    // Set the stats to display for this client (one of the chase isSpectator stats or...)
//    if (client->respawn.isSpectator)
//        SVG_HUD_SetSpectatorStats(ent->GetServerEntity());
//    else
//        SVG_HUD_SetClientStats(ent->GetServerEntity());
//
//    SVG_HUD_CheckChaseStats(ent->GetServerEntity());
//
//    SVG_Client_SetEvent(ent);
//
//    SVG_Client_SetEffects(ent);
//
//    SVG_Client_SetSound(ent);
//
//    SVG_Client_SetAnimationFrame(ent);
//
//    // Store velocity and view angles.
//    client->oldVelocity = ent->GetVelocity();
//    client->oldViewAngles = client->playerState.pmove.viewAngles;
//
//    // Reset weapon kicks to zer0.
//    client->kickOrigin = vec3_zero();
//    client->kickAngles = vec3_zero();
//
//    // if the scoreboard is up, update it
//    if (client->showScores && !(level.frameNumber & 31)) {
//        SVG_HUD_GenerateDMScoreboardLayout(ent, ent->GetEnemy());
//        gi.Unicast(ent->GetServerEntity(), false);
//    }
//}

