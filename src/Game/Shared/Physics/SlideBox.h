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
#include "Game/Shared/SharedGame.h"
#include "Game/Shared/Tracing.h"

struct SGTraceResult;

/**
*	SlideBox Configuration.
**/
//! Maximum amount of planes to clip against when executing a SlideBox movement.
static constexpr int32_t MAX_SLIDEBOX_CLIPPING_PLANES	= 16;
//! Maximum amount of entities that can be touched at the same time.
static constexpr int32_t MAX_SLIDEBOX_TOUCH_ENTITIES	= 32;

//! Bounce Velocity Clipping value.
static constexpr float SLIDEBOX_CLIP_BOUNCE		= 1.01f;
//! SlideMove 'Stop Epsilon' for velocities that are nearing 0.
static constexpr double SLIDEBOX_STOP_EPSILON		= 0.015625;

//! The actual maximum step height a slidemove is allowed to step.
static constexpr float SLIDEBOX_STEP_HEIGHT = 18.0f;
//! SlideMove 'Minimal Step Height'. If a step exceeds it, it'll resort to NOT positioning the entity, but have it
// drop by gravity instead.
static constexpr float SLIDEBOX_STEP_HEIGHT_MIN	= 4.0f;

//! The distance between a root motion move's bounding box and the ground. (An offset we stick to, preventing getting stuck.)
static constexpr float SLIDEBOX_GROUND_DISTANCE = 0.03125f;

//! Friction for ground movement.
static constexpr float SLIDEBOX_GROUND_FRICTION = 6.f;
//! Friction for water movement.
static constexpr float SLIDEBOX_WATER_FRICTION = 1.f;

//! The Stop speed, at which it decreases to a halt.
static constexpr float SLIDEBOX_STOP_SPEED = 75.f;
//! Determins the minimum speed value for Z velocity before determinging a slidemove should bother with floor logic.
static constexpr float SLIDEBOX_SPEED_UP = 0.1f;

//! Fall time speeds.
static constexpr float SLIDEBOX_SPEED_LAND			= -280;
static constexpr float SLIDEBOX_SPEED_FALL			= -700;
static constexpr float SLIDEBOX_SPEED_FALL_FAR		= -900;



/***
*
*
*	SlideMove Flags and Functions.
*
*
***/
/**
*	@brief	For holding slide box movement state data. Each frame, this struct needs
*			to be properly set up, after which it can be used to call slide move
*			functions on and apply the final results to the entity. (If movement succeeded.)
**/
struct SlideBoxMove {
	//! The solid that we're tracing.
	int32_t traceType = 0; // 0 == Bounds Box, 1 == Sphere

	//! Velocity.
	vec3_t velocity = vec3_zero();
	//! Origin.
	vec3_t origin = vec3_zero();

	//! Mins/Maxs of our box to trace.
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();

	//! Remaining time for a slidebox move.
	float remainingTime = 0.f;

	//! Direction to apply gravitational forces to.
	vec3_t gravityDir = vec3_zero();

	//! Slide Bounce reflection.
	float slideBounce = SLIDEBOX_CLIP_BOUNCE;
	//! The ground we're standing on, -1 if nothing.
	int32_t groundEntity = -1;
	//! Entity to skip for our box traces.
	int32_t skipEntityNumber = -1;
	//! Content Mask to trace with.
	int32_t contentMask = 0;

	// Number of touched plane normals.
	int32_t numClipPlanes = 0;
	vec3_t clipPlaneNormals[ MAX_SLIDEBOX_CLIPPING_PLANES ];

	//! Number of touched entities.
	int32_t numTouchEntities = 0;
	int32_t touchEntities[ MAX_SLIDEBOX_TOUCH_ENTITIES ];

	//! The ground plane we hit, set properly in case groundEntity >= 0
	CollisionPlane groundEntityPlane;
	//! The ground surface we hit, set properly in case groundEntity >= 0
	CollisionSurface groundEntitySurface;

	//! The plane we hit when touching other entities.
	CollisionPlane touchEntityPlanes[ MAX_SLIDEBOX_TOUCH_ENTITIES ];
	//! The surfaces we hit when touching other entities.
	CollisionSurface *touchEntitySurfaces[ MAX_SLIDEBOX_TOUCH_ENTITIES ];
};

/**
*	@brief
**/
const int32_t SG_SlideMove( SlideBoxMove *move );
