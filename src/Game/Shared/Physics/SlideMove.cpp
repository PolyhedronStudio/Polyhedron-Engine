/***
*
*	License here.
*
*	@file
*
*	BoxSlide Movement implementation for SharedGame Physics.
*
*	NOTE: The GroundEntity info has to be up to date before pulling off a SlideMove.
* 
***/
#pragma once

// Shared Game.
#include "../SharedGame.h"

#ifdef SHAREDGAME_SERVERGAME 
	#include "../../Server/ServerGameLocals.h"
	#include "../../Server/World/ServerGameWorld.h"
#endif
#ifdef SHAREDGAME_CLIENTGAME
	#include "../../Client/ClientGameLocals.h"
	#include "../../Client/World/ClientGameWorld.h"
#endif

// Physics.
#include "Physics.h"
#include "SlideMove.h"


void SlideMove_FixCheckBottom( GameEntity *geCheck ) {
	geCheck->SetFlags( geCheck->GetFlags() | EntityFlags::PartiallyOnGround );
}
#define STEPSIZE 18
const bool SlideMove_CheckBottom( MoveState *moveState ) {
	// Get the moveEntity.
	SGGameWorld *gameWorld = GetGameWorld();
	GameEntity *geCheck = SGGameWorld::ValidateEntity( gameWorld->GetPODEntityByIndex( moveState->moveEntityNumber ) );
	
	//vec3_t	mins, maxs, start, stop;
	SGTraceResult trace;
	int32_t 		x, y;
	float	mid, bottom;

	const vec3_t origin = moveState->origin;
	vec3_t mins = origin + moveState->mins;
	vec3_t maxs = origin + moveState->maxs;
	
// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
	vec3_t start, stop;

	start[2] = mins[2] - 1;
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (SG_PointContents (start) != BrushContents::Solid) {
				goto realcheck;
			}
		}

	return true;		// we got out easy

realcheck:
//
// check it for real...
//
	start[2] = mins[2];
	
// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
	stop[2] = start[2] - 2*STEPSIZE;
	trace = SG_Trace (start, vec3_zero(), vec3_zero(), stop, geCheck, BrushContentsMask::MonsterSolid);

	if (trace.fraction == 1.0) {
		return false;
	}
	mid = bottom = trace.endPosition[2];
	
// the corners must be within 16 of the midpoint	
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			
			trace = SG_Trace (start, vec3_zero(), vec3_zero(), stop, geCheck, BrushContentsMask::MonsterSolid);
			
			if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
				bottom = trace.endPosition[2];
			if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
				return false;
		}

	return true;
}

/**
*	@return	Clipped by normal velocity.
**/
inline vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, const float overbounce ) {
	float backoff = vec3_dot( inVelocity, normal );

	if( backoff <= 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	// Calculate out velocity vector.
	vec3_t outVelocity = ( inVelocity - vec3_scale( normal, backoff ) );

	// SlideMove clamp it.
#if defined(SG_SLIDEMOVE_CLAMPING) && SG_SLIDEMOVE_CLAMPING == 1
	{
		float oldSpeed = vec3_length(inVelocity);
		float newSpeed = vec3_length(outVelocity);

		if (newSpeed > oldSpeed) {
			outVelocity = vec3_scale(vec3_normalize(outVelocity), oldSpeed);
		}
	}
#endif
	return outVelocity;
}

//==================================================







/**
*	@brief	If within limits: Adds the geToucher GameEntity to the MoveState's touching entities list.
**/
static void SG_AddTouchEnt( MoveState *moveState, GameEntity *geToucher ) {
	// Get the touch entity number for storing in our touch entity list.
	int32_t touchEntityNumber = (geToucher ? geToucher->GetNumber() : -1);

	if( !moveState || !geToucher || moveState->numTouchEntities >= 32 || touchEntityNumber < 0) {
		// Warn print:
		if (!geToucher) {
			SG_Physics_PrintWarning( std::string(__func__) + "Trying to add a(nullptr) GameEntity" );
		} else if (geToucher->GetNumber() < 0) {
			SG_Physics_PrintWarning( std::string(__func__) + "Trying to add an invalid GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i) number" );
		} else if (!moveState) {
			SG_Physics_PrintWarning( std::string(__func__) + "moveState == (nullptr) while trying to add GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i)" );
		} else if (moveState->numTouchEntities >= 32) {
			SG_Physics_PrintWarning( std::string(__func__) + "moveState->numTouchEntities >= 32 while trying to add GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i)" );
		}
		
		return;
	}

	// See if it is already added.
	for( int32_t i = 0; i < moveState->numTouchEntities; i++ ) {
		if( moveState->touchEntites[i] == touchEntityNumber ) {
			return;
		}
	}

	// Otherwise, add the entity to our touched list.
	moveState->touchEntites[moveState->numTouchEntities] = touchEntityNumber;
	moveState->numTouchEntities++;
}

/**
*	@brief	Clears the MoveState's clipping plane list. 
*			(Does not truly vec3_zero them, just sets numClipPlanes to 0. Adding a 
*			new clipping plane will overwrite the old values.)
**/
static void SG_ClearClippingPlanes( MoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numClipPlanes = 0;
}

/**
*	@brief	Clips the moveState's velocity to all the normals stored in its current clipping plane normal list.
**/
static void SG_ClipVelocityToClippingPlanes( MoveState *moveState ) {
	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
		// Get const ref to our clip plane normal.
		const vec3_t &clipPlaneNormal = moveState->clipPlaneNormals[i];

		// Skip if its looking in the same direction than the velocity,
		if( vec3_dot( moveState->velocity, clipPlaneNormal ) > (FLT_EPSILON - 1.0f) ) {
			continue;
		}

		// Clip velocity to the clipping plane normal.
		moveState->velocity = SG_ClipVelocity( moveState->velocity, clipPlaneNormal, moveState->slideBounce );
	}
}

