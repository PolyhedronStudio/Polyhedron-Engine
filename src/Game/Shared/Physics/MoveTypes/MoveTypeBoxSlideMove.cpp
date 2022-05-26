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
	    SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
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
*	@return	The number of the ground entity this entity is covering ground on.
**/
int32_t SG_BoxSlideMove_CheckForGround( GameEntity *geCheck ) {
	if (!geCheck) {
		// Warn, or just remove check its already tested elsewhere?
		return -1;
	}

	// In case of a flying or swimming monster there's no need to check for ground.
	// If anything we clear out the ground pointer in case the entity did acquire
	// flying skills.
	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
	}

	//// In case the entity has a client and its velocity is high
	//if( geCheck->GetClient() && geCheck->GetVelocity().z > 100) {//180) {
	//	geCheck->SetGroundEntity( SGEntityHandle() );
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
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
	}

	// If velocity is up, and the actual trace result did not start inside of a solid, it means we have no ground.
	const vec3_t geCheckVelocity = geCheck->GetVelocity();
	if( geCheckVelocity.z > 1 && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
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

		return traceResult.gameEntity->GetNumber();
	}

	// Should never happen..?
	return -1;
}

/**
*	@brief	Starts performing the BoxSlide move process.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, MoveState &slideMoveState ) {
	/**
	*	Ensure our SlideMove Game Entity is non (nullptr).
	**/
	if (!geSlider) {
		SG_Physics_PrintWarning( std::string(__func__) + "*geSlider is (nullptr)!" );
		return 0;
	}

	/**
	*	Store old origin and velocity. We'll need them to perhaps reset in case of any invalid movement.
	**/
	const vec3_t	oldOrigin			= geSlider->GetOrigin();
	const vec3_t	oldVelocity			= geSlider->GetVelocity();
	float			oldVelocityLength	= vec3_length( oldVelocity );

	/**
	*	Get Ground Game Entity Number, if any, store -1 (none) otherwise.
	**/
	// Validate the ground entity.
	GameEntity *geGroundEntity = SGGameWorld::ValidateEntity( geSlider->GetGroundEntityHandle() );
	// Get groundEntity number.
	int32_t groundEntityNumber = (geGroundEntity ? geGroundEntity->GetNumber() : -1);

	/**
	*	Setup our MoveState structure. Keeps track of state for the current frame's move we're about to perform.
	**/
	// The structure containing the current state of the move we're trying to perform.
	//MoveState slideMoveState = { 
	slideMoveState = {
		// Geometric Attributes.
		.velocity = geSlider->GetVelocity(),
		.origin = geSlider->GetOrigin(),
		.mins = geSlider->GetMins(),
		.maxs = geSlider->GetMaxs(),
		
		// The remaining time: Set to FRAMETIME(The time a frame takes.). Meaning, we move over time through frame.
		.remainingTime = FRAMETIME.count(),

		// Gravity Direction.
		.gravityDir = vec3_down(),
		// Slide Bounce Value.
		.slideBounce = slideBounce,

		// Ground Entity Link Count, if any Ground Entity is set, 0 otherwise.
		.groundEntityLinkCount = (groundEntityNumber >= 0 ? geSlider->GetGroundEntityLinkCount() : 0),
		// Number of the ground entity that's set on geSlider.
		.groundEntityNumber = groundEntityNumber,

		// Entity that we're moving around.
		.moveEntityNumber = geSlider->GetNumber(),
		// Entity we want to exclude(skip) from our move testing traces.
		.skipEntityNumber = geSlider->GetNumber(),

		// Entity Flags and Content Mask.
		.entityFlags = geSlider->GetFlags(),
		.contentMask = contentMask,

		// Zero clip planes and/or entities are touched at a clean move state.
		.numClipPlanes = 0,
		.numTouchEntities = 0,
	};

	/**
	*	Begin Movement by applying gravity if there's no covered ground, otherwise apply ground friction.
	**/
	//// Apply gravitational force if no ground entity is set. Otherwise, apply ground friction forces.
	//if( !geGroundEntity) {
	//	SG_AddGravity( geSlider );
	//} else {
	//	SG_AddGroundFriction( geSlider, friction ); // Horizontal Friction.
	//}

	/**
	*	If the geSlider entity has any velocty, we'll start attempting to move it around.
	**/
	// Stores the Result Move Flags after the move has completed.
	int32_t blockedMask = 0;


	if( oldVelocityLength > 0 ) {
		/**
		*	Step #1: Start attempting to slide move at our velocity.
		**/
		blockedMask = SG_SlideMove( &slideMoveState );
		
	const vec3_t org0 = slideMoveState.origin;
	const vec3_t vel0 = slideMoveState.velocity;
		//// Step down.
		//if (geGroundEntity && slideMoveState.velocity.z <= 0.1) {
		//	const vec3_t down = vec3_fmaf( slideMoveState.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		//	const SGTraceResult downTrace = SG_Trace(slideMoveState.origin, slideMoveState.mins, slideMoveState.maxs, down, geSlider, slideMoveState.contentMask );

		//	if ( downTrace.fraction >= 1.0f && !downTrace.podEntity ) {
		//		blockedMask |= SlideMoveFlags::EdgeBlocked;
		//	}

		//	// If it's not all in a solid, and the fraction is < 1, then we are stepping down
		//	// a stair or something of sorts. Fraction > 1 means we're stepping down a ledge.
		//	if ( !downTrace.allSolid ) {
		//		// Check if it is a legitimate stair case.
		//		if (downTrace.podEntity && !(downTrace.plane.normal.z >= PM_STEP_NORMAL) ) {
		//		//if ( SG_SlideMove_CheckBottom( moveState ) ) {
		//			slideMoveState.origin = downTrace.endPosition;

		//			// Add flag to our mask.
		//			blockedMask |= SlideMoveFlags::SteppedDown;
		//		}
		//	} else {
		//		// Add flag to our mask.
		//	//	blockedMask |= SlideMoveFlags::EdgeBlocked;
		//	}
		//}


		// Got blocked by a wall...
		if (blockedMask & SlideMoveFlags::WallBlocked) {

		}

		// We touched something, try and step over it.
		if ( (blockedMask & SlideMoveFlags::PlaneTouched) ) {
			//const vec3_t org1 = slideMoveState.origin;
			//const vec3_t vel1 = slideMoveState.velocity;

			//const vec3_t up = vec3_fmaf( org0, PM_STEP_HEIGHT, vec3_up() );
			//const SGTraceResult upTrace = SG_Trace( org0, slideMoveState.mins, slideMoveState.maxs, up, geSlider, slideMoveState.contentMask );

			//// There is open space to start moving from up above us.
			//if ( !upTrace.allSolid ) {
			//	// Slide Move from the higher position, using the original velocity
			//	slideMoveState.origin = upTrace.endPosition;
			//	slideMoveState.velocity = vel0;

			//	// SlideMove and addition our mask.
			//	blockedMask |= SG_SlideMove( &slideMoveState );

			//	// If we've moved, AND, did not get blocked.
			//	if ( (blockedMask & SlideMoveFlags::Moved) && !(blockedMask & SlideMoveFlags::EdgeBlocked) ) {
			//		// Settle to the new ground, keeping the step if and only if it was successful
			//		const vec3_t down = vec3_fmaf( slideMoveState.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
			//		const SGTraceResult downTrace = SG_Trace( slideMoveState.origin, slideMoveState.mins, slideMoveState.maxs, down, geSlider, slideMoveState.contentMask );

			//		if ( !downTrace.allSolid && downTrace.podEntity && ( downTrace.plane.normal.z >= PM_STEP_NORMAL ) ) { //PM_CheckStep(&downTrace)) {
			//			slideMoveState.origin = downTrace.endPosition;
			//			blockedMask |= SlideMoveFlags::SteppedUp;
			//		} else {
			//			// Store interpolation height.

			//		}
			//	} else {
			//		// Store back to old origin and velocity if we never got here.
			//		slideMoveState.origin = org0;
			//		slideMoveState.velocity = vel0;
			//	}
			//} else {
			//	// Store back to old origin and velocity if we never got here.
			//	slideMoveState.origin = org0;
			//	slideMoveState.velocity = vel0;
			//}
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

	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

    /**
    *	Step #1: Validate our geBoxSlide Entity Handle.
    **/
    GameEntity *geBoxSlide = SGGameWorld::ValidateEntity( entityHandle );

    if ( !geBoxSlide ) {
	    SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
        return;
    }


	/**
	*	Step #2:	If there is a valid ground entity, store that we're 'on-ground'.
	*				If there is no valid ground entity, check and see if we've got one for this frame.
	**/
    // Get ground entity.
    GameEntity* geGroundEntity = SGGameWorld::ValidateEntity( geBoxSlide->GetGroundEntityHandle() );

    // Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( geGroundEntity ? true : false );

	// Defaults to -1.
	int32_t groundEntityNumber = -1;

    // If we have no ground entity.
    if ( !geGroundEntity ) {
        // Ensure we check if we aren't on one in this frame already. If so, store its number for our
		// movement below.
        groundEntityNumber = SG_BoxSlideMove_CheckForGround( geBoxSlide );
    } else {
		groundEntityNumber = geGroundEntity->GetNumber();
	}


	/**
	*	Step #3:	- Check and clamp our Velocity.
	*				- Apply Rotation Friction to Angular Velocity.
	*				- Apply Ground Friction to Velocity
	*				- LASTLY: Apply Gravity:
	*							- For Walking Monsters	: Gravity if not on ground, ground friction otherwise.
	*							- For Swimming Monsters	: Gravity if not in water (Try and fall into water.)
	*							- For Flying Monsters	: ...
	**/
	const float oldVelocityLength = vec3_length( geBoxSlide->GetVelocity() );

	// Bound our velocity within sv_maxvelocity limits.
	SG_CheckVelocity( geBoxSlide );

    // Check for angular velocities. If found, add rotational friction.
    vec3_t angularVelocity = geBoxSlide->GetAngularVelocity();

    if (angularVelocity.x || angularVelocity.y || angularVelocity.z) {
        SG_AddRotationalFriction( geBoxSlide );
	}

    // Re-ensure we fetched its latest angular velocity.
    angularVelocity = geBoxSlide->GetAngularVelocity();

	// Get EntityFlags.
	const int32_t entityFlags = geBoxSlide->GetFlags();
	const int32_t waterLevel = geBoxSlide->GetWaterLevel();

	// ## Walking Monsters:
	if ( !wasOnGround ) {
		// In case of: Walking Monsters:
		if ( !( entityFlags & EntityFlags::Fly ) && !( entityFlags & EntityFlags::Swim ) ) {
			// Set HitSound Playing to True in case the velocity was a downfall one.
            if ( geBoxSlide->GetVelocity().z < GetSVGravity()->value * -0.1f ) {
                hitSound = true;
            }

            // They don't fly, and if it ain't in any water... well, add gravity.
            if ( geBoxSlide->GetWaterLevel() == 0 ) {
                SG_AddGravity( geBoxSlide );
            }
		}
	} else {
		// TODO: Move elsewhere.
		static constexpr int32_t FRICTION = 10;
		SG_AddGroundFriction( geBoxSlide, FRICTION );
	}

	// ## Flying Monsters:
    if ( ( geBoxSlide->GetFlags() & EntityFlags::Fly ) ) {
		// Friction for Vertical Velocity.
		if ( ( geBoxSlide->GetVelocity().z != 0 ) ) {
			const float speed = fabs( geBoxSlide->GetVelocity().z );
			const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
			const float friction = STEPMOVE_FRICTION / 3;
			float newSpeed = speed - ( FRAMETIME.count() * control * friction );
			if ( newSpeed < 0 ) {
				newSpeed = 0;
			}
			newSpeed /= speed;
			const vec3_t velocity = geBoxSlide->GetVelocity();
			geBoxSlide->SetVelocity( { velocity.x, velocity.y, velocity.z * newSpeed } );
		}
	}

	// ## Swimming Monsters:
	if ( ( geBoxSlide->GetFlags() & EntityFlags::Swim ) ) {
		// Friction for swimming monsters that have been given vertical velocity
		if ( (geBoxSlide->GetFlags() & EntityFlags::Swim ) && ( geBoxSlide->GetVelocity().z != 0 ) ) {
			const float speed = fabs( geBoxSlide->GetVelocity().z );
			const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
			float newSpeed = speed - ( FRAMETIME.count() * control * STEPMOVE_WATERFRICTION * geBoxSlide->GetWaterLevel() );
			if (newSpeed < 0) {
				newSpeed = 0;
			}
			newSpeed /= speed;
			const vec3_t velocity = geBoxSlide->GetVelocity();
			geBoxSlide->SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
		}
    }

	//  /**
	//  *	@brief //      // Apply friction: Let dead NPCs who aren't completely onground slide.
	//  **/
	//  if ( geBoxSlide->GetVelocity().z || geBoxSlide->GetVelocity().y || geBoxSlide->GetVelocity().x ) {
	//      // Apply friction: Let dead NPCs who aren't completely onground slide.
	//      if ( ( wasOnGround ) || ( geBoxSlide->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) ) {
	//          if ( geBoxSlide->GetDeadFlag() == DeadFlags::Dead) {//!( geBoxSlide->GetHealth() <= 0.0 ) ) {
	//              vec3_t newVelocity = geBoxSlide->GetVelocity();
	//              const float speed = sqrtf( newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1] );
	//              if (speed) {
	//                  const float friction = STEPMOVE_FRICTION;
	//                  const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
	//                  float newSpeed = speed - FRAMETIME.count() * control * friction;

	//                  if (newSpeed < 0) {
	//                      newSpeed = 0;
	//			}
	//                  newSpeed /= speed;

	//                  newVelocity[0] *= newSpeed;
	//                  newVelocity[1] *= newSpeed;

	//                  // Set the velocity.
	//                  geBoxSlide->SetVelocity( newVelocity );
	//              }
	//          }
	//}


	/**
	*	Step #4:	- Get appropriate Clip Mask.
	*				- Try and perform our slide move: Including, if wished for, stepping down/up.
	*				- ?
	**/
    // Default mask is solid.
    int32_t mask = BrushContentsMask::Solid;

    // In case of a monster, monstersolid.
    if ( geBoxSlide->GetServerFlags() & EntityServerFlags::Monster ) {
        mask = BrushContentsMask::MonsterSolid;
	}
        
	// Store old velocity for stepping.
	const vec3_t vel0 = geBoxSlide->GetVelocity();
	const vec3_t org0 = geBoxSlide->GetOrigin();

    // Execute "BoxSlideMove", essentially also our water move.
	MoveState slideMoveState;
    int32_t blockedMask = SG_BoxSlideMove( geBoxSlide, ( mask ? mask : BrushContentsMask::PlayerSolid ), 1.01f, 10, slideMoveState );

	#if defined(SG_SLIDEMOVE_DEBUG_BLOCKMASK) && SG_SLIDEMOVE_DEBUG_BLOCKMASK == 1
	if (blockedMask != 0) {
		std::string blockMaskString = "SlideMove Entity(#" + std::to_string(geBoxSlide->GetNumber()) + ") blockMask: (";
		if (blockedMask & SlideMoveFlags::SteppedUp) { blockMaskString += "SteppedUp, "; }
		if (blockedMask & SlideMoveFlags::SteppedDown) { blockMaskString += "SteppedDown, "; }
		if (blockedMask & SlideMoveFlags::PlaneTouched) { blockMaskString += "PlaneTouched, "; }
		if (blockedMask & SlideMoveFlags::WallBlocked) { blockMaskString += "WallBlocked, "; }
		if (blockedMask & SlideMoveFlags::Trapped) { blockMaskString += "Trapped, "; }
		if (blockedMask & SlideMoveFlags::EdgeBlocked) { blockMaskString += "EdgeBlocked, "; }
		if (blockedMask & SlideMoveFlags::Moved) { blockMaskString += "Moved "; }
		blockMaskString += ")";
		
		SG_Physics_PrintWarning( blockMaskString );
	} else {
		std::string blockMaskString = "SlideMove Entity(#%i) blockMask: (0)\n";
		SG_Physics_PrintWarning( blockMaskString );
	}
#endif
	/**
	*	Step #6:	- The Move has been Performed: Update Entity Attributes.
	**/
	// Double validate ground entity at this moment in time.
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( slideMoveState.groundEntityNumber ) );

	// Update the entity with the resulting moveState values.
	geBoxSlide->SetOrigin( slideMoveState.origin );
	geBoxSlide->SetVelocity( slideMoveState.velocity );
	geBoxSlide->SetMins( slideMoveState.mins );
	geBoxSlide->SetMaxs( slideMoveState.maxs );
	geBoxSlide->SetFlags( slideMoveState.entityFlags );
	geBoxSlide->SetGroundEntity( geNewGroundEntity );
	geBoxSlide->SetGroundEntityLinkCount( slideMoveState.groundEntityLinkCount );

	// Link entity in.
	geBoxSlide->LinkEntity();

	/**
	*	Step #5:	- Prevent us from moving into water if we're not a swimming monster.
	*				- Try and perform our slide move: Including, if wished for, stepping down/up.
	*				- ?
	**/
		//// This is set to false whenever one of the following checks failed:
		//// - Water Check:	We don't want these entities to just walk into a pool of water.
		//// - Edge Check:	Unless the edge is of a legit staircase height, we don't want the
		////					NPC entity to blindly walk off of it.
		//// - 
		//bool setOrigin = true;

	//----------------------------
	// CHECK: Water Check
	//----------------------------
	// To prevent this entity from going into the water. We assume that if 
	// its waterLevel already was 0, we'll maintain it at 0.
	//----------------------------
	if ( !( entityFlags & EntityFlags::Swim ) ) {
		if ( geBoxSlide->GetWaterLevel() == 0 ) {
			vec3_t point = {
				slideMoveState.origin.x,
				slideMoveState.origin.y,
				slideMoveState.origin.z + slideMoveState.mins.z - 1,
			};
			const int32_t pointContents = SG_PointContents(point);

			if (pointContents & BrushContentsMask::Liquid) {
				// TODO: ?? DO SOMETHING, MOVE FAILED.
				//return false;
			}
		}
	}

	//----------------------------
	// CHECK: Edge Check
	//----------------------------
	// Notify entity it hit an edge/a ledge.
	//----------------------------


	
	/**
	*	Step #7:	- Execute Touch Callbacks in case we add any blockedMask set.
	**/
	// Execute touch callbacks.
	if( blockedMask != 0 ) {
		GameEntity *otherEntity = nullptr;

		// Call Touch Triggers on our slide box entity for its new position.
		SG_TouchTriggers( geBoxSlide );

		// Dispatch 'Touch' callback functions to all touched entities we caught and stored in our moveState.
		for( int32_t i = 0; i < slideMoveState.numTouchEntities; i++ ) {
			otherEntity = gameWorld->GetGameEntityByIndex( slideMoveState.touchEntites[i] );
			
			// Don't touch projectiles.
			if( !otherEntity || !otherEntity->IsInUse() ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			// Call Touch for the other entity right before calling touch on geSlider.
			otherEntity->DispatchTouchCallback( otherEntity, geBoxSlide, nullptr, nullptr );

			if ( geBoxSlide ) {
				// Now call Touch on geSlider.
				geBoxSlide ->DispatchTouchCallback( geBoxSlide, otherEntity, nullptr, nullptr );
			}

			// Check if it may have been freed by the touch function, if so, break out.
			if( !geBoxSlide->IsInUse() ) {
				break;
			}
		}
	}


	/**
	*	Step #8:	- If still in use, check for ground, and see if our velocity came to a halt
	*				so we can safely trigger a Stop Dispatch callback.
	**/
	// If it's still in use, search for ground.
	if ( geBoxSlide && geBoxSlide->IsInUse() ) {
		// Check for ground entity.
		int32_t groundEntityNumber = SG_BoxSlideMove_CheckForGround( geBoxSlide );

		// Revalidate it
		GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( groundEntityNumber ) );

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geNewGroundEntity && vec3_length( geBoxSlide->GetVelocity() ) <= 1.f && oldVelocityLength > 1.f ) {
			// Zero out velocities.
			geBoxSlide->SetVelocity( vec3_zero() );
			geBoxSlide->SetAngularVelocity( vec3_zero() );

			// Stop.
			geBoxSlide->DispatchStopCallback( );
		}
	}

		// Execute touch triggers.
        SG_TouchTriggers( geBoxSlide );

        // Can't continue if this entity wasn't in use.
        if ( !geBoxSlide->IsInUse( ) ) {
            return;
		}

