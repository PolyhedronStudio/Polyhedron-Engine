/***
*
*	License here.
*
*	@file
* 
*   Client Side FuncRotating -> Intented to be predicted someday.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
//! BaseMover.
#include "Game/Client/Entities/Base/CLGBaseMover.h"
#include "Game/Client/Entities/Func/FuncRotating.h"
//! Gamemodes.
#include "Game/Client/Gamemodes/IGamemode.h"
//! Game World.
#include "Game/Client/World/ClientGameWorld.h"

//===============
// FuncRotating::ctor
//===============
FuncRotating::FuncRotating( Entity* entity )
	: Base( entity ) {
}

/**
*	@brief
**/
void FuncRotating::Spawn() {
	Base::Spawn();
	// Allow for angles to determine direction.

	//SetMoveDirection( vec3_normalize( GetAngles() ) );

	//// Reset Angles.
	SetAngles(vec3_zero());

	// TODO: Implement properly. Someone, feel free to submit a PR :)
	// Set the axis of rotation, support custom axes as well
	if ( GetSpawnFlags() & SF_XAxis ) {
	    moveDirection = { 0.0f, 0.0f, 1.0f };
	} else if ( GetSpawnFlags() & SF_YAxis ) {
		moveDirection = { 1.0f, 0.0f, 0.0f };
	} else {
		moveDirection = vec3_t { 0.0f, 1.0f, 0.0f };
	}

	SetSolid( Solid::BSP );
	SetModel( GetModel() );
	SetMoveType( ( GetSpawnFlags() & SF_StopOnBlock ) ? MoveType::Stop : MoveType::Push );

	if ( GetSpawnFlags() & SF_Reverse ) {
	    moveDirection = vec3_negate(moveDirection);
	}

	if ( !GetSpeed() ) {
		speed = 100.0f;
	}
	moveInfo.dir = moveDirection;
	moveInfo.state = FuncRotateState::Stopped;

	if ( GetDamage() ) {
		SetDamage( 2.0f );
		SetBlockedCallback( &FuncRotating::RotatorBlocked );
	}

	SetUseCallback( &FuncRotating::RotatorUse );

	if ( GetSpawnFlags() & SF_Animated ) {
		SetEffects( GetEffects() | EntityEffectType::AnimCycleAll2hz );
	} else if ( GetSpawnFlags() & SF_AnimatedFast ) {
		SetEffects( GetEffects() | EntityEffectType::AnimCycleAll30hz );
	}
	if ( GetNumber() == 37) {
		SetRenderEffects( RenderEffects::DebugBoundingBox );
	}
	LinkEntity();
}

/**
*	@brief
**/
void FuncRotating::PostSpawn() {
	if ( GetSpawnFlags() & SF_StartOn ) {
		DispatchUseCallback( nullptr, nullptr );
	}
}

