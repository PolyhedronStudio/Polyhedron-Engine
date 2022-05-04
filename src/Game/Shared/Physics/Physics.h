/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once

// Shared Game.
#include "../SharedGame.h"


/**
*	SlideBox Configuration.
**/
//! Maximum amount of planes to clip against when executing a SlideBox movement.
static constexpr int32_t MAX_SLIDEMOVE_CLIP_PLANES	= 16;
//! Maximum amount of entities that can be touched at the same time.
static constexpr int32_t MAX_SLIDEMOVE_TOUCH		= 32;

//! SlideMove 'Stop Epsilon' for velocities that are nearing 0.
static constexpr float SLIDEMOVE_STOP_EPSILON		= 0.1f;

//! Uncomment for printing Debug Information Output when a SlideMove gets trapped.
//#define SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES
//! Comment to disable SlideMove velocity clamping.
#define SG_SLIDEMOVE_CLAMPING



/***
*
*
*	SlideMove Flags and Functions.
*
*
***/
/**
*	@brief The possible flags returned from executing a SlideMove on a MoveState.
**/
struct SlideMoveFlags {
	static constexpr int32_t PlaneTouched	= 16;
	static constexpr int32_t WallBlocked	= 8;
	static constexpr int32_t Trapped		= 4;
	//! When Blocekd flag is set, it doesn't mean it didn't slide along the blocking object.
	static constexpr int32_t Blocked		= 2;
	static constexpr int32_t Moved			= 1;
};


/**
*	@brief	Contains the status of an entities physics move state.
**/
struct MoveState {
	vec3_t velocity = vec3_zero();
	vec3_t origin = vec3_zero();
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();
	float remainingTime = 0.f;

	vec3_t gravityDir = vec3_zero();
	float slideBounce = 0.f;
	GameEntity *groundEntity = nullptr;

	GameEntity *passEntity = nullptr;
	int32_t contentMask = 0;

	int32_t numClipPlanes = 0;
	vec3_t clipPlaneNormals[MAX_SLIDEMOVE_CLIP_PLANES];

	int32_t numTouchEntities = 0;
	GameEntity *touchEntites[MAX_SLIDEMOVE_TOUCH];
};


//========================================================================

void SG_PhysicsEntityWPrint(const std::string &functionName, const std::string &functionSector, const std::string& message);
/*
* GS_ClipVelocity
*/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce );

//================================================================================

/**
*	@brief	Applies 'downward' gravity forces to the entity.
**/
void SG_AddGravity( GameEntity *sharedGameEntity );

/**
*	@brief	Apply ground friction forces to entity.
**/
void SG_AddGroundFriction( GameEntity *sharedGameEntity, float friction );


/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, int32_t contentMask, float slideBounce, float friction );

static inline bool IsWalkablePlane(const CollisionPlane& plane) {
	return plane.normal.z >= 0.7f ? true : false;
}
//#define ISWALKABLEPLANE( x ) ( ( (CollisionPlane )x ).normal.z >= 0.7f )

//================================================================================

//pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.
//
//onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects
//
//doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
//bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
//corpses are SOLID_NOT and MOVETYPE_TOSS
//crates are SOLID_BBOX and MOVETYPE_TOSS
//walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
//flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY
//
//solid_edge items only clip against bsp models.


/**
*	@brief	Tests whether the entity position would be trapped in a Solid.
*	@return	(nullptr) in case it is free from being trapped. Worldspawn entity otherwise.
**/
GameEntity *SG_TestEntityPosition(GameEntity *geTestSubject);

/**
*	@brief	Called when two entities have touched so we can safely call their touch callback functions.
**/
void SG_Impact( GameEntity *entityA, const SGTraceResult &trace );
/*
* G_RunEntity
*
*/
void SG_RunEntity(SGEntityHandle &entityHandle);