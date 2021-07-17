/*
// LICENSE HERE.

// FuncButton.cpp
*/

#include "../../g_local.h"
#include "../../effects.h"
#include "../../entities.h"
#include "../../utils.h"
#include "../../physics/stepmove.h"
#include "../../brushfuncs.h"

#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"
#include "../base/SVGBaseMover.h"

#include "FuncButton.h"

//===============
// FuncButton::ctor
//===============
FuncButton::FuncButton( Entity* svEntity )
	: SVGBaseMover( svEntity ) {

}

//===============
// FuncButton::Precache
//===============
void FuncButton::Precache() {
	Base::Precache();
}

//===============
// FuncButton::Spawn
//===============
void FuncButton::Spawn() {
	Base::Spawn();

	// Mappers set angles to determine the movement direction of the button,
	// so we gotta set the movement direction, then zero the entity's angles
	SetMoveDirection( GetAngles() );
	SetAngles( vec3_zero() );

	SetModel( GetModel() );
	SetMoveType( MoveType::Stop );
	SetSolid( Solid::BSP );

	// If the mapper didn't specify a sound
	if ( GetSound() != 1 ) {
		moveInfo.startSoundIndex = gi.SoundIndex( "switches/butn2.wav" );
	} // If the mapper didn't specify speed, set it to 40 u/s
	if ( !GetSpeed() ) {
		SetSpeed( 40.0f );
	} // If the mapper didn't specify acceleration & deceleration, set the defaults
	if ( !GetAcceleration() ) {
		SetAcceleration( GetSpeed() );
	}
	if ( !GetDeceleration() ) {
		SetDeceleration( GetSpeed() );
	} // If the mapper didn't specify 'wait until return', then default to 3 seconds
	if ( !GetWaitTime() ) {
		SetWaitTime( 3.0f );
	} // Lip: how much to subtract from the door's travel distance
	if ( !GetLip() ) {
		SetLip( 8.0f );
	}
	// Set up the trajectory
	SetStartPosition( GetOrigin() );
	SetEndPosition( CalculateEndPosition() );

	SetEffects( EntityEffectType::AnimCycleFrames01hz2 );

	if ( GetHealth() ) {
		SetMaxHealth( GetHealth() );
		SetDieCallback( &FuncButton::ButtonDie );
		SetTakeDamage( TakeDamage::Yes );
	} else if ( nullptr == serverEntity->targetName ) {
		SetTouchCallback( &FuncButton::ButtonTouch );
	}

	SetUseCallback( &FuncButton::ButtonUse );

	// Set up moveInfo stuff
	// Button starts off
	moveInfo.state			= MoverState::Bottom;
	moveInfo.speed			= GetSpeed();
	moveInfo.acceleration	= GetAcceleration();
	moveInfo.deceleration	= GetDeceleration();
	moveInfo.wait			= GetWaitTime();
	moveInfo.startOrigin	= GetStartPosition();
	moveInfo.startAngles	= GetAngles();
	moveInfo.endOrigin		= GetEndPosition();
	moveInfo.endAngles		= GetAngles();

	LinkEntity();
}

//===============
// FuncButton::SpawnKey
//===============
void FuncButton::SpawnKey( const std::string& key, const std::string& value ) {
	// I think serverEntity variables should just be set in SVGBaseEntity::SpawnKey
	// It doesn't make sense to set them only here, if these variables are available to every entity
	if ( key == "speed" ) {
		ParseFloatKeyValue( key, value, serverEntity->speed );
	} else if ( key == "lip" ) {
		ParseFloatKeyValue( key, value, lip );
	} else {
		return Base::SpawnKey( key, value );
	}
}

//===============
// FuncButton::OnButtonDone
//===============
void FuncButton::OnButtonDone( Entity* self ) {
	FuncButton* button = static_cast<FuncButton*>(self->classEntity);
	button->ButtonDone();
}

//===============
// FuncButton::ButtonDone
//===============
void FuncButton::ButtonDone() {
	moveInfo.state = MoverState::Bottom;
	serverEntity->state.effects &= ~(EntityEffectType::AnimCycleFrames23hz2);
	serverEntity->state.effects |= EntityEffectType::AnimCycleFrames01hz2;
}

//===============
// FuncButton::ButtonReturn
//===============
void FuncButton::ButtonReturn() {
	moveInfo.state = MoverState::Down;
	BrushMoveCalc( moveInfo.startOrigin, OnButtonDone );
	SetFrame( 0 );

	if ( GetHealth() ) {
		SetTakeDamage( TakeDamage::Yes );
	}
}

//===============
// FuncButton::OnButtonWait
//===============
void FuncButton::OnButtonWait( Entity* self ) {
	FuncButton* button = static_cast<FuncButton*>(self->classEntity);
	button->ButtonWait();
}

//===============
// FuncButton::ButtonWait
//===============
void FuncButton::ButtonWait() {
	moveInfo.state = MoverState::Top;
	serverEntity->state.effects &= ~(EntityEffectType::AnimCycleFrames01hz2);
	serverEntity->state.effects |= EntityEffectType::AnimCycleFrames23hz2;
	SetFrame( 1 );

	UseTargets( GetActivator() );

	if ( moveInfo.wait >= 0.0f ) {
		SetNextThinkTime( level.time + moveInfo.wait );
		SetThinkCallback( &FuncButton::ButtonReturn );
	}
}

//===============
// FuncButton::ButtonFire
//===============
void FuncButton::ButtonFire() {
	if ( moveInfo.state == MoverState::Up || moveInfo.state == MoverState::Top ) {
		return;
	}

	moveInfo.state = MoverState::Up;
	if ( moveInfo.startSoundIndex && !(flags & EntityFlags::TeamSlave) ) {
		gi.Sound( serverEntity, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0 );
	}
	
	BrushMoveCalc( moveInfo.endOrigin, OnButtonWait );
}

//===============
// FuncButton::ButtonUse
//===============
void FuncButton::ButtonUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	this->activator = activator;
	ButtonFire();
}

//===============
// FuncButton::ButtonTouch
//===============
void FuncButton::ButtonTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
	if ( !other->GetClient() || other->GetHealth() <= 0 ) {
		return;
	}

	activator = other;
	ButtonFire();
}

//===============
// FuncButton::ButtonDie
//===============
void FuncButton::ButtonDie( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point ) {
	activator = attacker;
	SetHealth( GetMaxHealth() );
	SetTakeDamage( TakeDamage::No );
}
