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
*	@return	Clipped by normal velocity.
**/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce );

/*
* GS_SlideMove
*/
int32_t SG_SlideMove( MoveState *moveState );