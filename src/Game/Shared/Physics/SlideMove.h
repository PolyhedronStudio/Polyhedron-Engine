/***
*
*	License here.
*
*	@file
*
*	BoxSlide Movement implementation for SharedGame Physics.
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
/*#define SG_SLIDEMOVE_DEBUG_TRAPPED_MOVES*/
//! Uncomment for printing Debug blockMask results of SlideMoves.
#define SG_SLIDEMOVE_DEBUG_BLOCKMASK 1
//! Comment to disable SlideMove velocity clamping.
#define SG_SLIDEMOVE_CLAMPING 1


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
	//! Set whenever the move is capable of stepping up.
	static constexpr int32_t CanStepUp		= 64;
	//! Set whenever the movei s capable of stepping down.
	static constexpr int32_t CanStepDown	= 32;

	//! Set whenever we've touched any entity that is not WorldSpawn.
	static constexpr int32_t EntityTouched	= 32;
	//! Set whenever we've touched a brush plane.
	static constexpr int32_t PlaneTouched	= 16;

	//! When Blocekd flag is set, it doesn't mean it didn't slide along the blocking object.
	static constexpr int32_t WallBlocked	= 8;
	//! NOTE: Set only in case of trouble. It shouldn't happen.
	static constexpr int32_t Trapped		= 4;

	//! Set if the move became groundless, and was unable to step down to new floor.
	static constexpr int32_t EdgeMoved		= 2;
	//! Set whenever the move has been completed for the remainingTime.
	static constexpr int32_t Moved			= 1;
};

/**
*	@brief	Contains the status of an entities physics move state.
**/
struct MoveState {
	//! Physical Properties.
	vec3_t velocity	= vec3_zero();
	vec3_t origin	= vec3_zero();
	vec3_t mins		= vec3_zero();
	vec3_t maxs		= vec3_zero();

	//! Remaining time that's left for this move in this frame.
	float	remainingTime	=  0.f;

	//! Direction of gravity.
	vec3_t	gravityDir	= vec3_zero();
	//! Slide Bounce Factor.
	float	slideBounce	= 0.f;

	//! Ground Entity Trace. Updated during the move processing.
	SGTraceResult groundTrace;
	//! Ground Entity.
	int32_t	groundEntityLinkCount	= 0;
	//GameEntity	*groundEntity			= nullptr;
	int32_t	groundEntityNumber = -1;

	//! Entity that we're trying to move around.
	int32_t	moveEntityNumber = -1;	//GameEntity *moveEntity = nullptr;
	//! Entity to skip when executing traces. (Usually the same entity as moveEntity.)
	int32_t	skipEntityNumber = -1;//GameEntity *skipEntity = nullptr;

	//! Entity Flags and Content Mask.
	int32_t	entityFlags = 0;
	int32_t	contentMask = 0;

	//! Number of, and normals of each plane we want to clip against to.
	int32_t numClipPlanes = 0;
	vec3_t	clipPlaneNormals[MAX_SLIDEMOVE_CLIP_PLANES];

	//! Number of, and pointers to the entities we touched and want to dispatch a 'Touch' callback to.
	int32_t	numTouchEntities = 0;
	int32_t	touchEntites[MAX_SLIDEMOVE_TOUCH];
};


/**
*	@return	Clipped by normal velocity.
**/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce );

/*
* GS_SlideMove
*/
int32_t SG_SlideMove( MoveState *moveState );

/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, MoveState &boxSlideMove  );

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
*	@return	The number of the ground entity this entity is covering ground on.
**/
int32_t SG_BoxSlideMove_CheckForGround( GameEntity *geCheck );

/**
*	@brief	Processes rotational friction calculations.
**/
void SG_AddRotationalFriction( SGEntityHandle entityHandle );