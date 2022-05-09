/***
*
*	License here.
*
*	@file
*
*	Client-Side FuncPlat Entity Support.
*
***/
#include "../../ClientGameLocals.h"
//#include "../../Effects.h"
//#include "../../Entities.h"
//#include "../../Utilities.h"
//#include "../../Physics/SlideMove.h"

#include "../Base/CLGBasePacketEntity.h"
#include "../Base/CLGBaseTrigger.h"
#include "../Base/CLGBaseMover.h"

//#include "../../GameModes/IGameMode.h"
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
	//SetModel(GetModel());
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
void FuncRotating::RotatorBlocked( GameEntity* other ) {
	//GetGameMode()->InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

//===============
// FuncRotating::RotatorHurtTouch
//===============
void FuncRotating::RotatorHurtTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

//===============
// FuncRotating::RotatorUse
//===============
void FuncRotating::RotatorUse( GameEntity* other, GameEntity* activator ) {
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