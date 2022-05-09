/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once

// Shared Game.
#include "../../SharedGame.h"

#ifdef SHAREDGAME_SERVERGAME 
	#include "../../../Server/ServerGameLocals.h"
	#include "../../../Server/World/ServerGameWorld.h"
#endif
#ifdef SHAREDGAME_CLIENTGAME
	#include "../../../Client/ClientGameLocals.h"
	#include "../../../Client/World/ClientGameWorld.h"
#endif

// Physics.
#include "../Physics.h"
#include "../SlideMove.h"
// TODO: This needs some fixing hehe... ugly method but hey.
#ifdef SHAREDGAME_SERVERGAME
extern cvar_t *sv_maxvelocity;
extern cvar_t *GetSVGravity();
extern void CheckSVCvars();
#endif

#ifdef SHAREDGAME_CLIENTGAME
extern cvar_t *GetSVMaxVelocity();
extern cvar_t *GetSVGravity();
#endif







//==================================================
// The following should actually be more monster code.
//==================================================
bool SG_SlideMove_CheckBottom(GameEntity* geCheck) {
    vec3_t  start, stop;
    SGTraceResult trace;
    int32_t x, y;
    float   mid, bottom;

    vec3_t mins = geCheck->GetOrigin() + geCheck->GetMins(); //VectorAdd(geCheck->currentState.origin, geCheck->mins, mins);
    vec3_t maxs = geCheck->GetOrigin() + geCheck->GetMaxs(); //VectorAdd(geCheck->currentState.origin, geCheck->maxs, maxs);


    // if all of the points under the corners are solid world, don't bother
    // with the tougher checks
    // the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (SG_PointContents(start) != BrushContents::Solid) {
                goto realcheck;
			}
        }
	}

    return true;        // we got out easy

realcheck:
    //
    // check it for real...
    //
    start[2] = mins[2];

    // the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2] = start[2] - 2 * PM_STEP_HEIGHT;
    trace = SG_Trace(start, vec3_zero(), vec3_zero(), stop, geCheck, SG_SolidMaskForGameEntity(geCheck));

    if (trace.fraction == 1.0) {
        return false;
	}
    mid = bottom = trace.endPosition[2];

    // The corners must be within 16 of the midpoint.
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = SG_Trace(start, vec3_zero(), vec3_zero(), stop, geCheck, SG_SolidMaskForGameEntity(geCheck));

            if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
                bottom = trace.endPosition[2];
            if (trace.fraction == 1.0 || mid - trace.endPosition[2] > PM_STEP_HEIGHT)
                return false;
        }
	}

    return true;
}

//==================================================

//========================================================================


static constexpr float STEPMOVE_STOPSPEED = 100.f;
static constexpr float STEPMOVE_FRICTION = 6.f;
static constexpr float STEPMOVE_WATERFRICTION = 1.f;

/**
*	@brief	Processes rotational friction calculations.
**/
static void SG_AddRotationalFriction( SGEntityHandle entityHandle ) { 
	// Assign handle to base entity.
    GameEntity *ent = *entityHandle;

    // Ensure it is a valid entity.
    if ( !ent ) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Acquire the rotational velocity first.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    // Set angles in proper direction.
    ent->SetAngles( vec3_fmaf(ent->GetAngles(), FRAMETIME.count(), angularVelocity) );

    // Calculate adjustment to apply.
    float adjustment = FRAMETIME.count() * STEPMOVE_STOPSPEED * STEPMOVE_FRICTION;

    // Apply adjustments.
    angularVelocity = ent->GetAngularVelocity();
    for (int32_t n = 0; n < 3; n++) {
        if (angularVelocity[n] > 0) {
            angularVelocity[n] -= adjustment;
            if (angularVelocity[n] < 0)
                angularVelocity[n] = 0;
        } else {
            angularVelocity[n] += adjustment;
            if (angularVelocity[n] > 0)
                angularVelocity[n] = 0;
        }
    }

    // Last but not least, set the new angularVelocity.
    ent->SetAngularVelocity( angularVelocity );
}

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
**/
static void SG_BoxSlideMove_CheckForGround( GameEntity *geCheck ) {
	if (!geCheck) {
		// Warn, or just remove check its already tested elsewhere?
		return;
	}

	// In case of a flying or swimming monster there's no need to check for ground.
	// If anything we clear out the ground pointer in case the entity did acquire
	// flying skills.
	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	//// In case the entity has a client and its velocity is high
	//if( geCheck->GetClient() && geCheck->GetVelocity().z > 100) {//180) {
	//	geCheck->SetGroundEntity(nullptr);
	//	geCheck->SetGroundEntityLinkCount(0);
	//	return;
	//}

	// if the hull point one-quarter unit down is solid the entity is on ground
	const vec3_t geOrigin = geCheck->GetOrigin();
	vec3_t traceEndPoint = {
		geOrigin.x,
		geOrigin.y,
		geOrigin.z - 0.25f
	};

	// Execute ground seek trace.
	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck));


	// Check steepness.
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// If velocity is up, and the actual trace result did not start inside of a solid, it means we have no ground.
	const vec3_t geCheckVelocity = geCheck->GetVelocity();
	if( geCheckVelocity.z > 1 && !traceResult.startSolid ) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// The trace did not start, or end inside of a solid.
	if( !traceResult.startSolid && !traceResult.allSolid ) {
		//VectorCopy( trace.endpos, ent->s.origin );
		geCheck->SetGroundEntity(traceResult.gameEntity);
		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;

		// Since we've found ground, we make sure that any negative Z velocity is zero-ed out.
		if( geCheckVelocity .z < 0) {
			geCheck->SetVelocity({ 
				geCheckVelocity.x,
				geCheckVelocity.y,
				0
			});
		}
	}
}

