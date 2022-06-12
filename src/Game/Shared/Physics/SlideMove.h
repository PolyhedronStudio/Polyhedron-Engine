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
//! Bounce Velocity Clipping value.
static constexpr float SLIDEMOVE_CLIP_BOUNCE		= 1.01f;

//! SlideMove 'Minimal Step Height'. If a step exceeds it, it'll resort to NOT positioning the entity, but have it
// drop by gravity instead.
static constexpr float SLIDEMOVE_STEP_HEIGHT_MIN	= 4.0f;

//! The actual maximum step height a slidemove is allowed to step.
static constexpr float SLIDEMOVE_STEP_HEIGHT = 18.0f;

//! The distance between a slide move and the ground. (An offset, to prevent trouble.)
static constexpr float SLIDEMOVE_GROUND_DISTANCE = 0.25f;

//! Determins the minimum speed value for Z velocity before determinging a slidemove should bother with floor logic.
static constexpr float SLIDEMOVE_SPEED_UP = 0.1f;


static constexpr float SLIDEMOVE_SPEED_LAND			= -280.f;
static constexpr float SLIDEMOVE_SPEED_FALL			= -700.f;
static constexpr float SLIDEMOVE_SPEED_FALL_FAR		= -900.f;





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
*	@brief The possible flags returned from executing a SlideMove on a SlideMoveState.
**/
struct SlideMoveMoveFlags {
	static constexpr int32_t FoundGround	= (1 << 0); // Mover found new ground.
	static constexpr int32_t OnGround		= (1 << 1); // Mover is on ground.
	static constexpr int32_t LostGround		= (1 << 2); // Mover lost ground.

	static constexpr int32_t Ducked					= (1 << 4);  // Mover ducked.
	static constexpr int32_t Jumped					= (1 << 5);  // Mover jumped.
	//static constexpr int32_t JumpHeld				= (1 << 6);  // Mover's jump key is down.
	static constexpr int32_t OnLadder				= (1 << 7);  // Mover is on ladder.
	static constexpr int32_t UnderWater				= (1 << 8);  // Mover is under water.
	static constexpr int32_t TimePushed				= (1 << 9);  // Time before can seek ground.
	static constexpr int32_t TimeWaterJump			= (1 << 10);  // Time before control.
	static constexpr int32_t TimeLand				= (1 << 11);  // Time before jump eligible.

	static constexpr int32_t TimeMask = (TimePushed | TimeWaterJump | TimeLand);
};

struct SlideMoveFlags {
	// Blabla

	//! Set whenever the move has been completed for the remainingTime.
	static constexpr int32_t Moved			= (1 << 1);

	//! Set when the move steps up.
	static constexpr int32_t SteppedUp		= (1 << 2);
	//! Set when the move steps down.
	static constexpr int32_t SteppedDown	= (1 << 3);
	//! Set if the move became groundless, and was unable to step down to new floor.
	static constexpr int32_t SteppedEdge	= (1 << 4);
	//! Set if the move resulted in stepping to ground while falling down.
	static constexpr int32_t SteppedFall	= (1 << 5);

	//! Set when the move has no ground resulting in gravity affecting the move and falling down.
	static constexpr int32_t FallingDown	= (1 << 20);

	//! Set when an entity has been touched.
	static constexpr int32_t EntityTouched	= (1 << 28);
	//! Set when we've touched a brush plane that we can't move ahead into, to clip velocity to it and try again.
	static constexpr int32_t PlaneTouched	= (1 << 29);
	//! Set if something is blocking our path. NOTE: It doesn't mean it didn't slide along the blocking object.
	static constexpr int32_t WallBlocked	= (1 << 30);
	//! NOTE: Set only in case of trouble. It shouldn't happen.
	static constexpr int32_t Trapped		= (1 << 31);
};

/**
*	@brief	Contains the status of an entities physics move state.
**/
struct SlideMoveState {
	//! Original Physical Properties.
	vec3_t originalVelocity	= vec3_zero();
	vec3_t originalOrigin	= vec3_zero();
	vec3_t originalMins		= vec3_zero();
	vec3_t originalMaxs		= vec3_zero();

	//! Physical Properties.
	vec3_t velocity	= vec3_zero();
	vec3_t origin	= vec3_zero();
	vec3_t mins		= vec3_zero();
	vec3_t maxs		= vec3_zero();

	//! Direction of gravity.
	vec3_t	gravityDir	= vec3_zero();
	//! Slide Bounce Factor.
	float	slideBounce	= 0.f;

	//! Remaining time that's left for this move in this frame.
	float	remainingTime	=  0.f;

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

	//! Set by categorize position, used to determine what "brush" we are in. (This means open space too, yes.)
	int32_t categorizedContent = 0;
	//! Keeps track of the water type we're in.
	int32_t waterType	= 0;
	//! Keeps track of the water level we reside in.
	int32_t waterLevel	= 0;

	//! Number of, and normals of each plane we want to clip against to.
	int32_t numClipPlanes = 0;
	vec3_t	clipPlaneNormals[MAX_SLIDEMOVE_CLIP_PLANES];

	//! Number of, and pointers to the entities we touched and want to dispatch a 'Touch' callback to.
	int32_t	numTouchEntities = 0;
	int32_t	touchEntites[MAX_SLIDEMOVE_TOUCH];
	
	//! Move State flags: Store state like on-ground etc.
	int32_t moveFlags		= 0;
	//! Set to miliseconds to maintain (if needed) flag state.
	int32_t moveFlagTime	= 0;
};


/**
*	@return	Clipped by normal velocity.
**/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce );

/*
* GS_SlideMove
*/
int32_t SG_SlideMove( SlideMoveState *moveState );

/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, SlideMoveState *boxSlideMove  );

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
*	@return	The number of the ground entity this entity is covering ground on.
**/
int32_t SG_BoxSlideMove_CheckForGround( GameEntity *geCheck );

/**
*	@brief	Processes rotational friction calculations.
**/
const vec3_t SG_AddRotationalFriction( SGEntityHandle entityHandle );