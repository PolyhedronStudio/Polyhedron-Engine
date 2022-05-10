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

// Physics.
#include "Physics.h"
#include "SlideMove.h"


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
#ifdef SG_SLIDEMOVE_CLAMPING
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

	if( !moveState || !geToucher || moveState->numTouchEntities >= 32 || geToucher->GetNumber() < 0) {
		// Warn print:
		if (!geToucher) {
			SG_PhysicsEntityWPrint(__func__, "[start]", "Trying to add a (nullptr) GameEntity\n" );
		} else if (geToucher->GetNumber() < 0) {
			SG_PhysicsEntityWPrint(__func__, "[start]", "Trying to add an invalid GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i) number\n" );
		} else if (!moveState) {
			SG_PhysicsEntityWPrint(__func__, "[start]", "moveState == (nullptr) while trying to add GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i)\n" );
		} else if (moveState->numTouchEntities >= 32) {
			SG_PhysicsEntityWPrint(__func__, "[start]", "moveState->numTouchEntities >= 32 while trying to add GameEntity(#" + std::to_string(geToucher->GetNumber()) + "% i)\n" );
		}
		
		return;
	}

	// See if it is already added.
	for( int32_t i = 0; i < moveState->numTouchEntities; i++ ) {
		if( moveState->touchEntites[i] == geToucher ) {
			return;
		}
	}

	// Otherwise, add the entity to our touched list.
	moveState->touchEntites[moveState->numTouchEntities] = geToucher;
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
	int i;

	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
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
		SG_PhysicsEntityWPrint(__func__, "[end]", "MAX_SLIDEMOVE_CLIP_PLANES reached\n" );
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
    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	SG_SlideMoveClipMove( moveState, false );

	// See if we should step down.
	if ( moveState->groundEntity && moveState->velocity.z <= 0 ) {
		const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		const SGTraceResult downTrace = SG_Trace(moveState->origin, moveState->mins, moveState->maxs, down, moveState->skipEntity, moveState->contentMask );

		// Check if we should step down or not.
		if ( !downTrace.allSolid ) {

			// Check if it is a legitimate stair case.
			if (downTrace.podEntity && !downTrace.plane.normal.z >= PM_STEP_NORMAL) {
			//if ( SG_SlideMove_CheckBottom( moveState ) ) {
				moveState->origin = downTrace.endPosition;
			}
			//}
		}
	}

    // If we are blocked, we will try to step over the obstacle.
    const vec3_t org1 = moveState->origin;
    const vec3_t vel1 = moveState->velocity;

    const vec3_t up = vec3_fmaf( org0, PM_STEP_HEIGHT, vec3_up() );
    const SGTraceResult upTrace = SG_Trace( org0, moveState->mins, moveState->maxs, up, moveState->skipEntity, moveState->contentMask );

    if ( !upTrace.allSolid ) {
        // Step from the higher position, with the original velocity
        moveState->origin = upTrace.endPosition;
        moveState->velocity = vel0;

		SG_SlideMoveClipMove( moveState, false );
        //PM_StepSlideMove_();

        // Settle to the new ground, keeping the step if and only if it was successful
        const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, moveState->skipEntity, moveState->contentMask );

        if ( !downTrace.allSolid && ( downTrace.podEntity && downTrace.plane.normal.z >= PM_STEP_NORMAL ) ) { //PM_CheckStep(&downTrace)) {
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
	int32_t blockedMask = 0;

	// Trace for the current remainingTime, by MA-ing the move velocity.
	const vec3_t endPosition = vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, endPosition, moveState->skipEntity, moveState->contentMask );
	
	// If the result was all solid, escape and add SlideMoveFlags::Trapped to our blockedMask.
	if( traceResult.allSolid ) {
		if( traceResult.gameEntity ) {
			SG_AddTouchEnt( moveState, traceResult.gameEntity );
		}
		return blockedMask | SlideMoveFlags::Trapped;
	}

	// Was able to perform the full move cleanly without any interruptions.
	if( traceResult.fraction == 1.0f ) {
		moveState->origin = traceResult.endPosition;
		moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
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
			// Subtract remainintg time.
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			// We've finished our move, add the SlideMoveFlags::Moved flag to our blockedMask.
			blockedMask |= SlideMoveFlags::Moved;
		}

		// If the plane is a wall however, and we're 'stepping', try to step up the wall.
		if( !IsWalkablePlane( traceResult.plane ) ) {
			// Step up/over the wall, otherwise add SlideMoveFlags::WalLBlocked flag to our blockedMask.
			if( stepping && SG_StepUp( moveState ) ) {
				// Return blockedMask. We won't be adding the clipping plane.
				return blockedMask;
			} else {
				// Add SliideMoveFlags::WallBlocked flag.
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
*			- SlideMoveFlags::Trapped			:	The move failed, and resulted in the moveState getting trapped.
*												When this is set the last valid Origin is stored in the MoveState.
*			- SlideMoveFlags::EdgeBlocked	:	The move got blocked by an edge. (In other words, there was no legitimate step to perform.)
*			- SlideMoveFlags::Moved			:	The move succeeded.
**/
int32_t SG_SlideMove( MoveState *moveState ) {
	static constexpr int32_t MAX_SLIDEMOVE_ATTEMPTS = 18;
	int32_t blockedMask = 0;

	// If the velocity is too small, just stop.
	if( vec3_length( moveState->velocity ) < SLIDEMOVE_STOP_EPSILON ) {
		// Zero out its velocity.
		moveState->velocity = vec3_zero();
		moveState->remainingTime = 0;
		return 0;
	}

	const vec3_t originalVelocity = moveState->velocity;
	vec3_t lastValidOrigin = moveState->origin;

	// Reset clipping plane list for the current 'SlideMove' that we're about to execute.
	SG_ClearClippingPlanes( moveState );
	moveState->numTouchEntities = 0;

	for( int32_t count = 0; count < MAX_SLIDEMOVE_ATTEMPTS; count++ ) {
		// Get the original velocity and clip it to all the planes we got in the list.
		moveState->velocity = originalVelocity;
		SG_ClipVelocityToClippingPlanes( moveState );

		// Process the actual slide move for the moveState.
		blockedMask = SG_SlideMoveClipMove( moveState, true /*, stepping*/ );

#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
		{
			//trace_t trace;
			//module_Trace( &trace, moveState->origin, moveState->mins, moveState->maxs, moveState->origin, moveState->passent, moveState->contentmask, 0 );
			SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, moveState->origin, moveState->skipEntity, moveState->contentMask );
			if( traceResult.startSolid ) {
				blockedMask |= SlideMoveFlags::Trapped;
			}
		}
#endif

		// Can't continue.
		if( blockedMask & SlideMoveFlags::Trapped ) {
#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
			SG_PhysicsEntityWPrint(__func__, "[end]", "SlideMoveFlags::Trapped\n" );
#endif
			moveState->remainingTime = 0.0f;
			// Copy back in the last valid origin we had, because the move had failed.
			moveState->origin = lastValidOrigin;
			return blockedMask;
		}

		// Update the last validOrigin to the current moveState origin.
		lastValidOrigin = moveState->origin;

		// Touched a plane, re-clip velocity and retry.
		if( blockedMask & SlideMoveFlags::PlaneTouched ) {
			continue;
		}

		// If it didn't touch anything the move should be completed
		if( moveState->remainingTime > 0.0f ) {
			SG_PhysicsEntityWPrint(__func__, "[end]", "Slidemove finished with remaining time\n" );
			moveState->remainingTime = 0.0f;
		}

		break;
	}

	return blockedMask;
}
