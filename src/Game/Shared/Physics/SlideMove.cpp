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


//void SlideMove_FixCheckBottom( GameEntity *geCheck ) {
//	geCheck->SetFlags( geCheck->GetFlags() | EntityFlags::PartiallyOnGround );
//}
//#define STEPSIZE 18
//const bool SlideMove_CheckBottom( SlideMoveState *moveState ) {
//	// Get the moveEntity.
//	SGGameWorld *gameWorld = GetGameWorld();
//	GameEntity *geCheck = SGGameWorld::ValidateEntity( gameWorld->GetPODEntityByIndex( moveState->moveEntityNumber ) );
//	
//	//vec3_t	mins, maxs, start, stop;
//	SGTraceResult trace;
//	int32_t 		x, y;
//	float	mid, bottom;
//
//	const vec3_t origin = moveState->origin;
//	vec3_t mins = origin + moveState->mins;
//	vec3_t maxs = origin + moveState->maxs;
//	
//	// if all of the points under the corners are solid world, don't bother
//	// with the tougher checks
//	// the corners must be within 16 of the midpoint
//	vec3_t start, stop;
//
//	start[2] = mins[2] - 1;
//	for	(x=0 ; x<=1 ; x++)
//		for	(y=0 ; y<=1 ; y++)
//		{
//			start[0] = x ? maxs[0] : mins[0];
//			start[1] = y ? maxs[1] : mins[1];
//			if (SG_PointContents (start) != BrushContents::Solid) {
//				goto realcheck;
//			}
//		}
//
//	return true;		// we got out easy
//
//realcheck:
//	//
//	// check it for real...
//	//
//	start[2] = mins[2];
//	
//	// the midpoint must be within 16 of the bottom
//	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
//	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
//	stop[2] = start[2] - 2*STEPSIZE;
//	trace = SG_Trace (start, vec3_zero(), vec3_zero(), stop, geCheck, BrushContentsMask::MonsterSolid);
//
//	if (trace.fraction == 1.0) {
//		return false;
//	}
//	mid = bottom = trace.endPosition[2];
//	
//	// the corners must be within 16 of the midpoint	
//	for	(x=0 ; x<=1 ; x++)
//		for	(y=0 ; y<=1 ; y++)
//		{
//			start[0] = stop[0] = x ? maxs[0] : mins[0];
//			start[1] = stop[1] = y ? maxs[1] : mins[1];
//			
//			trace = SG_Trace (start, vec3_zero(), vec3_zero(), stop, geCheck, BrushContentsMask::MonsterSolid);
//			
//			if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
//				bottom = trace.endPosition[2];
//			if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
//				return false;
//		}
//
//	return true;
//}

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
*	@brief	If within limits: Adds the geToucher GameEntity to the SlideMoveState's touching entities list.
**/
static void SG_AddTouchEnt( SlideMoveState *moveState, GameEntity *geToucher ) {
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
*	@brief	Clears the SlideMoveState's clipping plane list. 
*			(Does not truly vec3_zero them, just sets numClipPlanes to 0. Adding a 
*			new clipping plane will overwrite the old values.)
**/
static void SG_ClearClippingPlanes( SlideMoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numClipPlanes = 0;
}

/**
*	@brief	Clips the moveState's velocity to all the normals stored in its current clipping plane normal list.
**/
static void SG_ClipVelocityToClippingPlanes( SlideMoveState *moveState ) {
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
*	@brief	If the list hasn't exceeded MAX_SLIDEMOVE_CLIP_PLANES: Adds the plane normal to the SlideMoveState's clipping plane normals list.
**/
static void SG_AddClippingPlane( SlideMoveState *moveState, const vec3_t &planeNormal ) {
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
static const int32_t SG_SlideMoveClipMove( SlideMoveState *moveState, const bool stepping );
static bool SG_StepUp( SlideMoveState *moveState ) {
    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	// Slide move, but skip stair testing the plane that we skipped by calling into this function.
	SG_SlideMoveClipMove( moveState, false );

	/**
	*	Step Down.
	**/
	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get ground entity.
	GameEntity	*geGroundEntity	= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

	// See if we should step down.
	if ( geGroundEntity && moveState->velocity.z <= 0.1f ) {
		// Trace downwards.
		const vec3_t down = vec3_fmaf( org0 , PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		const SGTraceResult downTrace = SG_Trace(org0, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );

		// Check if we should step down or not.
		if ( !downTrace.allSolid ) {
			// Check if it is a legitimate stair case.
			if (downTrace.podEntity && !(downTrace.plane.normal.z >= PM_STEP_NORMAL) ) {
				moveState->origin = downTrace.endPosition;
			}
		}
	}


	/**
	*	Step Over.
	**/
    // If we are blocked, we will try to step over the obstacle.
    const vec3_t org1 = moveState->origin;
    const vec3_t vel1 = moveState->velocity;

    const vec3_t up = vec3_fmaf( org1, PM_STEP_HEIGHT, vec3_up() );
    const SGTraceResult upTrace = SG_Trace( org1, moveState->mins, moveState->maxs, up, geSkip, moveState->contentMask );

    if ( !upTrace.allSolid ) {
        // Step from the higher position, with the original velocity
        moveState->origin = upTrace.endPosition;
        moveState->velocity = vel0;

		// Slide move, but skip stair testing the plane that we skipped by calling into this function.
		SG_SlideMoveClipMove( moveState, true );

		/**
		*	Settle to Ground.
		**/
        // Settle to the new ground, keeping the step if and only if it was successful
        const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );

        if ( !downTrace.allSolid && downTrace.plane.normal.z >= PM_STEP_NORMAL ) { //PM_CheckStep(&downTrace)) {
            // Quake2 trick jump secret sauce
//#if 0     
			//SGGameWorld *gameWorld = GetGameWorld();
			//GameEntity *geGroundEntity = SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
			//if ( (geGroundEntity) || vel0.z < PM_SPEED_UP ) {
//#endif
				//if ( !SlideMove_CheckBottom( moveState ) ) {
					// Yeah... I knwo.
					moveState->origin = downTrace.endPosition;
					moveState->velocity = vel1;
					if (geGroundEntity) {
						moveState->groundEntityNumber = geGroundEntity->GetNumber();
						moveState->groundEntityLinkCount = geGroundEntity->GetLinkCount();
					}
				//}
				// Calculate step height.
				//moveState->stepHeight = moveState->origin.z - moveState-> 
//#if 0
			//} else {
			//	//// Set it back?
   // //            moveState->origin = org0;
			//	//moveState->velocity = vel0;
			//	//return false;
			//	//pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
   //         }
//#endif
			return true;
		}
    }
	
	// Save end results.
    moveState->origin = org1;
    moveState->velocity = vel1;

	return false;
}

/**
*	@brief	Traces whether there has been an edge, instead of a stair, or
*			no difference at all, between two set positions.
*
*	@return	Mask containing the SlideMoveFlags.
**/
static const int32_t SG_SlideMove_CheckEdgeMove(const vec3_t& start, const vec3_t& end, const vec3_t &mins, const vec3_t &maxs, GameEntity *geSkip, float stepHeight, int32_t contentMask, int32_t groundEntityNumber) {
	// Mask that gets set the resulting move flags.
	int32_t blockedMask = 0;

	// Calculate the end point for the start ground trace.
	const vec3_t  startTraceEnd  = vec3_fmaf( start, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
	SGTraceResult startTrace = SG_Trace( start, mins, maxs, startTraceEnd, geSkip, contentMask );

	const vec3_t  endTraceEnd  = vec3_fmaf( end, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
	SGTraceResult endTrace = SG_Trace( end, mins, maxs, endTraceEnd, geSkip, contentMask );

	// If we had ground entity.
	if ( groundEntityNumber != -1 ) {
		// Check if it is a different one from the current ground.
		GameEntity *geStartGroundEntity = SGGameWorld::ValidateEntity( startTrace.gameEntity );
		GameEntity *geEndGroundEntity = SGGameWorld::ValidateEntity( endTrace.gameEntity );

		// See if the ground that we had, still matches with our start point.
		if ( !( geStartGroundEntity && groundEntityNumber != geStartGroundEntity->GetNumber() ) ) {
			// blockedMask |= SlideMoveFlags::StartedOnNewGround
		}

		if ( !( geEndGroundEntity && groundEntityNumber != geEndGroundEntity->GetNumber() ) ) {
			if (startTrace.plane.dist != endTrace.plane.dist) {
				// If the fraction is less than 1, it means we could step on it.
				if ( IsWalkablePlane( endTrace.plane ) && endTrace.fraction < 1.0f ) {
					blockedMask |= SlideMoveFlags::CanStepDown;
				}
				if ( !IsWalkablePlane( endTrace.plane) ) {
					blockedMask |= SlideMoveFlags::EdgeMoved;
				}
			}
		}
	}

	return blockedMask;
}

/**
*	@brief	GS_Performs a substep of the actual SG_SlideMove
**/
static const int32_t SG_SlideMoveClipMove( SlideMoveState *moveState, const bool stepping ) {
	// Contains all flags indicating the results of this moveState's move.
	int32_t blockedMask = 0;

	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity( gameWorld->GetPODEntityByIndex( moveState->skipEntityNumber ) );
	// Get Skip Entity for trace testing.
	GameEntity	*geGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetPODEntityByIndex( moveState->skipEntityNumber ) );

	// Store old origin and velocity.
	const vec3_t oldOrigin		= moveState->origin;
	const vec3_t oldVelocity	= moveState->velocity;

	/**
	*	Step #1: Trace forwards, attempt to do a full move.
	**/
	// Trace for the current remainingTime, by MA-ing the move velocity.
	const vec3_t	slideTraceEnd	= vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	SGTraceResult	slideTrace		= SG_Trace( moveState->origin, moveState->mins, moveState->maxs, slideTraceEnd, geSkip, moveState->contentMask );
	
	// Problem: If the result was all solid, something unintended happened. 
	// Return with an added SlideMoveFlags::Trapped to our blockedMask so that the Entiy can handle it himself.
	if( slideTrace.allSolid ) {
		if( slideTrace.gameEntity ) {
			// Add Touch Entity to our list.
			SG_AddTouchEnt( moveState, slideTrace.gameEntity );

			// If the touched entity was no world entity, add EntityTouched flag instead of PlaneTouched.
			if ( slideTrace.gameEntity->GetNumber() > 0) {
				// Add SlideMoveFlags::EntityTouched to our blockedMask.
				blockedMask |= SlideMoveFlags::EntityTouched;
			}
		}

		// Subtract total fraction that we've moved from our remaining time.
		moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );

		// Return with an additional Trapped flag.
		return blockedMask | SlideMoveFlags::Trapped;
	}

	// Was able to perform the full move cleanly without any interruptions.
	if( slideTrace.fraction == 1.0f ) {
		// Update new move settlement origin.
		moveState->origin = slideTrace.endPosition;

		// Check whether we moved over an edge, and add it to our mask.
		blockedMask |= SG_SlideMove_CheckEdgeMove(oldOrigin, moveState->origin, moveState->mins, moveState->maxs, geSkip, PM_STEP_HEIGHT + PM_GROUND_DIST, moveState->contentMask, moveState->groundEntityNumber);

		// Subtract total fraction of remaining time.
		moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );

		// Move succeeded without stepping, add Moved to our blockedMask and return.
		return blockedMask | SlideMoveFlags::Moved;
	}

	// Unable to make the full move.
	if( slideTrace.fraction < 1.0f ) { // Wasn't able to make the full move.
		// Add the touched entity to our list.
		SG_AddTouchEnt( moveState, slideTrace.gameEntity );

		// If the touched entity was no world entity, add EntityTouched flag instead of PlaneTouched.
		if ( slideTrace.gameEntity && slideTrace.gameEntity->GetNumber() > 0) {
			// Add SlideMoveFlags::EntityTouched to our blockedMask.
			blockedMask |= SlideMoveFlags::EntityTouched;
		} else  {
			// If the trace had a valid plane, add Plane Touched.
			if ( slideTrace.plane.dist ) {
				// Add SlideMoveFlags::planeTouched to our blockedMask.
				blockedMask |= SlideMoveFlags::PlaneTouched;
			}
		}

		// Move the 'fraction' that we at least can move.
		if( slideTrace.fraction > 0.0 ) {
			// Assign origin.
			moveState->origin = slideTrace.endPosition;

			// Check whether we moved over an edge, and add it to our mask.
			blockedMask |= SG_SlideMove_CheckEdgeMove(oldOrigin, moveState->origin, moveState->mins, moveState->maxs, geSkip, PM_STEP_HEIGHT + PM_GROUND_DIST, moveState->contentMask, moveState->groundEntityNumber);

			// Subtract remainint time.
			moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );

			// Add the SlideMoveFlags::Moved flag to our blockedMask.
			blockedMask |= SlideMoveFlags::Moved;
		}

		// If the plane is a wall however, and we're 'stepping', try to step up the wall.
		if( !IsWalkablePlane( slideTrace.plane ) ) {
			// Step up/over the wall, otherwise add SlideMoveFlags::WalLBlocked flag to our blockedMask.
			if( stepping ) { //}&& SG_StepUp(moveState) ) {
				// Return blockedMask. We won't be adding the clipping plane.
				//return blockedMask;
				return blockedMask |= SlideMoveFlags::CanStepUp;
			} else {
				// Add SlideMoveFlags::WallBlocked flag.
				blockedMask |= SlideMoveFlags::WallBlocked;
			}
		} else {
			if ( stepping ) {
			//	blockedMask |= SlideMoveFlags::SteppedDown;
			} else {
			//	blockedMask |= SlideMoveFlags::EdgeMoved;
			}
		}

		// If we've reached this point, it's clear this is one of our planes to clip against.
		SG_AddClippingPlane( moveState, slideTrace.plane.normal );
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
*												When this is set the last valid Origin is stored in the SlideMoveState.
*			- SlideMoveFlags::EdgeMoved	:	The move got blocked by an edge. (In other words, there was no legitimate step to perform.)
*			- SlideMoveFlags::Moved			:	The move succeeded.
**/
int32_t SG_SlideMove( SlideMoveState *moveState ) {
	static constexpr int32_t MAX_SLIDEMOVE_ATTEMPTS = 18;
	//! Keeps score of the 
	int32_t blockedMask = 0;

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

	// Store the original Origin and Velocity for Sliding and possible reversion.
	const vec3_t originalOrigin		= moveState->origin;
	const vec3_t originalVelocity	= moveState->velocity;

	// The 'last valid' origin and velocity, they are set after each valid slide attempt.
	vec3_t lastValidOrigin		= moveState->origin;
	vec3_t lastValidVelocity	= moveState->velocity;

	for( int32_t count = 0; count < MAX_SLIDEMOVE_ATTEMPTS; count++ ) {
		// Get the original velocity and clip it to all the planes we got in the list.
		moveState->velocity = originalVelocity;
		SG_ClipVelocityToClippingPlanes( moveState );

		// Process the actual slide move for the moveState.
		blockedMask = SG_SlideMoveClipMove( moveState, true );

		// For Edge Handling.
		if ( blockedMask & SlideMoveFlags::EdgeMoved ) {
			//float movedDistance = 0.f;
			//const vec3_t normalizedDirection = vec3_normalize_length( lastValidOrigin - moveState->origin, movedDistance );
			//const vec3_t negatedNormal = vec3_negate( normalizedDirection );
			//
			//moveState->origin = originalOrigin;
			//moveState->velocity = lastValidVelocity;

			//// Add a "fake" clipping plane that is negated of our move direction.
			//SG_AddClippingPlane( moveState, negatedNormal );
			//SG_ClipVelocityToClippingPlanes( moveState );
			// Continue.
			//continue;
		}
#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
		{
			// Get GameWorld.
			SGGameWorld	*gameWorld		= GetGameWorld();
			// Get Skip Entity for trace testing.
			GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

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

		// Touched a plane, re-clip velocity and retry.
		if( blockedMask & SlideMoveFlags::PlaneTouched ) {
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

//		{
//			/**
//			*	#0: Store previousGroundTrace reference and trace to new ground.
//			**/
//			// Get a reference to our previous ground trace.
//			SGTraceResult	&previousGroundTrace		= moveState->groundTrace;
//
//			// Trace for the current remainingTime, by MA-ing the move velocity.
//			const vec3_t	endPosition				= vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
//			SGTraceResult	newGroundTraceResult	= SG_Trace( moveState->origin, moveState->mins, moveState->maxs, endPosition, geSkip, moveState->contentMask );
//
//			/**
//			*	#1: Get and Validate Ground Game Entities. Check and store if they differed.
//			**/
//			// Validate both ground entities, previous and new.
//			GameEntity *gePreviousGround		= previousGroundTrace.gameEntity;
//			GameEntity *geNewGround				= newGroundTraceResult.gameEntity;
//			GameEntity *geValidPreviousGround	= (gePreviousGround ? SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( gePreviousGround->GetNumber() ) ) : nullptr);
//			GameEntity *geValidNewGround		= (geNewGround ? SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( geNewGround->GetNumber() ) ) : nullptr);
//
//			// If their numbers differ, it means we got new ground to step down to.
//			bool newGroundEntity = false;
//
//			if ( ( geValidPreviousGround && geValidNewGround ) && ( geValidPreviousGround->GetNumber() != geValidNewGround->GetNumber() ) ) {
//				newGroundEntity = true;
//			}
//
//			if ( moveState->groundEntityNumber != -1 && !SlideMove_CheckBottom( moveState ) ) {
//				
//
//float movedDistance		 = 0.f;
//float wishedDistance	= 0.f;
//const vec3_t normalizedDirection = vec3_normalize_length( oldOrigin - slideTraceEnd, wishedDistance );
//const vec3_t negatedNormal = vec3_negate( normalizedDirection );
//const vec3_t normalizedDirectionx = vec3_normalize_length( oldOrigin - moveState->origin, movedDistance);
//
//				moveState->velocity = vec3_cross( moveState->velocity, negatedNormal );
//				moveState->remainingTime = (moveState->remainingTime / wishedDistance) * movedDistance;
//					//SG_AddClippingPlane( moveState, negatedNormal );
//				moveState->entityFlags |= EntityFlags::PartiallyOnGround;
//				return blockedMask |= SlideMoveFlags::EdgeMoved;
//			}
//			/**
//			*	#3: Test for the new ground to see if we should step on it.
//			**/
//			if (newGroundEntity || newGroundTraceResult.plane.dist != previousGroundTrace.plane.dist) {
//				// We've got new ground. See if it's a walkable plane.
//				if ( IsWalkablePlane( newGroundTraceResult.plane ) ) {
//					// Subtract total fraction of remaining time.
//					moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );
//
//					// Step down to it.
//					moveState->origin = newGroundTraceResult.endPosition;
//
//					// Make sure to store the new ground entity information.
//					moveState->groundTrace = newGroundTraceResult;
//
//					// Update the entity number and aspect.
//					if (geNewGround) {
//						moveState->groundEntityNumber		= geValidNewGround->GetNumber();
//						moveState->groundEntityLinkCount	= geValidNewGround->GetLinkCount();
//					} else {
//						moveState->groundEntityNumber		= -1;
//						moveState->groundEntityLinkCount	= 0;
//					}
//
//					// Add SlideMoveFlags::SteppedDown flag to our blockedMask and return.
//					return blockedMask | SlideMoveFlags::SteppedDown | SlideMoveFlags::Moved;
//				} else {
//					// Since we're being blocked by an edge, meaning:
//					// "An empty space, something we can't "realistically" step down to."
//					//
//					// We take the direction normal we are moving into, and add that as a 
//					// "fake" clipping plane.
//
//					// Get the length, of the total wished for distance to move.
//	//				float movedDistance = 0.f;
//	//				const vec3_t normalizedDirection = vec3_normalize_length( oldOrigin - slideTraceEnd, movedDistance );
//	//				const vec3_t negatedNormal = vec3_negate( normalizedDirection );
//
//	//				// Adjust origin back to old.
//	//				//moveState->origin = oldOrigin;
//	//				//moveState->velocity = SG_ClipVelocity( moveState->velocity, negatedNormal, moveState->slideBounce );
//
//	//				// Adjust move time to the fraction of what we've moved.
//	//				moveState->remainingTime -= ( (movedDistance / moveState->remainingTime) * moveState->remainingTime );
//
//	//				// Get direction of the trace.
//	////				const vec3_t directionNormal = vec3_negate( vec3_normalize( oldOrigin - slideTraceEnd ) );
//
//	//				// If we've reached this point, it's clear this is one of our planes to clip against.
//	//				SG_AddClippingPlane( moveState, negatedNormal );
//
//	//				std::string debugstr = "Edge Clipped to plane normal: ";
//	//				debugstr += std::to_string(normalizedDirection.x) + " ";
//	//				debugstr += std::to_string(normalizedDirection.y) + " ";
//	//				debugstr += std::to_string(normalizedDirection.z) + " \n";
//	//				SG_Physics_PrintWarning(debugstr);
//					// Add SlideMoveFlags::EdgeMoved mask flag.
//					//return blockedMask |= SlideMoveFlags::EdgeMoved;
//					//// Calculate remaining time.
//					//moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );
//				}
//			} else {
//				// Subtract total fraction of remaining time.
//				moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );
//			}
//		}
