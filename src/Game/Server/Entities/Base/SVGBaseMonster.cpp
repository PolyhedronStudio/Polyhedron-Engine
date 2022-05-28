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
	// Base Spawn.
	Base::Spawn();

	// Set ClipMask to MonsterSolid.
	SetClipMask( BrushContentsMask::MonsterSolid );
	// Set Monster ServerFlag.
	SetServerFlags( EntityServerFlags::Monster );
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
*	Monster Entity Functions.
*
***/
/**
*	@brief	Categorizes what other contents the entity resides in. (Water, Lava, or...)
**/
void SVGBaseMonster::CategorizePosition() {
	// WaterLevel: Feet.
	vec3_t point = GetOrigin();
	point.z += GetMins().z + 1.f;

	// Get contents at point.
	int32_t contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents& BrushContents::Water ) ) {
		SetWaterLevel(WaterLevel::None);
		SetWaterType(0);
		return;
	}

	// Store water type.
	SetWaterType(contents);
	SetWaterLevel(1);

	// WaterLevel: Waist.
	point.z += 40.f;

	// Get contents at point.
	contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents & BrushContents::Water ) ) {
		SetWaterType(2);
		return;
	}

	// WaterLevel: Waist.
	point.z += 45.f;

	// Get contents at point.
	contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents & BrushContents::Water ) ) {
		SetWaterType(3);
		return;
	}
	
}

/**
*	@brief	Rotates/Turns the monster into the Ideal Yaw angle direction.
*	@return	The delta yaw angles of this Turn.
**/
float SVGBaseMonster::TurnToIdealYawAngle() {
	// Get current(and to be, previous) Angles.
	const vec3_t _previousAngles = GetAngles();

	// Angle Mod the current angles and compare to Ideal Yaw angle.
	float _currentYawAngle = AngleMod( _previousAngles[vec3_t::Yaw] );
	float _idealYawAngle = GetIdealYawAngle();
		
	if ( _currentYawAngle == _idealYawAngle) {
		return 0.f;
	}

	// We're not at ideal yaw angle yet, so we'll calculate how
	// far to move it, and at what speed.
	float _yawMove = _idealYawAngle - _currentYawAngle;
	float _yawTurningSpeed = GetYawSpeed();

	if ( _idealYawAngle > _currentYawAngle) {
		if ( _yawMove >= 180.f) {
			_yawMove = _yawMove - 360.f;
		}
	} else {
		if ( _yawMove <= -180.f ) {
			_yawMove = _yawMove + 360.f;
		}
	}
	if ( _yawMove > 0.f ) {
		if ( _yawMove > _yawTurningSpeed ) {
			_yawMove = _yawTurningSpeed;
		}
	} else {
		if ( _yawMove < -_yawTurningSpeed ) {
			_yawMove = - _yawTurningSpeed;
		}
	}

	// Set the new angles, Angle Modding the Yaw.
	SetAngles( { _previousAngles.x, AngleMod( _currentYawAngle + _yawMove * FRAMETIME.count() * _yawTurningSpeed ), _previousAngles.z});

	// Return delta angles.
	return GetAngles()[vec3_t::Yaw] - GetIdealYawAngle();
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
	
//	gi.DPrintf("---------------------\n");

    // Flying monsters don't step up.
    if ( GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) {
		//gi.DPrintf("In if ( GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) )\n");
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

	//gi.DPrintf("oldOrigin: %f %f %f\n", oldOrigin.x, oldOrigin.y, oldOrigin.z);
	//gi.DPrintf("endOrigin: %f %f %f\n", endOrigin.x, endOrigin.y, endOrigin.z);
	//gi.DPrintf("stepOrigin: %f %f %f\n", stepOrigin.x, stepOrigin.y, stepOrigin.z);

    SGTraceResult trace = SG_Trace( stepOrigin, GetMins(), GetMaxs(), endOrigin, this, BrushContentsMask::MonsterSolid);

    // TODO: Make a flag for whether this stepmove entity should check for steps.
    if ( trace.allSolid ) {
		//gi.DPrintf("%s : %i : return false\n", __func__, 258);
        return false;
	}

    if ( trace.startSolid ) {
        stepOrigin.z -= stepsize;
        trace = SG_Trace( stepOrigin, GetMins(), GetMaxs(), endOrigin, this, BrushContentsMask::MonsterSolid);
		//gi.DPrintf("%s : %i : trace = SG_Trace(...)\n", __func__, 258);
        // TODO: Make a flag for whether this stepmove entity should check for steps.
        //if (trace.allSolid || trace.startSolid)
        //    return false;
        if (trace.allSolid || trace.startSolid) {
			//gi.DPrintf("%s : %i : return false\n", __func__, 279);
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
			//gi.DPrintf("In if ( (brushContents & BrushContentsMask::Liquid) ) {\n");
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

			//gi.DPrintf("%s : %i : return true\n", __func__, 308);
			// Success.
            return true;
        }

	    // TODO: Make a flag for whether this stepmove entity should check for steps.
		//gi.DPrintf("%s : %i : return false\n", __func__, 314);
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

			//gi.DPrintf("%s : %i : return true\n", __func__, 329);
			// Success.
            return true;
        }

		// Revert to old Origin.
		SetOrigin(oldOrigin);

		//gi.DPrintf("%s : %i : return false\n", __func__, 337);
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

	//gi.DPrintf("%s : %i : return true: %f %f %f\n", __func__, 355, GetOrigin().x, GetOrigin().y, GetOrigin().z);
    return true;
}

/**
*	@brief	Steps the entity 'dist' distance into the given yaw angle direction.
*	@param	yawDirectionAngle	The angle to turn and step into.
*	@param	stepDistance		The distance to step towards the yawTurnAngle with.
*	@return	True if successful, false otherwise.
**/
const bool SVGBaseMonster::StepMove_WalkDirection(const float yawDirectionAngle, const float stepDistance) {
    const double radYaw = Radians( yawDirectionAngle );

	// Calculate the stepOffset vector into the 'yaw direction'.
	const vec3_t stepOffset = {
		(float)cos(radYaw) * stepDistance * (float)FRAMETIME_S.count(),
		(float)sin(radYaw) * stepDistance * (float)FRAMETIME_S.count(),
		0.f
	};

	// Return the results of the move.
	return StepMove_Step( stepOffset, false );
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
