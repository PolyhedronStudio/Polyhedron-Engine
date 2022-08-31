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
static constexpr float SLIDEBOX_STOP_EPSILON		= 0.1f;

//! The actual maximum step height a slidemove is allowed to step.
static constexpr float SLIDEBOX_STEP_HEIGHT = 18.0f;
//! SlideMove 'Minimal Step Height'. If a step exceeds it, it'll resort to NOT positioning the entity, but have it
// drop by gravity instead.
static constexpr float SLIDEBOX_STEP_HEIGHT_MIN	= 4.0f;

//! The distance between a root motion move's bounding box and the ground. (An offset we stick to, preventing getting stuck.)
static constexpr float SLIDEBOX_GROUND_DISTANCE = 0.25f;

//! Friction for ground movement.
static constexpr float SLIDEBOX_GROUND_FRICTION = 6.f;
//! Friction for water movement.
static constexpr float SLIDEBOX_WATER_FRICTION = 1.f;

//! The Stop speed, at which it decreases to a halt.
static constexpr float SLIDEBOX_STOP_SPEED = 100.f;
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