/**
*	@brief
**/
void FuncRotating::RotatorBlocked( IClientGameEntity* other ) {
	// Non existent in client game for now.
	//GetGameMode()->InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

/**
*	@brief
**/
void FuncRotating::RotatorHurtTouch( IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

/**
*	@brief
**/
void FuncRotating::RotatorUse( IClientGameEntity* other, IClientGameEntity* activator ) {
	// Figure out if we are accelerating,, or if we are already moving at full speed.
	if ( moveInfo.state == FuncRotateState::Accelerating || moveInfo.state == FuncRotateState::FullSpeed ) {
		if ( GetDeceleration() == 0 ) {
			// Set us at a constant speed.
			SetAngularVelocity( vec3_zero() );
			moveInfo.speed = 0;
			SetTouchCallback( nullptr );
			SetThinkCallback( nullptr );
			moveInfo.state = FuncRotateState::Stopped;
		} else {
			moveInfo.state = FuncRotateState::Decelarting;
			// Prepare for deceleration.
			SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
			SetNextThinkTime( level.time + FRAMERATE_MS );
		}
	// Otherwise, prepare to start accelerating. If no acceleration force has been set it'll immediately
	// move into a constant speed.
	} else {
		//SetSound(...);

		// See if our keyspawn acceleration was set at all, 
		if ( GetAcceleration() == 0 ) {
			// Set us at a constant speed.
			SetAngularVelocity( vec3_scale( moveInfo.dir, speed * FRAMETIME_S.count()  ) );
			moveInfo.state = FuncRotateState::FullSpeed;
		} else {
			// Prepare for acceleration.
			SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
			SetNextThinkTime( level.time + FRAMERATE_MS );
			moveInfo.state = FuncRotateState::Accelerating;
		}
	}

	SetEventID( moveInfo.state );

	// Take care of hurt touching if set.
	if ( GetSpawnFlags() & SF_HurtTouch ) {
		SetTouchCallback( &FuncRotating::RotatorHurtTouch );
	}
}

/**
*	@brief Accelerates the rotator to full speed.
**/
void FuncRotating::Callback_AccelerateThink() {
	const float targetSpeed = GetSpeed();
	// It has reached full speed, make sure to clamp our speed properly.
	if( moveInfo.speed >= targetSpeed ) {
		// Maintain angular velocity within bounds.
		if ( moveInfo.speed != targetSpeed ) {
			SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
			moveInfo.speed = targetSpeed;
		}
		CLG_Print( PrintType::Developer, fmt::format( "CLG: Callback_AccelerateThink(speed({}), state(FuncRotateState::FullSpeed))\n", moveInfo.speed ) );
		SetThinkCallback( &FuncRotating::CLGBasePacketEntityThinkNull );
		moveInfo.state = FuncRotateState::FullSpeed;
		return;
	}
	
	// If we got here, we need to continue accelerating to full speed.
	moveInfo.speed += GetAcceleration() * FRAMETIME_S.count();
	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
	CLG_Print( PrintType::Developer, fmt::format( "CLG: Callback_AccelerateThink(speed({}))\n", moveInfo.speed ) );
	// Set next think.
	SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
	SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
}

/**
*	@brief Decelerates the rotator to a halted stop.
**/
void FuncRotating::Callback_DecelerateThink() {
	// It has reached 0 speed or possibly lower, make sure to clamp our speed properly.
	if ( moveInfo.speed <= 0. ) {
		// Maintain angular velocity within bounds.
		if ( moveInfo.speed != 0. ) {
			SetAngularVelocity( vec3_zero() );
			moveInfo.speed = 0.;
		}
		CLG_Print( PrintType::Developer, fmt::format( "CLG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Stopped))\n", moveInfo.speed ) );
		SetThinkCallback( &FuncRotating::CLGBasePacketEntityThinkNull );
		moveInfo.state = FuncRotateState::Stopped;
		return;
	}

	// If we got here, we need to continue accelerating to full speed.
	moveInfo.speed -= GetDeceleration() * FRAMETIME_S.count();
	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
	CLG_Print( PrintType::Developer, fmt::format( "SVG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Decelerate))\n", moveInfo.speed ) );

	// Set next think.
	SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
	SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
}

/**
*	@brief
**/
void SG_Physics_Pusher( SGEntityHandle &handle );
void FuncRotating::OnEventID( uint32_t eventID ) {
	if ( eventID == FuncRotateState::Accelerating ) {
		// Change state.
		moveInfo.state = FuncRotateState::Accelerating;

		// Prepare acceleration think method.
		SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
		SetNextThinkTime( level.time + FRAMERATE_MS );
	} else if ( eventID == FuncRotateState::Decelarting ) {
		moveInfo.state = FuncRotateState::Decelarting;
		// Prepare deceleration think method.
		SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
		SetNextThinkTime( level.time + FRAMERATE_MS );
	} else {
		// We got no business here.
		//return;
	}



	// TODO: REMOVE THIS!!
	// Make sure our move dir is adjusted.
	moveInfo.dir = { 0.f, 1.f, 0.f };
	SetMoveType( MoveType::Push );
	//



	// Perform physics startup call to always be a frame ahead.
	SGEntityHandle ehThis = this;
	level.time += FRAMERATE_MS;
	SG_Physics_Pusher( ehThis );
	level.time -= FRAMERATE_MS;
	EnableExtrapolation();
}

/**
*	@brief
**/
void FuncRotating::RotatorThink() {

	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &FuncRotating::RotatorThink );
}

/**
*	@brief
**/
void FuncRotating::SpawnKey(const std::string& key, const std::string& value) { 
	if (key == "speed") {
        // Parsed int.
        float parsedFloat = 0.f;

        // Parse.
        ParseKeyValue(key, value, parsedFloat);

		// Assign.
		speed = parsedFloat;
	} else {
		Base::SpawnKey(key, value); 
	}
}

///**
//*	@brief Accelerates the rotator to full speed.
//**/
//void FuncRotating::Callback_AccelerateThink() {
//	const float targetSpeed = GetSpeed();
//	// It has reached full speed, make sure to clamp our speed properly.
//	if( moveInfo.speed >= targetSpeed ) {
//		// Maintain angular velocity within bounds.
//		if ( moveInfo.speed != targetSpeed ) {
//			SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
//			moveInfo.speed = targetSpeed;
//		}
//		CLG_Print( PrintType::Developer, fmt::format( "CLG: #0 Callback_AccelerateThink(speed({})), state(FuncRotateState::FullSpeed)\n", moveInfo.speed ) );
//
//		SetThinkCallback( &FuncRotating::CLGBasePacketEntityThinkNull );
//		moveInfo.state = FuncRotateState::FullSpeed;
//		return;
//	}
//	moveInfo.state = FuncRotateState::Accelerating;
//	// If we got here, we need to continue accelerating to full speed.
//	moveInfo.speed += GetAcceleration() * FRAMETIME_S.count();
//	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
//	CLG_Print( PrintType::Developer, fmt::format( "CLG: #2 Callback_AccelerateThink( ent(#{}), speed({}), avelocity({},{},{}) )\n", GetNumber(), moveInfo.speed, GetAngularVelocity().x, GetAngularVelocity().y, GetAngularVelocity().z ) );
//
//	// Set next think.
//	SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
//	SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
//}
//
///**
//*	@brief
//**/
//void FuncRotating::Callback_DecelerateThink() {
//	// It has reached 0 speed or possibly lower, make sure to clamp our speed properly.
//	if ( moveInfo.speed <= 0. ) {
//		// Maintain angular velocity within bounds.
//		if ( moveInfo.speed != 0. ) {
//			SetAngularVelocity( vec3_zero() );
//			moveInfo.speed = 0.;
//		}
//		CLG_Print( PrintType::Developer, fmt::format( "CLG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Stopped))\n", moveInfo.speed ) );
//
//		SetThinkCallback( &FuncRotating::CLGBasePacketEntityThinkNull );
//		DisableExtrapolation();
//		moveInfo.state = FuncRotateState::Stopped;
//		return;
//	}
//	// If we got here, we need to continue accelerating to full speed.
//	moveInfo.state = FuncRotateState::Decelarting;
//	moveInfo.speed -= GetDeceleration() * FRAMETIME_S.count();
//	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
//	CLG_Print( PrintType::Developer, fmt::format( "CLG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Decelerate))\n", moveInfo.speed ) );
//
//	// Set next think.
//	SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
//	SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
//}
///**
//*	@brief	Implements triggering door state, effectively allowing a slight client-side prediction.
//**/
//void FuncRotating::OnEventID( uint32_t eventID ) {
//	if ( eventID == FuncRotateState::Accelerating ) {
//		moveInfo.state = FuncRotateState::Accelerating;
//	} else if ( eventID == FuncRotateState::Decelarting ) {
//		moveInfo.state = FuncRotateState::Decelarting;
//	} else {
//	}
//		moveInfo.dir = { 0.f, 1.f, 0.f };
//
//	if ( moveInfo.state == FuncRotateState::Accelerating || moveInfo.state == FuncRotateState::FullSpeed ) {
//		if ( GetDeceleration() == 0 ) {
//			// Set us at a constant speed.
//			SetAngularVelocity( vec3_zero() );
//			moveInfo.speed = 0;
//			SetTouchCallback( nullptr );
//			SetThinkCallback( nullptr );
//			moveInfo.state = FuncRotateState::Stopped;
//
//			// Return since we do not wish to perform a physics startup call.
//			return;
//		} else {
//			moveInfo.state = FuncRotateState::Decelarting;
//			// Prepare for deceleration.
//			SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
//			SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
//		}
//	// Otherwise, prepare to start accelerating. If no acceleration force has been set it'll immediately
//	// move into a constant speed.
//	} else {
//		// See if our keyspawn acceleration was set at all, 
//		if ( GetAcceleration() == 0 ) {
//			// Set us at a constant speed.
//			SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed * FRAMETIME_S.count()  ) );
//			moveInfo.state = FuncRotateState::FullSpeed;
//		} else {
//			// Prepare for acceleration.
//			SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
//			SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
//			moveInfo.state = FuncRotateState::Accelerating;
//		}
//	}
//
//	// Perform physics startup call to always be a frame ahead.
//	SGEntityHandle ehThis = this;
//	level.time += FRAMERATE_MS;
//	SG_Physics_Pusher( ehThis );
//	level.time -= FRAMERATE_MS;
//	EnableExtrapolation();
//
//	//if ( eventID == FuncRotateState::Accelerating ) {
//	//CLG_Print( PrintType::Developer, fmt::format( "CLG: eventID=FuncRotateState::Accelerating )\n", moveInfo.speed ) );	
//
//	//	moveInfo.state = FuncRotateState::Accelerating;
//	//	// HACK: Needs to get set in spawn but it unsets somewhere?
//	//	moveInfo.dir = { 0.f, 1.f, 0.f };
//	//	// UNHACK.
//
//	//	// Set next think.
//	//	SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
//	//	SetNextThinkTime( level.time + FRAMERATE_MS );
//	//	moveInfo.state = FuncRotateState::Accelerating;
//
//	//	SGEntityHandle ehThis = this;
//	//	level.time += FRAMERATE_MS;
//	//	SG_Physics_Pusher( ehThis );
//	//	level.time -= FRAMERATE_MS;
//	//	EnableExtrapolation();
//	//}
//	//if ( eventID == FuncRotateState::Decelarting ) {
//	//CLG_Print( PrintType::Developer, fmt::format( "CLG: eventID=FuncRotateState::Decelarting )\n", moveInfo.speed ) );	
//
//
//	//	// Set next think.
//	//	moveInfo.state = FuncRotateState::Decelarting;
//	//	// Prepare for deceleration.
//	//	SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
//	//	SetNextThinkTime( level.time + FRAMERATE_MS );
//
//	//	SGEntityHandle ehThis = this;
//	//	level.time += FRAMERATE_MS;
//	//	SG_Physics_Pusher( ehThis );
//	//	level.time -= FRAMERATE_MS;
//	//	EnableExtrapolation();
//	//}
//
//}
//
///**
//*	@brief
//**/
//void FuncRotating::RotatorThink() {
//
//	SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
//	SetThinkCallback( &FuncRotating::RotatorThink );
//}
//
///**
//*	@brief
//**/
//void FuncRotating::SpawnKey(const std::string& key, const std::string& value) { 
//	Base::SpawnKey(key, value);
//}