/**
*	@brief	If the list hasn't exceeded MAX_SLIDEMOVE_CLIP_PLANES: Adds the plane normal to the MoveState's clipping plane normals list.
**/
static void SG_AddClippingPlane( MoveState *moveState, const vec3_t &planeNormal ) {
	// Ensure we stay within limits of MAX_SLIDEMOVE_CLIP_PLANES . Warn if we don't.
	if( moveState->numClipPlanes + 1 == MAX_SLIDEMOVE_CLIP_PLANES ) {
		SG_Physics_PrintWarning( std::string(__func__) + "MAX_SLIDEMOVE_CLIP_PLANES reached" );
		return;
	}

	// See if we are already clipping to this plane.
	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
		if( vec3_dot( planeNormal, moveState->clipPlaneNormals[i] ) > (FLT_EPSILON - 1.0f) ) {
			return;
		}
	}

	// Add the plane.
	moveState->clipPlaneNormals[moveState->numClipPlanes] = planeNormal;
	moveState->numClipPlanes++;
}

/**
*	@brief	Handles checking whether an entity can step up a brush or not. (Or entity, of course.)
*	@return	True if the move stepped up.
**/
static const int32_t SG_SlideMoveClipMove( MoveState *moveState, const bool stepping );
static bool SG_StepUp( MoveState *moveState ) {
	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();
	// Get and Validate Ground Entity.
	GameEntity *geGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( moveState->groundEntityNumber ) );
	// Get and Validate Skip Entity.
	GameEntity *geSkipEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( moveState->skipEntityNumber ) );

    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	// See if we should step down.
	if ( geGroundEntity && moveState->velocity.z <= 0 ) {
		const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		const SGTraceResult downTrace = SG_Trace(moveState->origin, moveState->mins, moveState->maxs, down, geSkipEntity, moveState->contentMask );

		// Check if we should step down or not.
		if ( !downTrace.allSolid ) {

			// Check if it is a legitimate stair case.
			if (downTrace.podEntity && !(downTrace.plane.normal.z >= PM_STEP_NORMAL)) {
			//if ( SG_SlideMove_CheckBottom( moveState ) ) {
				moveState->origin = downTrace.endPosition;
			}
			//}
		}
	}
	
	SG_SlideMoveClipMove( moveState, false );

    // If we are blocked, we will try to step over the obstacle.
    const vec3_t org1 = moveState->origin;
    const vec3_t vel1 = moveState->velocity;

    const vec3_t up = vec3_fmaf( org0, PM_STEP_HEIGHT, vec3_up() );
    const SGTraceResult upTrace = SG_Trace( org0, moveState->mins, moveState->maxs, up, geSkipEntity, moveState->contentMask );

    if ( !upTrace.allSolid ) {
        // Step from the higher position, with the original velocity
        moveState->origin = upTrace.endPosition;
        moveState->velocity = vel0;

		SG_SlideMoveClipMove( moveState, false );
        //PM_StepSlideMove_();

        // Settle to the new ground, keeping the step if and only if it was successful
        const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkipEntity, moveState->contentMask );

        if ( !downTrace.allSolid && ( downTrace.podEntity && downTrace.plane.normal.z >= PM_STEP_NORMAL ) ) { //PM_CheckStep(&downTrace)) {
            // Quake2 trick jump secret sauce
#if 0            
			if ( (moveState->groundEntity) || vel0.z < PM_SPEED_UP ) {
#endif
				//if ( SG_SlideMove_CheckBottom( moveState ) ) {
					// Yeah... I knwo.
					moveState->origin = downTrace.endPosition;
				//}
				// Calculate step height.
				//moveState->stepHeight = moveState->origin.z - moveState-> 
#if 0
			} else {
				// Set it back?
//                moveState->origin = org1;
//				return false;
				//pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
            }
#endif

            return true;
        }
    }
	
	// Save end results.
    moveState->origin = org1;
    moveState->velocity = vel1;

	return false;
}

