/*
// LICENSE HERE.

// FuncButton.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

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
	} else if ( GetTargetName().empty() ) {
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
	    float parsedFloat = 0.f;
		ParseFloatKeyValue( key, value, parsedFloat);
	    SetSpeed(parsedFloat);
	} else if ( key == "lip" ) {
		float parsedFloat = 0.f;
		ParseFloatKeyValue( key, value, parsedFloat );
		SetLip(parsedFloat);
	} else {
		return Base::SpawnKey( key, value );
	}
}

//===============
// FuncButton::OnButtonDone
//===============
void FuncButton::OnButtonDone(SVGBaseEntity* self) {
	// Chances are nihil of this happening, but let's be sure to check so we can assist ourselves and other devs.
    if (!self->IsSubclassOf<FuncButton>()) { 
		gi.DPrintf("Warning: In function %s, base entity #%i is not of type %s\n", __func__, self->GetNumber());
		return;
	}

	// Cast and execute.
	FuncButton* button = static_cast<FuncButton*>(self);
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
	SetAnimationFrame( 0 );

	if ( GetHealth() ) {
		SetTakeDamage( TakeDamage::Yes );
	}
}

//===============
// FuncButton::OnButtonWait
//===============
void FuncButton::OnButtonWait(SVGBaseEntity* self) {
	// Chances are nihil of this happening, but let's be sure to check so we can assist ourselves and other devs.
    if (!self->IsSubclassOf<FuncButton>()) { 
		gi.DPrintf("Warning: In function %s, base entity #%i is not of type %s\n", __func__, self->GetNumber());
		return;
	}

	// Cast and execute.
	FuncButton* button = static_cast<FuncButton*>(self);
	button->ButtonWait();
}

//===============
// FuncButton::ButtonWait
//===============
void FuncButton::ButtonWait() {
	moveInfo.state = MoverState::Top;
	serverEntity->state.effects &= ~(EntityEffectType::AnimCycleFrames01hz2);
	serverEntity->state.effects |= EntityEffectType::AnimCycleFrames23hz2;
	SetAnimationFrame( 1 );

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
	SetActivator(activator);
	ButtonFire();
}

//===============
// FuncButton::ButtonTouch
//===============
void FuncButton::ButtonTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
	if ( !other->GetClient() || other->GetHealth() <= 0 ) {
		return;
	}

	SetActivator(other);
	ButtonFire();
}

//===============
// FuncButton::ButtonDie
//===============
void FuncButton::ButtonDie( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point ) {
	SetActivator(attacker);
	SetHealth( GetMaxHealth() );
	SetTakeDamage( TakeDamage::No );
}