/**
*	@brief	Starts performing the BoxSlide move process.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction ) {
	// The structure containing the current state of the move we're trying to perform.
	MoveState entMove = { };

	// The mask containing a set of SlideMoveFlags describing the outcome of this move.
	int32_t blockedMask = 0;

	// Warn and exit in case our entity is invalid.
	if ( !geSlider ) {
		SG_PhysicsEntityWPrint(__func__, "[start]", "geSlider is (nullptr)!\n");
		return 0;
	}

	// Store old origin and velocity.
	const vec3_t oldOrigin = geSlider->GetOrigin();
	float oldVelocity = vec3_length( geSlider->GetVelocity() );

	// Apply gravitational force if no ground entity is set. Otherwise, apply ground friction forces.
	if( !geSlider->GetGroundEntityHandle( ) ) {
		SG_AddGravity( geSlider );
	} else {
		SG_AddGroundFriction( geSlider, friction ); // Horizontal Friction.
	}

	// Initialize our move state.
	entMove.numClipPlanes		= 0;
	entMove.numTouchEntities	= 0;

	// When the entity isn't idle, move its properties over into our move state.
	if( oldVelocity > 0 ) {
		// Setup remaining move time.
		Frametime remainingTime = level.time;
		entMove.remainingTime = FRAMETIME.count( );

		// Setup general properties.
		entMove.skipEntity		= geSlider;
		entMove.entityFlags		= geSlider->GetFlags();
		entMove.contentMask		= contentMask;
		entMove.gravityDir		= vec3_t { 0.f, 0.f, -1.f };
		entMove.slideBounce		= slideBounce;
		entMove.groundEntity	= ( *geSlider->GetGroundEntityHandle() );

		// Setup Physical properties.
		entMove.origin			= geSlider->GetOrigin( );
		entMove.velocity		= geSlider->GetVelocity( );
		entMove.mins			= geSlider->GetMins( );
		entMove.maxs			= geSlider->GetMaxs( );

		// Execute actual slide movement.
		blockedMask = SG_SlideMove( &entMove );

		// Got blocked by a wall...
		if (blockedMask & SlideMoveFlags::WallBlocked) {

		}

		// This is set to false whenever one of the following checks failed:
		// - Water Check:	We don't want these entities to just walk into a pool of water.
		// - Edge Check:	Unless the edge is of a legit staircase height, we don't want the
		//					NPC entity to blindly walk off of it.
		// - 
		bool setOrigin = true;

		//----------------------------
		// CHECK: Water Check
		//----------------------------
		// To prevent this entity from going into the water. We assume that if 
		// its waterLevel already was 0, we'll maintain it at 0.
		//----------------------------
		if (geSlider->GetWaterLevel()  == 0) {
			vec3_t point = {
				entMove.origin.x,
				entMove.origin.y,
				entMove.origin.z + entMove.mins.z - 1,
			};
			const int32_t pointContents = SG_PointContents(point);

			if (pointContents & BrushContentsMask::Liquid) {
				return false;
			}
		}


	//
	//	Seemingly needs re-enabling only after we got func_plat etc to work again.
	//
	//if (trace.fraction == 1)
	//{
	//// if monster had the ground pulled out, go ahead and fall
	//	if ( ent->flags & FL_PARTIALGROUND )
	//	{
	//		VectorAdd (ent->s.origin, move, ent->s.origin);
	//		if (relink)
	//		{
	//			GClip_LinkEntity (ent);
	//			GClip_TouchTriggers (ent);
	//		}
	//		ent->groundentity = NULL;
	//		return true;
	//	}
	//
	//	return false;		// walked off an edge
	//}

		//-----------------
		// Set origin early on to do special ledge checking.
		//-----------------
		if ( setOrigin ) {
			geSlider->SetOrigin( entMove.origin );
		}

		//----------------------------
		// CHECK: Edge Check
		//----------------------------
		// Prevents the Entity from falling off of non-stair-case edges by resorting to its old origin.
		//----------------------------
		// Check to see if we aren't actually moving off a ledge that does not comply to being a stair.
		if ( entMove.velocity.z >= 0 && !SG_SlideMove_CheckBottom(geSlider) ) {
			// TODO: It's never set anyhow, and I suppose... well we'll see.
			if ( geSlider->GetFlags( ) & EntityFlags::PartiallyOnGround ) {
				// Entity had floor mostly pulled out from underneath it and is trying to correct.
				geSlider->LinkEntity( );

				// Touch Triggers.
				SG_TouchTriggers( geSlider );

				// Not sure if to set it here as well...
				blockedMask |= SlideMoveFlags::EdgeBlocked;

				// TODO: If we do keep this block, we gotta move the code below this statement
				// into an else statement since we don't want to return from here.
				// return true;
			}

			// Add Edge Blocked flag.
			blockedMask |= SlideMoveFlags::EdgeBlocked;

			// Set origin back to old 
			geSlider->SetOrigin( oldOrigin );
		}

		// Update the entity with the resulting moveState values.
		geSlider->SetVelocity( entMove.velocity );
		geSlider->SetFlags( entMove.entityFlags );
		geSlider->SetGroundEntity( entMove.groundEntity );
		geSlider->SetGroundEntityLinkCount( entMove.groundEntityLinkCount );		

		// Link entity in.
		geSlider->LinkEntity();
	}

	// Execute touch callbacks.
	if( contentMask != 0 ) {
		GameEntity *otherEntity = nullptr;
		//GClip_TouchTriggers( ent );
		SG_TouchTriggers( geSlider );

		// Dispatch 'Touch' callback functions to all touched entities residing in our moveState.
		for( int32_t i = 0; i < entMove.numTouchEntities; i++ ) {
			otherEntity = entMove.touchEntites[i];
			
			// Don't touch projectiles.
			if( !otherEntity ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			// Call Touch for the other entity right before calling touch on geSlider.
			otherEntity->DispatchTouchCallback( otherEntity, geSlider, nullptr, nullptr );

			if ( geSlider ) {
				// Now call Touch on geSlider.
				geSlider->DispatchTouchCallback( geSlider, otherEntity, nullptr, nullptr );
			}

			// Check if it may have been freed by the touch function, if so, break out.
			if( !geSlider->IsInUse() ) {
				break;
			}
		}
	}

	// If it's still in use, search for ground.
	if ( geSlider && geSlider->IsInUse() ) {
		// Check for ground entity.
		SG_BoxSlideMove_CheckForGround( geSlider );

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geSlider->GetGroundEntityHandle() && vec3_length(geSlider->GetVelocity()) <= 1.f && oldVelocity > 1.f) {
			// Zero out velocities.
			geSlider->SetVelocity( vec3_zero() );
			geSlider->SetAngularVelocity( vec3_zero() );

			// Stop.
			geSlider->DispatchStopCallback( );
		}
	}

	return blockedMask;
}

/**
*	@brief	Performs a velocity based 'slidebox' movement for general NPC's.
*			If FL_SWIM or FL_FLY are not set, it'll check whether there is any
*			ground at all, if it has been removed the monster will be pushed
*			downwards due to gravity effect kicking in.
**/
void SG_Physics_BoxSlideMove(SGEntityHandle &entityHandle) {
    // Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;

    // Check if handle is valid.    
    GameEntity *ent = *entityHandle;

    if (!ent) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Retrieve ground entity.
    GameEntity* groundEntity = *ent->GetGroundEntityHandle();

    // If we have no ground entity.
    if (!groundEntity) {
        // Ensure we check if we aren't on one in this frame already.
        SG_BoxSlideMove_CheckForGround(ent);
    }

    //if (!groundEntity) {
    //    return;
    //}

    // Store whether we had a ground entity at all.
    qboolean wasOnGround = (groundEntity ? true : false);

    // Bound our velocity within sv_maxvelocity limits.
    SG_CheckVelocity(ent);

    // Check for angular velocities. If found, add rotational friction.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    if (angularVelocity.x || angularVelocity.y || angularVelocity.z) {
        SG_AddRotationalFriction(ent);
	}

    // Re-ensure we fetched its latest angular velocity.
    angularVelocity = ent->GetAngularVelocity();

    // Add gravity except for: 
    // - Flying monsters
    // - Swimming monsters who are in the water
    if (!wasOnGround) {
        // If it is not a flying monster, we are done.
        if (!(ent->GetFlags() & EntityFlags::Fly)) {
            // In case the swim mosnter is not in water...
            if (!((ent->GetFlags() & EntityFlags::Swim) && (ent->GetWaterLevel() > 2))) {
                // Determine whether to play a "hit sound".
                if (ent->GetVelocity().z < GetSVGravity()->value * -0.1) {
                    hitSound = true;
                }

                // Add gravity in case the monster is not in water, it can't fly, so it falls.
                if (ent->GetWaterLevel() == 0) {
                    SG_AddGravity(ent);
                }
            }
        }
    }

    // Friction for flying monsters that have been given vertical velocity
    if ((ent->GetFlags() & EntityFlags::Fly) && (ent->GetVelocity().z != 0)) {
        const float speed = fabs(ent->GetVelocity().z);
        const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
        const float friction = STEPMOVE_FRICTION / 3;
        float newSpeed = speed - (FRAMETIME.count() * control * friction);
        if (newSpeed < 0) {
            newSpeed = 0;
		}
        newSpeed /= speed;
        const vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
    }

    // Friction for flying monsters that have been given vertical velocity
    if ((ent->GetFlags() & EntityFlags::Swim) && (ent->GetVelocity().z != 0)) {
        const float speed = fabs(ent->GetVelocity().z);
        const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
        float newSpeed = speed - (FRAMETIME.count() * control * STEPMOVE_WATERFRICTION * ent->GetWaterLevel());
        if (newSpeed < 0) {
            newSpeed = 0;
		}
        newSpeed /= speed;
        const vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
    }

    // In case we have velocity, execute movement logic.
    if (ent->GetVelocity().z || ent->GetVelocity().y || ent->GetVelocity().x) {
        // Apply friction: Let dead NPCs who aren't completely onground slide.
        if ((wasOnGround) || (ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly))) {
            if (!(ent->GetHealth() <= 0.0)) {
                vec3_t vel = ent->GetVelocity();
                const float speed = sqrtf(vel[0] * vel[0] + vel[1] * vel[1]);
                if (speed) {
                    const float friction = STEPMOVE_FRICTION;
                    const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
                    float newSpeed = speed - FRAMETIME.count() * control * friction;

                    if (newSpeed < 0) {
                        newSpeed = 0;
					}
                    newSpeed /= speed;

                    vel[0] *= newSpeed;
                    vel[1] *= newSpeed;

                    // Set the velocity.
                    ent->SetVelocity(vel);
                }
            }
		}

        // Default mask is solid.
        int32_t mask = BrushContentsMask::Solid;

        // In case of a monster, monstersolid.
        if (ent->GetServerFlags() & EntityServerFlags::Monster) {
            mask = BrushContentsMask::MonsterSolid;
		}
        
        // Execute "BoxSlideMove", essentially also our water move.
        SG_BoxSlideMove(ent, ( mask ? mask : BrushContentsMask::PlayerSolid ), 1.01f, 10 );
		//SVG_FlyMove(ent, FRAMETIME.count(), mask);

        // Link.
        ent->LinkEntity();

		// Execute touch triggers.
        SG_TouchTriggers(ent);

        // Can't continue if this entity wasn't in use.
        if ( !ent->IsInUse( ) ) {
            return;
		}

#ifdef SHAREDGAME_SERVERGAME
        // Check for whether to play a land sound.
        if ( ent->GetGroundEntityHandle() ) {
            if ( !wasOnGround ) {
                if ( hitSound ) {
                    SVG_Sound(ent, 0, gi.SoundIndex("world/land.wav"), 1, 1, 0);
                }
            }
        }
#endif
    }

    // Last but not least, give the entity a chance to think for this frame.
    SG_RunThink(ent);
}
