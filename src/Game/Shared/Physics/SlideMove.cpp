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

//#define CHECK_TRAPPED
#define GS_SLIDEMOVE_CLAMPING

#define STOP_EPSILON    0.1


// box slide movement code (not used for player)
#define MAX_SLIDEMOVE_CLIP_PLANES   16

#define SLIDEMOVE_PLANEINTERACT_EPSILON 0.05
#define SLIDEMOVEFLAG_PLANE_TOUCHED 16
#define SLIDEMOVEFLAG_WALL_BLOCKED  8
#define SLIDEMOVEFLAG_TRAPPED       4
#define SLIDEMOVEFLAG_BLOCKED       2   // it was blocked at some point, doesn't mean it didn't slide along the blocking object
#define SLIDEMOVEFLAG_MOVED     1

//#define IsGroundPlane( normal, gravityDir ) ( DotProduct( normal, gravityDir ) < -0.45f )

//==================================================
// SNAP AND CLIP ORIGIN AND VELOCITY
//==================================================

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
	//#ifdef GS_SLIDEMOVE_CLAMPING
	{
		float oldSpeed = vec3_length(inVelocity);
		float newSpeed = vec3_length(outVelocity);

		if (newSpeed > oldSpeed) {
			outVelocity = vec3_scale(vec3_normalize(outVelocity), oldSpeed);
		}
	}
	return outVelocity;
	//#endif GS_SLIDEMOVE_CLAMPING
	//for( i = 0; i < 3; i++ ) {
	//	change = normal[i] * backoff;
	//	out[i] = in[i] - change;
	//}
	//#ifdef GS_SLIDEMOVE_CLAMPING
	//	{
	//		float oldspeed, newspeed;
	//		oldspeed = VectorLength( in );
	//		newspeed = VectorLength( out );
	//		if( newspeed > oldspeed ) {
	//			VectorNormalize( out );
	//			VectorScale( out, oldspeed, out );
	//		}
	//	}
	//#endif
}


//==================================================

///*
//* GS_LinearMovement
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
//* GS_LinearMovementDelta
//*/
//void GS_LinearMovementDelta( const entity_state_t *ent, int64_t oldTime, int64_t curTime, vec3_t dest ) {
//	vec3_t p1, p2;
//	GS_LinearMovement( ent, oldTime, p1 );
//	GS_LinearMovement( ent, curTime, p2 );
//	VectorSubtract( p2, p1, dest );
//}

//==================================================
// SLIDE MOVE
//
// Note: groundentity info should be up to date when calling any slide move function
//==================================================

