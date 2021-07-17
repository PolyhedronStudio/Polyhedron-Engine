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
void SVGBaseMover::SetMoveDirection(const vec3_t& angles) {
	vec3_t VEC_UP = { 0, -1, 0 };
	vec3_t MOVEDIR_UP = { 0, 0, 1 };
	vec3_t VEC_DOWN = { 0, -2, 0 };
	vec3_t MOVEDIR_DOWN = { 0, 0, -1 };

	//if (VectorCompare(angles, VEC_UP)) {
	//	VectorCopy(MOVEDIR_UP, moveDirection);
	//} else if (VectorCompare(angles, VEC_DOWN)) {
	//	VectorCopy(MOVEDIR_DOWN, moveDirection);
	//} else {
	//	AngleVectors(angles, &moveDirection, NULL, NULL);
	//}

	////VectorClear(angles);
	//SetAngles(vec3_zero());

	if (vec3_equal(angles, VEC_UP)) {
		moveDirection = MOVEDIR_UP;
	} else if (vec3_equal(angles, VEC_DOWN)) {
		moveDirection = MOVEDIR_DOWN;
	} else {
		AngleVectors(angles, &moveDirection, NULL, NULL);
	}

	// Admer: is this really intended? Some entities may control their angle
	// and align it directly with the movement direction.
	// I suggest we add a bool parameter to this method, 'resetAngles',
	// which will zero the entity's angles if true
	SetAngles(vec3_zero());
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
	SetOrigin( GetEndPosition() );			// origin = endpos
	SetEndPosition( GetStartPosition() );	// endpos = startpos
	SetStartPosition( GetOrigin() );		// startpos = origin
}

// =========================
// SVGBaseMover::BrushMoveDone
// =========================
void SVGBaseMover::BrushMoveDone()
{
	SetVelocity( vec3_zero() );
	moveInfo.OnEndFunction( serverEntity );
}

//===============
// SVGBaseMover::BrushMoveFinal
//===============
void SVGBaseMover::BrushMoveFinal()
{
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
void SVGBaseMover::BrushMoveBegin()
{
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
// SVGBaseMover::BrushMoveCalc
//===============
void SVGBaseMover::BrushMoveCalc( const vec3_t& destination, PushMoveEndFunction* function )
{
	PushMoveInfo& mi = moveInfo;

	SetVelocity( vec3_zero() );
	mi.dir = destination - GetOrigin();
	mi.remainingDistance = VectorNormalize( moveInfo.dir );
	mi.OnEndFunction = function;

	if ( mi.speed == mi.acceleration && mi.speed == mi.deceleration ) {
		if ( level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this) ) {
			BrushMoveBegin();
		} else {
			SetThinkCallback( &SVGBaseMover::BrushMoveBegin );
			SetNextThinkTime( level.time + FRAMETIME );
		}
	} else {
		// accelerative
		mi.currentSpeed = 0;

		// Implement Think_AccelMove first
		//SetThinkCallback( &SVGBaseMover::BrushAccelMove );
		SetNextThinkTime( level.time + FRAMETIME );
	}
}

