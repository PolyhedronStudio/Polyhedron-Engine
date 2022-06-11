/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once

// Shared Game.
#include "../../SharedGame.h"

#ifdef SHAREDGAME_SERVERGAME 
	#include "../../../Server/ServerGameLocals.h"
	#include "../../../Server/World/ServerGameWorld.h"
#endif
#ifdef SHAREDGAME_CLIENTGAME
	#include "../../../Client/ClientGameLocals.h"
	#include "../../../Client/World/ClientGameWorld.h"
#endif

// Physics.
#include "../Physics.h"
#include "../SlideMove.h"
// TODO: This needs some fixing hehe... ugly method but hey.
#ifdef SHAREDGAME_SERVERGAME
extern cvar_t *sv_maxvelocity;
extern cvar_t *GetSVGravity();
extern void CheckSVCvars();
#endif

#ifdef SHAREDGAME_CLIENTGAME
extern cvar_t *GetSVMaxVelocity();
extern cvar_t *GetSVGravity();
#endif







//==================================================
// The following should actually be more monster code.
//==================================================
bool SG_SlideMove_CheckBottom(GameEntity* geCheck) {
    vec3_t  start, stop;
    SGTraceResult trace;
    int32_t x, y;
    float   mid, bottom;

    vec3_t mins = geCheck->GetOrigin() + geCheck->GetMins(); //VectorAdd(geCheck->currentState.origin, geCheck->mins, mins);
    vec3_t maxs = geCheck->GetOrigin() + geCheck->GetMaxs(); //VectorAdd(geCheck->currentState.origin, geCheck->maxs, maxs);


    // if all of the points under the corners are solid world, don't bother
    // with the tougher checks
    // the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (SG_PointContents(start) != BrushContents::Solid) {
                goto realcheck;
			}
        }
	}

    return true;        // we got out easy

realcheck:
    //
    // check it for real...
    //
    start[2] = mins[2];

    // the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2] = start[2] - 2 * PM_STEP_HEIGHT;
    trace = SG_Trace(start, vec3_zero(), vec3_zero(), stop, geCheck, SG_SolidMaskForGameEntity(geCheck));

    if (trace.fraction == 1.0) {
        return false;
	}
    mid = bottom = trace.endPosition[2];

    // The corners must be within 16 of the midpoint.
    for (x = 0; x <= 1; x++) {
        for (y = 0; y <= 1; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = SG_Trace(start, vec3_zero(), vec3_zero(), stop, geCheck, SG_SolidMaskForGameEntity(geCheck));

            if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
                bottom = trace.endPosition[2];
            if (trace.fraction == 1.0 || mid - trace.endPosition[2] > PM_STEP_HEIGHT)
                return false;
        }
	}

    return true;
}

//==================================================

//========================================================================


static constexpr float STEPMOVE_STOPSPEED = 100.f;
static constexpr float STEPMOVE_FRICTION = 6.f;
static constexpr float STEPMOVE_WATERFRICTION = 1.f;

/**
*	@brief	Processes rotational friction calculations.
**/
const vec3_t SG_AddRotationalFriction( SGEntityHandle entityHandle ) { 
	// Assign handle to base entity.
    GameEntity *ent = *entityHandle;

    // Ensure it is a valid entity.
    if ( !ent ) {
	    SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
        return vec3_zero();
    }

    // Acquire the rotational velocity first.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    // Set angles in proper direction.
    ent->SetAngles( vec3_fmaf(ent->GetAngles(), FRAMETIME.count(), angularVelocity) );

    // Calculate adjustment to apply.
    float adjustment = FRAMETIME.count() * STEPMOVE_STOPSPEED * STEPMOVE_FRICTION;

    // Apply adjustments.
    angularVelocity = ent->GetAngularVelocity();
    for (int32_t n = 0; n < 3; n++) {
        if (angularVelocity[n] > 0) {
            angularVelocity[n] -= adjustment;
            if (angularVelocity[n] < 0)
                angularVelocity[n] = 0;
        } else {
            angularVelocity[n] += adjustment;
            if (angularVelocity[n] > 0)
                angularVelocity[n] = 0;
        }
    }

	// Return the angular velocity.
	return angularVelocity;
}

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
*	@return	The number of the ground entity this entity is covering ground on.
**/
int32_t SG_BoxSlideMove_CheckForGround( GameEntity *geCheck ) {
	if (!geCheck) {
		// Warn, or just remove check its already tested elsewhere?
		return -1;
	}

	// In case of a flying or swimming monster there's no need to check for ground.
	// If anything we clear out the ground pointer in case the entity did acquire
	// flying skills.
	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
	}

	//// In case the entity has a client and its velocity is high
	//if( geCheck->GetClient() && geCheck->GetVelocity().z > 100) {//180) {
	//	geCheck->SetGroundEntity( SGEntityHandle() );
	//	geCheck->SetGroundEntityLinkCount(0);
	//	return;
	//}

	// if the hull point one-quarter unit down is solid the entity is on ground
	const vec3_t geOrigin = geCheck->GetOrigin();
	vec3_t traceEndPoint = {
		geOrigin.x,
		geOrigin.y,
		geOrigin.z - 0.25f
	};

	// Execute ground seek trace.
	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck));


	// Check steepness.
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
	}

	// If velocity is up, and the actual trace result did not start inside of a solid, it means we have no ground.
	const vec3_t geCheckVelocity = geCheck->GetVelocity();
	if( geCheckVelocity.z > 1 && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return -1;
	}

	// The trace did not start, or end inside of a solid.
	if( !traceResult.startSolid && !traceResult.allSolid ) {
		//VectorCopy( trace.endpos, ent->s.origin );
		geCheck->SetGroundEntity(traceResult.gameEntity);
		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;

		// Since we've found ground, we make sure that any negative Z velocity is zero-ed out.
		if( geCheckVelocity .z < 0) {
			geCheck->SetVelocity({ 
				geCheckVelocity.x,
				geCheckVelocity.y,
				0
			});
		}

		return traceResult.gameEntity->GetNumber();
	}

	// Should never happen..?
	return -1;
}

