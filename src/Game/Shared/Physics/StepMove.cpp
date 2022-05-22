/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// m_move.c -- monster movement

// Core.
#include "../ServerGameLocals.h"
#include "../Entities.h"
#include "../Utilities.h"

// UP-STEP height in "Quake Units". This is used commonly all over for each stepmove entity.
// TODO: In the future it is likely one would want to be able to set this property to a custom
// value for each entity type.
#define STEPSIZE    18

/*
=============
SVG_StepMove_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean SVG_StepMove_CheckBottom(IServerGameEntity* ent)
{
    vec3_t  start, stop;
    SVGTraceResult trace;
    int32_t x, y;
    float   mid, bottom;

    vec3_t mins = ent->GetOrigin() + ent->GetMins(); //VectorAdd(ent->currentState.origin, ent->mins, mins);
    vec3_t maxs = ent->GetOrigin() + ent->GetMaxs(); //VectorAdd(ent->currentState.origin, ent->maxs, maxs);


                                                     // if all of the points under the corners are solid world, don't bother
                                                     // with the tougher checks
                                                     // the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0; x <= 1; x++)
        for (y = 0; y <= 1; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (SG_PointContents(start) != BrushContents::Solid)
                goto realcheck;
        }

    c_yes++;
    return true;        // we got out easy

realcheck:
    c_no++;
    //
    // check it for real...
    //
    start[2] = mins[2];

    // the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2] = start[2] - 2 * STEPSIZE;
    trace = SVG_Trace(start, vec3_zero(), vec3_zero(), stop, ent, BrushContentsMask::MonsterSolid);

    if (trace.fraction == 1.0)
        return false;
    mid = bottom = trace.endPosition[2];

    // the corners must be within 16 of the midpoint
    for (x = 0; x <= 1; x++)
        for (y = 0; y <= 1; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = SVG_Trace(start, vec3_zero(), vec3_zero(), stop, ent, BrushContentsMask::MonsterSolid);

            if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
                bottom = trace.endPosition[2];
            if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
                return false;
        }

    c_yes++;
    return true;
}

void SG_StepMove_CheckGround(IServerGameEntity* ent)
{
    vec3_t      point;
    SVGTraceResult     trace;

    if (ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly))
        return;

    if (ent->GetVelocity().z > 100) {
        ent->SetGroundEntity( SGEntityHandle() );
        return;
    }

    // if the hull point one-quarter unit down is solid the entity is on ground
    point = ent->GetOrigin() - vec3_t {
        0.f, 0.f, 0.25f 
    };

    trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), point, ent, BrushContentsMask::MonsterSolid);

    // check steepness
    //if ((trace.plane.normal[2] < 0.7 && !trace.allSolid)
    if ((trace.plane.normal[2] < 0.7 && !trace.allSolid) || (!trace.gameEntity)) {
        ent->SetGroundEntity( SGEntityHandle() );
        return;
    }

    //  ent->groundentity = trace.gameEntity;
    //  ent->groundentity_linkcount = trace.gameEntity->linkcount;
    //  if (!trace.startsolid && !trace.allsolid)
    //      VectorCopy (trace.endpos, ent->s.origin);
    if ((!trace.startSolid && !trace.allSolid) &&
        (trace.gameEntity && trace.gameEntity->GetPODEntity())) {
        ent->SetOrigin(trace.endPosition);
        ent->SetGroundEntity(trace.gameEntity);
        ent->SetGroundEntityLinkCount(trace.gameEntity->GetLinkCount());
        vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, 0 });
    }
}

/**
*	@brief	This is the actual SG_Stepmove implementation. Tries to move
*			the entity a given distance over a second while trying to 
*			step over and off obstacles if needed. It is influenced by
*			and dependent on the game's tick rate.
*
*	@return	False if the move has failed and the entity remains at its position.
*			True otherwise.
**/
bool SG_MoveStep(GameEntity* geStepMove, vec3_t move, qboolean relink)
{
    float       dz;
    SVGTraceResult    trace;
    int         i;
    float       stepsize;
    vec3_t      test;
    int         contents;

    // Get Old Origin.
    vec3_t oldOrigin = geStepMove->GetOrigin();

	// Calculate wished for origin.
    vec3_t newOrigin = oldOrigin + move;

    // Flying monsters don't step up.
    if (geStepMove->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
        // Try one move with vertical motion, then one without.
        for (i = 0; i < 2; i++) {
			// newOrigin = geStepMove->GetOrigin() + move;

            if (i == 0 && geStepMove->GetEnemy()) {
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
            trace = SG_Trace(geStepMove->GetOrigin(), geStepMove->GetMins(), geStepMove->GetMaxs(), newOrigin, geStepMove, BrushContentsMask::MonsterSolid);

            // Fly monsters don't enter water voluntarily.
            if (geStepMove->GetFlags() & EntityFlags::Fly) {
                if (!geStepMove->GetWaterLevel()) {
					// Calculate test position.
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + geStepMove->GetMins().z + 1;

					// Test for water content.
                    contents = SG_PointContents(test);
                    if (contents & BrushContentsMask::Liquid) {
                        // Failure.
						return false;
					}
                }
            }

            // Swim monsters don't exit water voluntarily.
            if (geStepMove->GetFlags() & EntityFlags::Swim) {
                if (geStepMove->GetWaterLevel() < 2) {
					// Calculate test position.
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + geStepMove->GetMins().z + 1;

					// Test for non water content.
                    contents = SG_PointContents(test);
                    if (!(contents & BrushContentsMask::Liquid)) {
						// Failure.
						return false;
					}
                }
            }

			// If we hit nothing, perform the full move.
            if (trace.fraction == 1) {
				// Perform move.
                geStepMove->SetOrigin(trace.endPosition);

				// Relink if need be.
                if (relink) {
                    geStepMove->LinkEntity();
                    SG_TouchTriggers(geStepMove);
                }

				// Success.
                return true;
            }

            if (!geStepMove->GetEnemy()) {
                break;
			}
        }

		// Failure.
        return false;
    }

    // Push down from a step height above the wished position.
    //    if (!(geStepMove->monsterInfo.aiflags & AI_NOSTEP))
    stepsize = STEPSIZE;
    //else
    //    stepsize = 1;

    newOrigin[2] += stepsize;
    vec3_t end = newOrigin;
    end[2] -= stepsize * 2;

    trace = SG_Trace(newOrigin, geStepMove->GetMins(), geStepMove->GetMaxs(), end, geStepMove, BrushContentsMask::MonsterSolid);

    // TODO: Make a flag for whether this stepmove entity should check for steps.
    if (trace.allSolid)
        return false;

    if (trace.startSolid) {
        newOrigin[2] -= stepsize;
        trace = SG_Trace(newOrigin, geStepMove->GetMins(), geStepMove->GetMaxs(), end, geStepMove, BrushContentsMask::MonsterSolid);

        // TODO: Make a flag for whether this stepmove entity should check for steps.
        //if (trace.allSolid || trace.startSolid)
        //    return false;
    }


    // Check for water.
    if (geStepMove->GetWaterLevel() == 0) {
        test[0] = trace.endPosition[0];
        test[1] = trace.endPosition[1];
        test[2] = trace.endPosition[2] + geStepMove->GetMins().z + 1;
        contents = SG_PointContents(test);

		// Don't start swimming now ayyyy captain.
        if (contents & BrushContentsMask::Liquid) {
            return false;
		}
    }

    if (trace.fraction == 1) {
        // If monster had the ground pulled out, go ahead and fall.
        if (geStepMove->GetFlags() & EntityFlags::PartiallyOnGround) {
            geStepMove->SetOrigin(geStepMove->GetOrigin() + move);
            if (relink) {
                geStepMove->LinkEntity();
                SG_TouchTriggers(geStepMove);
            }
            geStepMove->SetGroundEntity( SGEntityHandle() );
            return true;
        }

	    // TODO: Make a flag for whether this stepmove entity should check for steps.
        //return false;       // walked off an edge
    }

    // Check point traces down for dangling corners.
    geStepMove->SetOrigin(trace.endPosition);

    if (!SVG_StepMove_CheckBottom(geStepMove)) {
        if (geStepMove->GetFlags() & EntityFlags::PartiallyOnGround) {
            // entity had floor mostly pulled out from underneath it
            // and is trying to correct
            if (relink) {
                geStepMove->LinkEntity();
                SG_TouchTriggers(geStepMove);
            }
            return true;
        }
        //geStepMove->SetOrigin(oldOrigin);
        //return false;
    }

    if (geStepMove->GetFlags() & EntityFlags::PartiallyOnGround) {
        geStepMove->SetFlags(geStepMove->GetFlags() & ~EntityFlags::PartiallyOnGround);
    }
    geStepMove->SetGroundEntity(trace.gameEntity);
    geStepMove->SetGroundEntityLinkCount(trace.gameEntity->GetLinkCount());

    // The move is ok.
    if (relink) {
        geStepMove->LinkEntity();
        SG_TouchTriggers(geStepMove);
    }

    return true;
}


//============================================================================

/**
*	@brief	Tries to correct the Yaw Angle to that which is desired.
*
*	@todo	Should move to monster code.
**/
static void SG_StepMove_ChangeYawAngle( GameEntity *geStepMove ) {
	// Sanity Check.
	if (!geStepMove) {
		return;
	}

	// Get 'current' Yaw Angle.
	float currentYawAngle = AngleMod( geStepMove->GetAngles()[vec3_t::Yaw] );

	// Get the 'Ideal' Yaw Angle we want to face to before stepping.
	float idealYawAngle = ( geStepMove ? geStepMove->GetIdealYawAngle() : 0 );

	// If it's already the ideal angle, leave it be.
	if (currentYawAngle == idealYawAngle) {
		return;
	}

	// Get the specific yaw angle direction to step into this frame.
	float yawMoveDirection = ideal - current;

	// Get Yaw Turning Speed.
	float yawTurningSpeed = geStepMove->GetYawSpeed();

	// Correct large angles.
	if (idealYawAngle > currentYawAngle) {
		if (yawMoveDirection >= 180) {
			yawMoveDirection = yawMoveDirection - 360;
		}
	} else if (yawMoveDirection >= -180) {
			yawMoveDirection = yawMoveDirection + 360;
	}

	// Adjust yaw turning speed to our move direction.
	if (yawMoveDirection > 0 && yawMoveDirection > yawTurningSpeed) {
		if (yawMoveDirection > yawTurningSpeed) {
			yawMoveDirection = yawTurningSpeed;
		}
	} else if (yawMoveDirection < -yawTurningSpeed) {
		yawMoveDirection = -yawTurningSpeed;
	}
}



/**
*	@brief	Turns to the movement direction, and walks the current distance if
*			facing it.
**/
bool SG_StepMove_StepDirection(SVGBaseEntity* ent, float yaw, float dist) {
	vec3_t      move, oldOrigin;
	float       delta;

	// Set the ideal yaw angle.
	ent->SetIdealYawAngle(yaw);

	// Correct yaw if need be.
	SG_StepMove_ChangeYawAngle(ent->GetPODEntity());

	// Calculate yaw angle
    yaw = yaw * M_PI * 2 / 360;

	// Calculate move direction.
	const vec3_t moveDirection = {
		cosf(yaw) * dist,
		sinf(yaw) * dist,
		0.f
	};

	// Store old origin so we can reset it in case we aren't turned into a wished for yaw angle yet.
	const vec3_t oldOrigin = ent->GetOrigin();

	// Try the StepMove.
	if (SVG_MoveStep(ent, move, false)) {
		// Get the delta between wished for and current yaw angles.
		delta = ent->GetPODEntity()->currentState.angles[vec3_t::Yaw] - ent->GetIdealYawAngle();

		// // Not turned far enough, reset our entity to its old origin.
		if (delta > 45 && delta < 315) {
			ent->SetOrigin(oldOrigin);
		}

		// Link Entity back in.
		ent->LinkEntity();

		// Perform a touch triggers.
		SG_TouchTriggers(ent);

		// Return true, because even though we did not move yet because of the yaw angle, the move was a valid one.
		return true;
	}

	// Link entity back in.
	ent->LinkEntity();

	// Perform a touch triggers.
	SG_TouchTriggers(ent);

	// Failure.
	return false;
}

/**
*	@brief	Steps the entity 'dist' distance into the given yaw angle direction.
*	@param	yawTurnAngle	The angle to turn and step into.
*	@param	distance		The distance to step towards the yawTurnAngle with.
*	@return	True if successful, false otherwise.
**/
qboolean SG_StepMove_Walk(IServerGameEntity* ent, float yawTurnAngle, float distance) {
	// Can't 'Step/Walk' without ground.
	if (!SGGameWorld::ValidateEntity(ent->GetGroundEntityHandle())) {
		return false;
	}

	// We can't Fly or Swim either.
	if ( !( ent->GetFlags() & (EntityFlags::Fly | EntityFlags::Swim) ) ) {
		return false;
	}

	// Calculate yaw angle.
    yaw = yaw * M_PI * 2 / 360;

	// Calculate move direction.
	const vec3_t moveDirection = {
		cosf(yaw) * dist,
		sinf(yaw) * dist,
		0.f
	};

	// Try and perform the actual step move.
    return SG_MoveStep(ent, move, true);
}
