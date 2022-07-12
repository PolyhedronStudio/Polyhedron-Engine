/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#include "Shared.h"


/**
*
*	Debug Configurations: 
*
*	In order to prevent an annoying flood of information to seek through, there is a unique 
*	define to enable debug output specifically for each section of the data generation process.
*
**/
#define DEBUG_MODEL_DATA 0
#define DEBUG_MODEL_BOUNDINGBOX_DATA 0


/**
*
*
*	Human Friendly Skeletal Model Data. Cached by server and client.
*
*
**/
/**
*	@return	Game compatible skeletal model data including: animation name, 
*			and frame data, bounding box data(per frame), and joints(name, index, parentindex)
**/
void SKM_GenerateModelData(model_t* model) {
	if (!model || !model->iqmData || !model->skeletalModelData) {
		// TODO: Warn.
		
		return ;
	}

	// Get SkeletalModelData ptr.
	SkeletalModelData *skm = model->skeletalModelData;

	/**
	*	Joints:
	**/
	// Get joint names sorted for indexing.
	char *jointNames = model->iqmData->jointNames;
	std::vector<std::string> parsedJointNames{};

	if (jointNames) {
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
		skm->numberOfJoints = model->iqmData->num_joints;
	} else {
		skm->numberOfJoints = 0;
	}

	// Get our Joint data sorted out nicely.
	for (int32_t jointIndex = 0; jointIndex < model->iqmData->num_joints; jointIndex++) {
		// Get the parent joints.
		const const int32_t jointParentIndex = (jointIndex == 0 ? -1 : model->iqmData->jointParents[jointIndex]);

		// Create our joint object.
		SkeletalModelData::Joint jointData = {
			.name = parsedJointNames[jointIndex],
			.index = jointIndex,
			.parentIndex = jointParentIndex,
		};
		// Store our joint in our list.
		skm->jointMap[jointData.name] = jointData;
		if (jointData.index < 256) {
			skm->jointArray[jointData.index] = jointData;
		} else {
			// TODO: Warn.
		}

		// If we are dealing with the "root" bone, store it.
		if (jointData.name == "mixamorig8:Hips") {
			skm->rootJointIndex = jointIndex;
		}
	}
// Debug Info:
#if DEBUG_MODEL_DATA == 1
	for (int32_t i = 0; i < skm->numberOfJoints; i++) {
		SkeletalModelData::Joint *joint = &skm->jointArray[i];
		
		if (joint->parentIndex >= 0 && joint->parentIndex < skm->numberOfJoints) {
			SkeletalModelData::Joint *parentJoint = &skm->jointArray[joint->parentIndex];

			Com_DPrintf("Joint(#%i, %s): parentJoint(%i, %s)\n",
				joint->index,
				joint->name.c_str(),
				joint->parentIndex,
				parentJoint->name.c_str()
			);
		} else {
			Com_DPrintf("Joint(#%i, %s): parentJoint(none)\n",
				joint->index,
				joint->name.c_str()
			);			
		}
	}
#endif

	/**
	*	Animations:
	**/
// Debug Info:
#if DEBUG_MODEL_DATA == 2
	Com_DPrintf("Animations:\n");
#endif
	// Get our animation data sorted out nicely.
	for (uint32_t animationIndex = 0; animationIndex < model->iqmData->num_animations; animationIndex++) {
		// Raw Animation Data.
		iqm_anim_t* animationData = &model->iqmData->animations[animationIndex];

		// Game Friendly Animation Data.
		// TODO: Do proper error checking for existing keys and warn.
		SkeletalAnimation *animation = &(skm->animationMap[animationData->name] = {
			.index = animationIndex,
			.name = animationData->name,
			.startFrame = animationData->first_frame,
			.endFrame = animationData->first_frame + animationData->num_frames,
			.numFrames = animationData->num_frames,
			.frametime = BASE_FRAMETIME,
			.loopingFrames = 0,
			.forceLoop = true, //(animationData->loop == 1 ? true : false)
		 });
		// Resize our vec if needed.
		if (skm->animations.size() <= animationIndex) {
			skm->animations.resize(animationIndex + 1);
		}
		skm->animations[animationIndex] = animation;

		// Calculate distances.
		if (skm->rootJointIndex != -1 && model->iqmData && model->iqmData->poses) {
			// Start and end pose pointers.
			const iqm_transform_t *startPose = &model->iqmData->poses[ skm->rootJointIndex + ( animation->startFrame * model->iqmData->num_poses ) ];
			const iqm_transform_t *endPose = (animation->startFrame == 0 ? startPose : &model->iqmData->poses[skm->rootJointIndex + ( (animation->endFrame - 1) * model->iqmData->num_poses)] );

			// Get the start and end pose translations.
			const vec3_t startFrameTranslate	= startPose->translate;
			const vec3_t endFrameTranslate		= endPose->translate;

			// Used to store the total translation distance from startFrame to end Frame,
			// We use this in order to calculate the appropriate distance between start and end frame.
			// (Ie, assuming an animation loops, we need that.)
			vec3_t totalTranslateDistance = vec3_zero();

			// The offset between the previous processed and the current processing frame.
			vec3_t offsetFrom = vec3_zero();

			for (int32_t i = animation->startFrame; i < animation->endFrame; i++) {
				// Get the Frame Pose.
				const iqm_transform_t *framePose = &model->iqmData->poses[skm->rootJointIndex + (i * model->iqmData->num_poses)];

				// Special Case: First frame has no offset really.
				if (i == animation->startFrame) {
					//const vec3_t totalStartTranslation = endFrameTranslate - startFrameTranslate;
					// Push the total traversed frame distance.
					const vec3_t frameTranslate = offsetFrom - framePose->translate;

					animation->frameDistances.push_back( vec3_length( frameTranslate ) );

					// Push the total translation between each frame.					
					animation->frameTranslates.push_back( frameTranslate );

					// Prepare offsetFrom with the current pose's translate for calculating next frame.
					offsetFrom = framePose->translate;

					//totalTranslationSum += totalStartTranslation;
				// Special Case: Take the total offset, subtract it from the end frame, and THEN
				
				//} else if (i == animation->endFrame) {
				//	// 
				//	const vec3_t frameTranslate = startFrameTranslate - endFrameTranslate; //*offsetFrom -*/ framePose->translate;
				//	const double frameDistance = vec3_distance_squared( startFrameTranslate, endFrameTranslate ); 

				//	const vec3_t totalBackTranslation = frameTranslate - offsetFrom; //startFrameTranslate - endFrameTranslate;
				//	//const vec3_t totalBackTranslation = startFrameTranslate - endFrameTranslate;

				//	// Push the total traversed frame distance.
				//	animation->frameDistances.push_back( frameDistance );//vec3_dlength( totalBackTranslation ) );

				//	// Push the total translation between each frame.					
				//	animation->frameTranslates.push_back( totalBackTranslation );

					// Calculate the full animation distance.
					//animation->animationDistance = vec3_distance_squared( endFrameTranslate, startFrameTranslate ); 

					//totalTranslationSum += totalBackTranslation;	
				// General Case: Set the offset we'll be coming from next frame.
				} else {
						
					// Calculate translation between the two frames.
					const vec3_t translate = offsetFrom - framePose->translate;
					//const vec3_t translate = offsetFrom - framePose->translate;

					// Calculate the distance between the two frame translations.
					const double frameDistance = vec3_distance_squared( offsetFrom, framePose->translate ); //vec3_dlength( translate );

					// Push the total traversed frame distance.
					animation->frameDistances.push_back( frameDistance );

					// Push the total translation between each frame.					
					animation->frameTranslates.push_back( translate );

					// Increment our total translation sum.
					//totalTranslationSum += translate; // or offsetfrom?

					// Prepare offsetFrom with the current pose's translate for calculating next frame.
					offsetFrom = framePose->translate;
				}
			}

			// Sum up all frame distances into one single value.
			//vec3_dlength(totalTranslationSum); //0.0;
			animation->animationDistance = 0.0;
			for (auto& distance : animation->frameDistances) {
				animation->animationDistance += distance;
			}
		}

#if DEBUG_MODEL_DATA == 1
		Com_DPrintf("Animation(#%i, %s): (startFrame=%i, endFrame=%i, numFrames=%i), (loop=%s, loopFrames=%i), (animationDistance=%f):\n",
			animationIndex,
			animationData->name, //animation.name, Since, temp var and .c_str()
			animation->startFrame,
			animation->endFrame,
			animation->numFrames,
			animation->forceLoop == true ? "true" : "false",
			animation->loopingFrames,
			animation->animationDistance);

		for (int i = 0; i < animation->frameDistances.size(); i++) {
			// Debug OutPut:
			int32_t frameIndex = i;
			Com_DPrintf("	Frame(#%i): Translate=(%f,%f,%f), Distance=%f\n", 
				frameIndex,
				animation->frameTranslates[frameIndex].x,
				animation->frameTranslates[frameIndex].y,
				animation->frameTranslates[frameIndex].z,
				animation->frameDistances[frameIndex]						
			);
		}
#endif
	}

	/**
	*	Model Bounds:
	**/
// Debug info:
#if DEBUG_MODEL_BOUNDINGBOX_DATA == 1
	Com_DPrintf("BoundingBoxes:\n");
#endif
	// Get the Model Bounds for each frame.
	float *bounds = model->iqmData->bounds;

	if (bounds) {
		for (int32_t frameIndex = 01; frameIndex < model->iqmData->num_frames; frameIndex++) {
			SkeletalModelData::BoundingBox box;
			box.mins = { bounds[0], bounds[1], bounds[2] };
			box.maxs = { bounds[3], bounds[4], bounds[5] };
			bounds+= 6;
		
	// Debug Info:
#if DEBUG_MODEL_BOUNDINGBOX_DATA == 1
	//Com_DPrintf("	Frame (#%i): (mins.x=%f, mins.y=%f, mins.z=%f), (maxs.x=%f, maxs.y=%f, maxs.z=%f)\n",
	//	frameIndex,
	//	box.mins.x, box.mins.y, box.mins.z,
	//	box.maxs.x, box.maxs.y, box.maxs.z
	//);
#endif
			skm->boundingBoxes.push_back(box);
		}
	}

	// Return our data.
	//return skm;
}