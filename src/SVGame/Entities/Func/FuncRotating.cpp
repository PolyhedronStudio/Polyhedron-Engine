/*
// LICENSE HERE.

// FuncRotating.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

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

	// Be sure to set angles first before calling Base::Spawn because
    // a func_door already does SetMoveAngles for us.
    SetAngles(vec3_zero());

    Base::Spawn();

	// TODO: Implement properly. Someone, feel free to submit a PR :)
	// Set the axis of rotation, support custom axes as well
    moveDirection = vec3_zero();
	if ( GetSpawnFlags() & SF_YAxis ) {
	    //SetMoveDirection(vec3_t { 0.0f, 0.0f, 1.0f }, false);
		moveDirection.z = 1.f;
	} else if ( GetSpawnFlags() & SF_XAxis ) {
	    moveDirection.x = 1.f;//SetMoveDirection(vec3_t { 1.0f, 0.0f, 0.0f }, false);
	} else {// if ( !vec3_length( moveDirection ) ) { // moveDirection can be automatically set with 
		moveDirection.y = 1.f;//SetMoveDirection(vec3_t { 0.0f, 1.0f, 1.0f }, false);  // keyvalues, so make sure to account for that
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
		Use( nullptr, nullptr );
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
void FuncRotating::RotatorBlocked( SVGBaseEntity* other ) {
	GetGamemode()->InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

//===============
// FuncRotating::RotatorHurtTouch
//===============
void FuncRotating::RotatorHurtTouch( SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

//===============
// FuncRotating::RotatorUse
//===============
void FuncRotating::RotatorUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
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