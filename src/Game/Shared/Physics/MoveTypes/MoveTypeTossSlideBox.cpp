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




/**
*	@brief	Performs a SlideBox move that expects the entity to have been 'tossed' by setting a 
*			velocity and angular velocity into a random direction. Does not perform 'Stepping' up
*			stairs. When touching a 'walkable' surface it'll gradually slow down by friction and
*			stop rotating.
**/
const int32_t SG_Physics_TossSlideBox( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, SlideBoxMove *slideBoxMove ) {
	/**
	*	Ensure our SlideMove Game Entity is non (nullptr).
	**/
	if (!geSlider) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): Can't perform RootMotion move because *geSlider == (nullptr)!\n", __func__, sharedModuleName ) );
		return 0;
	}
	
	// Bound our velocity within sv_maxvelocity limits.
	SG_BoundVelocity( geSlider );

    // Get angular velocity for applying rotational friction.
    const vec3_t angularVelocity = geSlider->GetAngularVelocity();

	// If we got any angular velocity, apply friction.
    if ( angularVelocity.x || angularVelocity.y || angularVelocity.z ) {

		geSlider->SetAngularVelocity( SG_CalculateRotationalFriction( geSlider ) );
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

			// Ground entity.
			.groundEntity = groundEntityNumber,
			// Entity we want to exclude(skip) from our move testing traces.
			.skipEntityNumber = geSlider->GetNumber(),
			
			// Entity Flags and Content Mask.
			.contentMask = SG_SolidMaskForGameEntity( geSlider ),

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
	if( blockedMask != 0 ) {
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

		// Apply ground friction now since we're officially on-ground.
		if (geNewGroundEntity) {
			SG_AddGroundFriction( geSlider, ( friction > 0 ? friction : SLIDEBOX_GROUND_FRICTION), SLIDEBOX_STOP_SPEED );
		}

		// Stop to a halt in case velocity becomes too low, this way it won't look odd and jittery.
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
	
    // Run think method.
    SG_RunThink(geSlider);

	return blockedMask;
}
