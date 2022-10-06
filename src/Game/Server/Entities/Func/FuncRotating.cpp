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

//===============
// FuncRotating::ctor
//===============
FuncRotating::FuncRotating( Entity* entity )
	: Base( entity ) {
}

//===============
// FuncRotating::Spawn
//===============
void FuncRotating::Spawn() {

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

	if ( !speed ) {
		speed = 100.0f;
	}
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

	LinkEntity();
}

void FuncRotating::PostSpawn() {
	if ( GetSpawnFlags() & SF_StartOn ) {
		DispatchUseCallback( nullptr, nullptr );
	}
}
//===============
// FuncRotating::RotatorBlocked
//===============
void FuncRotating::RotatorBlocked( IServerGameEntity* other ) {
	GetGameMode()->InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

//===============
// FuncRotating::RotatorHurtTouch
//===============
void FuncRotating::RotatorHurtTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

//===============
// FuncRotating::RotatorUse
//===============
void FuncRotating::RotatorUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	if ( !( angularVelocity[0] == 0 && angularVelocity[1] == 0 &&angularVelocity[2] == 0 ) ) {
		SetSound( 0 );
		SetAngularVelocity( vec3_zero() );
		SetTouchCallback( nullptr );
	} else {
		SetAngularVelocity( vec3_scale( moveDirection, speed ) );
		SetNextThinkTime( level.time + FRAMERATE_MS );
		SetThinkCallback( &FuncRotating::RotatorThink );
		if ( GetSpawnFlags() & SF_HurtTouch ) {
			SetTouchCallback( &FuncRotating::RotatorHurtTouch );
		}
	}
}

void FuncRotating::RotatorThink() {

	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &FuncRotating::RotatorThink );
}

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