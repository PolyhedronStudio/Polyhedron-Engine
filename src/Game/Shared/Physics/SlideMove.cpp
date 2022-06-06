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



/***
*
*	
*
*	Touch Entities.
*
*	
* 
***/
/**
*	@brief	If within limits: Adds the geToucher GameEntity to the SlideMoveState's touching entities list.
**/
static void SM_AddTouchEntity( SlideMoveState *moveState, GameEntity *geToucher ) {
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
static void SM_ClearTouchEntities( SlideMoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numTouchEntities = 0;
}


/***
*
*	
*
*	Clipping Planes.
*
*	
* 
***/
/**
*	@brief	If the list hasn't exceeded MAX_SLIDEMOVE_CLIP_PLANES: Adds the plane normal to the SlideMoveState's clipping plane normals list.
**/
static void SM_AddClippingPlane( SlideMoveState *moveState, const vec3_t &planeNormal ) {
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
*	@return	Clipped by normal velocity.
**/
inline vec3_t SM_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, const float overbounce ) {
	float backoff = vec3_dot( inVelocity, normal );

	if( backoff <= 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	// Calculate out velocity vector.
	vec3_t outVelocity = ( inVelocity - vec3_scale( normal, backoff ) );

	// SlideMove clamp it.
#if defined(SM_SLIDEMOVE_CLAMPING) && SM_SLIDEMOVE_CLAMPING == 1
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

/**
*	@brief	Clips the moveState's velocity to all the normals stored in its current clipping plane normal list.
**/
static void SM_ClipVelocityToClippingPlanes( SlideMoveState *moveState ) {
	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
		// Get const ref to our clip plane normal.
		const vec3_t &clipPlaneNormal = moveState->clipPlaneNormals[i];

		// Skip if its looking in the same direction than the velocity,
		if( vec3_dot( moveState->velocity, clipPlaneNormal ) > (FLT_EPSILON - 1.0f) ) {
			continue;
		}

		// Clip velocity to the clipping plane normal.
		moveState->velocity = SM_ClipVelocity( moveState->velocity, clipPlaneNormal, moveState->slideBounce );
	}
}

/**
*	@brief	Clears the SlideMoveState's clipping plane list. 
*			(Does not truly vec3_zero them, just sets numClipPlanes to 0. Adding a 
*			new clipping plane will overwrite the old values.)
**/
static void SM_ClearClippingPlanes( SlideMoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numClipPlanes = 0;
}



/***
*
*	
*
*	Sliding Internal API.
*
*	
* 
***/
/**
*	@brief	Traces whether there has been an edge, instead of a stair, or
*			no difference at all, between two set positions.
*
*	@return	Mask containing the SlideMoveFlags.
**/
static const int32_t SM_CheckForEdge(const vec3_t& start, const vec3_t& end, const vec3_t &mins, const vec3_t &maxs, GameEntity *geSkip, float stepHeight, int32_t contentMask, int32_t groundEntityNumber) {
	// Mask that gets set the resulting move flags.
	int32_t blockedMask = 0;

	//// Calculate the end point for the start ground trace.
	//const vec3_t  startTraceEnd  = vec3_fmaf( start, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
	//SGTraceResult startTrace = SG_Trace( start, mins, maxs, startTraceEnd, geSkip, contentMask );

	//const vec3_t  endTraceEnd  = vec3_fmaf( end, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
	//SGTraceResult endTrace = SG_Trace( end, mins, maxs, endTraceEnd, geSkip, contentMask );

	//// If we had ground entity.
	//if ( groundEntityNumber != -1 ) {
	//	// Check if it is a different one from the current ground.
	//	GameEntity *geStartGroundEntity = SGGameWorld::ValidateEntity( startTrace.gameEntity );
	//	GameEntity *geEndGroundEntity = SGGameWorld::ValidateEntity( endTrace.gameEntity );

	//	// See if the ground that we had, still matches with our start point.
	//	if ( !( geStartGroundEntity && groundEntityNumber != geStartGroundEntity->GetNumber() ) ) {
	//		// blockedMask |= SlideMoveFlags::StartedOnNewGround
	//	}

	//	if ( !( geEndGroundEntity && groundEntityNumber != geEndGroundEntity->GetNumber() ) ) {
	//		if (startTrace.plane.dist != endTrace.plane.dist) {
	//			// If the fraction is less than 1, it means we could step on it.
	//			if ( IsWalkablePlane( endTrace.plane ) && endTrace.fraction < 1.0f ) {
	//				blockedMask |= SlideMoveFlags::CanStepDown;
	//			}
	//			if ( !IsWalkablePlane( endTrace.plane) ) {
	//				blockedMask |= SlideMoveFlags::EdgeMoved;
	//			}
	//		}
	//	}
	//}

	return blockedMask;
}

/**
*	@brief	Performs a ground/step-move trace to determine whether we can step, or fall off an edge.
**/
static SGTraceResult SM_TraceGroundStep( SlideMoveState* moveState ) {
	/**
	*	#0: Get Gameworld, calculate ground end trace point, perform trace.
	**/
	// Get Gameworld.
	SGGameWorld *gameWorld = GetGameWorld();

	// Acquire our skip trace entity.
	GameEntity *geSkip = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( moveState->skipEntityNumber ) );

	// Perform our ground/step-move trace.
	const vec3_t  groundTraceEnd	= vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
	return SG_Trace( moveState->origin, moveState->mins, moveState->maxs, groundTraceEnd, geSkip, moveState->contentMask );
}

/**
*	@brief	Compares the old ground entity with a new ground trace to determine
*			whehter it can step down, or moved off an edge.
**/
static const int32_t SM_CheckForGroundStep( SlideMoveState* moveState ) {
	// Perform the ground trace.
	SGTraceResult groundTrace = SM_TraceGroundStep( moveState );

	// First validate the ground trace its entity.
	GameEntity *geGround = SGGameWorld::ValidateEntity( groundTrace.gameEntity );

	// Do we have ground?
	if ( geGround ) {
		// Was it the same ground?

	}
}

/**
*	@brief	Handles checking whether an entity can step down a brush or not. (Or entity, of course.)
**/
static const int32_t SM_SlideClipMove( SlideMoveState *moveState, const bool stepping );
static bool SG_StepDown( SlideMoveState *moveState ) {
	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get ground entity.
	GameEntity	*geGroundEntity	= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	SM_SlideClipMove( moveState, false );

	// See if we should step down.
	if ( geGroundEntity && moveState->velocity.z <= 0 ) {
        const vec3_t downA = vec3_fmaf( org0, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTraceA = SG_Trace( org0, moveState->mins, moveState->maxs, downA, geSkip, moveState->contentMask );

		const vec3_t downB = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		const SGTraceResult downTraceB = SG_Trace(moveState->origin, moveState->mins, moveState->maxs, downB, geSkip, moveState->contentMask );

		// Check if we should step down or not.
		if ( !downTraceB.allSolid ) {

			// Check if it is a legitimate stair case.
			if (downTraceA.plane.dist != downTraceB.plane.dist && downTraceB.podEntity && IsWalkablePlane(downTraceB.plane) ) {
			//if ( SG_SlideMove_CheckBottom( moveState ) ) {
				moveState->origin = downTraceB.endPosition;

				return true;
			} else {
				return false;
			}
			
		} else {
			return false;
		}
	}

	return false;
}

/**
*	@brief	Handles checking whether an entity can step up a brush or not. (Or entity, of course.)
*	@return	True if the move stepped up.
**/
static bool SG_StepUp( SlideMoveState *moveState ) {
	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get ground entity.
	GameEntity	*geGroundEntity	= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	SM_SlideClipMove( moveState, false );

	// See if we should step down.
	if ( geGroundEntity && moveState->velocity.z <= 0 ) {
        const vec3_t downA = vec3_fmaf( org0, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTraceA = SG_Trace( org0, moveState->mins, moveState->maxs, downA, geSkip, moveState->contentMask );

		const vec3_t downB = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
		const SGTraceResult downTraceB = SG_Trace(moveState->origin, moveState->mins, moveState->maxs, downB, geSkip, moveState->contentMask );

		// Check if we should step down or not.
		if ( !downTraceB.allSolid ) {

			// Check if it is a legitimate stair case.
			if ((downTraceB.podEntity != downTraceA.podEntity || downTraceA.plane.dist != downTraceB.plane.dist) && IsWalkablePlane(downTraceB.plane) ) {
				moveState->origin = downTraceB.endPosition;
				//SG_Physics_PrintDeveloper("SlideMove Entity(#" + std::to_string(moveState->moveEntityNumber) + ") SG_StepUp: firstDownTrace");
			}
		}
	}

    // If we are blocked, we will try to step over the obstacle.
    const vec3_t org1 = moveState->origin;
    const vec3_t vel1 = moveState->velocity;

    const vec3_t up = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT, vec3_up() );
    const SGTraceResult upTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, up, geSkip, moveState->contentMask);

    if ( !upTrace.allSolid ) {
        // Step from the higher position, with the original velocity
        moveState->origin = upTrace.endPosition;
        moveState->velocity = vel0;

		SM_SlideClipMove( moveState, false );
        
		//PM_StepSlideMove_();

        // Settle to the new ground, keeping the step if and only if it was successful
        const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );

        if ( !downTrace.allSolid ) {
     
			if ( (geGroundEntity && IsWalkablePlane( downTrace.plane ) ) ) {// || vel0.z < PM_SPEED_UP) {
				SG_Physics_PrintDeveloper("SlideMove Entity(#" + std::to_string(moveState->moveEntityNumber) + ") SG_StepUp: secondDownTrace - Position Set");


				return true;
					//moveState->origin = org1;
					
			} else {
				// Set it back?
				//return false;
//                moveState->origin = org1;
//				return false;
				//pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
            }
//#endif
        }
    }
	
	// Save end results.
    moveState->origin = org1;
    moveState->velocity = vel1;
			
	//SG_Physics_PrintDeveloper("SlideMove Entity(#" + std::to_string(moveState->moveEntityNumber) + ") SG_StepUp: Made it till the end");

	return false;
}

