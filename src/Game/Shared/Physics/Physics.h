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

/**
*	SlideBox Configuration.
**/
//! Maximum amount of planes to clip against when executing a SlideBox movement.
static constexpr int32_t MAX_SLIDEMOVE_CLIP_PLANES = 16;
//! Maximum amount of entities that can be touched at the same time.
static constexpr int32_t MAX_SLIDEMOVE_TOUCH = 32;


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
	ISharedGameEntity *groundEntity = nullptr;

	ISharedGameEntity *passEntity = nullptr;
	int32_t contentMask = 0;

	int32_t numClipPlanes = 0;
	vec3_t clipPlaneNormals[MAX_SLIDEMOVE_CLIP_PLANES];

	int32_t numTouchEntities = 0;
	ISharedGameEntity *touchEntites[MAX_SLIDEMOVE_TOUCH];
};