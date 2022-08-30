/***
*
*	License here.
*
*	@file
*
*	ClientGame Linear base mover entity.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Server Game Local headers.
#include "Game/Client/ClientGameLocals.h"


#include "CLGBaseTrigger.h"
#include "CLGBaseLinearMover.h"
#include "../../World/ClientGameWorld.h"


// Epsilon used for comparing floats.
static constexpr float BASEMOVER_EPSILON = 0.03125;

// Constructor/Deconstructor.
CLGBaseLinearMover::CLGBaseLinearMover( PODEntity *podEntity ) : Base( podEntity ) {

}

/**
*	@brief	Additional spawnkeys for movers.
**/
void CLGBaseLinearMover::SpawnKey( const std::string& key, const std::string& value ) {
	// Accel.
	if ( key == "accel" ) {
		// Parse float.
		float parsedFloat = 0.f;
		ParseKeyValue( key, value, parsedFloat );

		// Set acceleration.
		SetAcceleration( parsedFloat );
	}
	// Accel.
	else if ( key == "decel" ) {
		// Parse float.
		float parsedFloat = 0.f;
		ParseKeyValue( key, value, parsedFloat );

		// Set deceleration.
		SetDeceleration( parsedFloat );
	}
	// Lip.
	else if (key == "lip") {
		// Parse float.
		float parsedFloat = 0.f;
		ParseKeyValue( key, value, parsedFloat );

		// Set lip.
		SetLip( parsedFloat );
	}
	// Speed.
	else if ( key == "speed" ) {
		// Parse float.
		float parsedFloat = 0.f;
		ParseKeyValue( key, value, parsedFloat );

		// Set speed.
		SetSpeed( parsedFloat );
	}
	// Wait.
	else if ( key == "wait" ) {
		// Set wait.
		//SetWaitTime(parsedFloat);
		ParseKeyValue( key, value, waitTime );
	} else {
		Base::SpawnKey( key, value );
	}
}

//
//===============
// CLGBaseLinearMover::SetMoveDirection
//
//===============
//
void CLGBaseLinearMover::SetMoveDirection( const vec3_t& angles, const bool resetAngles ) {
	const vec3_t VEC_UP = { 0, -1.f, 0 };
	const vec3_t MOVEDIR_UP = { 0, 0, 1.f };
	const vec3_t VEC_DOWN = { 0, -2.f, 0 };
	const vec3_t MOVEDIR_DOWN = { 0, 0, -1.f };

	const vec3_t AnglesUp[] = { 
		{ 90.f, 0, 0 }, 
		{ -270.f, 0, 0 } 
	};
	
	const vec3_t AnglesDown[] = { 
		{ 270.f, 0, 0 }, 
		{ -90.f, 0, 0 } 
	};

	if ( vec3_equal( angles, VEC_UP ) || vec3_equal( angles, AnglesUp[0] ) || vec3_equal( angles, AnglesUp[1] ) ) {
		moveDirection = MOVEDIR_UP;
	} else if ( vec3_equal( angles, VEC_DOWN ) || vec3_equal( angles, AnglesDown[0] ) || vec3_equal( angles, AnglesDown[1] ) ) {
		moveDirection = MOVEDIR_DOWN;
	} else {
		AngleVectors( angles, &moveDirection, NULL, NULL );
		//SetAngles(angles);
	}

	// Admer: is this really intended? Some entities may control their angle
	// and align it directly with the movement direction.
	// I suggest we add a bool parameter to this method, 'resetAngles',
	// which will zero the entity's angles if true
	if ( resetAngles == true ) {
		SetAngles( vec3_zero() );
	}
}

//===============
// CLGBaseLinearMover::CalculateEndPosition
//===============
vec3_t CLGBaseLinearMover::CalculateEndPosition() {
	const vec3_t& size = GetSize();
	vec3_t absoluteDir{
		fabsf( moveDirection.x ),
		fabsf( moveDirection.y ),
		fabsf( moveDirection.z ) 
	};
	
	moveInfo.distance = ( absoluteDir.x * size.x ) + ( absoluteDir.y * size.y ) + ( absoluteDir.z * size.z ) - GetLip();

	return vec3_fmaf( GetStartPosition(), moveInfo.distance, moveDirection );
}

