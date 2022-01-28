/*
// LICENSE HERE.

// FuncDoorRotating.cpp
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

#include "../trigger/TriggerAutoDoor.h"

#include "FuncDoor.h"
#include "FuncDoorRotating.h"

//===============
// FuncDoorRotating::ctor
//===============
FuncDoorRotating::FuncDoorRotating( Entity* entity )
	: Base( entity ) {

}

//===============
// FuncDoorRotating::Spawn
//===============
void FuncDoorRotating::Spawn() {
	Base::Spawn();

	//SetAngles( vec3_zero() );

	// Set the axis of rotation
	moveDirection = vec3_zero();
	if ( GetSpawnFlags() & SF_XAxis ) {
		moveDirection.z = 1.0f;
	} else if ( GetSpawnFlags() & SF_YAxis ) {
		moveDirection.x = 1.0f;
	} else { // ZAxis
		moveDirection.y = 1.0f;
	}

	// Check for reverse rotation
	if ( GetSpawnFlags() & SF_Reverse ) {
		moveDirection = vec3_negate( moveDirection );
	}

	if ( !distance ) {
		gi.DPrintf( "entity: %i:%s with no distance set\n", GetNumber(), GetClassName() );
		distance = 90.0f;
	}

	SetStartPosition( GetAngles() );
	SetEndPosition( vec3_fmaf( GetAngles(), distance, moveDirection ) );
	moveInfo.distance = distance;

	// If it starts open, switch the positions
	if ( GetSpawnFlags() & SF_StartOpen ) {
		SwapPositions();
		moveDirection = vec3_negate( moveDirection );
	}

	moveInfo.startOrigin = GetOrigin();
	moveInfo.endOrigin = GetOrigin();
	moveInfo.startAngles = GetStartPosition();
	moveInfo.endAngles = GetEndPosition();
	moveInfo.dir = moveDirection;

	LinkEntity();
}

//===============
// FuncDoorRotating::SpawnKey
//===============
void FuncDoorRotating::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "distance" ) {
		ParseFloatKeyValue( key, value, distance );
	} else {
		Base::SpawnKey( key, value );
	}
}

//===============
// FuncDoorRotating::DoGoUp
//===============
void FuncDoorRotating::DoGoUp() {
	BrushAngleMoveCalc( OnDoorHitTop );
}

//===============
// FuncDoorRotating::DoGoDown
//===============
void FuncDoorRotating::DoGoDown() {
	BrushAngleMoveCalc( OnDoorHitBottom );
}