#ifdef SHAREDGAME_SERVERGAME
        // Check for whether to play a land sound.
        if ( geNewGroundEntity ) {
            if ( !wasOnGround ) {
                if ( hitSound ) {
                    SVG_Sound(geBoxSlide, 0, gi.SoundIndex("world/land.wav"), 1, 1, 0);
                }
            }
        }
#endif
	
	
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

			////-----------------
			//// Set origin early on to do special ledge checking.
			////-----------------
			//if ( setOrigin ) {
			//	geSlider->SetOrigin( entMove.origin );
			//}

			//----------------------------
			// CHECK: Edge Check
			//----------------------------
			// Prevents the Entity from falling off of non-stair-case edges by resorting to its old origin.
			//----------------------------
			//// Check to see if we aren't actually moving off a ledge that does not comply to being a stair.
			//if ( entMove.velocity.z >= 0 && !SG_SlideMove_CheckBottom(geSlider) ) {
			//	// TODO: It's never set anyhow, and I suppose... well we'll see.
			//	if ( geSlider->GetFlags( ) & EntityFlags::PartiallyOnGround ) {
			//		// Entity had floor mostly pulled out from underneath it and is trying to correct.
			//		geSlider->LinkEntity( );

			//		// Touch Triggers.
			//		SG_TouchTriggers( geSlider );

			//		// Not sure if to set it here as well...
			//		blockedMask |= SlideMoveFlags::EdgeBlocked;

			//		// TODO: If we do keep this block, we gotta move the code below this statement
			//		// into an else statement since we don't want to return from here.
			//		// return true;
			//	}

			//	// Add Edge Blocked flag.
			//	blockedMask |= SlideMoveFlags::EdgeBlocked;

			//	// Set origin back to old 
			//	geSlider->SetOrigin( oldOrigin );
			//}

		//	// Update the entity with the resulting moveState values.
		//	geSlider->SetVelocity( entMove.velocity );
		//	geSlider->SetFlags( entMove.entityFlags );
		//	geSlider->SetGroundEntity( entMove.groundEntity );
		//	geSlider->SetGroundEntityLinkCount( entMove.groundEntityLinkCount );		

		//	// Link entity in.
		//	geSlider->LinkEntity();
		//}

    // Last but not least, give the entity a chance to think for this frame.
    SG_RunThink(geBoxSlide);
}
