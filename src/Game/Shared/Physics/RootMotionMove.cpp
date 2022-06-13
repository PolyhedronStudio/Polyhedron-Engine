/***
*
*	License here.
*
*	@file
*
*	Root Bone Motion Movement implementation for SharedGame Physics.
*
*	NOTE: The GroundEntity info has to be up to date before pulling off a RootMotionMove.
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
#include "RootMotionMove.h"



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
*	@brief	If within limits: Adds the geToucher GameEntity to the RootMotionMoveState's touching entities list.
*	@return	The touch entity number. -1 if it's invalid, or has already been added.
**/
static int32_t RM_AddTouchEntity( RootMotionMoveState *moveState, GameEntity *geToucher ) {
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
		
		return -1;
	}

	// See if it is already added.
	for( int32_t i = 0; i < moveState->numTouchEntities; i++ ) {
		if( moveState->touchEntites[i] == touchEntityNumber ) {
			return -1;
		}
	}

	// Otherwise, add the entity to our touched list.
	moveState->touchEntites[moveState->numTouchEntities] = touchEntityNumber;
	moveState->numTouchEntities++;

	// Last but not least, return its number.
	return touchEntityNumber;
}

/**
*	@brief	Clears the RootMotionMoveState's clipping plane list. 
*			(Does not truly vec3_zero them, just sets numClipPlanes to 0. Adding a 
*			new clipping plane will overwrite the old values.)
**/
static void RM_ClearTouchEntities( RootMotionMoveState *moveState ) {
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
static const int32_t RM_SlideClipMove( RootMotionMoveState *moveState, const bool stepping );
/**
*	@brief	If the list hasn't exceeded MAX_ROOTMOTION_MOVE_CLIP_PLANES: Adds the plane normal to the RootMotionMoveState's clipping plane normals list.
**/
static void RM_AddClippingPlane( RootMotionMoveState *moveState, const vec3_t &planeNormal ) {
	// Ensure we stay within limits of MAX_ROOTMOTION_MOVE_CLIP_PLANES . Warn if we don't.
	if( moveState->numClipPlanes + 1 == MAX_ROOTMOTION_MOVE_CLIP_PLANES ) {
		SG_Physics_PrintWarning( std::string(__func__) + "MAX_ROOTMOTION_MOVE_CLIP_PLANES reached" );
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
inline vec3_t RM_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, const float overbounce ) {
	float backoff = vec3_dot( inVelocity, normal );

	if( backoff <= 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	// Calculate out velocity vector.
	vec3_t outVelocity = ( inVelocity - vec3_scale( normal, backoff ) );

	// RootMotionMove clamp it.
#if defined(RM_ROOTMOTION_MOVE_CLAMPING) && RM_ROOTMOTION_MOVE_CLAMPING == 1
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
static void RM_ClipVelocityToClippingPlanes( RootMotionMoveState *moveState ) {
	for( int32_t i = 0; i < moveState->numClipPlanes; i++ ) {
		// Get const ref to our clip plane normal.
		const vec3_t &clipPlaneNormal = moveState->clipPlaneNormals[i];

		// Skip if its looking in the same direction than the velocity,
		if( vec3_dot( moveState->velocity, clipPlaneNormal ) > (FLT_EPSILON - 1.0f) ) {
			continue;
		}

		// Clip velocity to the clipping plane normal.
		moveState->velocity = RM_ClipVelocity( moveState->velocity, clipPlaneNormal, moveState->slideBounce );
	}
}

/**
*	@brief	Clears the RootMotionMoveState's clipping plane list. 
*			(Does not truly vec3_zero them, just sets numClipPlanes to 0. Adding a 
*			new clipping plane will overwrite the old values.)
**/
static void RM_ClearClippingPlanes( RootMotionMoveState *moveState ) {
	if (!moveState) {
		return;
	}
	
	moveState->numClipPlanes = 0;
}



/***
*
*
*	Sliding Utilities.
*
*
***/
/**
*	@brief	Called at the start of RootMotionMove. Inspects and unsets previous frame flags if need be.
**/
static void RM_UpdateMoveFlags( RootMotionMoveState* moveState ) {
	// Check and unset if needed: FoundGround and LostGround flags. (You can't find it twice in two frames, or lose it twice, can ya?)
	if ( (moveState->moveFlags & RootMotionMoveFlags::FoundGround) ) {
		moveState->moveFlags &= ~RootMotionMoveFlags::FoundGround;
	}
	if ( (moveState->moveFlags & RootMotionMoveFlags::LostGround) ) {
		moveState->moveFlags &= ~RootMotionMoveFlags::LostGround;
	}
	if ( (moveState->moveFlags & RootMotionMoveFlags::OnGround) ) {
		moveState->moveFlags &= ~RootMotionMoveFlags::OnGround;
	}
}
/**
*	@brief	Updates the moveFlags timer.
**/
static void RM_UpdateMoveFlagsTime(RootMotionMoveState* moveState) {
    // Decrement the movement timer, used for "dropping" the player, landing after jumps, 
    // or falling off a ledge/slope, by the duration of the command.
	//
	// We do so by simulating it how the player does, we take FRAMETIME_MS.
    if ( moveState->moveFlagTime && moveState->moveFlagTime > 0 ) {
		// Clear the timer and timed flags.
        if ( moveState->moveFlagTime <= FRAMERATE_MS.count() ) {
            moveState->moveFlags	&= ~RootMotionMoveFlags::TimeMask;
            moveState->moveFlagTime	= 0;
        } else {
			// Or just decrement the timer.
            moveState->moveFlagTime	-= FRAMERATE_MS.count();
        }
    }
}

/**
*	@brief	Performs a ground/step-move trace to determine whether we can step, or fall off an edge.
**/
static SGTraceResult RM_Trace( RootMotionMoveState* moveState, const vec3_t *origin = nullptr, const vec3_t *mins = nullptr, const vec3_t *maxs = nullptr, const vec3_t *end = nullptr, const int32_t skipEntityNumber = -1, const int32_t contentMask = -1  ) {
	/**
	*	#0: Determine whether to use move state or custom values.
	**/
	const vec3_t traceOrigin	= ( origin != nullptr ? *origin : moveState->origin );
	const vec3_t traceMins		= ( mins != nullptr ? *mins: moveState->mins );
	const vec3_t traceMaxs		= ( maxs != nullptr ? *maxs : moveState->maxs );
	const vec3_t traceEnd		= ( end != nullptr ? *end : moveState->origin );

	const int32_t traceSkipEntityNumber = ( skipEntityNumber != -1 ? skipEntityNumber : moveState->skipEntityNumber );
	const int32_t traceContentMask = ( contentMask != -1 ? contentMask : moveState->contentMask );

	/**
	*	#1: Fetch needed skip entity.
	**/
	// Get Gameworld.
	SGGameWorld *gameWorld = GetGameWorld();
	// Acquire our skip trace entity.
	GameEntity *geSkip = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( traceSkipEntityNumber ) );

	// Perform and return trace results.
	return SG_Trace( traceOrigin, traceMins, traceMaxs, traceEnd, geSkip, traceContentMask );
}

/**
*	@brief	Checks whether the trace endPosition complies to the conditions of
*			being marked as a 'step'. 
*	@return	True in case the trace result complies. False otherwise.
**/
static const int32_t RM_CheckStep( RootMotionMoveState* moveState, const SGTraceResult& stepTrace ) {
	// Get ground entity pointer and its number.
	const int32_t groundEntityNumber = SG_GetEntityNumber( stepTrace.podEntity );

	if ( !stepTrace.allSolid ) {
		if (
				( groundEntityNumber != -1 && IsWalkablePlane( stepTrace.plane ) )  
					&&
				(	
					( groundEntityNumber != moveState->groundEntityNumber )
					|| 
					( stepTrace.plane.dist != moveState->groundTrace.plane.dist )	
				)
			){
			// Return true.
			return true;
		} else {
			//moveState->stepHeight = moveState->origin.z - moveState->originalOrigin.z;
		}
	}

	// Could not step.
	return false;
}



/***
*
*
*	Sliding Ground Work.
*
*
***/
/**
*	@brief	Checks whether this entity is 100% on-ground or not.
*	@return	Mask containing the RootMotionMoveResult.
**/
static const int32_t RM_CheckBottom(const vec3_t& start, const vec3_t& end, const vec3_t &mins, const vec3_t &maxs, GameEntity *geSkip, float stepHeight, int32_t contentMask, int32_t groundEntityNumber) {
	// Mask that gets set the resulting move flags.
	int32_t moveResultMask = 0;


	return moveResultMask;
}

/**
*	@brief	Checks whether the RootMotionMoveState is having any ground below its feet or not.
**/
static void RM_CheckGround( RootMotionMoveState *moveState ) {
    // If we jumped, or been pushed, do not attempt to seek ground
    if ( moveState->moveFlags & ( RootMotionMoveFlags::Jumped ) ){// | RootMotionMoveFlags::OnGround ) ) {
        return;
    }

	// Backup our previous ground trace entity number.
	int32_t oldgroundEntityNumber = moveState->groundEntityNumber;

    // Perform ground seek trace.
    const vec3_t downTraceEnd = moveState->origin + vec3_t{ 0.f, 0.f, -ROOTMOTION_MOVE_GROUND_DISTANCE };
	SGTraceResult downTraceResult = RM_Trace( moveState, &moveState->origin, &moveState->mins, &moveState->maxs, &downTraceEnd );
	    
	// Store our ground trace result.
	moveState->groundTrace = downTraceResult;

    // If we hit an upward facing plane, make it our ground
    if ( downTraceResult.gameEntity && IsWalkablePlane( downTraceResult.plane ) ) {
		// If we had no ground, set FoundGround flag and handle landing events.
        if ( moveState->groundEntityNumber == -1 ) {
			// Any landing terminates the water jump
            if ( moveState->moveFlags & RootMotionMoveFlags::TimeWaterJump ) {
				// Unset TimeWaterJump flag.
                moveState->moveFlags &= ~RootMotionMoveFlags::TimeWaterJump;

				// Reset move flag time.
                moveState->moveFlagTime = 0;
            }

            // Engage TimeLand flag state if we're falling down hard enough to demand a landing response.
            if ( moveState->velocity.z <= ROOTMOTION_MOVE_SPEED_LAND ) {
				// Set TimeLand flag.
                moveState->moveFlags |= RootMotionMoveFlags::TimeLand;
				// Set TimeLand time to: '1'.
                moveState->moveFlagTime = 1;

				// Falling down rough.
                if ( moveState->velocity.z <= ROOTMOTION_MOVE_SPEED_FALL ) {
                    // Set TimeLand time to: '16'.
					moveState->moveFlagTime = 16;

					// Faling down hard.
                    if ( moveState->velocity.z <= ROOTMOTION_MOVE_SPEED_FALL_FAR ) {
						// Set TimeLand time to: '256'.
                        moveState->moveFlagTime = 256;
                    }
                }
            } else {
				// Nothing.
            }

			// Set our FoundGround, and of course, OnGround flags.
			moveState->moveFlags |= RootMotionMoveFlags::FoundGround;
        } else {
			// Unset the found ground flag since it can't find it if already on it.
			if ( moveState->moveFlags & RootMotionMoveFlags::FoundGround ) {
				moveState->moveFlags &= ~RootMotionMoveFlags::FoundGround;
			}
		}

		// Set our OnGround flags.
        moveState->moveFlags |= RootMotionMoveFlags::OnGround;
		// Store ground entity number.
		moveState->groundEntityNumber = SG_GetEntityNumber( downTraceResult.podEntity );

		// Maintain ourselves "Sunk down" to the ground.
		moveState->origin = downTraceResult.endPosition;
		// Clip our velocity to the ground floor plane.
		moveState->velocity = RM_ClipVelocity(moveState->velocity, downTraceResult.plane.normal, ROOTMOTION_MOVE_CLIP_BOUNCE);
	} else {
		// Set LostGround flag, to notify we lost track of it.
		if (moveState->moveFlags & RootMotionMoveFlags::OnGround) {
			moveState->moveFlags |= RootMotionMoveFlags::LostGround;
		}

		// Unset OnGround flag, since we're N.O.T on ground.
		moveState->moveFlags &= ~RootMotionMoveFlags::OnGround;

		// Unset entity number.
		moveState->groundEntityNumber = -1;
	}

    // Always touch the entity, even if we couldn't stand on it
    RM_AddTouchEntity( moveState, downTraceResult.gameEntity );
}

/**
*	@brief	Categorizes the position of the current slide move.
**/
static void RM_CategorizePosition( RootMotionMoveState *moveState ) {
	// Get Origin, we'll need it.
	vec3_t pointOrigin = moveState->origin;

	//
	// Test For 'Feet':.
	//
	// Add 1, to test for feet.
	pointOrigin.z += 1.f;

	// Get contents at point.
	int32_t pointContents = SG_PointContents( pointOrigin );

	// We're not in water, no need to further test.
	if ( !( pointContents & BrushContents::Water ) ) {
		moveState->waterLevel	= WaterLevel::None;
		moveState->waterType	= 0;
		return;
	}

	// Store water type.
	moveState->waterType	= pointContents;
	moveState->waterLevel	= WaterLevel::Feet;

	//
	// Test For 'Waist' by climbing up the Z axis.
	//
	pointOrigin.z += 40.f;

	// Get contents at point.
	pointContents = SG_PointContents( pointOrigin );

	// We're not in water, no need to further test.
	if ( !( pointContents & BrushContents::Water ) ) {
		moveState->waterLevel = WaterLevel::Waist;
	}

	// WaterLevel: Head Under.
	pointOrigin.z += 45.f;

	// Get contents at point.
	pointContents = SG_PointContents( pointOrigin );

	// We're not in water, no need to further test.
	if ( !( pointContents & BrushContents::Water ) ) {
		moveState->waterLevel = WaterLevel::Under;
		return;
	}
	
}



/***
*
*
*	RootMotionMove 'Down Stepping'.
*
*
***/
/**
*	@brief	When slide moving moving off an edge, the choices are:
*				- Either step on a "staircase".
*				- Or move right off the edge and go into a free fall.
*	@return	True in case it stepped down. False if the step was higher than ROOTMOTION_MOVE_STEP_HEIGHT
*			"step" right off the edge and allow us to fall down.
**/
const bool RM_StepDownOrStepEdge_StepDown( RootMotionMoveState* moveState, const SGTraceResult &traceResult ) {
    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	

	// Only return true if it was high enough to "step".
	const float stepHeight = moveState->origin.z - traceResult.endPosition.z;
	const float absoluteStepHeight = fabs(stepHeight);

	if (absoluteStepHeight <= ROOTMOTION_MOVE_STEP_HEIGHT && IsWalkablePlane(traceResult.plane) ) {
		moveState->origin = traceResult.endPosition;
		RM_SlideClipMove( moveState, false );
		//moveState->stepHeight = stepHeight;
		return true;
	} else {
		//moveState->stepHeight = stepHeight;
		//return true; // Return false to prevent animating.?
	}

	return false;
}


/**
*	@brief	Checks whether we're stepping off a legit step, or moving off an edge and about to
*			fall down.
*	@return	Either 0 if no step or edge move could be made. Otherwise, RootMotionMoveResult::SteppedDown 
*			or RootMotionMoveResult::SteppedEdge.
**/
static int32_t RM_StepDown_StepEdge( RootMotionMoveState *moveState ) {
    // Store pre-move parameters
    const vec3_t org0 = moveState->origin;
    const vec3_t vel0 = moveState->velocity;

	// Move ahead.
	RM_SlideClipMove( moveState, false );

	// We lost ground, and/or our velocity is already downwards(Aka we're falling down.).
	if ( !(moveState->moveFlags & RootMotionMoveFlags::OnGround) && moveState->velocity.z <= ROOTMOTION_MOVE_STOP_EPSILON /* vel0.z <= ROOTMOTION_MOVE_STOP_EPSILON */) {
		// Trace down step height and ground distance.
		const vec3_t downTraceEnd = vec3_fmaf( moveState->origin, ROOTMOTION_MOVE_STEP_HEIGHT + ROOTMOTION_MOVE_GROUND_DISTANCE, vec3_down() );

		// Perform trace.
		const SGTraceResult downTraceResult = RM_Trace( moveState, &moveState->origin, &moveState->mins, &moveState->maxs, &downTraceEnd );
		
		// See if there's any ground changes, demanding us to check if we can step to new ground.
		if ( RM_CheckStep( moveState, downTraceResult) ) {
			// Check whether we're stepping down to stairsteps, or stepping off an edge instead.
			if ( RM_StepDownOrStepEdge_StepDown( moveState, downTraceResult ) ) {
				// To remain consistent, we unset the LostGround flag here since essentially
				// when walking down the stairs you are unlikely to walk down with both foot off-ground
				// at the same time, right?
				moveState->moveFlags &= ~RootMotionMoveFlags::LostGround;

				// If we were falling, we did step but not from a stair case situation.
				if ( (moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
					// Unset the time land flag.
					moveState->moveFlags &= ~RootMotionMoveFlags::TimeLand;

					// Return we stepped to ground from a fall.
					return RootMotionMoveResult::SteppedFall;
				} else {
					// Return regular SteppedDown.
					return RootMotionMoveResult::SteppedDown;
				}
			} else {

				if ( !(moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
					// Unset the time land flag.
					moveState->moveFlags |= RootMotionMoveFlags::TimeLand;

					// Return we stepped to ground from a fall.
					return RootMotionMoveResult::SteppedEdge;
				} else {
					// We shouldn't reach this point.
				}
			}
			//	if ( (moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
			//		return RootMotionMoveResult::FallingDown;
			//	}
				//// Set a time land flag so we can determine what landing animation to use.
			//if ( !(moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
			//	// Set TimeLand flag.
			//	moveState->moveFlags |= RootMotionMoveFlags::TimeLand;

			//	// We moved over the edge, about to drop down.
			//	return RootMotionMoveResult::SteppedEdge;
			//} else {
			//	// If the flag is already set, we don't want to return yet another SteppedEdge flag.
			//	// TODO: Return a FallDown MoveFlag? Or, just Moved? Might be useful for game state, it's data after all.
			//}
			//}
		} else {
			// If we got here, we are falling, or on-ground as is, there was no step.
			//if ( !(moveState->moveFlags & RootMotionMoveFlags::OnGround) && !(moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
			//	// Set the time land flag.
			//	moveState->moveFlags |= RootMotionMoveFlags::TimeLand;
			//}

			// 
			if ( !(moveState->moveFlags & RootMotionMoveFlags::OnGround) ) {
				if ( !(moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
					// Unset the time land flag.
					moveState->moveFlags |= RootMotionMoveFlags::TimeLand;
				}
				return RootMotionMoveResult::FallingDown;
					// Return we stepped to ground from a fall.
					//return RootMotionMoveResult::SteppedEdge;
				//} else {
				//	// Return we are falling.
				//	return RootMotionMoveResult::FallingDown;
				//}
			}

			// Set a time land flag so we can determine what landing animation to use.
			//if ( !(moveState->moveFlags & RootMotionMoveFlags::TimeLand) ) {
			//	// Set TimeLand flag.
			//	moveState->moveFlags |= RootMotionMoveFlags::TimeLand;

			//	// We moved over the edge, about to drop down.
			//	return RootMotionMoveResult::SteppedEdge;
			//} else {
			//	// If the flag is already set, we don't want to return yet another SteppedEdge flag.
			//	// TODO: Return a FallDown MoveFlag? Or, just Moved? Might be useful for game state, it's data after all.
			//}
		}
	} else {

			if (moveState->moveFlags & RootMotionMoveFlags::TimeLand) {
				return RootMotionMoveResult::FallingDown;
			}
		//moveState->origin = org0;	//moveState->velocity = vel0;
	}

	// Can't step or move off an edge.
	return 0;
}



/***
*
*
*	RootMotionMove 'Up Stepping'.
*
*
***/
/**
*	@brief	Everytime we stepped up by moving the moveState into a new origin of .z += ROOTMOTION_MOVE_STEP_HEIGHT
*			to perform another move forward, we find ourselves in need of seeking ground and stepping
*			to it.
*				- Either step on a "staircase".
*				- Or move right off the edge and go into a free fall.
*	@return	True in case it stepped down. False if the step was higher than ROOTMOTION_MOVE_STEP_HEIGHT
*			move right off the edge and allow us to fall down.
**/
const bool RM_StepUp_ToFloor( RootMotionMoveState *moveState, const SGTraceResult &traceResult ) {
	//// Set current origin to the trace end position.
	moveState->origin = traceResult.endPosition;
	
	RM_SlideClipMove( moveState, false );
	// Only return true if it was high enough to "step".
	const float stepHeight = moveState->origin.z - traceResult.endPosition.z;
	const float absoluteStepHeight = fabs(stepHeight);

	// Return true in case the step height was above minimal.
	if (absoluteStepHeight >= ROOTMOTION_MOVE_STEP_HEIGHT_MIN) {
		//moveState->stepHeight = stepHeight;
		return true; // TODO: Perhaps return the fact we stepped highly?
	} else {
		// We did not "truly step" here, it was just a minor bump.
		return true; // TODO: Perhaps return the fact we hit a bump?
	}

	// TODO: This is likely never needed, hell, it does not reach this right now either.
	return false;
}

/**
*	@brief	Handles checking whether an entity can step up or not.
*	@return	True if the move stepped up.
**/
static bool RM_StepUp_StepUp( RootMotionMoveState *moveState ) {
	/**
	*	#0: Perform an upwards trace and return false if it is allSolid.
	**/
	// Store origin and velocity in case of failure.
	const vec3_t org0 = moveState->origin;
	const vec3_t vel0 = moveState->velocity;

	// Up Wards trace end point.
	const vec3_t upTraceEnd = vec3_fmaf( org0, ROOTMOTION_MOVE_STEP_HEIGHT, vec3_up() );
	// Perform trace.
	const SGTraceResult upTraceResult = RM_Trace( moveState, &org0, &moveState->mins, &moveState->maxs, &upTraceEnd );
	
	// If it is all solid, we can skip stepping up from the higher position.
	if (upTraceResult.allSolid) {
		return false;
	}


	/**
	*	#1: Setup the move state at the upTrace resulting origin and attempt 
	*		to move using original velocity.
	**/
	// Prepare state for moving using original velocity.
	moveState->origin	= upTraceResult.endPosition;
	moveState->velocity = vel0;

	// Perform move.
	RM_SlideClipMove( moveState, false );


	/**
	*	#2: Try and settle to the ground by testing with RM_CheckStep:
	*			- When not complying: Return true and settle to the ground.
	*			- When not complying: Return false and revert to original origin & velocity.
	**/
	// Up Wards trace end point.
	const vec3_t downTraceEnd = vec3_fmaf( moveState->origin, ROOTMOTION_MOVE_STEP_HEIGHT + ROOTMOTION_MOVE_GROUND_DISTANCE, vec3_down() );
	// Perform Trace.
	const SGTraceResult downTraceResult = RM_Trace( moveState, &moveState->origin, &moveState->mins, &moveState->maxs, &downTraceEnd );
		
	// If the ground trace result complies to all conditions, step to it.
	if ( RM_CheckStep( moveState, downTraceResult ) ) {
		if ( (moveState->moveFlags & RootMotionMoveFlags::OnGround) || moveState->velocity.z < ROOTMOTION_MOVE_SPEED_UP ) {
			RM_StepUp_ToFloor(moveState, downTraceResult);
			return true;
		}
	}
	
	// Move failed, reset original origin and velocity.
	moveState->origin = org0;
	moveState->velocity = vel0;
			
	return false;
}



/***
*
*
*	RootMotionMove 'Clip Moving'.
*
*
***/
/**
*	@brief	Performs the move, adding its colliding plane(s) to our clipping last.
**/
static const int32_t RM_SlideClipMove( RootMotionMoveState *moveState, const bool stepping ) {
	// Returned containing all move flags that were set during this move.
	int32_t moveResultMask = 0;
	// Calculate the wished for move end position.
	const vec3_t endPosition = vec3_fmaf( moveState->origin, moveState->remainingTime, moveState->velocity );
	// Trace the move.
	SGTraceResult traceResult = RM_Trace( moveState, &moveState->origin, &moveState->mins, &moveState->maxs, &endPosition );
	
	// When this happens, chances are you got a shitty day coming up.
	if( traceResult.allSolid ) {
		// Add the entity trapping us to our touch list.
		if( traceResult.gameEntity ) {
			// Touched an other entity.
			RM_AddTouchEntity( moveState, traceResult.gameEntity );
		}

		// Notify we're trapped.
		moveResultMask |= RootMotionMoveResult::Trapped;
		return moveResultMask;
	}

	// This move had no obstalces getting in its way, however...
	if( traceResult.fraction == 1.0f ) {
		// Move ourselves, and see whether we are moving onto a step, or off an edge.
		moveState->origin = traceResult.endPosition;

		// Test for Step/Edge if requested.
		const int32_t edgeStepMask = ( stepping == true ? RM_StepDown_StepEdge( moveState) : 0 );

		// Stepping solution:
		if ( stepping && edgeStepMask != 0) {
			// Revert move origin.
			//moveState->origin = traceResult.endPositio
			// If we are edge moving, revert move and clip to normal?
			if ( edgeStepMask & RootMotionMoveResult::SteppedEdge ) {
				//moveState->origin = moveState->originalOrigin;
				// Keep our velocity, and clip it to our directional normal.
				//const vec3_t direction = 
			} else {
				//	moveState->origin = traceResult.endPosition;
			}

			// Move all the way.
			//moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );	

			return edgeStepMask;
		// Non stepping solution:
		} else {
			// Flawless move: Set new origin.
			moveState->origin = traceResult.endPosition;
			// Subtract remaining time.
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );	
			// Return moveResultMask with an addition Moved flag.
			return moveResultMask | RootMotionMoveResult::Moved;
		}
	}

	// Wasn't able to make the full move.
	if( traceResult.fraction < 1.0f ) {
		// Add the touched entity to our list of touched entities.
		const int32_t entityNumber = RM_AddTouchEntity( moveState, traceResult.gameEntity );
		
		// -1 = invalid, 0 = world entity, anything above is a legit touchable entity.
		if (entityNumber > 0) {
			moveResultMask |= RootMotionMoveResult::EntityTouched;
		}

		// Add a specific flag to our moveResultMask stating we bumped into something.
		moveResultMask |= RootMotionMoveResult::PlaneTouched;

		// Move up covered fraction which is up till the trace result's endPosition.
		if( traceResult.fraction > 0.0 ) {
			moveState->origin = traceResult.endPosition;
			moveState->remainingTime -= ( traceResult.fraction * moveState->remainingTime );
			moveResultMask |= RootMotionMoveResult::Moved;
		}

		// If we bumped into a non walkable plane(We'll consider it to be a wall.), try end up on top of it.
		if( !IsWalkablePlane( traceResult.plane ) ) {
			// Try and step up the obstacle if we can: If we did, return with an additional SteppedUp flag set.
			if (stepping && RM_StepUp_StepUp(moveState)) {
				return moveResultMask | RootMotionMoveResult::SteppedUp;
			} else {
				// We hit something that we can't step up on: Add an addition WalLBlocked flag.
				moveResultMask |= RootMotionMoveResult::WallBlocked;
			}
		}

		// We never had a move without conflict, and failed to step up the conflicting plane:
		// Add the resulting hit-traced plane normal to clipping plane list:
		RM_AddClippingPlane( moveState, traceResult.plane.normal );
	}

	return moveResultMask;
}



/***
*
*
*	RootMotionMove Public API.
*
*
***/
/**
*	@brief	Executes a Root Motion Move for the current moveState by clipping its velocity to the touching plane' normals.
*	@return	An int32_t containing the move result mask ( RootMotionMoveResult Flags )
**/
int32_t SG_RootMotion_MoveFrame( RootMotionMoveState *moveState ) {
	/**
	*	#0: Prepare the move.
	**/
	//! Maximum amount of planes to clip against.
	static constexpr int32_t MAX_CLIP_PLANES = 20;
	//! Maximum Root Motion Move attempt's we're about to try.
	static constexpr int32_t MAX_ROOTMOTION_MOVE_ATTEMPTS = MAX_CLIP_PLANES - 2;
	//! The final resulting move result mask of this slide move.
	int32_t moveResultMask = 0;

	// Store the original Origin and Velocity for Sliding and possible reversion.
	const vec3_t originalOrigin		= moveState->origin;
	const vec3_t originalVelocity	= moveState->velocity;

	// The 'last valid' origin and velocity, they are set after each valid slide attempt.
	vec3_t lastValidOrigin		= moveState->origin;
	vec3_t lastValidVelocity	= moveState->velocity;

	// Clear clipping planes.
	RM_ClearClippingPlanes( moveState );
	// Clear touched entities.
	RM_ClearTouchEntities( moveState );


	/**
	*	#1: Check, update and if need be correct the current move state before moving.
	**/
	// Update Move Flags.
	RM_UpdateMoveFlags( moveState );
	
	// Update Move Flags Time.
	RM_UpdateMoveFlagsTime( moveState);

	// Check for ground.
	RM_CheckGround( moveState );


	/**
	*	#2: Add clipping planes from ground (if on-ground), and our own velocity.
	**/
	//if ( moveState->moveFlags & RootMotionMoveFlags::OnGround ) {
	//	RM_AddClippingPlane( moveState, moveState->groundTrace.plane.normal );
	//}

	// Also don't turn against our original velocity.
	//RM_AddClippingPlane( moveState, moveState->velocity );

	//// If the velocity is too small, just stop.
	//if( vec3_length( moveState->velocity ) < 0.01f) {// ROOTMOTION_MOVE_STOP_EPSILON ) {
	//	// Zero out its velocity.
	//	moveState->velocity = vec3_zero();
	//	moveState->remainingTime = 0;
	//	return 0;
	//}

	/**
	*	#3: Start moving by clipping velocities to all planes we touch.
	**/
	for( int32_t count = 0; count < MAX_ROOTMOTION_MOVE_ATTEMPTS; count++ ) {
		// Get the original velocity and clip it to all the planes we got in the list.
		moveState->velocity = originalVelocity;

		// Clip the velocity to the current list of clipping planes.
		RM_ClipVelocityToClippingPlanes( moveState );

		// Retreive resulting move result mask from performing a slide clip move with 'stepping' logic enabled.
		const bool enableStepping = true;
		const int32_t slideMoveResultMask = RM_SlideClipMove(moveState, enableStepping);
		
		// Add the slide move result mask to our move result mask.
		moveResultMask |= slideMoveResultMask;

		// Debugging for being trapped.
		#ifdef SG_ROOTMOTION_MOVE_DEBUG_TRAPPED_MOVES
		SGTraceResult traceResult = RM_Trace( moveState, &moveState->origin, &moveState->mins, &moveState->maxs, &moveState->origin );
		if( traceResult.startSolid ) {
			SG_Physics_PrintWarning( std::string(__func__) + "RootMotionMoveResult::Trapped" );
		}
		#endif

		// Can't continue.
		if( moveResultMask & RootMotionMoveResult::Trapped ) {
			// Move is invalid, unset remaining time.
			moveState->remainingTime = 0.0f;

			// Copy back in the last valid origin we had, because the move had failed.
			moveState->origin = lastValidOrigin;
			
			// Break out of the loop and finish the move.
			//moveResultMask |= slideMoveResultMask;
			return moveResultMask;
		}

		// Update the last validOrigin to the current moveState origin.
		lastValidOrigin = moveState->origin;

		if ( (slideMoveResultMask & RootMotionMoveResult::SteppedEdge) ||
				( slideMoveResultMask & RootMotionMoveResult::SteppedDown )
				|| ( slideMoveResultMask & RootMotionMoveResult::SteppedUp ) 
			) 
		{
			continue;
		}

		// Touched a plane, re-clip velocity and retry.
		if( slideMoveResultMask & RootMotionMoveResult::PlaneTouched ) {
			//moveResultMask &= ~RootMotionMoveResult::PlaneTouched;
			continue;
		}

		// If it didn't touch anything the move should be completed
		if( moveState->remainingTime > 0.0f ) {
			SG_Physics_PrintWarning( std::string(__func__) + "Slidemove finished with remaining time" );
			moveState->remainingTime = 0.0f;
		}
		break;
	}

	/**
	*	#4: Done moving, check for ground, and 'categorize' our position.
	**/
	// Once again, check for ground.
	RM_CheckGround( moveState );

	// Categorize our position.
	RM_CategorizePosition( moveState );


	/**
	*	#: DONE! Return the results, at last.
	**/
	return moveResultMask;
}




//void RootMotionMove_FixCheckBottom( GameEntity *geCheck ) {
//	geCheck->SetFlags( geCheck->GetFlags() | EntityFlags::PartiallyOnGround );
//}
//#define STEPSIZE 18
//const bool RootMotionMove_CheckBottom( RootMotionMoveState *moveState ) {
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
//			if (RM_PointContents (start) != BrushContents::Solid) {
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
//static const int32_t RM_SlideClipMove( RootMotionMoveState *moveState, const bool stepping );
//static bool RM_StepUp( RootMotionMoveState *moveState ) {
//    // Store pre-move parameters
//    const vec3_t org0 = moveState->origin;
//    const vec3_t vel0 = moveState->velocity;
//
//	// Slide move, but skip stair testing the plane that we skipped by calling into this function.
//	RM_SlideClipMove( moveState, false );
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
//		const vec3_t down = vec3_fmaf( org0 , ROOTMOTION_MOVE_STEP_HEIGHT + ROOTMOTION_MOVE_GROUND_DISTANCE, vec3_down( ) );
//		const SGTraceResult downTrace = SG_Trace(org0, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );
//
//		// Check if we should step down or not.
//		if ( !downTrace.allSolid ) {
//			// Check if it is a legitimate stair case.
//			if (downTrace.podEntity && !(downTrace.plane.normal.z >= ROOTMOTION_MOVE_STEP_NORMAL) ) {
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
//    const vec3_t up = vec3_fmaf( org1, ROOTMOTION_MOVE_STEP_HEIGHT, vec3_up() );
//    const SGTraceResult upTrace = SG_Trace( org1, moveState->mins, moveState->maxs, up, geSkip, moveState->contentMask );
//
//    if ( !upTrace.allSolid ) {
//        // Step from the higher position, with the original velocity
//        moveState->origin = upTrace.endPosition;
//        moveState->velocity = vel0;
//
//		// Slide move, but skip stair testing the plane that we skipped by calling into this function.
//		RM_SlideClipMove( moveState, true );
//
//		/**
//		*	Settle to Ground.
//		**/
//        // Settle to the new ground, keeping the step if and only if it was successful
//        const vec3_t down = vec3_fmaf( moveState->origin, ROOTMOTION_MOVE_STEP_HEIGHT + ROOTMOTION_MOVE_GROUND_DISTANCE, vec3_down() );
//        const SGTraceResult downTrace = SG_Trace( moveState->origin, moveState->mins, moveState->maxs, down, geSkip, moveState->contentMask );
//
//        if ( !downTrace.allSolid && downTrace.plane.normal.z >= ROOTMOTION_MOVE_STEP_NORMAL ) { //ROOTMOTION_MOVE_CheckStep(&downTrace)) {
//            // Quake2 trick jump secret sauce
////#if 0     
//			//SGGameWorld *gameWorld = GetGameWorld();
//			//GameEntity *geGroundEntity = SGGameWorld::ValidateEntity(gameWorld->GetPODEntityByIndex(moveState->groundEntityNumber));
//			//if ( (geGroundEntity) || vel0.z < ROOTMOTION_MOVE_SPEED_UP ) {
////#endif
//				//if ( !RootMotionMove_CheckBottom( moveState ) ) {
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
//			const vec3_t	endPosition				= vec3_fmaf( moveState->origin, ROOTMOTION_MOVE_STEP_HEIGHT + ROOTMOTION_MOVE_GROUND_DISTANCE, vec3_down() );
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
//			if ( moveState->groundEntityNumber != -1 && !RootMotionMove_CheckBottom( moveState ) ) {
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
//					//RM_AddClippingPlane( moveState, negatedNormal );
//				moveState->entityFlags |= EntityFlags::PartiallyOnGround;
//				return moveResultMask |= RootMotionMoveResult::SteppedEdge;
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
//					// Add RootMotionMoveResult::SteppedDown flag to our moveResultMask and return.
//					return moveResultMask | RootMotionMoveResult::SteppedDown | RootMotionMoveResult::Moved;
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
//	//				//moveState->velocity = RM_ClipVelocity( moveState->velocity, negatedNormal, moveState->slideBounce );
//
//	//				// Adjust move time to the fraction of what we've moved.
//	//				moveState->remainingTime -= ( (movedDistance / moveState->remainingTime) * moveState->remainingTime );
//
//	//				// Get direction of the trace.
//	////				const vec3_t directionNormal = vec3_negate( vec3_normalize( oldOrigin - slideTraceEnd ) );
//
//	//				// If we've reached this point, it's clear this is one of our planes to clip against.
//	//				RM_AddClippingPlane( moveState, negatedNormal );
//
//	//				std::string debugstr = "Edge Clipped to plane normal: ";
//	//				debugstr += std::to_string(normalizedDirection.x) + " ";
//	//				debugstr += std::to_string(normalizedDirection.y) + " ";
//	//				debugstr += std::to_string(normalizedDirection.z) + " \n";
//	//				SG_Physics_PrintWarning(debugstr);
//					// Add RootMotionMoveResult::SteppedEdge mask flag.
//					//return moveResultMask |= RootMotionMoveResult::SteppedEdge;
//					//// Calculate remaining time.
//					//moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );
//				}
//			} else {
//				// Subtract total fraction of remaining time.
//				moveState->remainingTime -= ( slideTrace.fraction * moveState->remainingTime );
//			}
//		}