//===============
// CLGBaseLinearMover::SwapPositions
//===============
void CLGBaseLinearMover::SwapPositions() {
	vec3_t temp = GetEndPosition();			// temp = endpos
	SetEndPosition( GetStartPosition() );	// endpos = startpos
	SetStartPosition( temp );				// startpos = temp
}



/*
* GS_LinearMovement
*/
int32_t SG_LinearMovement( const LinearPushMoveInfo *pmi, const GameTime &time, vec3_t &dest ) {
	vec3_t dist = vec3_zero();
	GameTime moveTime = GameTime::zero();
	double moveFrac = 0.;

	moveTime = time - pmi->linearMovementTimeStamp;
	if( moveTime == GameTime::zero() || moveTime < 0ms ) {
		moveTime = 0ms;
	}

	if( pmi->linearMovementDuration > 0ms ) {
		if( moveTime > pmi->linearMovementDuration ) {
			moveTime = pmi->linearMovementDuration;
		}

		dist = pmi->linearMovementEnd - pmi->linearMovementBegin; //VectorSubtract( pmi->linearMovementEnd, pmi->linearMovementBegin, dist );
		moveFrac = (float)moveTime.count() / (float)pmi->linearMovementDuration.count();
		//#define Q_clamp( a, b, c ) ( ( b ) >= ( c ) ? ( a ) = ( b ) : ( a ) < ( b ) ? ( a ) = ( b ) : ( a ) > ( c ) ? ( a ) = ( c ) : ( a ) )
		//Q_clamp( moveFrac, 0, 1 );

		moveFrac = Clampf( moveFrac, 0.0f, 1.0f );
		dest = vec3_fmaf( pmi->linearMovementBegin, moveFrac, dist );
	} else {
		moveFrac = moveTime.count() * static_cast<double>(0.001);
		dest = vec3_fmaf( pmi->linearMovementBegin, moveFrac, pmi->linearMovementVelocity );
	}
	//CLG_Print( PrintType::Developer, fmt::format( "CLGBaseLinearMover: startOrigin({},{},{}), endOrigin({},{},{}), dest({},{},{})\n",
	//	pmi->startOrigin.x,
	//	pmi->startOrigin.y,
	//	pmi->startOrigin.z,
	//		   
	//	pmi->endOrigin.x,
	//	pmi->endOrigin.y,
	//	pmi->endOrigin.z,

	//	dest.x,
	//	dest.y,
	//	dest.z
	//));

	return moveTime.count();
}

/*
* GS_LinearMovementDelta
*/
void SG_LinearMovementDelta( const LinearPushMoveInfo *pmi, const GameTime &oldTime, const GameTime &curTime, vec3_t &dest ) {
	vec3_t p1, p2;
	SG_LinearMovement( pmi, oldTime, p1 );
	SG_LinearMovement( pmi, curTime, p2 );
	dest = p2 - p1;
}




// =========================
// CLGBaseLinearMover::BrushMoveDone
// =========================
void CLGBaseLinearMover::BrushMoveDone() {
	SetVelocity( vec3_zero() );
	moveInfo.OnEndFunction( this );
	DispatchStopCallback();
}

//===============
// CLGBaseLinearMover::BrushMoveFinal
//===============
void CLGBaseLinearMover::BrushMoveFinal() {}

