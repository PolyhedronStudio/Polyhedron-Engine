/*
// LICENSE HERE.

//
// SVGBaseMover.cpp
//
//
*/
#include "../../g_local.h"		// SVGame.
#include "../../effects.h"		// Effects.
#include "../../entities.h"		// Entities.
#include "../../utils.h"		// Util funcs.

// Class Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseMover.h"

// Constructor/Deconstructor.
SVGBaseMover::SVGBaseMover(Entity* svEntity) : SVGBaseTrigger(svEntity) {
	//
	// All callback functions best be nullptr.
	//
	//thinkFunction = nullptr;


	//
	// Set all entity pointer references to nullptr.
	//
	//activatorEntity = nullptr;
	//enemyEntity = nullptr;
	//groundEntity = nullptr;
	//oldEnemyEntity = nullptr;
	//teamChainEntity = nullptr;
	//teamMasterEntity = nullptr;

	//
	// Default values for members.
	//
	acceleration = 0;
	deceleration = 0;
	speed = 0;
	startPosition = { 0.f, 0.f, 0.f };
	endPosition = { 0.f, 0.f, 0.f };
}
SVGBaseMover::~SVGBaseMover() {

}

// Interface functions. 
//
//===============
// SVGBaseMover::Precache
//
//===============
//
void SVGBaseMover::Precache() {
	Base::Precache();
}

//
//===============
// SVGBaseMover::Spawn
//
//===============
//
void SVGBaseMover::Spawn() {
	Base::Spawn();


}

//
//===============
// SVGBaseMover::Respawn
// 
//===============
//
void SVGBaseMover::Respawn() {
	Base::Respawn();
}

//
//===============
// SVGBaseMover::PostSpawn
// 
//===============
//
void SVGBaseMover::PostSpawn() {
	Base::PostSpawn();
}

//
//===============
// SVGBaseMover::Think
//
//===============
//
void SVGBaseMover::Think() {
	Base::Think();
}

//
//===============
// SVGBaseMover::SpawnKey
//
//===============
//
void SVGBaseMover::SpawnKey(const std::string& key, const std::string& value) {
	if (key == "wait") {
		ParseFloatKeyValue(key, value, waitTime);
	} else if ( key == "lip" ) {
		ParseFloatKeyValue( key, value, lip );
	} else {
		Base::SpawnKey(key, value);
	}
}

//
//===============
// SVGBaseMover::SetMoveDirection
//
//===============
//
void SVGBaseMover::SetMoveDirection(const vec3_t& angles, const qboolean resetAngles) {
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

	if (vec3_equal(angles, VEC_UP) || vec3_equal(angles, AnglesUp[0]) || vec3_equal(angles, AnglesUp[1])) {
		moveDirection = MOVEDIR_UP;
	} else if (vec3_equal(angles, VEC_DOWN) || vec3_equal(angles, AnglesDown[0]) || vec3_equal(angles, AnglesDown[1])) {
		moveDirection = MOVEDIR_DOWN;
	} else {
		AngleVectors(angles, &moveDirection, NULL, NULL);
	}

	// Admer: is this really intended? Some entities may control their angle
	// and align it directly with the movement direction.
	// I suggest we add a bool parameter to this method, 'resetAngles',
	// which will zero the entity's angles if true
	if (resetAngles == true) {
		SetAngles(vec3_zero());
	}
}

//===============
// SVGBaseMover::CalculateEndPosition
//===============
vec3_t SVGBaseMover::CalculateEndPosition() {
	const vec3_t& size = GetSize();
	vec3_t absoluteDir{
		fabsf( moveDirection.x ),
		fabsf( moveDirection.y ),
		fabsf( moveDirection.z ) 
	};
	
	float distance = (absoluteDir.x * size.x) + (absoluteDir.y * size.y) + (absoluteDir.z * size.z) - GetLip();
	return vec3_fmaf( GetStartPosition(), distance, moveDirection );
}

//===============
// SVGBaseMover::SwapPositions
//===============
void SVGBaseMover::SwapPositions() {
	vec3_t temp = GetEndPosition();			// temp = endpos
	SetEndPosition( GetStartPosition() );	// endpos = startpos
	SetStartPosition( temp );				// startpos = temp
}

// =========================
// SVGBaseMover::BrushMoveDone
// =========================
void SVGBaseMover::BrushMoveDone() {
	SetVelocity( vec3_zero() );
	moveInfo.OnEndFunction( serverEntity );
}

//===============
// SVGBaseMover::BrushMoveFinal
//===============
void SVGBaseMover::BrushMoveFinal() {
	// We've traveled our world, time to rest
	if ( moveInfo.remainingDistance == 0.0f ) {
		BrushMoveDone();
		return;
	}

	// Move only as far as to clear the remaining distance
	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.remainingDistance / FRAMETIME ) );

	SetThinkCallback( &SVGBaseMover::BrushMoveDone );
	SetNextThinkTime( level.time + FRAMETIME );
}

//===============
// SVGBaseMover::BrushMoveBegin
//===============
void SVGBaseMover::BrushMoveBegin() {
	float frames;

	// It's time to stop
	if ( (moveInfo.speed * FRAMETIME) >= moveInfo.remainingDistance ) {
		BrushMoveFinal();
		return;
	}

	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );

	frames = floor( (moveInfo.remainingDistance / moveInfo.speed) / FRAMETIME );
	moveInfo.remainingDistance -= frames * moveInfo.speed * FRAMETIME;

	SetThinkCallback( &SVGBaseMover::BrushMoveFinal );
	SetNextThinkTime( level.time + (frames * FRAMETIME) );
}