/**
*	@brief	Starts performing the BoxSlide move process.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, SlideMoveState &slideMoveState ) {
	/**
	*	Ensure our SlideMove Game Entity is non (nullptr).
	**/
	if (!geSlider) {
		SG_Physics_PrintWarning( std::string(__func__) + "*geSlider is (nullptr)!" );
		return 0;
	}

	/**
	*	Store old origin and velocity. We'll need them to perhaps reset in case of any invalid movement.
	**/
	const vec3_t	oldOrigin			= geSlider->GetOrigin();
	const vec3_t	oldVelocity			= geSlider->GetVelocity();
	float			oldVelocityLength	= vec3_length( oldVelocity );

	/**
	*	Get Ground Game Entity Number, if any, store -1 (none) otherwise.
	**/
	// Validate the ground entity.
	GameEntity *geGroundEntity = SGGameWorld::ValidateEntity( geSlider->GetGroundEntityHandle() );
	// Get groundEntity number.
	int32_t groundEntityNumber = (geGroundEntity ? geGroundEntity->GetNumber() : -1);

	/**
	*	Setup our SlideMoveState structure. Keeps track of state for the current frame's move we're about to perform.
	**/
	// The structure containing the current state of the move we're trying to perform.
	//SlideMoveState slideMoveState = { 
	const int32_t moveFlags = slideMoveState.moveFlags;
	const int32_t moveFlagTime = slideMoveState.moveFlagTime;

	slideMoveState = {
		// Geometric Attributes.
		.velocity = geSlider->GetVelocity(),
		.origin = geSlider->GetOrigin(),
		.mins = geSlider->GetMins(),
		.maxs = geSlider->GetMaxs(),
		
		// The remaining time: Set to FRAMETIME(The time a frame takes.). Meaning, we move over time through frame.
		.remainingTime = FRAMETIME.count(),

		// Gravity Direction.
		.gravityDir = vec3_down(),
		// Slide Bounce Value.
		.slideBounce = slideBounce,

		.groundTrace = slideMoveState.groundTrace,
		// Ground Entity Link Count, if any Ground Entity is set, 0 otherwise.
		.groundEntityLinkCount = (groundEntityNumber >= 0 ? geSlider->GetGroundEntityLinkCount() : 0),
		// Number of the ground entity that's set on geSlider.
		.groundEntityNumber = groundEntityNumber,

		// Entity that we're moving around.
		.moveEntityNumber = geSlider->GetNumber(),
		// Entity we want to exclude(skip) from our move testing traces.
		.skipEntityNumber = geSlider->GetNumber(),

		// Entity Flags and Content Mask.
		.entityFlags = geSlider->GetFlags(),
		.contentMask = contentMask,

		// Zero clip planes and/or entities are touched at a clean move state.
		.numClipPlanes = 0,
		.numTouchEntities = 0,

		.moveFlags = moveFlags,
		.moveFlagTime = moveFlagTime
	};

	/**
	*	If the geSlider entity has any velocty, we'll start attempting to move it around.
	**/
	// Stores the Result Move Flags after the move has completed.
	int32_t blockedMask = 0;


	//if( oldVelocityLength > 0 ) {
		/**
		*	Step #1: Start attempting to slide move at our velocity.
		**/
		blockedMask = SG_SlideMove( &slideMoveState );
		
		const vec3_t org0 = slideMoveState.origin;
		const vec3_t vel0 = slideMoveState.velocity;

		// Got blocked by a wall...
		if (blockedMask & SlideMoveFlags::WallBlocked) {

		}

		// We touched something, try and step over it.
		if ( (blockedMask & SlideMoveFlags::PlaneTouched) ) {

		}
	//}

	return blockedMask;
}

/**
*	@brief	Performs a velocity based 'slidebox' movement for general NPC's.
*			If FL_SWIM or FL_FLY are not set, it'll check whether there is any
*			ground at all, if it has been removed the monster will be pushed
*			downwards due to gravity effect kicking in.
**/
void SG_Physics_BoxSlideMove(SGEntityHandle &entityHandle) {
    // Assign handle to base entity.
    GameEntity* gameEntity = *entityHandle;

    // Ensure it is a valid entity.
    if (!gameEntity) {
	    SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
        return;
    }

    // Run think method.
    SG_RunThink(gameEntity);
}