//===============
// CLGBaseLinearMover::BrushMoveBegin
//===============
void CLGBaseLinearMover::BrushMoveWatch() {
	GameTime moveTime = GameTime( level.time ) - moveInfo.linearMovementTimeStamp;

	if ( moveTime.count() >= moveInfo.linearMovementDuration.count() ) {
		SetThinkCallback( &CLGBaseLinearMover::BrushMoveDone );
		SetNextThinkTime( level.time + FRAMERATE_MS );
		return;
	}

	SetThinkCallback( &CLGBaseLinearMover::BrushMoveWatch );
	SetNextThinkTime( level.time + FRAMERATE_MS );
}
void CLGBaseLinearMover::BrushMoveUpdateLinearVelocity( const float distance, const float speed ) {
	int64_t duration = 0;

	if ( speed ) {
		duration = (float)distance * 1000.0f / speed;
		if (!duration) {
			duration = 1;
		}
	}

	moveInfo.isLinearMoving = speed != 0;
	if ( !moveInfo.isLinearMoving ) {
		return;
	} else {

	}

	moveInfo.linearMovementBegin = GetOrigin();
	moveInfo.linearMovementEnd = moveInfo.dest;
	
	moveInfo.linearMovementTimeStamp = level.time - FRAMERATE_MS;
	moveInfo.linearMovementDuration = GameTime( duration );

	CLG_Print( PrintType::Developer, fmt::format( "CLGBaseLinearMover: moveTimeStamp({}), moveDuration({}), moveBegin({},{},{}), moveEnd({},{},{})\n",
		moveInfo.linearMovementTimeStamp.count(),
		moveInfo.linearMovementDuration.count(),

		moveInfo.linearMovementBegin.x,
		moveInfo.linearMovementBegin.y,
		moveInfo.linearMovementBegin.z,

		moveInfo.linearMovementEnd.x,
		moveInfo.linearMovementEnd.y,
		moveInfo.linearMovementEnd.z
	));
}
void CLGBaseLinearMover::BrushMoveBegin() {
	// Calculate velocity.
	const vec3_t dir = moveInfo.dest - GetOrigin();
	float dist = 0.f;
	vec3_normalize_length( dir, dist );
	SetVelocity( vec3_scale( dir, moveInfo.speed ) );

	SetThinkCallback( &CLGBaseLinearMover::BrushMoveWatch );
	SetNextThinkTime( level.time + FRAMERATE_MS);

	BrushMoveUpdateLinearVelocity( dist, GetSpeed() );
	BrushMoveWatch();
}

//===============
// CLGBaseLinearMover::BrushMoveCalc
//===============
void CLGBaseLinearMover::BrushMoveCalc( const vec3_t& destination, LinearPushMoveEndFunction* function ) {
	SetVelocity( vec3_zero() );
	moveInfo.dest = destination;
	moveInfo.OnEndFunction = function;
	BrushMoveUpdateLinearVelocity( 0, 0 );

	ClientGameWorld *world = GetGameWorld();

	if ( level.currentEntity == world->GetClientGameEntity()){ //((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
		BrushMoveBegin();
	} else {
		SetThinkCallback( &CLGBaseLinearMover::BrushMoveBegin );
		SetNextThinkTime( level.time + FRAMERATE_MS );
	}


	//LinearPushMoveInfo& mi = moveInfo;

	//SetVelocity( vec3_zero() );
	//mi.dir = destination - GetOrigin();
	//mi.remainingDistance = VectorNormalize( moveInfo.dir );
	//mi.OnEndFunction = function;

	//if ( EqualEpsilonf( mi.speed, mi.acceleration, BASEMOVER_EPSILON ) && EqualEpsilonf( mi.speed, mi.deceleration, BASEMOVER_EPSILON  ) ) {
	//	if ( level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
	//		BrushMoveBegin();
	//	} else {
	//		SetThinkCallback( &CLGBaseLinearMover::BrushMoveBegin );
	//		SetNextThinkTime( level.time + FRAMERATE_MS );
	//	}
	//} else {
	//	// Accelerative movement
	//	mi.currentSpeed = 0;

	//	SetThinkCallback( &CLGBaseLinearMover::BrushAccelerateThink );
	//	SetNextThinkTime( level.time + FRAMERATE_MS );
	//}
}

//===============
// CLGBaseLinearMover::BrushAngleMoveDone
//===============
void CLGBaseLinearMover::BrushAngleMoveDone() {
	SetAngularVelocity( vec3_zero() );
	moveInfo.OnEndFunction( this );
}

