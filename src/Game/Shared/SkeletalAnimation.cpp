/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#include "SharedGame.h"


/**
*	@return	Game compatible IQM data.
**/
SkeletalModelData SG_SKM_GenerateModelData(model_t* model) {
	// Our final model data to return.
	SkeletalModelData skm;

	if (!model) {
		// TODO: Warn.
		return skm;
	}

	// Get our animation data sorted out nicely.
	for (uint32_t animationIndex = 0; animationIndex < model->iqmData->num_animations; animationIndex++) {
		iqm_anim_t* animationData = &model->iqmData->animations[animationIndex];

		// TODO: Do proper error checking for existing keys and warn.
		skm.animations[animationData->name] = {
			.index = animationIndex,
			.name = animationData->name,
			.startFrame = animationData->first_frame,
			.endFrame = animationData->num_frames,
			.loopingFrames = 0,
			.forceLoop = (animationData->loop == 1 ? true : false),
		};
		//const char* name = (const char*)header + header->ofs_text + src->name;
		//strncpy(dst->name, name, sizeof(dst->name));
		//dst->name[sizeof(dst->name) - 1] = 0;

		//dst->first_frame = src->first_frame;
		//dst->num_frames = src->num_frames;
		////dst->framerate = src->framerate;
		//dst->loop = (src->flags & IQM_LOOP) != 0;
	}

	// Return our data.
	return skm;
}

/**
* @brief	Calculates the current frame for the current time since the start time stamp.
* 
* @return   The frame and interpolation fraction for current time in an animation started at a given time.
*           When the animation is finished it will return frame -1. Takes looping into account. Looping animations
*           are never finished.
**/
double SG_FrameForTime( int32_t *frame, const GameTime &currentTimestamp, const GameTime &startTimeStamp, float frameTime, int32_t startFrame, int32_t endFrame, int32_t loopingFrames, bool forceLoop ) {
	// Always set frame to start frame if the current time stamp is lower than the animation start timestamp.
	if( currentTimestamp <= startTimeStamp ) {
		*frame = startFrame;
		return 0.0f;
	}

	// If start frame == end frame, it means we have no animation to begin with, return 1.0 fraction and set frame to startframe.
	if( startFrame == endFrame ) {
		*frame = startFrame;
		return 1.0f;
	}

	// Calculate current amount of time this animation is running for.
	GameTime runningTime = currentTimestamp - startTimeStamp;

	// Calculate frame fraction.
	double frameFraction = ( runningTime / frameTime ).count();
	int64_t frameCount = (int64_t)frameFraction;
	frameFraction -= frameCount;

	// Calculate current frame.
	uint32_t currentFrame = startFrame + frameCount;

	// If current frame is higher than last frame...
	if( currentFrame > endFrame ) {
		if( forceLoop && !loopingFrames ) {
			loopingFrames = endFrame - startFrame;
		}

		if( loopingFrames ) {
			// Calculate current loop start #.
			uint32_t startCount = ( endFrame - startFrame ) - loopingFrames;

			// Calculate the number of loops left to do.
			uint32_t numberOfLoops = ( frameCount - startCount ) / loopingFrames;

			// Calculate current frame.
			currentFrame -= loopingFrames * numberOfLoops;

			// Special frame fraction handling to play an animation just once.
			if( loopingFrames == 1 ) {
				frameFraction = 1.0;
			}
		} else {
			// Animation's finished, set current frame to -1 and get over with it.
			currentFrame = -1;
		}
	}

	// Assign new frame value.
	*frame = currentFrame;

	// Return frame fraction.
	return frameFraction;
}