//===============
// AreSame
// Checks if two floats are basically the same with an epsilon
//===============
bool AreSame( float a, float b, float epsilon = 0.1f ) {
	return fabs( a - b ) < epsilon;
}

//===============
// SVGBaseMover::BrushMoveCalc
//===============
void SVGBaseMover::BrushMoveCalc( const vec3_t& destination, PushMoveEndFunction* function ) {
	PushMoveInfo& mi = moveInfo;

	SetVelocity( vec3_zero() );
	mi.dir = destination - GetOrigin();
	mi.remainingDistance = VectorNormalize( moveInfo.dir );
	mi.OnEndFunction = function;

	if ( AreSame( mi.speed, mi.acceleration ) && AreSame( mi.speed, mi.deceleration ) ) {
		if ( level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
			BrushMoveBegin();
		} else {
			SetThinkCallback( &SVGBaseMover::BrushMoveBegin );
			SetNextThinkTime( level.time + FRAMETIME );
		}
	} else {
		// Accelerative movement
		mi.currentSpeed = 0;

		SetThinkCallback( &SVGBaseMover::BrushAccelerateThink );
		SetNextThinkTime( level.time + FRAMETIME );
	}
}

//===============
// SVGBaseMover::BrushAngleMoveDone
//===============
void SVGBaseMover::BrushAngleMoveDone() {
	SetAngularVelocity( vec3_zero() );
	moveInfo.OnEndFunction( GetServerEntity() );
}

//===============
// SVGBaseMover::BrushAngleMoveFinal
//===============
void SVGBaseMover::BrushAngleMoveFinal() {
	vec3_t move;

	if ( moveInfo.state == MoverState::Up ) {
		move = moveInfo.endAngles - GetAngles();
	} else {
		move = moveInfo.startAngles - GetAngles();
	}

	if ( vec3_equal( move, vec3_zero() ) ) {
		BrushAngleMoveDone();
		return;
	}

	SetAngularVelocity( vec3_scale( move, 1.0f / FRAMETIME ) );

	SetThinkCallback( &SVGBaseMover::BrushAngleMoveDone );
	SetNextThinkTime( level.time + FRAMETIME );
}

//===============
// SVGBaseMover::BrushAngleMoveBegin
//===============
void SVGBaseMover::BrushAngleMoveBegin() {
	vec3_t destinationDelta;
	float length, travelTime, frames;

	// Set destinationDelta to the vector needed to move
	if ( moveInfo.state == MoverState::Up ) {
		destinationDelta = moveInfo.endAngles - GetAngles();
	} else {
		destinationDelta = moveInfo.startAngles - GetAngles();
	}

	// Get the length of destinationDelta to then get time to reach the destination
	length = vec3_length( destinationDelta );
	travelTime = length / moveInfo.speed;

	if ( travelTime < FRAMETIME ) {
		BrushAngleMoveFinal();
		return;
	}

	frames = floor( travelTime / FRAMETIME );

	// Get the velocity by scaling the delta vector by the time spent traveling
	SetAngularVelocity( vec3_scale( destinationDelta, 1.0f / travelTime ) );

	SetThinkCallback( &SVGBaseMover::BrushAngleMoveFinal );
	SetNextThinkTime( level.time + frames * FRAMETIME );
}

//===============
// SVGBaseMover::BrushAngleMoveCalc
//===============
void SVGBaseMover::BrushAngleMoveCalc( PushMoveEndFunction* function ) {
	SetAngularVelocity( vec3_zero() );
	moveInfo.OnEndFunction = function;

	if ( level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
		BrushAngleMoveBegin();
	} else {
		SetThinkCallback( &SVGBaseMover::BrushAngleMoveBegin );
		SetNextThinkTime( level.time + FRAMETIME );
	}
}

//===============
// SVGBaseMover::BrushAccelerateCalc
//===============
void SVGBaseMover::BrushAccelerateCalc() {
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
// SVGBaseMover::BrushAccelerate
//===============
void SVGBaseMover::BrushAccelerate() {
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
// SVGBaseMover::BrushAccelerateThink
// 
// The team has completed a frame of movement,
// so change the speed for the next frame
//===============
void SVGBaseMover::BrushAccelerateThink() {
	moveInfo.remainingDistance -= moveInfo.currentSpeed;

	if ( moveInfo.currentSpeed == 0 ) { // Happens when starting or blocked
		BrushAccelerateCalc();
	}

	BrushAccelerate();

	// Will the entire move complete on the next frame
	if ( moveInfo.remainingDistance <= moveInfo.currentSpeed ) {
		BrushMoveFinal();
		return;
	}

	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.currentSpeed * 10.0f ) );

	SetThinkCallback( &SVGBaseMover::BrushAccelerateThink );
	SetNextThinkTime( level.time + FRAMETIME );
}

//===============
// SVGBaseMover::CalculateAccelerationDistance
//===============
float SVGBaseMover::CalculateAccelerationDistance( float targetSpeed, float accelerationRate ) {
	if ( accelerationRate == 0.0f ) {
		gi.DPrintf( "%s '%s': accelerationRate was 0!\n", 
					GetTypeInfo()->className, 
					GetTargetName().empty() ? "unnamed" : GetTargetName().c_str() );
		return 0.0f;
	}
	return (targetSpeed * ((targetSpeed / accelerationRate) + 1.0f) / 2.0f);
}
