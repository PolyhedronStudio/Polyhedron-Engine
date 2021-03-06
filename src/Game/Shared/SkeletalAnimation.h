/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#pragma once



//! MS Frametime for animations.
static constexpr float ANIMATION_FRAMETIME = BASE_FRAMETIME;//FRAMERATE_MS;


/**
* @brief	Calculates the current frame for the current time since the start time stamp.
* 
* @return   The frame and interpolation fraction for current time in an animation started at a given time.
*           When the animation is finished it will return frame -1. Takes looping into account. Looping animations
*           are never finished. In other words, they'll never return -1.
**/
double SG_FrameForTime( int32_t *frame, const GameTime &currentTimestamp, const GameTime &startTimeStamp, float frameTime, int32_t startFrame, int32_t endFrame, int32_t loopingFrames = 0, bool forceLoop =false);