/*
* GS_AddTouchEnt
*/
static void SG_AddTouchEnt( MoveState *moveState, GameEntity *geToucher ) {

	if( !moveState || !geToucher || moveState->numTouchEntities >= 32 || geToucher->GetNumber() < 0) {
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

/*
* SG_ClearClippingPlanes
*/
static void SG_ClearClippingPlanes( MoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numClipPlanes = 0;
}

/*
* GS_ClipVelocityToClippingPlanes
*/
static void SG_ClipVelocityToClippingPlanes( MoveState *moveState ) {
	int i;

	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
		if( vec3_dot( moveState->velocity, moveState->clipPlaneNormals[i] ) > (FLT_EPSILON - 1.0f) ) {
			continue; // looking in the same direction than the velocity
		}
//#ifndef TRACEVICFIX
//#ifndef TRACE_NOAXIAL
//		// this is a hack, cause non axial planes can return invalid positions in trace endpos
//		if( SetPlaneType( moveState->clipPlaneNormals[i] ) == PLANE_NONAXIAL ) {
//			// offset the origin a little bit along the plane normal
//			moveState->origin = vec3_fmaf( moveState->origin, 0.05, moveState->clipPlaneNormals[i] );
//		}
//#endif
//#endif

		moveState->velocity = SG_ClipVelocity( moveState->velocity, moveState->clipPlaneNormals[i], moveState->slideBounce );
	}
}

/*
* GS_AddClippingPlane
*/
static void SG_AddClippingPlane( MoveState *moveState, const vec3_t &planeNormal ) {
	int i;

	// see if we are already clipping to this plane
	for( i = 0; i < moveState->numClipPlanes; i++ ) {
		if( vec3_dot( planeNormal, moveState->clipPlaneNormals[i] ) > (FLT_EPSILON - 1.0f) ) {
			return;
		}
	}

	if( moveState->numClipPlanes + 1 == MAX_SLIDEMOVE_CLIP_PLANES ) {
		SG_PhysicsEntityWPrint(__func__, "[end]", "GS_AddTouchPlane: MAX_SLIDEMOVE_CLIP_PLANES reached\n" );
	}

	// add the plane
	VectorCopy( planeNormal, moveState->clipPlaneNormals[moveState->numClipPlanes] );
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
		return blockedMask | SLIDEMOVEFLAG_TRAPPED;
	}

	if( traceResult.fraction == 1.0f ) { // was able to cleanly perform the full move
		moveState->origin = traceResult.endPosition; //VectorCopy( trace.endpos, moveState->origin );
		moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
		return blockedMask | SLIDEMOVEFLAG_MOVED;
	}

	if( traceResult.fraction < 1.0f ) { // wasn't able to make the full move
		SG_AddTouchEnt( moveState, traceResult.gameEntity );
		blockedMask |= SLIDEMOVEFLAG_PLANE_TOUCHED;

		// move what can be moved
		if( traceResult.fraction > 0.0 ) {
			moveState->origin = traceResult.endPosition;
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			blockedMask |= SLIDEMOVEFLAG_MOVED;
		}

		// if the plane is a wall and stepping, try to step it up
		if( !IsWalkablePlane( traceResult.plane ) ) {
			//if( stepping && GS_StepUp( move ) ) {
			//	return blockedmask;  // solved : don't add the clipping plane
			//}
			//else {
			blockedMask |= SLIDEMOVEFLAG_WALL_BLOCKED;
			//}
		}

		SG_AddClippingPlane( moveState, traceResult.plane.normal );
	}

	return blockedMask;
}

/*
* GS_SlideMove
*/
int32_t SG_SlideMove( MoveState *moveState ) {
	static constexpr int32_t MAX_SLIDEMOVE_ATTEMPTS = 8;
	int32_t blockedMask = 0;

	// if the velocity is too small, just stop
	if( VectorLength( moveState->velocity ) < STOP_EPSILON ) {
		VectorClear( moveState->velocity );
		moveState->remainingTime = 0;
		return 0;
	}

	const vec3_t originalVelocity = moveState->velocity;
	vec3_t lastValidOrigin = moveState->origin;

	SG_ClearClippingPlanes( moveState );
	moveState->numTouchEntities = 0;

	for( int32_t count = 0; count < MAX_SLIDEMOVE_ATTEMPTS; count++ ) {
		// get the original velocity and clip it to all the planes we got in the list
		moveState->velocity = originalVelocity;
		SG_ClipVelocityToClippingPlanes( moveState );
		blockedMask = SG_SlideMoveClipMove( moveState /*, stepping*/ );

#ifdef CHECK_TRAPPED
		{
			trace_t trace;
			module_Trace( &trace, moveState->origin, moveState->mins, moveState->maxs, moveState->origin, moveState->passent, moveState->contentmask, 0 );
			if( trace.startsolid ) {
				blockedmask |= SLIDEMOVEFLAG_TRAPPED;
			}
		}
#endif

		// can't continue
		if( blockedMask & SLIDEMOVEFLAG_TRAPPED ) {
#ifdef CHECK_TRAPPED
			SG_PhysicsEntityWPrint(__func__, "[end]", "GS_SlideMove SLIDEMOVEFLAG_TRAPPED\n" );
#endif
			moveState->remainingTime = 0.0f;
			VectorCopy( lastValidOrigin, moveState->origin );
			return blockedMask;
		}

		lastValidOrigin = moveState->origin;

		// touched a plane, re-clip velocity and retry
		if( blockedMask & SLIDEMOVEFLAG_PLANE_TOUCHED ) {
			continue;
		}

		// if it didn't touch anything the move should be completed
		if( moveState->remainingTime > 0.0f ) {
			SG_PhysicsEntityWPrint(__func__, "[end]", "slidemove finished with remaining time\n" );
			moveState->remainingTime = 0.0f;
		}

		break;
	}

	return blockedMask;
}
