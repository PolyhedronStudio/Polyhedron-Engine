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
			.loopingFrames = 0,
			.frametime = BASE_FRAMETIME,
			.forceLoop = true, //(animationData->loop == 1 ? true : false)
		});
		// Resize our vec if needed.
		if (skm->animations.size() <= animationIndex) {
			skm->animations.resize(animationIndex + 1);
		}
		skm->animations[animationIndex] = animation;

		// Calculate distances.
		if (skm->rootJointIndex != -1 && model->iqmData && model->iqmData->poses) {
			// The offset between frames.
			vec3_t offsetFrom = vec3_zero();

			// Used to store the total translation distance from startFrame to end Frame,
			// We use this in order to calculate the appropriate distance between start and end frame.
			// (Ie, assuming an animation loops, we need that.)
			vec3_t totalTranslateDistance = vec3_zero();

			// Start and end pose pointers.
			const iqm_transform_t *startPose = &model->iqmData->poses[skm->rootJointIndex + (animation->startFrame * model->iqmData->num_poses)];
			const iqm_transform_t *endPose = &model->iqmData->poses[skm->rootJointIndex + ((animation->endFrame - animation->startFrame) * model->iqmData->num_poses)];

			// Get the start and end pose translations.
			const vec3_t startFrameTranslate	= startPose->translate;
			const vec3_t endFrameTranslate		= endPose->translate;

			// We count(sum) the final total translation of all frames between start and end so we can use
			// this to calculate their translational offsets with. (Otherwise we'd get minmally 2 frames where
			// no motion takes place, this looks choppy.) 
			vec3_t totalTranslation = vec3_zero();
			for (int32_t i = animation->startFrame; i <= animation->endFrame; i++) {
				// Get the Frame Pose.
				const iqm_transform_t *framePose = &model->iqmData->poses[skm->rootJointIndex + (i * model->iqmData->num_poses)];

				// Special Case: First frame has no offset really.
				if (i == animation->startFrame) {
					const vec3_t totalStartTranslation = startFrameTranslate; //- endFrameTranslate;
					// Push the total traversed frame distance.
					animation->frameDistances.push_back( vec3_dlength( totalStartTranslation ) );

					// Push the total translation between each frame.					
					animation->frameTranslates.push_back( totalStartTranslation );
				// Special Case: Take the total offset, subtract it from the end frame, and THEN
				} else if (i == animation->endFrame) {
					// 
					const vec3_t totalBackTranslation = startFrameTranslate - endFrameTranslate;
						
					// Push the total traversed frame distance.
					animation->frameDistances.push_back( vec3_dlength( totalBackTranslation ) );

					// Push the total translation between each frame.					
					animation->frameTranslates.push_back( totalBackTranslation );
				// General Case: Set the offset we'll be coming from next frame.
				} else {
						
					// Calculate translation between the two frames.
					const vec3_t translate = offsetFrom - framePose->translate;

					// Add it to our sum.
					totalTranslation += translate; // or offsetfrom?

					// Push the total traversed frame distance.
					animation->frameDistances.push_back(vec3_dlength(translate));

					// Push the total translation between each frame.					
					animation->frameTranslates.push_back(translate);

					// Store our new offsetFrom to the current pose's translate.
					offsetFrom = framePose->translate;
				}
			}
		}

		// Sum up all frame distances into one single value.
		animation->animationDistance = 0.0;
		for (auto& distance : animation->frameDistances) {
			animation->animationDistance += distance;
		}
#if DEBUG_MODEL_DATA == 1
		Com_DPrintf("Animation(#%i, %s): (startFrame=%i, endFrame=%i, numFrames=%i), (loop=%s, loopFrames=%i), (animationDistance=%d):\n",
			animationIndex,
			animationData->name, //animation.name, Since, temp var and .c_str()
			animation->startFrame,
			animation->endFrame,
			animation->numFrames,
			animation->forceLoop == true ? "true" : "false",
			animation->loopingFrames,
			animation->animationDistance);
#endif
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
	}

	/**
	*	Model Bounds:
	**/
// Debug info:
#if DEBUG_MODEL_DATA == 1
	Com_DPrintf("BoundingBoxes:\n");
#endif
	// Get the Model Bounds for each frame.
	float *bounds = model->iqmData->bounds;

	if (bounds) {
		for (int32_t frameIndex = 0; frameIndex < model->iqmData->num_frames; frameIndex++) {
			SkeletalModelData::BoundingBox box;
			box.mins = { bounds[0], bounds[1], bounds[2] };
			box.maxs = { bounds[3], bounds[4], bounds[5] };
			bounds+= 6;
		
	// Debug Info:
#if DEBUG_MODEL_DATA == 1
	Com_DPrintf("	Frame (#%i): (mins.x=%f, mins.y=%f, mins.z=%f), (maxs.x=%f, maxs.y=%f, maxs.z=%f)\n",
		frameIndex,
		box.mins.x, box.mins.y, box.mins.z,
		box.maxs.x, box.maxs.y, box.maxs.z
	);
#endif
			skm->boundingBoxes.push_back(box);
		}
	}

	// Return our data.
	//return skm;
}



