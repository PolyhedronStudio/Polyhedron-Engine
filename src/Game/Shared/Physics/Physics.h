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
	static constexpr int32_t SteppedUp		= 64;
	static constexpr int32_t SteppedDown	= 32;

	static constexpr int32_t PlaneTouched	= 16;
	static constexpr int32_t WallBlocked	= 8;
	static constexpr int32_t Trapped		= 4;
	//! When Blocekd flag is set, it doesn't mean it didn't slide along the blocking object.
	static constexpr int32_t EdgeMoved	= 2;
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


//========================================================================

void SG_Physics_PrintWarning(const std::string& message);

//================================================================================

/*
* GS_ClipVelocity
*/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, const float overbounce );
/**
*	@brief	Keep entity velocity within bounds.
**/
void SG_CheckVelocity( GameEntity *geCheck );

/**
*	@brief	Applies 'downward' gravity forces to the entity.
**/
void SG_AddGravity( GameEntity *sharedGameEntity );

/**
*	@brief	Apply ground friction forces to entity.
**/
void SG_AddGroundFriction( GameEntity *sharedGameEntity, const float friction );

/**
*	@brief	Pushes the entity. Does not change the entities velocity at all
**/
SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset );

/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, MoveState &boxSlideMove  );

/**
*	@return	The proper Solid mask to use for the passed game entity.
**/
int32_t SG_SolidMaskForGameEntity( GameEntity *gameEntity );

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
**/
void SG_CheckGround( GameEntity *geCheck );

/**
*	@brief	Utility function that determines whether a plane is too steep to walk on or not.
**/
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
*	@brief Logic for MoveType::(None, PlayerMove): Gives the entity a chance to 'Think', does not execute any physics logic.
**/
void SG_Physics_None(SGEntityHandle& entityHandle);
/**
*	@brief Logic for MoveType::(NoClip): Moves the entity based on angular- and regular- velocity. Does not clip to world or entities.
**/
void SG_Physics_NoClip(SGEntityHandle &entityHandle);

/**
*	@brief	Performs a velocity based 'slidebox' movement for general NPC's.
*			If FL_SWIM or FL_FLY are not set, it'll check whether there is any
*			ground at all, if it has been removed the monster will be pushed
*			downwards due to gravity effect kicking in.
**/
void SG_Physics_BoxSlideMove(SGEntityHandle &entityHandle);

/**
*	@brief Logic for MoveType::(Toss, TossSlide, Bounce, Fly and FlyMissile)
**/
void SG_Physics_Toss(SGEntityHandle& entityHandle);

/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle );

//================================================================================
/**
*	@brief	Tests whether the entity position would be trapped in a Solid.
*	@return	(nullptr) in case it is free from being trapped. Worldspawn entity otherwise.
**/
GameEntity *SG_TestEntityPosition(GameEntity *geTestSubject);

/**
*	@brief	Called when two entities have touched so we can safely call their touch callback functions.
**/
void SG_Impact( GameEntity *entityA, const SGTraceResult &trace );

/**
*	@brief	Processes active game and physics logic of this entity for the current time/frame.
**/
void SG_RunEntity(SGEntityHandle &entityHandle);

/**
*	@brief	Gives the entity a chance to process 'Think' callback logic if the
*			time is there for it to do so.
*	@return	True if it failed. Yeah, odd, I know, it was that way, it stays that way for now.
**/
qboolean SG_RunThink(GameEntity *geThinker);