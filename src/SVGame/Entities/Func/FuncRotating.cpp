/*
// LICENSE HERE.

// FuncRotating.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"
#include "../../BrushFunctions.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

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
	Base::Spawn();

	SetSolid( Solid::BSP );
	
	SetMoveType( (GetSpawnFlags() & SF_StopOnBlock) ? MoveType::Stop : MoveType::Push );

	// Set the axis of rotation, support custom axes as well
	if ( GetSpawnFlags() & SF_YAxis ) {
		moveDirection = vec3_t( 0.0f, 0.0f, 1.0f );
	} else if ( GetSpawnFlags() & SF_XAxis ) {
		moveDirection = vec3_t( 1.0f, 0.0f, 0.0f );
	} else if ( !vec3_length( moveDirection ) ) { // moveDirection can be automatically set with 
		moveDirection = vec3_t( 0.0f, 0.0f, 1.0f ); // keyvalues, so make sure to account for that
	}

	if ( GetSpawnFlags() & SF_Reverse ) {
		moveDirection = vec3_negate( moveDirection );
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
		serverEntity->state.effects |= EntityEffectType::AnimCycleAll2hz;
	} else if ( GetSpawnFlags() & SF_AnimatedFast ) {
		serverEntity->state.effects |= EntityEffectType::AnimCycleAll30hz;
	}

	SetModel( GetModel() );
	LinkEntity();
}

//===============
// FuncRotating::RotatorBlocked
//===============
void FuncRotating::RotatorBlocked( SVGBaseEntity* other ) {
	SVG_InflictDamage( other, this, this, vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}

//===============
// FuncRotating::RotatorHurtTouch
//===============
void FuncRotating::RotatorHurtTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
	if ( vec3_length( GetAngularVelocity() ) ) {
		RotatorBlocked( other );
	}
}

//===============
// FuncRotating::RotatorUse
//===============
void FuncRotating::RotatorUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	if ( vec3_equal( GetAngularVelocity(), vec3_zero() ) ) {
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