/**
*	@brief	GS_Performs a substep of the actual SG_SlideMove
**/
static const int32_t SG_SlideMoveClipMove( MoveState *moveState, const bool stepping ) {
	// Contains all flags indicating the results of this moveState's move.
	int32_t blockedMask = 0;

	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

	// Trace for the current remainingTime, by MA-ing the move velocity.
	const vec3_t endPosition = vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, endPosition, geSkip, moveState->contentMask );
	
	// If the result was all solid, escape and add SlideMoveFlags::Trapped to our blockedMask.
	if( traceResult.allSolid ) {
		if( traceResult.gameEntity ) {
			SG_AddTouchEnt( moveState, traceResult.gameEntity );
		}
		return blockedMask | SlideMoveFlags::Trapped;
	}

	// Was able to perform the full move cleanly without any interruptions.
	if( traceResult.fraction == 1.0f ) {
		// Update new move settlement origin.
		moveState->origin = traceResult.endPosition;
		// Remaining time should if all is well, be 0.
		moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
		// Move succeeded, add Moved to our blockedMask and return.
		return blockedMask | SlideMoveFlags::Moved;
	}

	// Unable to make the full move.
	if( traceResult.fraction < 1.0f ) { // Wasn't able to make the full move.
		// Add the touched entity to our list.
		SG_AddTouchEnt( moveState, traceResult.gameEntity );
		// Add SlideMoveFlags::planeTouched to our blockedMask.
		blockedMask |= SlideMoveFlags::PlaneTouched;

		// Move the 'fraction' that we at least can move.
		if( traceResult.fraction > 0.0 ) {
			// Assign origin.
			moveState->origin = traceResult.endPosition;
			// Subtract remainint time.
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			// We've finished our move, add the SlideMoveFlags::Moved flag to our blockedMask.
			blockedMask |= SlideMoveFlags::Moved;
		}

		// If the plane is a wall however, and we're 'stepping', try to step up the wall.
		if( !IsWalkablePlane( traceResult.plane ) ) {
			//// Step up/over the wall, otherwise add SlideMoveFlags::WalLBlocked flag to our blockedMask.
			if( stepping /* && SG_StepUp(moveState) */ ) {
			//	// Return blockedMask. We won't be adding the clipping plane.
				return blockedMask;
			} else {
				// Add SlideMoveFlags::WallBlocked flag.
				blockedMask |= SlideMoveFlags::WallBlocked;
			}
		}

		// If we've reached this point, it's clear this is one of our planes to clip against.
		SG_AddClippingPlane( moveState, traceResult.plane.normal );
	}

	// Return blockedmask.
	return blockedMask;
}

