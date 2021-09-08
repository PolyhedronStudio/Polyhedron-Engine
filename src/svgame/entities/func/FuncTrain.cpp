/*
// LICENSE HERE.

// FuncTrain.cpp
*/

#include "../../g_local.h"
#include "../../entities.h"

#include "../base/SVGBaseTrigger.h"
#include "../base/SVGBaseMover.h"

#include "../path/PathCorner.h"
#include "FuncTrain.h"

//===============
// FuncTrain::ctor
//===============
FuncTrain::FuncTrain( Entity* entity )
 : Base( entity ) {

}

//===============
// FuncTrain::Spawn
//===============
void FuncTrain::Spawn() {
	SetMoveType( MoveType::Push );
	SetSolid( Solid::BSP );
	SetAngles( vec3_zero() );

	if ( spawnFlags & SF_StopWhenBlocked ) {
		damage = 0;
	} else if ( !damage ) {
		damage = 100;
	}

	SetModel( GetModel() );

	if ( !GetSpeed() ) {
		SetSpeed( 100.0f );
	}

	moveInfo.acceleration = GetSpeed();
	moveInfo.deceleration = GetSpeed();

	LinkEntity();

	if ( targetStr.empty() ) {
		gi.DPrintf( "func_train without a target at %s\n", vec3_to_str( GetAbsoluteCenter() ).c_str() );
	}
}

//===============
// FuncTrain::PostSpawn
//===============
void FuncTrain::PostSpawn() {
	if ( targetStr.empty() ) {
		return;
	}

	SVGBaseEntity* ent = SVG_FindEntityByKeyValue( "targetname", targetStr );
	if ( nullptr == ent ) {
		gi.DPrintf( "FuncTrain: target '%s' not found, maybe you made a typo?\n", targetStr.c_str() );
		return;
	}

	if ( !ent->IsSubclassOf<PathCorner>() ) {
		gi.DPrintf( "FuncTrain: target '%s' is not a path entity\n", targetStr.c_str() );
		return;
	}

	serverEntity->state.origin = ent->GetOrigin() - GetMins();
	LinkEntity();

	// This train has no name, trigger it immediately
	if ( targetNameStr.empty() ) {
		spawnFlags |= SF_StartOn;
	}

	if ( spawnFlags & SF_StartOn ) {
		SetNextThinkTime( level.time + FRAMETIME );
		SetThinkCallback( nullptr );
		activator = this;
	}
}

//===============
// FuncTrain::NextCornerThink
//===============
void FuncTrain::NextCornerThink() {

}

//===============
// FuncTrain::WaitAtCorner
//===============
void FuncTrain::WaitAtCorner() {

}

//===============
// FuncTrain::TrainBlocked
//===============
void FuncTrain::TrainBlocked( SVGBaseEntity* other ) {

}