//===============
// CLGBaseLinearMover::BrushAngleMoveFinal
//===============
void CLGBaseLinearMover::BrushAngleMoveFinal() {
	vec3_t move;

	if ( moveInfo.state == LinearMoverState::Up ) {
		move = moveInfo.endAngles - GetAngles();
	} else {
		move = moveInfo.startAngles - GetAngles();
	}

	if ( vec3_equal( move, vec3_zero() ) ) {
		BrushAngleMoveDone();
		return;
	}

	SetAngularVelocity( vec3_scale( move, 1.0f / FRAMETIME_S.count() ) );

	SetThinkCallback( &CLGBaseLinearMover::BrushAngleMoveDone );
	SetNextThinkTime( level.time + FRAMETIME_S );
}

//===============
// CLGBaseLinearMover::BrushAngleMoveBegin
//===============
void CLGBaseLinearMover::BrushAngleMoveBegin() {
	vec3_t destinationDelta;
	float length, travelTime, frames;

	// Set destinationDelta to the vector needed to move
	if ( moveInfo.state == LinearMoverState::Up ) {
		destinationDelta = moveInfo.endAngles - GetAngles();
	} else {
		destinationDelta = moveInfo.startAngles - GetAngles();
	}

	// Get the length of destinationDelta to then get time to reach the destination
	length = vec3_length( destinationDelta );
	travelTime = length / moveInfo.speed;

	if ( travelTime < FRAMETIME_S.count() ) {
		BrushAngleMoveFinal();
		return;
	}

	frames = floor( travelTime / FRAMETIME_S.count() );

	// Get the velocity by scaling the delta vector by the time spent traveling
	SetAngularVelocity( vec3_scale( destinationDelta, 1.0f / travelTime ) );

	SetThinkCallback( &CLGBaseLinearMover::BrushAngleMoveFinal );
	SetNextThinkTime( level.time + frames * FRAMETIME_S );
}

//===============
// CLGBaseLinearMover::BrushAngleMoveCalc
//===============
void CLGBaseLinearMover::BrushAngleMoveCalc( LinearPushMoveEndFunction* function ) {
	SetAngularVelocity( vec3_zero() );
	moveInfo.OnEndFunction = function;

	if ( level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
		BrushAngleMoveBegin();
	} else {
		SetThinkCallback( &CLGBaseLinearMover::BrushAngleMoveBegin );
		SetNextThinkTime( level.time + FRAMERATE_MS );
	}
}

//===============
// CLGBaseLinearMover::BrushAccelerateCalc
//===============
void CLGBaseLinearMover::BrushAccelerateCalc() {
	float accelerationDistance, decelerationDistance;

	moveInfo.moveSpeed = moveInfo.speed;

	if ( moveInfo.remainingDistance < moveInfo.acceleration ) {
		moveInfo.currentSpeed = moveInfo.remainingDistance;
		return;
	}

	accelerationDistance = CalculateAccelerationDistance( moveInfo.speed, moveInfo.acceleration );
	decelerationDistance = CalculateAccelerationDistance( moveInfo.speed, moveInfo.deceleration );

	if ( (moveInfo.remainingDistance - accelerationDistance - decelerationDistance) < 0 ) {
		float factor = (moveInfo.acceleration + moveInfo.deceleration) / (moveInfo.acceleration * moveInfo.deceleration);
		moveInfo.moveSpeed = (-2.0f + sqrt(4.0f - 4.0f * factor * (-2.0f * moveInfo.remainingDistance))) / (2.0f * factor);
	}

	moveInfo.deceleratedDistance = decelerationDistance;
}

