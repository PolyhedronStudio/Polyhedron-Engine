#include "ServerGameLocals.h"
#include "Entities.h"

// Class Entities.
#include "Entities/Base/SVGBaseEntity.h"
#include "Entities/Base/SVGBaseTrigger.h"
#include "Entities/Base/SVGBaseItem.h"
#include "Entities/Base/SVGBaseItemWeapon.h"
#include "Entities/Base/SVGBasePlayer.h"

// Game Modes.
#include "GameModes/IGameMode.h"

// Game world.
#include "World/ServerGameWorld.h"

// Ballistics!
#include "Ballistics.h"


/***
*
*
*   Fire Support Routines.
*
*
***/
/**
*   @brief  This is an internal support routine used for bullet/pellet based weapons.   
**/
static void Ballistics_FireBullet(SVGBasePlayer *player, const vec3_t& start, const vec3_t& aimDirection, int32_t damage, int32_t kickForce, int32_t te_impact, int32_t horizontalSpread, int32_t verticalSpread, int32_t meansOfDeath) {
    // EULER Direction.
    vec3_t direction = vec3_zero();
    // FRU vectors.
    vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();
    // end?? lol
    vec3_t shotEndPosition = vec3_zero();
    // Position of where the water hit started.
    vec3_t liquidStartPosition = vec3_zero();
    // Set to true if we're tracing through/inside of a liquid surface.
    qboolean    inLiquid = false;
    // ???
    int32_t contentMask = BrushContentsMask::Shot | BrushContentsMask::Liquid;

    // Execute a bullet line trace.
    SVGTraceResult trace = SVG_Trace(player->GetOrigin(), vec3_zero(), vec3_zero(), start, player, BrushContentsMask::Shot);

    // Did we hit anything at all?
    if (!(trace.fraction < 1.0)) {
        // Calculate euler direction.
        direction = vec3_euler(aimDirection);

        // Calculate forward, right, and up vectors based on euler direction.
        AngleVectors(direction, &forward, &right, &up);

        // Grab a random number within range of horizontalSpread and verticalSpread.
        float rightOffset   = crandom() * horizontalSpread;
        float upOffset      = crandom() * verticalSpread;

        // Calculate end coordinate of where to trace to.
        shotEndPosition = vec3_fmaf(start, WORLD_SIZE, forward);
        shotEndPosition = vec3_fmaf(shotEndPosition, rightOffset, right);
        shotEndPosition = vec3_fmaf(shotEndPosition, upOffset, up);

        // Test if we started inside of liquid.
        if (gi.PointContents(start) & BrushContentsMask::Liquid) {
            // Exciting, we started inside of a liquid, yay.
            inLiquid = true;
            // Store start position of the liquid.
            liquidStartPosition = start;

            // Remvoe liquid content mask.
            contentMask &= ~BrushContentsMask::Liquid;
        }

        // Execute another trace.
        trace = SVG_Trace(start, vec3_zero(), vec3_zero(), shotEndPosition, player, contentMask);

        // Did we hit any water?
        if (trace.contents & BrushContentsMask::Liquid) {
            // Liquid splash type.
            int32_t splashType = 0;

            // Set inLiquid to true.
            inLiquid = true;

            // Set end position as the water start position.
            liquidStartPosition = trace.endPosition;
            
            // Check whether trace start is end position.
            if (!vec3_equal(start, trace.endPosition)) {
                if (trace.contents & BrushContents::Water) {
                    //if (strcmp(tr.surface->name, "*brwater") == 0) {
                    //    color = SplashType::BrownWater;
                    //} else {
                        splashType = SplashType::BlueWater;
                    //}
                } else if (trace.contents & BrushContents::Slime) {
                    splashType = SplashType::Slime;
                } else if (trace.contents & BrushContents::Lava) {
                    splashType = SplashType::Lava;
                } else {
                    splashType = SplashType::Unknown;
                }

                // Write out a splash temp entity event.
                if (splashType != SplashType::Unknown) {
                    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//gi.WriteByte(ServerGameCommand::TempEntityEvent);
                    gi.MSG_WriteUint8(TempEntityEvent::Splash);//gi.WriteByte(TempEntityEvent::Splash);
                    gi.MSG_WriteUint8(8);//gi.WriteByte(8);
                    gi.MSG_WriteVector3(trace.endPosition, false);
                    gi.MSG_WriteVector3(trace.plane.normal, false);
                    gi.MSG_WriteUint8(splashType);//gi.WriteByte(color);
                    gi.Multicast(trace.endPosition, Multicast::PVS);
                }

                // Change bullet's course when it enters liquid.
                vec3_t direction = vec3_euler(shotEndPosition - start);
                AngleVectors(direction, &forward, &right, &up);
                
                // Grab a random number within range of horizontalSpread and verticalSpread.
                float rightOffset   = crandom() * horizontalSpread * 2;
                float upOffset      = crandom() * verticalSpread * 2;

                // Calculate end coordinate of where to trace to.
                shotEndPosition = vec3_fmaf(start, WORLD_SIZE, forward);
                shotEndPosition = vec3_fmaf(shotEndPosition, rightOffset, right);
                shotEndPosition = vec3_fmaf(shotEndPosition, upOffset, up);
            }

            // Re-trace ignoring the liquid this time.
            trace = SVG_Trace(liquidStartPosition, vec3_zero(), vec3_zero(), shotEndPosition, player, BrushContentsMask::Shot);
        }
    }

    // Send gun puff / flash
    if ( !(trace.surface && trace.surface->flags & SurfaceFlags::Sky) ) {
        if (trace.fraction < 1.0) {
            if (trace.gameEntity->GetTakeDamage()) {
                GetGameMode()->InflictDamage(trace.gameEntity, player, player, aimDirection, trace.endPosition, trace.plane.normal, damage, kickForce, DamageFlags::Bullet, meansOfDeath);
            } else {
                if (strncmp(trace.surface->name, "sky", 3) != 0 && strncmp(trace.surface->name, "sky_surfacelight", 16) != 0) {
                    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
                    gi.MSG_WriteUint8(te_impact);//WriteByte(te_impact);
                    gi.MSG_WriteVector3(trace.endPosition, false);
                    gi.MSG_WriteVector3(trace.plane.normal, false);
                    gi.Multicast(trace.endPosition, Multicast::PVS);

                    // If a player, make a player noise.
                    if (player->GetClient()) {
                        player->PlayerNoise(player, trace.endPosition, PlayerNoiseType::Impact);
                    }
                }
            }
        }
    }

    // If went through water, determine where the end is at and make a bubble trail.
    if (inLiquid) {
        vec3_t direction = vec3_normalize(trace.endPosition - liquidStartPosition);
        vec3_t position = vec3_fmaf(trace.endPosition, -2, direction);

        if (gi.PointContents(position) & BrushContentsMask::Liquid) {
            trace.endPosition = position;
        } else {
            trace = SVG_Trace(position, vec3_zero(), vec3_zero(), liquidStartPosition, trace.gameEntity, BrushContentsMask::Liquid);
        }

        position = vec3_scale(liquidStartPosition + trace.endPosition, 0.5f);

        gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);
        gi.MSG_WriteUint8(TempEntityEvent::BubbleTrailA);
        gi.MSG_WriteVector3(liquidStartPosition, false);
        gi.MSG_WriteVector3(trace.endPosition, false);
        gi.Multicast(position, Multicast::PVS);
    }
}



/***
*
*
*   Ballistic Fire Methods.
*
*
***/
/**
*   @brief  Fires a single 9mm bullet.
**/
void SVG_FireBullet(SVGBasePlayer *player, const vec3_t& start, const vec3_t& aimDirection, int32_t damage, int32_t kickForce, int32_t horizontalSpread, int32_t verticalSpread, int32_t meansOfDeath) {
    Ballistics_FireBullet(player, start, aimDirection, damage, kickForce, TempEntityEvent::Gunshot, horizontalSpread, verticalSpread, meansOfDeath);
}