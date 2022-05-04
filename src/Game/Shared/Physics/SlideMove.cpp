/***
*
*	License here.
*
*	@file
*
*	BoxSlide Movement implementation for SharedGame Physics.
* 
***/
#pragma once

// Shared Game.
#include "../SharedGame.h"

// Physics.
#include "Physics.h"
#include "SlideMove.h"


//#define IsGroundPlane( normal, gravityDir ) ( DotProduct( normal, gravityDir ) < -0.45f )

//==================================================

///*
//* SG_LinearMovement
//*/
//int GS_LinearMovement( const entity_state_t *ent, int64_t time, vec3_t dest ) {
//	vec3_t dist;
//	int moveTime;
//	float moveFrac;
//
//	moveTime = time - ent->linearMovementTimeStamp;
//	if( moveTime < 0 ) {
//		moveTime = 0;
//	}
//
//	if( ent->linearMovementDuration ) {
//		if( moveTime > (int)ent->linearMovementDuration ) {
//			moveTime = ent->linearMovementDuration;
//		}
//
//		VectorSubtract( ent->linearMovementEnd, ent->linearMovementBegin, dist );
//		moveFrac = (float)moveTime / (float)ent->linearMovementDuration;
//		Q_clamp( moveFrac, 0, 1 );
//		VectorMA( ent->linearMovementBegin, moveFrac, dist, dest );
//	} else {
//		moveFrac = moveTime * 0.001f;
//		VectorMA( ent->linearMovementBegin, moveFrac, ent->linearMovementVelocity, dest );
//	}
//
//	return moveTime;
//}
//
///*
//* SG_LinearMovementDelta
//*/
//void SG_LinearMovementDelta( const entity_state_t *ent, int64_t oldTime, int64_t curTime, vec3_t dest ) {
//	vec3_t p1, p2;
//	GS_LinearMovement( ent, oldTime, p1 );
//	GS_LinearMovement( ent, curTime, p2 );
//	VectorSubtract( p2, p1, dest );
//}

/**
*	@return	Clipped by normal velocity.
**/
inline vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce ) {
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
// SLIDE MOVE
//
// Note: groundentity info should be up to date when calling any slide move function
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

/*
* GS_SlideMoveClipMove
*/
static int SG_SlideMoveClipMove( MoveState *moveState /*, const bool stepping*/ ) {
	int32_t blockedMask = 0;

	const vec3_t endPosition = vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	//module_Trace( &trace, moveState->origin, moveState->mins, moveState->maxs, endpos, moveState->passent, moveState->contentmask, 0 );
	SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, endPosition, moveState->passEntity, moveState->contentMask );
	if( traceResult.allSolid ) {
		if( traceResult.gameEntity ) {
			SG_AddTouchEnt( moveState, traceResult.gameEntity );
		}
		return blockedMask | SlideMoveFlags::Trapped;
	}

	if( traceResult.fraction == 1.0f ) { // Was able to cleanly perform the full move.
		moveState->origin = traceResult.endPosition;
		moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
		return blockedMask | SlideMoveFlags::Moved;
	}

	if( traceResult.fraction < 1.0f ) { // Wasn't able to make the full move.
		SG_AddTouchEnt( moveState, traceResult.gameEntity );
		blockedMask |= SlideMoveFlags::PlaneTouched;

		// move what can be moved
		if( traceResult.fraction > 0.0 ) {
			moveState->origin = traceResult.endPosition;
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			blockedMask |= SlideMoveFlags::Moved;
		}

		// if the plane is a wall and stepping, try to step it up
		if( !IsWalkablePlane( traceResult.plane ) ) {
			//if( stepping && SG_StepUp( move ) ) {
			//	return blockedmask;  // solved : don't add the clipping plane
			//}
			//else {
			blockedMask |= SlideMoveFlags::WallBlocked;
			//}
		}

		SG_AddClippingPlane( moveState, traceResult.plane.normal );
	}

	return blockedMask;
}

/**
*	@brief	Executes a SlideMove for the current entity by clipping its velocity to the touching plane' normals.
*	@return	The blockedMask with the following possible flags, which when set mean:
*			- SlideMoveFlags::PlaneTouched	:	The move has touched a plane.
*			- SlideMoveFlags::WallBlocked	:	The move got blocked by a wall.
*			- SlideMoveFlags::Trapped			:	The move failed, and resulted in the moveState getting trapped.
*												When this is set the last valid Origin is stored in the MoveState.
*			- SLIDEMOVEFLAG_BLOCKED			:	The move got blocked.
*			- SlideMoveFlags::Moved			:	The move succeeded.
**/
int32_t SG_SlideMove( MoveState *moveState ) {
	static constexpr int32_t MAX_SLIDEMOVE_ATTEMPTS = 8;
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
		blockedMask = SG_SlideMoveClipMove( moveState /*, stepping*/ );

#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
		{
			//trace_t trace;
			//module_Trace( &trace, moveState->origin, moveState->mins, moveState->maxs, moveState->origin, moveState->passent, moveState->contentmask, 0 );
			SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, moveState->origin, moveState->passEntity, moveState->contentMask );
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