/**
*	@brief	Performs the move, adding its colliding plane(s) to our clipping last.
**/
static const int32_t SM_SlideClipMove( SlideMoveState *moveState, const bool stepping ) {
	// Returned containing all move flags that were set during this move.
	int32_t blockedMask = 0;

	// Get GameWorld.
	SGGameWorld	*gameWorld		= GetGameWorld();
	// Get ground entity.
	GameEntity	*geGroundEntity	= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
	// Get Skip Entity for trace testing.
	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

	// Calculate the wished for move end position.
	const vec3_t endPosition = vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	// Trace the move.
	SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, endPosition, geSkip, moveState->contentMask );
	
	// When this happens, chances are you got a shitty day coming up.
	if( traceResult.allSolid ) {
		// Add the entity trapping us to our touch list.
		if( traceResult.gameEntity ) {
			// Touched an other entity.
			SM_AddTouchEntity( moveState, traceResult.gameEntity );
		}

		// Notify we're trapped.
		return blockedMask | SlideMoveFlags::Trapped;
	}

	// Managed to move without any complications.
	if( traceResult.fraction == 1.0f ) {
		if( ( stepping && SG_StepDown( moveState ) ) ) {
			moveState->origin = traceResult.endPosition;
			//moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			blockedMask |= SlideMoveFlags::SteppedDown | SlideMoveFlags::Moved;
			return blockedMask;
		} else {
			moveState->origin = traceResult.endPosition;
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );	
			return blockedMask | SlideMoveFlags::Moved;
		}
		
	}

	// Wasn't able to make the full move.
	if( traceResult.fraction < 1.0f ) {
		// Add the touched entity to our list of touched entities.
		SM_AddTouchEntity( moveState, traceResult.gameEntity );
				
		// Add a specific flag to our blockedMask stating we bumped into something.
		blockedMask |= SlideMoveFlags::PlaneTouched;

		// Move up covered fraction which is up till the trace result's endPosition.
		if( traceResult.fraction > 0.0 ) {
			moveState->origin = traceResult.endPosition;
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			blockedMask |= SlideMoveFlags::Moved;
		}

		// If we bumped into a non walkable plane(We'll consider it to be a wall.), try and up on top of it.
		if( !IsWalkablePlane( traceResult.plane ) ) {
			// If SG_StepUp returns true, it means the move stepped.
			if( stepping && SG_StepUp( moveState ) ) {
				//moveState->remainingTime = 0.0f;
				blockedMask |= SlideMoveFlags::SteppedUp;
				return blockedMask;
			} else {
				blockedMask |= SlideMoveFlags::WallBlocked;
			}
		}

		// Add plane normal to clipping plane list: We didn't step, the plane(assuming wall) blocked us.
		SM_AddClippingPlane( moveState, traceResult.plane.normal );
	}

	return blockedMask;
}



