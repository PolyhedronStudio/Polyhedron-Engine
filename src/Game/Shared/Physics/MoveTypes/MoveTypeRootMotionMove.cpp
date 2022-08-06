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

// SharedGame header itself.
#define SHAREDGAME_UNIT
#include "Game/Shared/SharedGame.h"

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
#include "../RootMotionMove.h"
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






/**
*	@brief	Performs a velocity based 'Root Motion' movement for general NPC's.
*			If EntityFlags::Swim or EntityFlags::Fly are not set, it'll be affected
*			by gravity and perform continious ground and step checks in order
*			to navigate the terrain properly.
**/
void SG_Physics_RootMotionMove(SGEntityHandle &entityHandle) {
    // Assign handle to base entity.
    GameEntity* gameEntity = *entityHandle;

    // Ensure it is a valid entity.
    if (!gameEntity) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
	    //SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
        return;
    }

    // Run think method.
    SG_RunThink(gameEntity);
}

/**
*	@brief	Processes rotational friction calculations.
**/
const vec3_t SG_AddRotationalFriction( SGEntityHandle entityHandle ) { 
	// Assign handle to base entity.
    GameEntity *ent = *entityHandle;

    // Ensure it is a valid entity.
    if ( !ent ) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return vec3_zero();
    }

    // Acquire the rotational velocity first.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    // Set angles in proper direction.
    ent->SetAngles( vec3_fmaf(ent->GetAngles(), FRAMETIME_S.count(), angularVelocity) );

    // Calculate adjustment to apply.
    float adjustment = FRAMETIME_S.count() * ROOTMOTION_MOVE_STOP_SPEED * ROOTMOTION_MOVE_GROUND_FRICTION;

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
int32_t SG_RootMotionMove_CheckForGround( GameEntity *geCheck ) {
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
*	@brief	Starts performing the RootMotion move process.
**/
const int32_t SG_RootMotion_PerformMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, RootMotionMoveState *rootMotionMoveState ) {
	/**
	*	Ensure our SlideMove Game Entity is non (nullptr).
	**/
	if (!geSlider) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): Can't perform RootMotion move because *geSlider == (nullptr)!\n", __func__, sharedModuleName ) );
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
	*	Setup our RootMotionMoveState structure. Keeps track of state for the current frame's move we're about to perform.
	**/
	// The structure containing the current state of the move we're trying to perform.
	//RootMotionMoveState rootMotionMoveState = { 
	const int32_t oldMoveFlags		= rootMotionMoveState->moveFlags;
	const int32_t oldMoveFlagTime	= rootMotionMoveState->moveFlagTime;
	SGTraceResult oldGroundTrace	= rootMotionMoveState->groundTrace;

	*rootMotionMoveState = {
		// Original Physical Attributes.
		.originalVelocity	= geSlider->GetVelocity(),
		.originalOrigin		= geSlider->GetOrigin(),
		.originalMins		= geSlider->GetMins(),
		.originalMaxs		= geSlider->GetMaxs(),

		// Working State & Final Move Attributes.
		.velocity	= geSlider->GetVelocity(),
		.origin		= geSlider->GetOrigin(),
		.mins		= geSlider->GetMins(),
		.maxs		= geSlider->GetMaxs(),
		
		// Gravity Direction.
		.gravityDir = vec3_down(),
		// Slide Bounce Value.
		.slideBounce = slideBounce,


		// The remaining time: Set to FRAMETIME_S(The time a frame takes.). Meaning, we move over time through frame.
		.remainingTime = FRAMETIME_S.count(),


		.groundTrace = oldGroundTrace,
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

		// Entity 'Categorization'.
		.categorizedContent = 0,
		//! Keeps track of the water type we're in.
		.waterType	= geSlider->GetWaterType(),
		//! Keeps track of the water level we reside in.
		.waterLevel	= geSlider->GetWaterLevel(),

		// Zero clip planes and/or entities are touched at a clean move state.
		.numClipPlanes = 0,
		.numTouchEntities = 0,

		.moveFlags = oldMoveFlags,
		.moveFlagTime = oldMoveFlagTime
	};

	/**
	*	If the geSlider entity has any velocty, we'll start attempting to move it around.
	**/
	// Stores the Result Move Flags after the move has completed.
	int32_t moveResultMask = 0;


	//if( oldVelocityLength > 0 ) {
		/**
		*	Step #1: Start attempting to slide move at our velocity.
		**/
		// Start Origin and Velocity.
		const vec3_t org0 = rootMotionMoveState->origin;
		const vec3_t vel0 = rootMotionMoveState->velocity;

		// Move.
		moveResultMask = SG_RootMotion_MoveFrame( rootMotionMoveState );

		if ( (moveResultMask & RootMotionMoveResult::Trapped) ) { //} || !(moveResultMask & RootMotionMoveResult::Moved)) {
			rootMotionMoveState->origin = org0;
			rootMotionMoveState->velocity = vel0;
			return -1;
		}
	//}
		//// if ( ... moveResultMask ... & Flags::Trapped)
		//// return ?
		//// endif

		/**
		*	Step #2: Try and move from origin.z + stepheight.
		**/
		/// Origin and Velocity after the first move.
		//const vec3_t org1 = rootMotionMoveState->origin;
		//const vec3_t vel1 = rootMotionMoveState->velocity;

		//// Calculate 'Up' trace point.
		//const vec3_t upTraceEnd = vec3_fmaf( rootMotionMoveState->origin, ROOTMOTION_MOVE_STEP_HEIGHT, vec3_up() );
		//// Perform 'Up' trace.
		//const SGTraceResult upTraceResult = RM_Trace( 
		//	rootMotionMoveState, 
		//	&upTraceEnd, 
		//	&rootMotionMoveState->mins, 
		//	&rootMotionMoveState->maxs, 
		//	&upTraceEnd);

		//// If it's all solid, we can't step up.
		//if ( upTraceResult.allSolid ) {
		//	// return.
		//	return -1;
		//}

		//// See if we can move from the up position with original velocity.
		//rootMotionMoveState->origin = upTraceResult.endPosition;
		//rootMotionMoveState->velocity = org0;

		//// Move.
		//// The remaining time: Set to FRAMETIME_S(The time a frame takes.). Meaning, we move over time through frame.
		//rootMotionMoveState->remainingTime = FRAMETIME_S.count(),
		//moveResultMask = SG_RootMotion_MoveFrame( rootMotionMoveState );

		//if ( (moveResultMask & RootMotionMoveResult::Trapped) ) { //|| !(moveResultMask & RootMotionMoveResult::Moved)) {
		//	return -1;
		//}

		///**
		//*	Step #3: Push down to find ground. 
		//**/
		//const vec3_t org2 = rootMotionMoveState->origin;
		//const vec3_t vel2 = rootMotionMoveState->velocity;

		//const bool onGround = (SG_GetEntityNumber( rootMotionMoveState->groundTrace.podEntity ) != -1 ? true : false);

		//vec3_fmaf( org2, (onGround ? ROOTMOTION_MOVE_STEP_HEIGHT : ROOTMOTION_MOVE_STEP_HEIGHT * 2), vec3_down() );
		//const vec3_t downTraceEnd = org2 + vec3_t { 
		//	0.f, 
		//	0.f, 
		//	(onGround ? ROOTMOTION_MOVE_STEP_HEIGHT : ROOTMOTION_MOVE_STEP_HEIGHT * 2)
		//};
		//// Perform 'Down' trace.
		//const SGTraceResult downTraceResult = RM_Trace( 
		//	rootMotionMoveState, 
		//	&rootMotionMoveState->origin, 
		//	&rootMotionMoveState->mins, 
		//	&rootMotionMoveState->maxs, 
		//	&downTraceEnd);

		//if (!downTraceResult.allSolid) {
		//	rootMotionMoveState->origin = downTraceResult.endPosition;
		//} 

		//const vec3_t up = rootMotionMoveState->origin;

		//// See which went furthest.
		//// decide which one went farther
		//double down_dist = (org1[0] - org0[0]) * (org1[0] - org0[0])
		//	+ (org1[1] - org0[1]) * (org1[1] - org0[1]);
		//double up_dist = (up[0] - org1[0]) * (up[0] - org1[0])
		//	+ (up[1] - org1[1]) * (up[1] - org1[1]);

		//if (down_dist > up_dist || downTraceResult.plane.normal[2] < 0.7) {
		//	rootMotionMoveState->origin = org2;
		//	rootMotionMoveState->velocity = vel2;
		//	return moveResultMask;
		//}

		////!! Special case
		//// if we were walking along a plane, then we need to copy the Z over
		//rootMotionMoveState->velocity[2] = vel2[2];

		//// Got blocked by a wall...
		//if ( (moveResultMask & RootMotionMoveResult::WallBlocked) ) {
		//	// We'll try and step over it.

	//}

	return moveResultMask;
}

