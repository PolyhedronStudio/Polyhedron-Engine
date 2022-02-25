/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#include "Shared/Shared.h"
#include "SharedGame/SharedGame.h"
#include "SharedGame/SkeletalAnimation.h"


/**
* @brief	Calculates the current frame for the current time since the start time stamp.
* 
* @return   The frame and interpolation fraction for current time in an animation started at a given time.
*           When the animation is finished it will return frame -1. Takes looping into account. Looping animations
*           are never finished.
**/
float SG_FrameForTime(int32_t* frame, int32_t curTime, int32_t startTimeStamp, float frametime, int32_t firstFrame, int32_t lastframe, int32_t loopingFrames, bool forceLoop) {
    // If current time is lower than start timestamp, we're at our first frame.
    if (curTime <= startTimeStamp) {
	    *frame = firstFrame;
	    return 0.0f;
    }

    if (firstFrame == lastframe) {
	    *frame = firstFrame;
	    return 1.0f;
    }

    int32_t runningTime = curTime - startTimeStamp;
    float frameFraction = ((double)runningTime / (double)frametime);
    int32_t frameCount = (unsigned int)frameFraction;
    frameFraction -= frameCount;
        
    int32_t currentFrame = firstFrame + frameCount;
    if (currentFrame > lastframe) {
	    if (forceLoop && !loopingFrames)
	        loopingFrames = lastframe - firstFrame;

	    if (loopingFrames) {
	        unsigned int numberOfLoops;
	        unsigned int startcount;

	        startcount = (lastframe - firstFrame) - loopingFrames;

	        numberOfLoops = (frameCount - startcount) / loopingFrames;
	        currentFrame -= loopingFrames * numberOfLoops;

			if (loopingFrames == 1) {
				frameFraction = 1.0f;
			}
	    } else {
			currentFrame = -1;
	    }
    }

    *frame = currentFrame;

    return frameFraction;
}