/***
*
*	License here.
*
*	@file
* 
*   Client Side FuncDoorRotating -> Intented to be predicted someday.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
//! BaseMover.
#include "Game/Client/Entities/Func/FuncDoor.h"
#include "Game/Client/Entities/Func/FuncDoorRotating.h"
//! Game World.
#include "Game/Client/World/ClientGameWorld.h"

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
	// Be sure to set angles first before calling Base::Spawn because
	// a func_door already does SetMoveAngles for us.
	SetAngles( vec3_zero() );

	// Set acceleration to speed in case it isn't set.
	if (!GetAcceleration()) {
		SetAcceleration(GetSpeed());
	}
	// Set deceleration to speed in case it isn't set.
	if (!GetDeceleration()) {
		SetDeceleration(GetSpeed());
	}

	// FuncDoor spawn.
	Base::Spawn();

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

	// Set a default distance.
	if ( !distance ) {
		//gi.DPrintf( "entity: %i:%s with no distance set\n", GetNumber(), GetClassname() );
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

	// Setup our moveInfo.
	moveInfo.state = MoverState::Bottom;
	moveInfo.speed = GetSpeed();
	moveInfo.acceleration = GetAcceleration();
	moveInfo.deceleration = GetDeceleration();
	moveInfo.wait = GetWaitTime();
	moveInfo.startOrigin = GetOrigin();
	moveInfo.endOrigin = GetOrigin();
	moveInfo.startAngles = GetStartPosition();
	moveInfo.endAngles = GetEndPosition();
	moveInfo.dir = moveDirection;

	// Link door.
	LinkEntity();
}

//===============
// FuncDoorRotating::SpawnKey
//===============
void FuncDoorRotating::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "distance" ) {
		// Distance value.
		float parsedFloat = 0;

		// Parse.
		ParseKeyValue( key, value, parsedFloat);

		distance = parsedFloat;
	} else {
		Base::SpawnKey( key, value );
	}
}

/**
*	@brief	Implements triggering door state, effectively allowing a slight client-side prediction.
**/
void FuncDoorRotating::OnEventID(uint32_t eventID) {
	// TODO: Acquire the activator entity?
	uint16_t activatorEntityNumber = 0;
	
	// Get Game World.
	ClientGameWorld *gameWorld = GetGameWorld();
	auto *playerEntity = gameWorld->GetClientGameEntity();
//		auto *playerEntity = GetGameWorld()->GetGameEntityByIndex(1);
	switch( eventID ) {
	case 1: //: DOOR_OPEN:
		CLG_Print( PrintType::Developer, fmt::format( "{}: Received (eventID: #{}, 'DOOR_OPEN')!\n", __func__, eventID ) );

		// Start now so we can catch up to last frame.
		//DoGoUp();
		//DoGoUp();
		DoorGoUp( playerEntity );


		//DoorUse(playerEntity, nullptr);
		break;
	case 2: //: DOOR_CLOSE.
		CLG_Print( PrintType::Developer, fmt::format( "{}: Received (eventID: #{}, 'DOOR_CLOSE')!\n", __func__, eventID ) );
		// Start now, so we can catch up to last frame.
		//DoGoDown();
		//DoGoDown();

		DoorGoDown( );
		break;
	default:

		break;
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
