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

//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"

// Physics.
#include "../Physics.h"
#include "../SlideBox.h"

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
void SG_Physics_SlideBoxMove( SGEntityHandle &entityHandle ) {
    // Assign handle to base entity.
    GameEntity* gameEntity = *entityHandle;

    // Ensure it is a valid entity.
    if (!gameEntity) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return;
    }

    // Run think method.
    SG_RunThink(gameEntity);
}

///**
//*	@brief	Processes rotational friction calculations.
//**/
//static const vec3_t SG_AddRotationalFriction( SGEntityHandle entityHandle ) { 
//	// Assign handle to base entity.
//    GameEntity *ent = *entityHandle;
//
//    // Ensure it is a valid entity.
//    if ( !ent ) {
//	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
//        return vec3_zero();
//    }
//
//    // Acquire the rotational velocity first.
//    vec3_t angularVelocity = ent->GetAngularVelocity();
//
//    // Set angles in proper direction.
//    ent->SetAngles( vec3_fmaf(ent->GetAngles(), FRAMETIME_S.count(), angularVelocity) );
//
//    // Calculate adjustment to apply.
//    float adjustment = FRAMETIME_S.count() * ROOTMOTION_MOVE_STOP_SPEED * ROOTMOTION_MOVE_GROUND_FRICTION;
//
//    // Apply adjustments.
//    angularVelocity = ent->GetAngularVelocity();
//    for (int32_t n = 0; n < 3; n++) {
//        if (angularVelocity[n] > 0) {
//            angularVelocity[n] -= adjustment;
//            if (angularVelocity[n] < 0)
//                angularVelocity[n] = 0;
//        } else {
//            angularVelocity[n] += adjustment;
//            if (angularVelocity[n] > 0)
//                angularVelocity[n] = 0;
//        }
//    }
//
//	// Return the angular velocity.
//	return angularVelocity;
//}
//
///**
//*	@brief	Checks if this entity should have a groundEntity set or not.
//*	@return	The number of the ground entity this entity is covering ground on.
//**/
//int32_t SG_RootMotionMove_CheckForGround( GameEntity *geCheck ) {
//	if (!geCheck) {
//		// Warn, or just remove check its already tested elsewhere?
//		return -1;
//	}
//
//	// In case of a flying or swimming monster there's no need to check for ground.
//	// If anything we clear out the ground pointer in case the entity did acquire
//	// flying skills.
//	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
//		geCheck->SetGroundEntity( SGEntityHandle() );
//		geCheck->SetGroundEntityLinkCount(0);
//		return -1;
//	}
//
//	//// In case the entity has a client and its velocity is high
//	//if( geCheck->GetClient() && geCheck->GetVelocity().z > 100) {//180) {
//	//	geCheck->SetGroundEntity( SGEntityHandle() );
//	//	geCheck->SetGroundEntityLinkCount(0);
//	//	return;
//	//}
//
//	// if the hull point one-quarter unit down is solid the entity is on ground
//	const vec3_t geOrigin = geCheck->GetOrigin();
//	vec3_t traceEndPoint = {
//		geOrigin.x,
//		geOrigin.y,
//		geOrigin.z - 0.25f
//	};
//
//	// Execute ground seek trace.
//	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck));
//
//
//	// Check steepness.
//	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
//		geCheck->SetGroundEntity( SGEntityHandle() );
//		geCheck->SetGroundEntityLinkCount(0);
//		return -1;
//	}
//
//	// If velocity is up, and the actual trace result did not start inside of a solid, it means we have no ground.
//	const vec3_t geCheckVelocity = geCheck->GetVelocity();
//	if( geCheckVelocity.z > 1 && !traceResult.startSolid ) {
//		geCheck->SetGroundEntity( SGEntityHandle() );
//		geCheck->SetGroundEntityLinkCount(0);
//		return -1;
//	}
//
//	// The trace did not start, or end inside of a solid.
//	if( !traceResult.startSolid && !traceResult.allSolid ) {
//		//VectorCopy( trace.endpos, ent->s.origin );
//		geCheck->SetGroundEntity(traceResult.gameEntity);
//		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;
//
//		// Since we've found ground, we make sure that any negative Z velocity is zero-ed out.
//		if( geCheckVelocity .z < 0) {
//			geCheck->SetVelocity({ 
//				geCheckVelocity.x,
//				geCheckVelocity.y,
//				0
//			});
//		}
//
//		return traceResult.gameEntity->GetNumber();
//	}
//
//	// Should never happen..?
//	return -1;
//}
/**
*	@brief	Starts performing the RootMotion move process.
**/
/**
*	@brief	Processes rotational friction calculations.
**/
extern const vec3_t SG_AddRotationalFriction( SGEntityHandle entityHandle );

