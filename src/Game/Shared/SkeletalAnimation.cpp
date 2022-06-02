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

#define DEBUG_MODEL_DATA 1

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

	/**
	*	Model Bounds:
	**/
// Debug info:
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
	gi.DPrintf("SG_SKM_GenerateModelData(%s) -> {\n");
	gi.DPrintf("BoundingBoxes:\n");
#endif
	// Get the Model Bounds for each frame.
	float *bounds = model->iqmData->bounds;

	for (int32_t frameIndex = 1; frameIndex < model->iqmData->num_frames; frameIndex++) {
		SkeletalModelData::BoundingBox box;
		box.mins = { bounds[0], bounds[1], bounds[2] };
		box.maxs = { bounds[3], bounds[4], bounds[5] };
		bounds+= 6;
		
// Debug Info:
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
		gi.DPrintf("	Frame (#%i):	(mins.x=%f, mins.y=%f, mins.z=%f), (maxs.x=%f, maxs.y=%f, maxs.z=%f)\n",
			frameIndex,
			box.mins.x, box.mins.y, box.mins.z,
			box.maxs.x, box.maxs.y, box.maxs.z
		);
#endif
		skm.boundingBoxes.push_back(box);
	}

	/**
	*	Animations:
	**/
// Debug Info:
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
		gi.DPrintf("Animations:\n");
#endif
	// Get our animation data sorted out nicely.
	for (uint32_t animationIndex = 0; animationIndex < model->iqmData->num_animations; animationIndex++) {
		// Raw Animation Data.
		iqm_anim_t* animationData = &model->iqmData->animations[animationIndex];

		// Game Friendly Animation Data.
		// TODO: Do proper error checking for existing keys and warn.
		SkeletalAnimation animation = skm.animations[animationData->name] = {
			.index = animationIndex,
			.name = animationData->name,
			.startFrame = animationData->first_frame,
			.endFrame = animationData->first_frame + animationData->num_frames,
			.numFrames = animationData->num_frames,
			.loopingFrames = 0,
			.forceLoop = (animationData->loop == 1 ? true : false)
		};

		// Output animation data.
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
		gi.DPrintf("Animation(#%i, %s): (startFrame=%i, endFrame=%i, numFrames=%i), (loopFrames=%i, loop=%s)\n",
			animationIndex,
			animationData->name, //animation.name, Since, temp var and .c_str()
			animation.startFrame,
			animation.endFrame,
			animation.numFrames,
			animation.loopingFrames,
			animation.forceLoop == true ? "true" : "false");
#endif
	}

	/**
	*	Joints:
	**/
	// Get joint names sorted for indexing.
	char *jointNames = model->iqmData->jointNames;
	std::vector<std::string> parsedJointNames{};
	int32_t nameLength = strlen(jointNames);
	int32_t id = 0;
	while (nameLength) {
		// Push back next joint name.
		parsedJointNames.push_back(jointNames);
		// Increase index ofc.
		jointNames += nameLength + 1;
		// Fetch next name block length.
		nameLength = strlen(jointNames);
	}

	// First store our actual number of joints.
	skm.numberOfJoints = model->iqmData->num_joints;

	// Get our Joint data sorted out nicely.
	for (int32_t jointIndex = 0; jointIndex < model->iqmData->num_joints; jointIndex++) {
		// Get the parent joints.
		const int32_t jointParentIndex = (jointIndex == 0 ? -1 : model->iqmData->jointParents[jointIndex]);

		// Create our joint object.
		SkeletalModelData::Joint jointData = {
			.name = parsedJointNames[jointIndex],
			.index = jointIndex,
			.parentIndex = jointParentIndex,
		};
		// Store our joint in our list.
		skm.jointMap[jointData.name] = jointData;
		if (jointData.index < 256) {
			skm.jointArray[jointData.index] = jointData;
		} else {
			// TODO: Warn.
		}

		// If we are dealing with the "root" bone, store it.
		if (jointData.name == "root") {
			skm.rootJointIndex = jointIndex;
		}
	}
// Debug Info:
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
	for (int32_t i = 0; i < skm.numberOfJoints; i++) {
		SkeletalModelData::Joint *joint = &skm.jointArray[i];
		
		if (joint->parentIndex >= 0 && joint->parentIndex < skm.numberOfJoints) {
			SkeletalModelData::Joint *parentJoint = &skm.jointArray[joint->parentIndex];

			gi.DPrintf("Joint(#%i, %s): parentJoint(%i, %s)\n",
				joint->index,
				joint->name.c_str(),
				joint->parentIndex,
				parentJoint->name.c_str()
			);

		} else {
			gi.DPrintf("Joint(#%i, %s): parentJoint(none)\n",
				joint->index,
				joint->name.c_str()
			);			
		}
	}
#endif
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