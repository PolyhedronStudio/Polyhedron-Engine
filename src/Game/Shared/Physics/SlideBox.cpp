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

//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"

// Physics.
#include "Physics.h"
#include "SlideBox.h"



/**
*	Constants.
**/
//! Maximum of Clipping Planes to bump and slide along to.
static constexpr int32_t MAX_SLIDEBOX_CLIP_PLANES = 16;

//! Epsilon value for plane interaction tests.
static constexpr float SLIDEBOX_PLANEINTERACT_EPSILON = 0.015625;

// 
static constexpr int32_t SLIDEBOXFLAG_PLANE_TOUCHED = 16;
static constexpr int32_t SLIDEBOXFLAG_WALL_BLOCKED = 8;
static constexpr int32_t SLIDEBOXFLAG_TRAPPED = 4;
static constexpr int32_t SLIDEBOXFLAG_BLOCKED = 2;   // it was blocked at some point, doesn't mean it didn't slide along the blocking object
static constexpr int32_t SLIDEBOXFLAG_MOVED = 1;

//#define SG_SLIDEBOX_DEBUG_TRAPPED

static constexpr float STOP_EPSILON = 0.1;

//static inline const bool IsGroundPlane( const CollisionPlane &plane, const vec3_t &gravityDir) {
//	return ( vec3_dot( plane.normal, gravityDir ) < -0.45f);
//}



/***
*
*
*	SlideBox Movement.
*
*
***/
/**
*	@brief	Wrapper for easily tracing our slide box moves. All pointer arguments are optionable
*			in that if set, it overrides the values it'd otherwise grab from the SlideBoxMove* itself.
**/
static SGTraceResult SBM_Trace( SlideBoxMove* move, const vec3_t *origin, const vec3_t *mins, const vec3_t *maxs, const vec3_t *end, const int32_t skipEntityNumber = -1, const int32_t contentMask = -1 ) {
	/**
	*	#0: Determine whether to use move state or custom values.
	**/
	const vec3_t traceOrigin	= ( origin != nullptr ? *origin : move->origin );
	const vec3_t traceMins		= ( mins != nullptr ? *mins: move->mins );
	const vec3_t traceMaxs		= ( maxs != nullptr ? *maxs : move->maxs );
	const vec3_t traceEnd		= ( end != nullptr ? *end : move->origin );

	const int32_t traceSkipEntityNumber = ( skipEntityNumber != -1 ? skipEntityNumber : move->skipEntityNumber );
	const int32_t traceContentMask = ( contentMask != -1 ? contentMask : move->contentMask );

	/**
	*	#1: Fetch needed skip entity.
	**/
	// Get Gameworld.
	SGGameWorld *gameWorld = GetGameWorld();
	// Acquire our skip trace entity.
	GameEntity *geSkip = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( traceSkipEntityNumber ) );

	// Perform and return trace results.
	if ( geSkip ) {
		// Sphere tracing if need be.
		if ( geSkip->GetSolid() == Solid::Sphere ) {
			return SG_Trace( traceOrigin, traceMins, traceMaxs, traceEnd, geSkip, traceContentMask, 1 );
		}
	}
	return SG_Trace( traceOrigin, traceMins, traceMaxs, traceEnd, geSkip, traceContentMask );
}

/**
*	@brief	
**/
static void SG_AddTouchEntity( SlideBoxMove *move, const int32_t entNum, const CollisionPlane &plane, CollisionSurface *surface  ) {
	if( move->numTouchEntities >= MAX_SLIDEBOX_TOUCH_ENTITIES || entNum < 0 ) {
		return;
	}

	// See if it is already added
	for( int32_t i = 0; i < move->numTouchEntities; i++ ) {
		if( move->touchEntities[i] == entNum ) {
			return;
		}
	}

	// add it
	move->touchEntities[move->numTouchEntities] = entNum;
	move->touchEntityPlanes[move->numTouchEntities] = plane;
	if ( surface ) {
		move->touchEntitySurfaces[move->numTouchEntities] = surface;
	} else {
		move->touchEntitySurfaces[move->numTouchEntities] = nullptr;
	}
	move->numTouchEntities++;
}

/**
*	@brief	
**/
static void SG_ClearClippingPlanes( SlideBoxMove *move ) {
	move->numClipPlanes = 0;
}

/**
*	@brief	
**/
static void SG_ClipVelocityToClippingPlanes( SlideBoxMove *move ) {
	int32_t i;

	for( i = 0; i < move->numClipPlanes; i++ ) {
		if( vec3_dot( move->velocity, move->clipPlaneNormals[i] ) >= SLIDEBOX_PLANEINTERACT_EPSILON ) {
			continue; // looking in the same direction than the velocity
		}
		move->velocity = SG_BounceVelocity( move->velocity, move->clipPlaneNormals[i], move->slideBounce );
	}
}

/**
*	@brief	
**/
static void SG_AddClippingPlane( SlideBoxMove *move, const vec3_t &planeNormal ) {
	int32_t i;

	// see if we are already clipping to this plane
	for( i = 0; i < move->numClipPlanes; i++ ) {
		if( vec3_dot( planeNormal, move->clipPlaneNormals[i] ) >= ( 1.0f - MAX_SLIDEBOX_CLIPPING_PLANES ) ) {
			return;
		}
	}

	if( move->numClipPlanes + 1 == MAX_SLIDEBOX_CLIPPING_PLANES ) {
		SG_Error( ErrorType::Drop, "SG_AddClippingPlane: MAX_SLIDEBOX_CLIPPING_PLANES reached!" );
	}

	// add the plane
	move->clipPlaneNormals[move->numClipPlanes] = planeNormal;
	move->numClipPlanes++;
}