/**
*	@brief	Executes a SlideMove for the current entity by clipping its velocity to the touching plane' normals.
*	@return	The blockedMask with the following possible flags, which when set mean:
*			- SlideMoveFlags::PlaneTouched	:	The move has touched a plane.
*			- SlideMoveFlags::WallBlocked	:	The move got blocked by a wall.
*			- SlideMoveFlags::Trapped		:	The move failed, and resulted in the moveState getting trapped.
*												When this is set the last valid Origin is stored in the MoveState.
*			- SlideMoveFlags::EdgeBlocked	:	The move got blocked by an edge. (In other words, there was no legitimate step to perform.)
*			- SlideMoveFlags::Moved			:	The move succeeded.
**/
int32_t SG_SlideMove( MoveState *moveState ) {
	static constexpr int32_t MAX_SLIDEMOVE_ATTEMPTS = 18;
	//! Keeps score of the 
	int32_t blockedMask = 0;
	
	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

	// If the velocity is too small, just stop.
	if( vec3_length( moveState->velocity ) < SLIDEMOVE_STOP_EPSILON ) {
		// Zero out its velocity.
		moveState->velocity = vec3_zero();
		// Zero out remaining time.
		moveState->remainingTime = 0;
		// Return blockedMask.
		return blockedMask; // blockedMask | SlideMoveFlags::SmallVelocity
	}

	// Reset clipping plane list for the current 'SlideMove' that we're about to execute.
	SG_ClearClippingPlanes( moveState );
	// Reset number of touched entities.
	moveState->numTouchEntities = 0;

	// Store original velocity to use each slidemove attempt.
	const vec3_t originalVelocity = moveState->velocity;

	// Keep track of our last valid origin during our slidemove attempts.
	vec3_t lastValidOrigin = moveState->origin;
	vec3_t lastValidVelocity = moveState->velocity;

	for( int32_t count = 0; count < MAX_SLIDEMOVE_ATTEMPTS; count++ ) {
		// Get the original velocity and clip it to all the planes we got in the list.
		moveState->velocity = originalVelocity;
		SG_ClipVelocityToClippingPlanes( moveState );

		// Process the actual slide move for the moveState.
		blockedMask = SG_SlideMoveClipMove( moveState, true );

#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
		{
			SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, moveState->origin, geSkip, moveState->contentMask );
			if( traceResult.startSolid ) {
				blockedMask |= SlideMoveFlags::Trapped;
			}
		}
#endif

		// Can't continue.
		if( blockedMask & SlideMoveFlags::Trapped ) {
#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
			SG_Physics_PrintWarning( std::string(__func__) + "SlideMoveFlags::Trapped" );
#endif
			moveState->remainingTime = 0.0f;
			// Copy back in the last valid origin we had, because the move had failed.
			moveState->origin = lastValidOrigin;
			return blockedMask;
		}
		

		// Update the last validOrigin to the current moveState origin.
		lastValidOrigin = moveState->origin;
		lastValidVelocity = moveState->velocity;

		// Moved: Try and step down to find ground.
		if ( blockedMask & SlideMoveFlags::Moved ) {

			if ( lastValidVelocity.z <= 0.1f ) {
				const vec3_t down = vec3_fmaf( lastValidOrigin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
				const SGTraceResult downTrace = SG_Trace( lastValidOrigin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );

				// Did our trace start non solid, hit an entity, and is it a walkable plane?
				if ( !downTrace.allSolid && downTrace.podEntity && IsWalkablePlane(downTrace.plane)) {
					// Good, then we've now stepped down.
					moveState->origin = downTrace.endPosition;

					// Add to our mask.
					blockedMask |= SlideMoveFlags::SteppedDown;
				} else {
					// Something is iffy, so check the fraction.
					if (downTrace.fraction >= 1.0f) {
						blockedMask |= SlideMoveFlags::EdgeBlocked;
					}
				}
			}
		}

		// Touched a plane, re-clip velocity and retry.
		if( blockedMask & SlideMoveFlags::PlaneTouched && !( blockedMask & SlideMoveFlags::EdgeBlocked )  ) {
////////////////////////////////////////////////////////
			//continue;
			const vec3_t up = vec3_fmaf( lastValidOrigin, PM_STEP_HEIGHT, vec3_up() );
			const SGTraceResult upTrace = SG_Trace( lastValidOrigin, moveState->mins, moveState->maxs, up, geSkip, moveState->contentMask );

			// There is open space to start moving from up above us.
			if ( !upTrace.allSolid ) {
				// Slide Move from the higher position, using the original velocity
				moveState->origin = upTrace.endPosition;
				moveState->velocity = originalVelocity;

				// SlideMove and addition our mask.
				blockedMask |= SG_SlideMoveClipMove( moveState, false );

				const vec3_t org1 = moveState->origin;
				const vec3_t vel1 = moveState->velocity;

				// If we've moved, AND, did not get blocked.
				if ( (blockedMask & SlideMoveFlags::Moved) && !(blockedMask & SlideMoveFlags::EdgeBlocked) ) {
					// Settle to the new ground, keeping the step if and only if it was successful
					const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
					const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );

					if ( !downTrace.allSolid && downTrace.podEntity && ( downTrace.plane.normal.z >= PM_STEP_NORMAL ) ) { //PM_CheckStep(&downTrace)) {
						moveState->origin = downTrace.endPosition;
						lastValidOrigin = moveState->origin;
						lastValidVelocity = moveState->velocity;
						blockedMask |= SlideMoveFlags::SteppedUp;
					} else {
						// Store interpolation height.
						// -- TODO : calculate.

						// Store back to old origin and velocity if we never got here.
						moveState->origin = lastValidOrigin;	
						moveState->velocity = vel1;
						continue;
					}
				} else {
					// Store back to old origin and velocity if we never got here.
					moveState->origin = lastValidOrigin;
					moveState->velocity = vel1;
					
					continue;
				}
			} else {
				// Store back to old origin and velocity if we never got here.
				//moveState->origin	= org1;
				//moveState->velocity	= vel1;

				continue;
			}
			continue;
		}

		// If it didn't touch anything the move should be completed
		if( moveState->remainingTime > 0.0f ) {
			SG_Physics_PrintWarning( std::string(__func__) + "Slidemove finished with remaining time" );
			moveState->remainingTime = 0.0f;
		}

		break;
	}

	return blockedMask;
}
