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
// Sets up the onEnd callback.
//===============
//
void SVGBaseMover::Spawn() {
	// Spawn base.
	Base::Spawn();

	// Set on end callback.
	SetOnEndCallback(&SVGBaseMover::BaseMoverOnEnd)
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
void SVGBaseMover::SetMoveDirection(const vec3_t& angles, bool resetAngles) {
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
	if (resetAngles) {
		SetAngles(vec3_zero());
	}
}

//===============
// SVGBaseMover::CalculateEndPosition
//===============
vec3_t SVGBaseMover::CalculateEndPosition() {
	const vec3_t& size = GetSize();
	vec3_t absoluteDir{
		fabsf(moveDirection.x),
		fabsf(moveDirection.y),
		fabsf(moveDirection.z)
	};

	float distance = (absoluteDir.x * size.x) + (absoluteDir.y * size.y) + (absoluteDir.z * size.z) - GetLip();
	return vec3_fmaf(GetStartPosition(), distance, moveDirection);
}

//===============
// SVGBaseMover::SwapPositions
//===============
void SVGBaseMover::SwapPositions() {
	SetOrigin(GetEndPosition());		// Origin		 = EndPosition
	SetEndPosition(GetStartPosition());	// EndPosition	 = StartPosition
	SetStartPosition(GetOrigin());		// StartPosition = Origin
}

//===============
// SVGBaseMover::CalculateMove
// 
// Prepares this basemover to move into the given destination.
// It'll start the brush movement process if it isn't moving already.
//===============
void SVGBaseMover::CalculateMove(const vec3_t& destination) {
	// Reset velocity.
	SetVelocity(vec3_zero());

	// Set direction to travel. 
	moveInfo.dir = destination - GetOrigin();
	moveInfo.remainingDistance = VectorNormalize(moveInfo.dir);
	// WID: Got to try this function instead, uggh... old math lib sucks :P 
	//vec3_normalize_length(destination, moveInfo.remainingDistance);
	// 
	//moveInfo.OnEndFunction = func;

	// If move speed is equal to accel and decel...
	if (moveInfo.speed == moveInfo.acceleration && moveInfo.speed == moveInfo.deceleration) {
		// Depending on which entity we are processing, slave, or master, start the process differently.
		if (level.currentEntity == ((GetFlags() & EntityFlags::TeamSlave) ? GetTeamMasterEntity() : this))         {
			BeginMove();
		} else {
			SetNextThinkTime(level.time + FRAMETIME);
			SetThinkCallback(&SVGBaseMover::BaseMoverBeginMoveThink);
		}
	} else {
		// Reset speed.
		moveInfo.currentSpeed = 0;

		// Setup next think.
		SetNextThinkTime(level.time + FRAMETIME);
		// WID: TODO: Still gotta add this.
		//SetThinkCallback(&SVGBaseMover::BaseMoverAccelMoveThink);
	}
}

//===============
// SVGBaseMover::BaseMoverBeginMoveThink
// 
// Think callback which'll trigger the beginning of movement for this brush.
//===============
void SVGBaseMover::BaseMoverBeginMoveThink() {
	// Begin move.
	BeginMove();
}

//===============
// SVGBaseMover::BeginMove
// 
// Used to engage movement, it allows it to "start".
//===============
void SVGBaseMover::BeginMove() {
	// Check if we got to engage our final movement part.
	if ((moveInfo.speed * FRAMETIME) >= moveInfo.remainingDistance) {
		// Engage final movement part.
		FinalizeMove();
	}

	// Set new velocity.
	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.speed ) );
	
	// Try and calculate the frames that are left to move.
	float framesLeft = floor((moveInfo.remainingDistance / moveInfo.speed) / FRAMETIME);

	// Calculate the updated remaining distance to travel for this basemover.
	moveInfo.remainingDistance -= framesLeft * moveInfo.speed * FRAMETIME;

	// Setup finalize movement think callback.
	SetNextThinkTime(level.time + (framesLeft * FRAMETIME));
	SetThinkCallback(&SVGBaseMover::BaseMoverMoveFinalizeThink);
}

//===============
// SVGBaseMover::BaseMoverMoveFinalThink
// 
// Think callback which'll trigger the "finalization" of movement for this brush.
//===============
void SVGBaseMover::BaseMoverMoveFinalizeThink() {
	// Finalize move.
	FinalizeMove();
}

//===============
// SVGBaseMover::FinalizeMove
//===============
void SVGBaseMover::FinalizeMove() {
	// Check if the move is finished, or not.
	if (moveInfo.remainingDistance == 0) {
		// Call to finished move.
		MoveFinished();

		// Return.
	    return;
	}
	
	// Move only as far as to clear the remaining distance
	SetVelocity( vec3_scale( moveInfo.dir, moveInfo.remainingDistance / FRAMETIME ) );
	
	// Set next think callback to finish the move in the next frame.
	SetNextThinkTime(level.time + FRAMETIME);
	SetThinkCallback(&SVGBaseMover::BaseMoverMoveFinishedThink);
}

//===============
// SVGBaseMover::BaseMoverMoveFinishedThink
// 
// Think callback which'll trigger the "finishing" of movement for this brush.
//===============
void SVGBaseMover::BaseMoverMoveFinishedThink() {
	// Finish move.
	MoveFinished();
}

//===============
// SVGBaseMover::MoveFinished
// 
// Used to finish a move of a basemover.
//===============
void SVGBaseMover::MoveFinished() {
	// Reset velocity.
	SetVelocity( vec3_zero() );
	// WID: TODO: Got to do this still.
	// 
	// 
	// 	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//! DO THIS TOMORROW!!!!
	//! 
	//! 
	//moveInfo.OnEndFunction( ent );
}

//===============
// SVGBaseMover::BaseMoverOnEnd
//===============
void SVGBaseMover::BaseMoverOnEnd() {
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//! DO THIS TOMORROW...
}