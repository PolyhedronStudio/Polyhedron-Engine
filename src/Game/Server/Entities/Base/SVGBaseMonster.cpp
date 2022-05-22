/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
#include "../../ServerGameLocals.h"   // SVGame.
#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.
#include "../../Physics/StepMove.h"  // Stepmove funcs.

// Server Game Base Entity.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseSkeletalAnimator.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseMonster.h"
#define STEPSIZE 18

//! Constructor/Destructor.
SVGBaseMonster::SVGBaseMonster(PODEntity *svEntity) : Base(svEntity) { }
SVGBaseMonster::~SVGBaseMonster() { }


/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseMonster::Precache() { 
	Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseMonster::Spawn() { 
	Base::Spawn(); 
}



/**
*   @brief 
**/
void SVGBaseMonster::PostSpawn() { 
	Base::Spawn(); 
}

/**
*   @brief 
**/
void SVGBaseMonster::Respawn() { 
	Base::Respawn(); 
}

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseMonster::Think() { 
	// Base think.
	Base::Think();
}

/**
*   @brief 
**/
void SVGBaseMonster::SpawnKey(const std::string& key, const std::string& value) { 
	Base::SpawnKey(key, value); 
}


/***
* 
*   Step Entity functions.
* 
***/
/**
*	@brief	This is the actual SG_Stepmove implementation. Tries to move
*			the entity a given distance over a second while trying to 
*			step over and off obstacles if needed. It is influenced by
*			and dependent on the game's tick rate.
*
*	@return	False if the move has failed and the entity remains at its position.
*			True otherwise.
**/
const bool SVGBaseMonster::StepMove_Step(const vec3_t &stepOffset, bool relink) {
    //float       dz;
    //SVGTraceResult    trace;
    //int         i;
    //float       stepsize;
    //vec3_t      test;
    //int         contents;

    // Get Old Origin.
    const vec3_t oldOrigin = GetOrigin();

	// Calculate wished for origin.
    const vec3_t newOrigin = oldOrigin + stepOffset;
	
	gi.DPrintf("---------------------\n");

    // Flying monsters don't step up.
    if ( GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) {
		gi.DPrintf("In if ( GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) )\n");
        // Try one move with vertical motion, then one without.
        for ( int32_t i = 0; i < 2; i++ ) {
			vec3_t stepOrigin = GetOrigin() + stepOffset;

			// If we've got an enemy set, head towards it.
            if ( i == 0 && GetEnemy() ) {
				// Set our enemy as our goal entity if we had none set.
				if ( !GetGoalEntity() ) {

					// Update the actual Goal Entity.
					SetGoalEntity( GetEnemy() );
				}

				// Calculate delta z.
				float deltaZ = ( !GetGoalEntity() ? 0 : GetOrigin().z - GetGoalEntity()->GetOrigin().z );

				// Use a different move method if it is a client.
				if ( GetGoalEntity()->GetClient()) {
					// Move up if the delta z is > 40.
					if (deltaZ > 40.f) {
						// Adjust origin.
						stepOrigin.z -= 8.f;
					}

					// Special for Non Swim.
					if ( !(( GetFlags() & EntityFlags::Swim ) && ( GetWaterLevel() < 2 )) ) {
						if (deltaZ < 30.f) {
							stepOrigin.z += 8.f;
						}
					}
				} else {
					if (deltaZ > 8.f) {
						stepOrigin.z -= 8.f;
					} else if (deltaZ > 0) {
						stepOrigin.z -= deltaZ;
					} else if (deltaZ < -8.f) {
						stepOrigin.z += 8.f;
					} else {
						stepOrigin.z += deltaZ;
					}
				}
                //if (!geStepMove->GetPODEntity()->goalEntityPtr)
                //    geStepMove->GetPODEntity()->goalEntityPtr = geStepMove->GetEnemy()->GetPODEntity();
                //dz = geStepMove->GetPODEntity()->currentState.origin[2] - geStepMove->GetPODEntity()->goalEntityPtr->currentState.origin[2];
                //if (geStepMove->GetPODEntity()->goalEntityPtr->client) {
                //    if (dz > 40)
                //        newOrigin.z -= 8;
                //    if (!((geStepMove->GetFlags() & EntityFlags::Swim) && (geStepMove->GetWaterLevel() < 2)))
                //        if (dz < 30)
                //            newOrigin.z += 8;
                //}
                //else {
                //    if (dz > 8)
                //        newOrigin.z -= 8;
                //    else if (dz > 0)
                //        newOrigin.z -= dz;
                //    else if (dz < -8)
                //        newOrigin.z += 8;
                //    else
                //        newOrigin.z += dz;
                //}
            }

			// Perform move trace.
            SGTraceResult trace = SG_Trace( GetOrigin(), GetMins(), GetMaxs(), newOrigin, this, BrushContentsMask::MonsterSolid);

            // Fly monsters don't enter water voluntarily.
            if ( GetFlags() & EntityFlags::Fly ) {
                if ( !GetWaterLevel() ) {
					// Calculate test position.
                    const vec3_t testPosition = {
						trace.endPosition.x,
						trace.endPosition.y,
						trace.endPosition.z + GetMins().z + 1.f
					};

					// Test for non water content.
                    int32_t brushContents = SG_PointContents( testPosition );
                    if ( (brushContents & BrushContentsMask::Liquid) ) {
						// Failure.
						return false;
					}
                }
            }

            // Swim monsters don't exit water voluntarily.
            if ( GetFlags() & EntityFlags::Swim ) {
                if ( GetWaterLevel() < 2 ) {
					// Calculate test position.
                    const vec3_t testPosition = {
						trace.endPosition.x,
						trace.endPosition.y,
						trace.endPosition.z + GetMins().z + 1.f
					};

					// Test for non water content.
                    int32_t brushContents = SG_PointContents( testPosition );
                    if ( !(brushContents & BrushContentsMask::Liquid) ) {
						// Failure.
						return false;
					}
                }
            }

			// If we hit nothing, perform the full move.
            if ( trace.fraction == 1 ) {
				// Perform move.
                SetOrigin(trace.endPosition);

				// Relink if need be.
                if ( relink ) {
                    LinkEntity();
                    SG_TouchTriggers(this);
                }

				// Success.
                return true;
            }

			// Break..?
            if ( !GetEnemy() ) {
                break;
			}
        }

		// Failure.
        return false;
    }

    // Push down from a step height above the wished position.
    //    if (!(geStepMove->monsterInfo.aiflags & AI_NOSTEP))

    float stepsize = STEPSIZE;

    //else
    //    stepsize = 1;

    vec3_t stepOrigin = newOrigin;
	stepOrigin.z += stepsize;
    vec3_t endOrigin = stepOrigin;
    endOrigin.z -= stepsize * 2;

	gi.DPrintf("oldOrigin: %f %f %f\n", oldOrigin.x, oldOrigin.y, oldOrigin.z);
	gi.DPrintf("endOrigin: %f %f %f\n", endOrigin.x, endOrigin.y, endOrigin.z);
	gi.DPrintf("stepOrigin: %f %f %f\n", stepOrigin.x, stepOrigin.y, stepOrigin.z);

    SGTraceResult trace = SG_Trace( stepOrigin, GetMins(), GetMaxs(), endOrigin, this, BrushContentsMask::MonsterSolid);

    // TODO: Make a flag for whether this stepmove entity should check for steps.
    if ( trace.allSolid ) {
		gi.DPrintf("%s : %i : return false\n", __func__, 258);
        return false;
	}

    if ( trace.startSolid ) {
        stepOrigin.z -= stepsize;
        trace = SG_Trace( stepOrigin, GetMins(), GetMaxs(), endOrigin, this, BrushContentsMask::MonsterSolid);
		gi.DPrintf("%s : %i : trace = SG_Trace(...)\n", __func__, 258);
        // TODO: Make a flag for whether this stepmove entity should check for steps.
        //if (trace.allSolid || trace.startSolid)
        //    return false;
        if (trace.allSolid || trace.startSolid) {
			gi.DPrintf("%s : %i : return false\n", __func__, 279);
            return false;
		}
    }


    // Check for water.
    if ( GetWaterLevel() == 0 ) {
		// Calculate test position.
        const vec3_t testPosition = {
			trace.endPosition.x,
			trace.endPosition.y,
			trace.endPosition.z + GetMins().z + 1.f
		};

		// Failed the move if we're not in water, but we did hit it.
        int32_t brushContents = SG_PointContents( testPosition );
        if ( (brushContents & BrushContentsMask::Liquid) ) {
			gi.DPrintf("In if ( (brushContents & BrushContentsMask::Liquid) ) {\n");
			// Failure.
			return false;
		}
    }

    if ( trace.fraction == 1.f ) {
        // If monster had the ground pulled out, go ahead and fall.
        if ( GetFlags() & EntityFlags::PartiallyOnGround ) {
			// Set new origin.
            SetOrigin( GetOrigin() + stepOffset );

			// Relink if wished for.
            if ( relink ) {
                LinkEntity();
                SG_TouchTriggers( this );
            }

			// Unset Ground Entity.
            SetGroundEntity( SGEntityHandle() );

			gi.DPrintf("%s : %i : return true\n", __func__, 308);
			// Success.
            return true;
        }

	    // TODO: Make a flag for whether this stepmove entity should check for steps.
		gi.DPrintf("%s : %i : return false\n", __func__, 314);
        return false;       //return false; // walked off an edge
    }

    // Check point traces down for dangling corners.
    SetOrigin( trace.endPosition );

    if ( !StepMove_CheckBottom( ) ) {
        if ( GetFlags() & EntityFlags::PartiallyOnGround ) {
            // Entity had floor mostly pulled out from underneath it and is trying to correct.
            if ( relink ) {
                LinkEntity();
                SG_TouchTriggers( this );
            }

			gi.DPrintf("%s : %i : return true\n", __func__, 329);
			// Success.
            return true;
        }

		// Revert to old Origin.
		SetOrigin(oldOrigin);

		gi.DPrintf("%s : %i : return false\n", __func__, 337);
		// Failure.
		return false;
    }

    if ( GetFlags() & EntityFlags::PartiallyOnGround ) {
        SetFlags( GetFlags() & ~EntityFlags::PartiallyOnGround );
    }

    SetGroundEntity( trace.gameEntity );
    SetGroundEntityLinkCount( trace.gameEntity->GetLinkCount() );

    // The move is ok.
    if ( relink ) {
        LinkEntity();
        SG_TouchTriggers( this );
    }

	gi.DPrintf("%s : %i : return true: %f %f %f\n", __func__, 355, GetOrigin().x, GetOrigin().y, GetOrigin().z);
    return true;
}

/**
*	@brief	Steps the entity 'dist' distance into the given yaw angle direction.
*	@param	yawDirectionAngle	The angle to turn and step into.
*	@param	stepDistance		The distance to step towards the yawTurnAngle with.
*	@return	True if successful, false otherwise.
**/
const bool SVGBaseMonster::StepMove_WalkDirection(const float yawDirectionAngle, const float stepDistance) {
	// Set the ideal yaw angle.
	SetIdealYawAngle(yawDirectionAngle);

	// Correct yaw angle to steer towards the yawDirectionAngle
	const double idYawA = GetIdealYawAngle();
	StepMove_CorrectYawAngle( );

	// Calculate yaw angle
	const double idYawB = GetIdealYawAngle();
    const double radYaw = Radians(GetIdealYawAngle() );

	// Calculate the stepOffset vector into the 'yaw direction'.
	const vec3_t stepOffset = {
		(float)cos(radYaw) * stepDistance * (float)FRAMETIME_S.count(),
		(float)sin(radYaw) * stepDistance * (float)FRAMETIME_S.count(),
		0.f
	};

	// Print.
	gi.DPrintf("idYawA: %f, idYawB: %f, radYaw: %f, stepOffset.x %f, stepOffset.y %f\n", idYawA, idYawB, radYaw, stepOffset.x, stepOffset.y);

	// Store old origin so we can reset it in case we aren't turned into a wished for yaw angle yet.
	const vec3_t oldOrigin = GetOrigin();
	const vec3_t oldAngles = GetAngles();

	// Try the StepMove.
	if ( StepMove_Step( stepOffset, false ) ) {
		// Get the delta between wished for and current yaw angles.
		const float deltaYawAngle = GetAngles()[vec3_t::Yaw] - GetIdealYawAngle();


		        if (!stepDistance || vec3_distance(GetOrigin(), oldOrigin) >= stepDistance * (1.f / 16.f)) {
		// // Not turned far enough, reset our entity to its old origin.
		if (deltaYawAngle > 45 && deltaYawAngle < 315) {
			SetOrigin(oldOrigin);
			gi.DPrintf("Delta=%f Set Old Origin... \n", deltaYawAngle, stepOffset.x,stepOffset.y,stepOffset.z);
		} else {
			gi.DPrintf("Delta=%f\n", deltaYawAngle, stepOffset.x,stepOffset.y,stepOffset.z);
		}
				}

		// Link Entity back in.
		LinkEntity();

		// Perform a touch triggers.
		SG_TouchTriggers(this);

		// Return true, because even though we did not move yet because of the yaw angle, the move was a valid one.
		return true;
	}

	// Link entity back in.
	LinkEntity();

	// Perform a touch triggers.
	SG_TouchTriggers(this);

	// Failure.
	return false;
}

/**
*	@brief	Tries to correct the Yaw Angle to that which is desired (The 'idealYawAngle').
*
*	@todo	Should move to monster code.
**/
void SVGBaseMonster::StepMove_CorrectYawAngle( ) {
	// Get 'current' Angles.
	vec3_t currentAngles = GetAngles();

	// Get 'current' Yaw Angle.
	float currentYawAngle = AngleMod( currentAngles[vec3_t::Yaw] );

	// Get the 'Ideal' Yaw Angle we want to face to before stepping.
	float idealYawAngle = GetIdealYawAngle();

	// If it's already the ideal angle, leave it be.
	if ( currentYawAngle == idealYawAngle ) {
		return;
	}

	// Get the specific yaw angle direction to turn to this frame.
	float yawMoveDirection = idealYawAngle - currentYawAngle;

	// Get Yaw Turning Speed.
	float yawTurningSpeed = GetYawSpeed();

	// Correct large angles.
	if ( idealYawAngle > currentYawAngle ) {
		// Positive direction.
		if ( yawMoveDirection >= 180 ) {
			yawMoveDirection = yawMoveDirection - 360;
		}
	// Negative direction.
	} else if ( yawMoveDirection >= -180 ) {
		yawMoveDirection = yawMoveDirection + 360;
	}

	// Adjust yaw turning speed to our move direction.
	if ( yawMoveDirection > 0 && yawMoveDirection > yawTurningSpeed ) {
		// Positive direction.
		if ( yawMoveDirection > yawTurningSpeed ) {
			yawMoveDirection = yawTurningSpeed;
		}
	// Negative direction.
	} else if ( yawMoveDirection < -yawTurningSpeed ) {
		yawMoveDirection = -yawTurningSpeed;
	}

	// Update current angles.
	currentAngles[vec3_t::Yaw] = AngleMod( currentYawAngle + yawMoveDirection );

	// Set angles.
	SetAngles( currentAngles );
}

void SVGBaseMonster::StepMove_FixCheckBottom() {
	SetFlags( GetFlags() | EntityFlags::PartiallyOnGround );
}
const bool SVGBaseMonster::StepMove_CheckBottom() {
	//vec3_t	mins, maxs, start, stop;
	SVGTraceResult trace;
	int32_t 		x, y;
	float	mid, bottom;

	const vec3_t origin = GetOrigin();
	vec3_t mins = origin + GetMins();
	vec3_t maxs = origin + GetMaxs();
	
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
	trace = SVG_Trace (start, vec3_zero(), vec3_zero(), stop, this, BrushContentsMask::MonsterSolid);

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
			
			trace = SVG_Trace (start, vec3_zero(), vec3_zero(), stop, this, BrushContentsMask::MonsterSolid);
			
			if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
				bottom = trace.endPosition[2];
			if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
				return false;
		}

	return true;
}





/***
*
*	SlideBox Entity Functions.
*
***/