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
	SetMoveDirection(vec3_normalize(GetAngles()));

	// Reset Angles.
	SetAngles(vec3_zero());

	// TODO: Implement properly. Someone, feel free to submit a PR :)
	// Set the axis of rotation, support custom axes as well
	if ( GetSpawnFlags() & SF_XAxis ) {
	    SetMoveDirection(vec3_t { 0.0f, 0.0f, 1.0f }, true);
	} else if ( GetSpawnFlags() & SF_YAxis ) {
		SetMoveDirection(vec3_t { 1.0f, 0.0f, 0.0f }, true);
	} else if ( !vec3_length( moveDirection ) ) { // If moveDirection had no specific angles or X/Y flag set, default to Z.
		SetMoveDirection(vec3_t { 0.0f, 1.0f, 0.0f }, true);
	}

	SetSolid(Solid::BSP);
	SetModel(GetModel());
	SetMoveType((GetSpawnFlags() & SF_StopOnBlock) ? MoveType::Stop : MoveType::Push);

	if ( GetSpawnFlags() & SF_Reverse ) {
	    moveDirection = vec3_negate(moveDirection);
	}

	if ( !GetSpeed() ) {
		SetSpeed( 100.0f );
	}
	if ( !GetDamage() ) {
		SetDamage( 2.0f );
	}

	SetUseCallback( &FuncRotating::RotatorUse );
	SetBlockedCallback( &FuncRotating::RotatorBlocked );

	if ( GetSpawnFlags() & SF_StartOn ) {
		DispatchUseCallback( nullptr, nullptr );
	}

	if ( GetSpawnFlags() & SF_Animated ) {
		SetEffects(GetEffects() | EntityEffectType::AnimCycleAll2hz);
	} else if ( GetSpawnFlags() & SF_AnimatedFast ) {
		SetEffects(GetEffects() | EntityEffectType::AnimCycleAll30hz);
	}


	LinkEntity();
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
	if ( !vec3_equal( GetAngularVelocity(), vec3_zero() ) ) {
		SetSound( 0 );
		SetAngularVelocity( vec3_zero() );
		SetTouchCallback( nullptr );
	} else {
		SetSound( moveInfo.middleSoundIndex );
		SetAngularVelocity( vec3_scale( moveDirection, GetSpeed() ) );
		if ( GetSpawnFlags() & SF_HurtTouch ) {
			SetTouchCallback( &FuncRotating::RotatorHurtTouch );
		}
	}
}

void FuncRotating::SpawnKey(const std::string& key, const std::string& value) { 
	Base::SpawnKey(key, value); 
}