const int32_t SG_Physics_SlideBoxMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, SlideBoxMove *slideBoxMove ) {
	/**
	*	Ensure our SlideMove Game Entity is non (nullptr).
	**/
	if (!geSlider) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): Can't perform RootMotion move because *geSlider == (nullptr)!\n", __func__, sharedModuleName ) );
		return 0;
	}
	
	// Bound our velocity within sv_maxvelocity limits.
	SG_CheckVelocity( geSlider );

    // Get angular velocity for applying rotational friction.
    const vec3_t angularVelocity = geSlider->GetAngularVelocity();

	// If we got any angular velocity, apply friction.
    if ( angularVelocity.x || angularVelocity.y || angularVelocity.z ) {
		SG_AddRotationalFriction( geSlider );
	}

    // Run think method.
    SG_RunThink(geSlider);

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
	*	
	**/
	int32_t blockedMask = 0;

	if ( oldVelocityLength > 0 ) {
		

		// Setup our SlideBoxMove structure. Keeps track of state for the current frame's move we're about to perform.
		*slideBoxMove = {
			// Working State & Final Move Attributes.
			.velocity	= geSlider->GetVelocity(),
			.origin		= geSlider->GetOrigin(),
			.mins		= geSlider->GetMins(),
			.maxs		= geSlider->GetMaxs(),

			// The remaining time: Set to FRAMETIME_S(The time a frame takes.). Meaning, we move over time through frame.
			.remainingTime = FRAMETIME_S.count(),

			// Gravity Direction.
			.gravityDir = vec3_down(),
			// Slide Bounce Value.
			.slideBounce = slideBounce,

			// Entity we want to exclude(skip) from our move testing traces.
			.skipEntityNumber = geSlider->GetNumber(),

			// Entity Flags and Content Mask.
			.contentMask = contentMask,

			// Zero clip planes and/or entities are touched at a clean move state.
			.numClipPlanes = 0,
			.numTouchEntities = 0,
		};

		// Perform move.
		blockedMask = SG_SlideMove( slideBoxMove );

		// Copy back results.
		geSlider->SetOrigin( slideBoxMove->origin );
		geSlider->SetVelocity( slideBoxMove->velocity );
		
		// Copy back ground entity results.
		if ( slideBoxMove->groundEntity >= 0 ) {
			GameEntity *geGroundEntity = SG_GetGameEntityByNumber( slideBoxMove->groundEntity );
			if ( geGroundEntity ) {
				geSlider->SetGroundEntity( geGroundEntity );
				geSlider->SetGroundEntityLinkCount( geGroundEntity->GetLinkCount() );
			}
		} else {
				geSlider->SetGroundEntity( SGEntityHandle() );
				geSlider->SetGroundEntityLinkCount( 0 );
		}

		// Link it in.
		geSlider->LinkEntity();
	}

	/**
	*	Touch Callbacks.
	**/
	// Execute touch callbacks.
	if( contentMask != 0 ) {
		GameEntity *otherEntity = nullptr;

		// Call Touch Triggers on our slide box entity for its new position.
		SG_TouchTriggers( geSlider );

		// Dispatch 'Touch' callback functions to all touched entities we caught and stored in our moveState.
		for( int32_t i = 0; i < slideBoxMove->numTouchEntities; i++ ) {
			otherEntity = SG_GetGameEntityByNumber( slideBoxMove->touchEntities[i] );
			
			// Don't touch projectiles.
			if( !otherEntity || !otherEntity->IsInUse() ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			// First dispatch a touch on the object we're hitting.
			otherEntity->DispatchTouchCallback( otherEntity, geSlider, nullptr, nullptr );

			// Now dispatch a touch callback for THIS entity.
			geSlider->DispatchTouchCallback( geSlider, otherEntity, nullptr, nullptr );

			// In case touch callbacks caused it to be non 'in-use':
			if( !geSlider->IsInUse() ) {
				break; // Break here.
			}
		}
	}

	/**
	*	Step #5:	- If still in use, check for ground, and see if our velocity came to a halt
	*				so we can safely trigger a Stop Dispatch callback.
	**/
	// Assuming we're still in use, set ourselves to a halt if 
	if ( geSlider->IsInUse() ) {
		// Check for ground entity.
		int32_t groundEntityNumber = slideBoxMove->groundEntity;

		// Revalidate it
		GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( SG_GetGameEntityByNumber( groundEntityNumber ) );

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geNewGroundEntity && vec3_length( geSlider->GetVelocity() ) <= 1.f && oldVelocityLength > 1.f ) {
			// Zero out velocities.
			geSlider->SetVelocity( vec3_zero() );
			geSlider->SetAngularVelocity( vec3_zero() );

			// Stop.
			geSlider->DispatchStopCallback( );
		}
	}

	// Execute touch triggers (Since entities might move during touch callbacks, we might've
	// hit new entities.)
    SG_TouchTriggers( geSlider );

	return blockedMask;
}
