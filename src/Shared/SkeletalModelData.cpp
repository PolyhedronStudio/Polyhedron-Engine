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

#define DEBUG_MODEL_DATA 1

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
		const int32_t jointParentIndex = (jointIndex == 0 ? -1 : model->iqmData->jointParents[jointIndex]);

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
		if (jointData.name == "root") {
			skm->rootJointIndex = jointIndex;
		}
	}
// Debug Info:
#if DEBUG_MODEL_DATA
	for (int32_t i = 0; i < skm->numberOfJoints; i++) {
		SkeletalModelData::Joint *joint = &skm->jointArray[i];
		
		if (joint->parentIndex >= 0 && joint->parentIndex < skm->numberOfJoints) {
			SkeletalModelData::Joint *parentJoint = &skm->jointArray[joint->parentIndex];

			//Com_DPrintf("Joint(#%i, %s): parentJoint(%i, %s)\n",
			//	joint->index,
			//	joint->name.c_str(),
			//	joint->parentIndex,
			//	parentJoint->name.c_str()
			//);

		} else {
			//Com_DPrintf("Joint(#%i, %s): parentJoint(none)\n",
			//	joint->index,
			//	joint->name.c_str()
			//);			
		}
	}
#endif

	/**
	*	Animations:
	**/
// Debug Info:
#if DEBUG_MODEL_DATA
		//Com_DPrintf("Animations:\n");
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
			for (int32_t i = animation->startFrame; i <= animation->endFrame; i++) {
				const iqm_transform_t *pose = &model->iqmData->poses[skm->rootJointIndex + (i * model->iqmData->num_poses)];

				//if (absolute) {
					//animation.frameDistances.push_back(pose->translate);
				//} else {
					vec3_t offsetFrom;
					vec3_t translate;
					VectorSubtract(offsetFrom, pose->translate, translate);
					VectorCopy(pose->translate, offsetFrom);
					animation->frameDistances.push_back(vec3_length(translate));
					animation->frameTranslates.push_back(translate);
				//}
			}
		}
		// Output animation data.
#if SHAREDGAME_SERVERGAME && DEBUG_MODEL_DATA
		Com_DPrintf("Animation(#%i, %s): (startFrame=%i, endFrame=%i, numFrames=%i), (loopFrames=%i, loop=%s)\n",
			animationIndex,
			animationData->name, //animation.name, Since, temp var and .c_str()
			animation->startFrame,
			animation->endFrame,
			animation->numFrames,
			animation->loopingFrames,
			animation->forceLoop == true ? "true" : "false");
#endif
	}

	/**
	*	Model Bounds:
	**/
// Debug info:
#if DEBUG_MODEL_DATA
	//Com_DPrintf("SG_SKM_GenerateModelData(%s) -> {\n");
	//Com_DPrintf("BoundingBoxes:\n");
#endif
	// Get the Model Bounds for each frame.
	float *bounds = model->iqmData->bounds;

	if (bounds) {
		for (int32_t frameIndex = 1; frameIndex < model->iqmData->num_frames; frameIndex++) {
			SkeletalModelData::BoundingBox box;
			box.mins = { bounds[0], bounds[1], bounds[2] };
			box.maxs = { bounds[3], bounds[4], bounds[5] };
			bounds+= 6;
		
	// Debug Info:
	#if DEBUG_MODEL_DATA
			//Com_DPrintf("	Frame (#%i):	(mins.x=%f, mins.y=%f, mins.z=%f), (maxs.x=%f, maxs.y=%f, maxs.z=%f)\n",
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