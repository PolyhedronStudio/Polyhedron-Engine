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
*	@brief	Parsed from the modelname.cfg file. Stores specific data that
*			belongs to an animation. Such as: frametime, loop or no loop.
**/
struct SkeletalAnimation {
	/**
	*	Look-up Properties, (Name, index, IQM StartFrame, IQM EndFrame.)
	**/
	//! Actual animation look-up index.
	uint32_t index = 0;
	//! Actual animation loop-uk name.
	std::string name = "";


	/**
	*	Animation Properties.
	**/
	//! Animation start IQM frame number.
	uint32_t startFrame = 0;
	//! Animation end IQM frame number.
	uint32_t endFrame = 0;
	//! Number of times the animation has to loop before it ends.
	//! When set to '0', it'll play continuously until stopped by code.
	uint32_t loopingFrames = 0;
	//! When not set in the config file, defaults to whichever ANIMATION_FRAMETIME is set to.
	double frametime = ANIMATION_FRAMETIME;
	//! When 'true', force looping the animation. (It'll never trigger a stop event after each loop.)
	bool forceLoop = false;
};

/**
* @brief	Calculates the current frame for the current time since the start time stamp.
* 
* @return   The frame and interpolation fraction for current time in an animation started at a given time.
*           When the animation is finished it will return frame -1. Takes looping into account. Looping animations
*           are never finished. In other words, they'll never return -1.
**/
double SG_FrameForTime( int32_t *frame, const GameTime &currentTimestamp, const GameTime &startTimeStamp, float frameTime, int32_t startFrame, int32_t endFrame, int32_t loopingFrames = 0, bool forceLoop =false);