/**
*	@brief	
**/
static const int32_t SG_SlideMoveClipMove( SlideBoxMove *move /*, const bool stepping*/ ) {
	int32_t blockedMask = 0;

	// Determine end position fraction in relation to the remaining time.
	vec3_t endPosition = vec3_fmaf( move->origin, move->remainingTime, move->velocity );

	// Trace from move origin to end position.
	SGTraceResult trace = SBM_Trace( move, &move->origin, &move->mins, &move->maxs, &endPosition, move->skipEntityNumber, move->contentMask );

	// Get the actual entity number. (-1 if not hitting any entity.)
	const int32_t entityNumber = SG_GetEntityNumber( trace.podEntity );

	// If all solid, only add touch entity if not world(0), and return with a trapped flag additioned to our mask.
	if( trace.allSolid ) {
		if( entityNumber > 0) {
			SG_AddTouchEntity( move, entityNumber, trace.plane, trace.surface );
		}
		return blockedMask | SLIDEBOXFLAG_TRAPPED;
	}

	// We managed to move all the way without bumping into anything.
	if( trace.fraction == 1.0f ) {
		// Store new origin.
		move->origin = trace.endPosition;
		// Subtract remaining time based on our moved fraction.
		move->remainingTime -= ( trace.fraction * move->remainingTime );
		// Return blockedMask with additional 'moved' flag set.
		return blockedMask | SLIDEBOXFLAG_MOVED;
	}

	// We weren't able to move all the way, move that which we can.
	if( trace.fraction < 1.0f ) {
		// Add touch entity.
		SG_AddTouchEntity( move, entityNumber,  trace.plane, trace.surface );
		// Impose we also touched a plane because of that.
		blockedMask |= SLIDEBOXFLAG_PLANE_TOUCHED;

		// Move whichever fraction we got to move for.
		if( trace.fraction > 0.0 ) {
			// Store new origin.
			move->origin = trace.endPosition;
			// Subtract remaining time based on our moved fraction.
			move->remainingTime -= ( trace.fraction * move->remainingTime );
			// Return blockedMask with additional 'moved' flag set.
			blockedMask |= SLIDEBOXFLAG_MOVED;
		}

		// If the plane is a wall and stepping, try to step it up
		if( !IsWalkablePlane( trace.plane ) ) {
			//if( stepping && SG_StepUp( move ) ) {
			//	return blockedmask;  // solved : don't add the clipping plane
			//}
			//else {
			blockedMask |= SLIDEBOXFLAG_WALL_BLOCKED;
			//}
		}

		SG_AddClippingPlane( move, trace.plane.normal );
	}

	return blockedMask;
}

/**
*	@brief	
**/
const int32_t SG_SlideMove( SlideBoxMove *move ) {
	constexpr int32_t MAX_SLIDEBOX_ATTEMPTS = 8;

	// Mask containing the blocked(or none) results of our slide clip move.
	int32_t blockedMask= 0;

	// if the velocity is too small, just stop
	if( vec3_length( move->velocity ) < STOP_EPSILON ) {
		move->velocity = vec3_zero();
		move->remainingTime = 0;
		return 0;
	}

	// Store original velocity for clipping plane tests.
	vec3_t originalVelocity = move->velocity;
	// Store our last valid origin (presumably, the current.)
	vec3_t lastValidOrigin = move->origin;

	// Reset our clipping plane list count.
	SG_ClearClippingPlanes( move );
	// Reset touched entity count.
	move->numTouchEntities = 0;

	// Iterate the amount of attempts we want to try sliding for this frame.
	for( int32_t count = 0; count < MAX_SLIDEBOX_ATTEMPTS; count++ ) {
		// Assign the original velocity to our move, and clip it to all the planes we got in the list.
		move->velocity = originalVelocity;
		SG_ClipVelocityToClippingPlanes( move );
		blockedMask = SG_SlideMoveClipMove( move /*, stepping*/ );

#ifdef SG_SLIDEBOX_DEBUG_TRAPPED
		{
			SGTraceResult trace = SBM_Trace( move, &move->origin, &move->mins, &move->maxs, &move->origin, move->skipEntityNumber, move->contentMask );
		
			if( trace.startSolid ) {
				blockedMask |= SLIDEBOXFLAG_TRAPPED;
			}
		}
#endif

		// can't continue
		if( blockedMask & SLIDEBOXFLAG_TRAPPED ) {
#ifdef SG_SLIDEBOX_DEBUG_TRAPPED
			SG_Print( PrintType::DeveloperWarning, "SG_SlideMove SLIDEBOXFLAG_TRAPPED\n" );
#endif
			move->remainingTime = 0.0f;
			move->origin = lastValidOrigin;
			return blockedMask;
		}

		lastValidOrigin = move->origin;

		// touched a plane, re-clip velocity and retry
		if( blockedMask & SLIDEBOXFLAG_PLANE_TOUCHED ) {
			continue;
		}

		// if it didn't touch anything the move should be completed
		if( move->remainingTime > 0.0f ) {
			SG_Print( PrintType::DeveloperWarning, fmt::format( "SlideBox Move finished with remaining time({})!\n", move->remainingTime ) );
			move->remainingTime = 0.0f;
		}

		break;
	}

	return blockedMask;
}