/**
*
*
*	Entity Skeleton
*
*
**/
/**
*	@brief	Generates a Linear Bone List, as well as a BoneNode Tree Hierachy, using the current
*			actual SkeletalModelData.
**/
static void CreateBoneTreeNode(SkeletalModelData* skm, EntitySkeleton* es, int32_t boneNumber, EntitySkeletonBoneNode *parentNode = nullptr ) {
	// Go over all joints, if the parent number matches, enlist it in our bone children array.
	//int32_t boneChildren[SKM_MAX_JOINTS];
	//int32_t childCount = 0;

	//for (int32_t jointIndex = 0; jointIndex < es->bones.size(); jointIndex++) {
	//	// If the parent number matches.
	//	if (es->bones[jointIndex].parentNumber == boneNumber) {
	//		// Store the jointIndex in our list of found children.
	//		boneChildren[childCount] = jointIndex;

	//		// Next up, increment child count.
	//		childCount++;
	//	}
	//}
	// Pointer to the actual Bone Tree Node.
	EntitySkeletonBoneNode *boneTreeNode = ( parentNode ? parentNode : &es->boneTree );

	//// If it's not the first bone, get the parent node, push back a new node, 
	//// and assign it to our boneNode pointer.
	//if (!parentNode) {
	//	const int32_t parentBoneNumber = es->bones[boneNumber].parentNumber;
	//	// Get parent node.
	//	EntitySkeletonBoneNode *boneTreeParentNode = es->bones[parentBoneNumber].boneNode;


	//	// Assign pointer to child node.
	//	boneTreeNode = &boneTreeParentNode->boneChildren.back();
	//}

	// Assign Bone Number to the Bone Tree Node.
	boneTreeNode->boneNumber = boneNumber;

	// Now we can assign the node's pointer to that of our bones linear list.
	if (boneNumber != -1) {
		es->bones[boneNumber].boneNode = boneTreeNode;
	}

	// Create a node tree list for each of our child bones.
	//if (childCount) {
	//	for (int32_t childIndex = 0; childIndex < childCount; childIndex++) {
	//		// Recursively create a node tree for each child's own children.
	//		CreateBoneTreeNode( skm, es, boneChildren[childIndex] );
	//	}
	//}
	// Go over all joints, if the parent number matches, enlist it in our bone children array.
	for (int32_t jointIndex = 0; jointIndex < es->bones.size(); jointIndex++) {
		// If the parent number matches.
		const int32_t parentNumber = es->bones[jointIndex].parentNumber;

		if (parentNumber >= 0 && parentNumber == boneNumber) {
			// So we can add a node to its children that points to the current bone.
			boneTreeNode->boneChildren.push_back({
				.boneNumber = boneNumber
			});

			// Create a tree for the child bone.
			CreateBoneTreeNode( skm, es, boneNumber, boneTreeNode );
			//// Store the jointIndex in our list of found children.
			//boneChildren[childCount] = jointIndex;

			//// Next up, increment child count.
			//childCount++;
		}
	}
}

/**
*	@brief	Sets up an entity skeleton using the specified skeletal model data.
**/
const bool SKM_CreateEntitySkeletonFrom( SkeletalModelData* skm, EntitySkeleton* es ) {
	// Ensure we got valid pointers, oh my.
	if ( !skm || !es ) {
		// Warn?
		return false;
	}

	/**
	*	#0: Clear any old skeleton data if there was any.
	**/
	// Scan our linear bone list and check each bone's boneNode for children, and clear those out.
	for ( int32_t i = 0; i < es->bones.size(); i++ ) {
		// Get boneNode pointer.
		EntitySkeletonBoneNode *boneNode = es->bones[i].boneNode;

		// Make sure it is valid.
		if ( boneNode ) {
			// Clear children.
			boneNode->boneChildren.clear();
			// Reset bone number.
			boneNode->boneNumber = -1;
		}
	}

	// Clear out Linear Bone List.
	es->bones.clear();

	/**
	*	#1: Generate a new bones list by copying over needed joint information to build our skeleton with.
	**/
	// Resize the skeleton's bones array to proper size.
	es->bones.resize( skm->numberOfJoints );

	// Copy over specific joint data.
	for ( auto &joint : skm->jointArray ) {
		if ( joint.index < es->bones.size() ) {
			es->bones[joint.index] = {
				.name = joint.name,
				.parentNumber = joint.parentIndex,
				.boneNode = nullptr,
			};
		}
	}

	/**
	*	#2: Generate the BoneNode Tree Hierachy for our Linear Bone List and set their node pointers.
	**/
	CreateBoneTreeNode( skm, es, skm->rootJointIndex );

	// Done.
	return true;
}