/***
*
*	
*
*	Sliding External API.
*
*	
* 
***/
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
	int32_t blockedMask = 0;

	// If the velocity is too small, just stop.
	if( vec3_length( moveState->velocity ) < SLIDEMOVE_STOP_EPSILON ) {
		// Zero out its velocity.
		moveState->velocity = vec3_zero();
		moveState->remainingTime = 0;
		return 0;
	}

	// Reset clipping plane list for the current 'SlideMove' that we're about to execute.
	SM_ClearClippingPlanes( moveState );
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
		SM_ClipVelocityToClippingPlanes( moveState );

		// Process the actual slide move for the moveState.
		blockedMask |= SM_SlideClipMove(moveState, true /*, stepping*/);

#ifdef SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
		// Get GameWorld.
		SGGameWorld	*gameWorld		= GetGameWorld();
		// Get Skip Entity for trace testing.
		GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));

		SGTraceResult traceResult = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, moveState->origin, geSkip, moveState->contentMask );
		if( traceResult.startSolid ) {
			blockedMask |= SlideMoveFlags::Trapped;
		}
#endif

		// Can't continue.
		if( blockedMask & SlideMoveFlags::Trapped ) {
#ifdef SM_SLIDEMOVE_DEBUG_TRAPPED_MOVES
			SG_Physics_PrintWarning( std::string(__func__) + "SlideMoveFlags::Trapped" );
#endif
			moveState->remainingTime = 0.0f;
			// Copy back in the last valid origin we had, because the move had failed.
			moveState->origin = lastValidOrigin;
			return blockedMask;
		}

		// Update the last validOrigin to the current moveState origin.
		lastValidOrigin = moveState->origin;

		// Stepped up, so skip adding the clipping plane.
		if (blockedMask & SlideMoveFlags::SteppedUp) {
			continue;
		}

		// Touched a plane, re-clip velocity and retry.
		if( blockedMask & SlideMoveFlags::PlaneTouched ) {
			//blockedMask &= ~SlideMoveFlags::PlaneTouched;
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
//			if (SM_PointContents (start) != BrushContents::Solid) {
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


///**
//*	@brief	Handles checking whether an entity can step up a brush or not. (Or entity, of course.)
//*	@return	True if the move stepped up.
//**/
//static const int32_t SM_SlideClipMove( SlideMoveState *moveState, const bool stepping );
//static bool SM_StepUp( SlideMoveState *moveState ) {
//    // Store pre-move parameters
//    const vec3_t org0 = moveState->origin;
//    const vec3_t vel0 = moveState->velocity;
//
//	// Slide move, but skip stair testing the plane that we skipped by calling into this function.
//	SM_SlideClipMove( moveState, false );
//
//	/**
//	*	Step Down.
//	**/
//	// Get GameWorld.
//	SGGameWorld	*gameWorld		= GetGameWorld();
//	// Get ground entity.
//	GameEntity	*geGroundEntity	= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
//	// Get Skip Entity for trace testing.
//	GameEntity	*geSkip			= SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->skipEntityNumber));
//
//	// See if we should step down.
//	if ( geGroundEntity && moveState->velocity.z <= 0.1f ) {
//		// Trace downwards.
//		const vec3_t down = vec3_fmaf( org0 , PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down( ) );
//		const SGTraceResult downTrace = SG_Trace(org0, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );
//
//		// Check if we should step down or not.
//		if ( !downTrace.allSolid ) {
//			// Check if it is a legitimate stair case.
//			if (downTrace.podEntity && !(downTrace.plane.normal.z >= PM_STEP_NORMAL) ) {
//				moveState->origin = downTrace.endPosition;
//			}
//		}
//	}
//
//
//	/**
//	*	Step Over.
//	**/
//    // If we are blocked, we will try to step over the obstacle.
//    const vec3_t org1 = moveState->origin;
//    const vec3_t vel1 = moveState->velocity;
//
//    const vec3_t up = vec3_fmaf( org1, PM_STEP_HEIGHT, vec3_up() );
//    const SGTraceResult upTrace = SG_Trace( org1, moveState->mins, moveState->maxs, up, geSkip, moveState->contentMask );
//
//    if ( !upTrace.allSolid ) {
//        // Step from the higher position, with the original velocity
//        moveState->origin = upTrace.endPosition;
//        moveState->velocity = vel0;
//
//		// Slide move, but skip stair testing the plane that we skipped by calling into this function.
//		SM_SlideClipMove( moveState, true );
//
//		/**
//		*	Settle to Ground.
//		**/
//        // Settle to the new ground, keeping the step if and only if it was successful
//        const vec3_t down = vec3_fmaf( moveState->origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down() );
//        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );
//
//        if ( !downTrace.allSolid && downTrace.plane.normal.z >= PM_STEP_NORMAL ) { //PM_CheckStep(&downTrace)) {
//            // Quake2 trick jump secret sauce
////#if 0     
//			//SGGameWorld *gameWorld = GetGameWorld();
//			//GameEntity *geGroundEntity = SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
//			//if ( (geGroundEntity) || vel0.z < PM_SPEED_UP ) {
////#endif
//				//if ( !SlideMove_CheckBottom( moveState ) ) {
//					// Yeah... I knwo.
//					moveState->origin = downTrace.endPosition;
//					moveState->velocity = vel1;
//					if (geGroundEntity) {
//						moveState->groundEntityNumber = geGroundEntity->GetNumber();
//						moveState->groundEntityLinkCount = geGroundEntity->GetLinkCount();
//					}
//				//}
//				// Calculate step height.
//				//moveState->stepHeight = moveState->origin.z - moveState-> 
////#if 0
//			//} else {
//			//	//// Set it back?
//   // //            moveState->origin = org0;
//			//	//moveState->velocity = vel0;
//			//	//return false;
//			//	//pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
//   //         }
////#endif
//			return true;
//		}
//    }
//	
//	// Save end results.
//    moveState->origin = org1;
//    moveState->velocity = vel1;
//
//	return false;
//}




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
//					//SM_AddClippingPlane( moveState, negatedNormal );
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
//	//				//moveState->velocity = SM_ClipVelocity( moveState->velocity, negatedNormal, moveState->slideBounce );
//
//	//				// Adjust move time to the fraction of what we've moved.
//	//				moveState->remainingTime -= ( (movedDistance / moveState->remainingTime) * moveState->remainingTime );
//
//	//				// Get direction of the trace.
//	////				const vec3_t directionNormal = vec3_negate( vec3_normalize( oldOrigin - slideTraceEnd ) );
//
//	//				// If we've reached this point, it's clear this is one of our planes to clip against.
//	//				SM_AddClippingPlane( moveState, negatedNormal );
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