//===============
// CLGBaseLinearMover::BrushAccelerate
//===============
void CLGBaseLinearMover::BrushAccelerate() {
	// Are we decelerating?
	if ( moveInfo.remainingDistance <= moveInfo.deceleratedDistance ) {
		if ( moveInfo.remainingDistance < moveInfo.deceleratedDistance ) {
			if ( moveInfo.nextSpeed ) {
				moveInfo.currentSpeed = moveInfo.nextSpeed;
				moveInfo.nextSpeed = 0.0f;
				return;
			}
			if ( moveInfo.currentSpeed > moveInfo.deceleration ) {
				moveInfo.currentSpeed -= moveInfo.deceleration;
			}
		}
		return;
	}

	// Are we at full speed and need to start decelerating during this move?
	if ( moveInfo.currentSpeed == moveInfo.moveSpeed ) {
		if ( (moveInfo.remainingDistance - moveInfo.currentSpeed) < moveInfo.deceleratedDistance ) {
			float p1Distance, p2Distance, distance;

			p1Distance = moveInfo.remainingDistance - moveInfo.deceleratedDistance;
			p2Distance = moveInfo.moveSpeed * (1.0f - (p1Distance / moveInfo.moveSpeed));
			distance = p1Distance + p2Distance;
			moveInfo.currentSpeed = moveInfo.moveSpeed;
			moveInfo.nextSpeed = moveInfo.moveSpeed - moveInfo.deceleration * (p2Distance / distance);
			return;
		}
	}

	// Are we accelerating?
	if ( moveInfo.currentSpeed < moveInfo.speed ) {
		float p1Speed, oldSpeed;
		float p1Distance, p2Distance, distance;

		oldSpeed = moveInfo.currentSpeed;

		// Figure simple acceleration up to move speed
		moveInfo.currentSpeed += moveInfo.acceleration;
		if ( moveInfo.currentSpeed > moveInfo.speed ) {
			moveInfo.currentSpeed = moveInfo.speed;
		}

		// Are we accelerating throughout this entire move?
		if ( (moveInfo.remainingDistance - moveInfo.currentSpeed) >= moveInfo.deceleratedDistance ) {
			return;
		}

		// During this move we will accelerate from currentSpeed to moveSpeed
		// and cross over the deceleratedDistance; figure the average speed for the entire move
		p1Distance = moveInfo.remainingDistance - moveInfo.deceleratedDistance;
		p1Speed = (oldSpeed + moveInfo.moveSpeed) / 2.0f;
		p2Distance = moveInfo.moveSpeed * (1.0f - (p1Distance / p1Speed));
		distance = p1Distance + p2Distance;
		moveInfo.currentSpeed = (p1Speed * (p1Distance / distance)) + (moveInfo.moveSpeed * (p2Distance / distance));
		moveInfo.nextSpeed = moveInfo.moveSpeed - moveInfo.deceleration * (p2Distance / distance);
		return;
	}

	// We are at constant velocity (moveSpeed)
	return;
}

//===============
// CLGBaseLinearMover::BrushAccelerateThink
// 
// The team has completed a frame of movement,
// so change the speed for the next frame
//===============
void CLGBaseLinearMover::BrushAccelerateThink() {
	// Calculate remaining distance.
	moveInfo.remainingDistance -= moveInfo.currentSpeed;

	// In case of a start or blockade speed == 0 so recalculate acceleration.
	if ( moveInfo.currentSpeed == 0 ) {
		BrushAccelerateCalc();
	}

	// Accelerate.
	BrushAccelerate();

	// Will the entire move complete on the next frame
	if ( moveInfo.remainingDistance <= moveInfo.currentSpeed ) {
		BrushMoveFinal();
		return;
	}

	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.currentSpeed * BASE_FRAMERATE ) );

	SetThinkCallback( &CLGBaseLinearMover::BrushAccelerateThink );
	SetNextThinkTime( level.time + FRAMERATE_MS );
}

//===============
// CLGBaseLinearMover::CalculateAccelerationDistance
//===============
float CLGBaseLinearMover::CalculateAccelerationDistance( float targetSpeed, float accelerationRate ) {
	if ( accelerationRate == 0.0f ) {
		//gi.DPrintf( "%s '%s': accelerationRate was 0!\n", 
		//			GetTypeInfo()->classname, 
		//			GetTargetName().empty() ? "unnamed" : GetTargetName().c_str() );
		return 0.0f;
	}
	return (targetSpeed * ((targetSpeed / accelerationRate) + 1.0f) / 2.0f);
}
