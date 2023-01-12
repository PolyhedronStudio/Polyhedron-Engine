/*
// LICENSE HERE.

// FuncRotating.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

#include "../../Gamemodes/IGamemode.h"
#include "FuncRotating.h"

/**
*	@brief
**/
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
		SetBlockedCallback( &FuncRotating::RotatorBlocked );
	} else {
		SetDamage( 2.0f );
		SetBlockedCallback( &FuncRotating::RotatorBlocked );
	}

	SetUseCallback( &FuncRotating::RotatorUse );

	if ( GetSpawnFlags() & SF_Animated ) {
		SetEffects( GetEffects() | EntityEffectType::AnimCycleAll2hz );
	} else if ( GetSpawnFlags() & SF_AnimatedFast ) {
		SetEffects( GetEffects() | EntityEffectType::AnimCycleAll30hz );
	}

	SVG_DPrint( fmt::format( "Ur mom's funcrotating has ID(#{})\n", GetNumber() ) );

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
void FuncRotating::RotatorBlocked( IServerGameEntity* other ) {
	if (!other) {
		return;
	}

    if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !(other->GetClient()) ) {
        // Give it a chance to go away on its own terms (like gibs)
        GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 10000, 1, 0, MeansOfDeath::Crush );

        // If it's still there, nuke it
        if ( other->GetHealth() > 0 || other->GetSolid() != Solid::Not ) {
            SVG_BecomeExplosion1( other );
        }
    }

    GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );

	//	GetGameMode()->InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

/**
*	@brief
**/
void FuncRotating::RotatorHurtTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

/**
*	@brief
**/
void FuncRotating::RotatorUse( IServerGameEntity* other, IServerGameEntity* activator ) {
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
		//SVG_DPrint( fmt::format( "SVG: Callback_AccelerateThink(speed({}), state(FuncRotateState::FullSpeed))\n", moveInfo.speed ) );
		SetThinkCallback( &FuncRotating::SVGBaseEntityThinkNull );
		moveInfo.state = FuncRotateState::FullSpeed;
		return;
	}
	
	// If we got here, we need to continue accelerating to full speed.
	moveInfo.speed += GetAcceleration() * FRAMETIME_S.count();
	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
	//SVG_DPrint( fmt::format( "SVG: Callback_AccelerateThink(speed({}))\n", moveInfo.speed ) );
	// Set next think.
	SetThinkCallback( &FuncRotating::Callback_AccelerateThink );
	SetNextThinkTime( level.time + FRAMERATE_MS );
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
		//SVG_DPrint( fmt::format( "SVG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Stopped))\n", moveInfo.speed ) );
		SetThinkCallback( &FuncRotating::SVGBaseEntityThinkNull );
		moveInfo.state = FuncRotateState::Stopped;
		return;
	}

	// If we got here, we need to continue accelerating to full speed.
	moveInfo.speed -= GetDeceleration() * FRAMETIME_S.count();
	SetAngularVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
	//SVG_DPrint( fmt::format( "SVG: Callback_DecelerateThink(speed({}), state(FuncRotateState::Decelerate))\n", moveInfo.speed ) );

	// Set next think.
	SetThinkCallback( &FuncRotating::Callback_DecelerateThink );
	SetNextThinkTime( level.time + FRAMERATE_MS );
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