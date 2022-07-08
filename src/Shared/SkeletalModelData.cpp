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




/**
*
*
*	Temporary, these also reside in iqm.cpp and need to go to math lib.
*
*
**/
// "multiply" 3x4 matrices, these are assumed to be the top 3 rows
// of a 4x4 matrix with the last row = (0 0 0 1)
static void Matrix34Multiply(const float* a, const float* b, float* out) {
	out[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8];
	out[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9];
	out[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10];
	out[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3];
	out[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8];
	out[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9];
	out[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10];
	out[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7];
	out[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8];
	out[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9];
	out[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10];
	out[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11];
}

static void JointToMatrix(const quat_t rot, const vec3_t scale, const vec3_t trans, float* mat) {
	float xx = 2.0f * rot[0] * rot[0];
	float yy = 2.0f * rot[1] * rot[1];
	float zz = 2.0f * rot[2] * rot[2];
	float xy = 2.0f * rot[0] * rot[1];
	float xz = 2.0f * rot[0] * rot[2];
	float yz = 2.0f * rot[1] * rot[2];
	float wx = 2.0f * rot[3] * rot[0];
	float wy = 2.0f * rot[3] * rot[1];
	float wz = 2.0f * rot[3] * rot[2];

	mat[0] = scale[0] * (1.0f - (yy + zz));
	mat[1] = scale[0] * (xy - wz);
	mat[2] = scale[0] * (xz + wy);
	mat[3] = trans[0];
	mat[4] = scale[1] * (xy + wz);
	mat[5] = scale[1] * (1.0f - (xx + zz));
	mat[6] = scale[1] * (yz - wx);
	mat[7] = trans[1];
	mat[8] = scale[2] * (xz - wy);
	mat[9] = scale[2] * (yz + wx);
	mat[10] = scale[2] * (1.0f - (xx + yy));
	mat[11] = trans[2];
}

static void Matrix34Invert(const float* inMat, float* outMat) {
	outMat[0] = inMat[0]; outMat[1] = inMat[4]; outMat[2] = inMat[8];
	outMat[4] = inMat[1]; outMat[5] = inMat[5]; outMat[6] = inMat[9];
	outMat[8] = inMat[2]; outMat[9] = inMat[6]; outMat[10] = inMat[10];

	float invSqrLen, * v;
	v = outMat + 0; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 4; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 8; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);

	vec3_t trans;
	trans[0] = inMat[3];
	trans[1] = inMat[7];
	trans[2] = inMat[11];

	outMat[3] = -DotProduct(outMat + 0, trans);
	outMat[7] = -DotProduct(outMat + 4, trans);
	outMat[11] = -DotProduct(outMat + 8, trans);
}

static void QuatSlerp(const quat_t from, const quat_t _to, float fraction, quat_t out) {
	// cos() of angle
	float cosAngle = from[0] * _to[0] + from[1] * _to[1] + from[2] * _to[2] + from[3] * _to[3];

	// negative handling is needed for taking shortest path (required for model joints)
	quat_t to;
	if (cosAngle < 0.0f) 	{
		cosAngle = -cosAngle;
		to[0] = -_to[0];
		to[1] = -_to[1];
		to[2] = -_to[2];
		to[3] = -_to[3];
	}
	else 	{
		QuatCopy(_to, to);
	}

	float backlerp, lerp;
	if (cosAngle < 0.999999f) 	{
		// spherical lerp (slerp)
		const float angle = acosf(cosAngle);
		const float sinAngle = sinf(angle);
		backlerp = sinf((1.0f - fraction) * angle) / sinAngle;
		lerp = sinf(fraction * angle) / sinAngle;
	}
	else 	{
		// linear lerp
		backlerp = 1.0f - fraction;
		lerp = fraction;
	}

	out[0] = from[0] * backlerp + to[0] * lerp;
	out[1] = from[1] * backlerp + to[1] * lerp;
	out[2] = from[2] * backlerp + to[2] * lerp;
	out[3] = from[3] * backlerp + to[3] * lerp;
}

static vec_t QuatNormalize2(const quat_t v, quat_t out) {
	float length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];

	if (length > 0.f) 	{
		/* writing it this way allows gcc to recognize that rsqrt can be used */
		float ilength = 1 / sqrtf(length);
		/* sqrt(length) = length * (1 / sqrt(length)) */
		length *= ilength;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
		out[3] = v[3] * ilength;
	}
	else 	{
		out[0] = out[1] = out[2] = 0;
		out[3] = -1;
	}

	return length;
}






/**
*
*
*	Entity Skeleton
*
*
**/
/**
*	@brief	Expects the boneTree's head node to already be set pointing at the
*			root bone.
**/
const bool ES_GenerateBoneTreeHierachy( SkeletalModelData *skm, EntitySkeleton *es, EntitySkeletonBoneNode &parentNode ) {
	// Ensure we got valid pointers, oh my.
	if ( !skm || !es ) {
		// Warn?
		return false;
	}

	// Ensure we got a parent node.
	//if ( parentNode ) {
	//	// Warn.
	//	return;
	//}

	// Get the bone the parent is pointing to.
	EntitySkeletonBone *esParentBone = parentNode.GetEntitySkeletonBone();

	if ( !esParentBone ) {
		// Warn
		return false;
	}

	// Iterate over the linear bone list, see if they should be added as child nodes to the parent node.
	for ( int32_t i = 0; i < es->bones.size(); i++ ) {
		// Get a pointer for easy access.
		EntitySkeletonBone *esBone = &es->bones[i];

		// This bone has a matching bone parentIndex to our parentNode's bone index.
		if (esBone->parentIndex == esParentBone->index) {
			// Add the bone as a child to our parentNode.
			EntitySkeletonBoneNode &childNode = parentNode.AddChild( esBone );

			// Add a pointer in our bone map to the node.
			es->boneMap[esBone->name] = &childNode;

			// Generate a tree hierachy for this bone as well.
			ES_GenerateBoneTreeHierachy( skm, es, childNode );
		} else {
			//Skip this bone.
			continue;
		}
	}

	return true;
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
		EntitySkeletonBoneNode *boneNode = es->bones[i].boneTreeNode;

		// Make sure it is valid.
		if ( boneNode ) {
			// Clear children.
			boneNode->GetChildren().clear();
		}
	}

	// Clear out Linear Bone List.
	es->bones.clear();

	// Clear out Bone Map.
	es->boneMap.clear();

	/**
	*	#1:	Generate a linear bone list array for fast and easy index access.
	**/
	// Resize the skeleton's bones array to proper size.
	es->bones.resize( skm->numberOfJoints + 1);

	// Copy over specific joint data.
	for ( auto &joint : skm->jointArray ) {

		// If it is the root bone, we start creating our head tree node.
		if ( joint.index == skm->rootJointIndex ) {
			// Point to root bone index, and keep parentNode as nullptr.
			es->boneTree = EntitySkeletonBoneNode( &es->bones[joint.index], nullptr );

			// Add it to our bone map.
			es->boneMap[ joint.name ] = &es->boneTree;
		}
		// Otherwise, just fill in bone data without a tree node parent.
		else if ( joint.index >= 0 && joint.index < es->bones.size() ) {
			es->bones[joint.index] = EntitySkeletonBone {
				.name			= joint.name,
				.parentIndex	= joint.parentIndex,
				.index			= joint.index,
				.boneTreeNode	= nullptr,	// We set this after the first loop.
			};
		}
	}

	/**
	*	#2: Generate a Bone Tree Hierachy with each node pointing to a bone in the
	*		linear bone access list.
	**/
	ES_GenerateBoneTreeHierachy( skm, es, es->boneTree );

	// Done.
	return true;
}

/**
*
*
*	Animation Processing Logic.
*
*
**/
/**
*	@brief	Computes all matrices for this model, assigns the {[model->num_poses] 3x4 matrices} in the (pose_matrices) array.
*
*			Treats it no different than as if it were a regular alias model going from fram A to B. And does not make use
*			of said node tree which is stored in the entity's skeleton.
**/
void ES_StandardComputeTransforms( const model_t* model, const r_entity_t* entity, float* pose_matrices ) {
	// Temporary bone Pose.
	EntitySkeletonBonePose temporaryBonePoses[IQM_MAX_JOINTS];
	EntitySkeletonBonePose* relativeBone = temporaryBonePoses;

	// Get IQM Model Data.
	const iqm_model_t *iqmData = model->iqmData;

	// Get current frame.
	const int frame = iqmData->num_frames ? entity->frame % (int)iqmData->num_frames : 0;
	// Get old frame.
	const int oldframe = iqmData->num_frames ? entity->oldframe % (int)iqmData->num_frames : 0;

	/**
	*	#0: Compute relative bone transforms.
	**/
	// copy or lerp animation frame pose
	if (oldframe == frame) {
		const EntitySkeletonBonePose* pose = &iqmData->poses[frame * iqmData->num_poses];
		for (uint32_t pose_idx = 0; pose_idx < iqmData->num_poses; pose_idx++, pose++, relativeBone++) {
			temporaryBonePoses->translate = pose->translate;
			temporaryBonePoses->scale = pose->scale;
			QuatCopy(pose->rotate, temporaryBonePoses->rotate);
		}
	} else {
		// Get back:erp.
		const float backLerp = entity->backlerp;
		// Get Lerp.
		const float lerp = 1.0f - backLerp;

		// Pose & Old Pose.
		const EntitySkeletonBonePose* pose = &iqmData->poses[frame * iqmData->num_poses];
		const EntitySkeletonBonePose* oldpose = &iqmData->poses[oldframe * iqmData->num_poses];

		// Iterate and transform bones accordingly.
		for ( uint32_t pose_idx = 0; pose_idx < iqmData->num_poses; pose_idx++, oldpose++, pose++, relativeBone++ ) {
			relativeBone->translate[0] = oldpose->translate[0] * backLerp + pose->translate[0] * lerp;
			relativeBone->translate[1] = oldpose->translate[1] * backLerp + pose->translate[1] * lerp;
			relativeBone->translate[2] = oldpose->translate[2] * backLerp + pose->translate[2] * lerp;

			relativeBone->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
			relativeBone->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
			relativeBone->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

			QuatSlerp( oldpose->rotate, pose->rotate, lerp, relativeBone->rotate );
		}
	}

	// Compute world, and local bone transforms.
	ES_ComputeWorldPoseTransforms( model, temporaryBonePoses, pose_matrices );
	ES_ComputeLocalPoseTransforms( model, temporaryBonePoses, pose_matrices );
	//// multiply by inverse of bind pose and parent 'pose mat' (bind pose transform matrix)
	//relativeBone = relativeBonePose;
	//const int* jointParent = model->jointParents;
	//const float* invBindMat = model->invBindJoints;
	//float* poseMat = pose_matrices;
	//for ( uint32_t pose_idx = 0; pose_idx < model->num_poses; pose_idx++, relativeBone++, jointParent++, invBindMat += 12, poseMat += 12 ) {
	//	float mat1[12], mat2[12];

	//	JointToMatrix( relativeBone->rotate, relativeBone->scale, relativeBone->translate, mat1 );

	//	if (*jointParent >= 0) {
	//		Matrix34Multiply( &model->bindJoints[(*jointParent) * 12], mat1, mat2 );
	//		Matrix34Multiply( mat2, invBindMat, mat1 );
	//		Matrix34Multiply( &pose_matrices[(*jointParent) * 12], mat1, poseMat );
	//	} else {
	//		Matrix34Multiply( mat1, invBindMat, poseMat );
	//	}
	//}

	//return true;
}

/**
*	@brief	Computes the LERP Pose result for in-between the old and current frame by calculating each 
*			relative transform for all bones.
*
*			
**/
void ES_LerpSkeletonPoses( const model_t *model, const int32_t rootBoneAxisFlags, int32_t currentFrame, int32_t oldFrame, float lerp, float backLerp, EntitySkeletonBonePose *outBonePose ) {
	// Get pointers to needed model data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	const SkeletalModelData *skmData = model->skeletalModelData;
	// Relative Joints.
	EntitySkeletonBonePose* relativeBonePose = outBonePose;

	// Sanity Checks:
	if (!iqmModel)	{ /* Todo: Warn */ return; }
	if (!skmData)	{ /* Todo: Warn */ return; }
	if (!relativeBonePose) { /* Todo: Warn */ return; }

	// Current Frame.
	//currentFrame = iqmModel->num_frames ? (currentFrame % (int) iqmModel->num_frames) : 0;
	if (currentFrame > iqmModel->num_frames || currentFrame < 0) {
		currentFrame = 0;
		// Todo: Warn.
	}
	
	// Old Frame.
	//oldFrame = iqmModel->num_frames ? (oldFrame % (int) iqmModel->num_frames) : 0;
	if (oldFrame > iqmModel->num_frames || oldFrame < 0) {
		oldFrame = 0;
		// Todo: Warn
	}

	// Copy or lerp animation currentFrame pose
	if (oldFrame == currentFrame) {
		const EntitySkeletonBonePose* pose = &iqmModel->poses[currentFrame * iqmModel->num_poses];
		for (uint32_t poseIndex = 0; poseIndex < iqmModel->num_poses; poseIndex++, pose++, relativeBonePose++) {
			// Do we have skeletal model data?
			if (skmData && poseIndex == skmData->rootJointIndex) {
				// Copy over the pose's translation.
				relativeBonePose->translate = pose->translate;

				// See whether to cancel/zero out any axis.
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
					relativeBonePose->translate.x = 0.0;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
					relativeBonePose->translate.y = 0.0;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
					relativeBonePose->translate.z = 0.0;
				}

				// Copy over the pose's scale.
				relativeBonePose->scale = pose->scale;
				// Copy over the pose's rotation.
				QuatCopy( pose->rotate, relativeBonePose->rotate );
				
				// Skip regular treatment.
				continue;
			}

			// Copy over the pose's translation.
			relativeBonePose->translate = pose->translate;
			// Copy over the pose's scale.
			relativeBonePose->scale = pose->scale;
			// Copy over the pose's rotation.
			QuatCopy( pose->rotate, relativeBonePose->rotate );
		}

	} else {
		const EntitySkeletonBonePose* pose = &iqmModel->poses[currentFrame * iqmModel->num_poses];
		const EntitySkeletonBonePose* oldpose = &iqmModel->poses[oldFrame * iqmModel->num_poses];
		for (uint32_t poseIndex = 0; poseIndex < iqmModel->num_poses; poseIndex++, oldpose++, pose++, relativeBonePose++)
		{
			// Do we have skeletal model data?
			if (skmData && poseIndex == skmData->rootJointIndex) {
				
				// Cancel out if the Zero*Translation flag is set, otherwise, calculate translation for that axis.
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
					relativeBonePose->translate.x = 0.0;
				} else {
					relativeBonePose->translate.x = oldpose->translate.x * backLerp + pose->translate.x * lerp;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
					relativeBonePose->translate.y = 0.0;
				} else {
					relativeBonePose->translate.y = oldpose->translate.y * backLerp + pose->translate.y * lerp;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
					relativeBonePose->translate.z = 0.0;
				} else {
					relativeBonePose->translate.z = oldpose->translate.z * backLerp + pose->translate.z * lerp;
				}
				
				// Calculate the Joint's pose scale.				
				relativeBonePose->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
				relativeBonePose->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
				relativeBonePose->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

				// Copy over the pose's rotation.
				//QuatCopy(pose->rotate, relativeBone->rotate);
				// Slerp rotation.
				QuatSlerp( oldpose->rotate, pose->rotate, lerp, relativeBonePose->rotate );

				// Skip regular treatment.
				continue;
			}

			// Calculate translation.
			relativeBonePose->translate[0] = oldpose->translate[0] * backLerp + pose->translate[0] * lerp;
			relativeBonePose->translate[1] = oldpose->translate[1] * backLerp + pose->translate[1] * lerp;
			relativeBonePose->translate[2] = oldpose->translate[2] * backLerp + pose->translate[2] * lerp;

			// Scale.
			relativeBonePose->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
			relativeBonePose->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
			relativeBonePose->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

			// Slerp rotation.
			QuatSlerp( oldpose->rotate, pose->rotate, lerp, relativeBonePose->rotate );
		}
	}
}

// Apply Flags to Root Bone relatives for use after applying animations.
void ES_ApplyRootBoneAxisFlags( const model_t* model, const int32_t rootBoneAxisFlags, const int32_t rootBoneNumber, EntitySkeletonBonePose* bonePoses, float fraction, float lerp, float backlerp ) {
	
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;
	
	if (!iqmModel) {
		return;
	}
			
	// Do we have skeletal model data?
	if (skmData && rootBoneNumber >= 0) {
		// Get Root Bone Pose Transform.
		EntitySkeletonBonePose *rootBone = bonePoses + rootBoneNumber;

		// Cancel out if the Zero*Translation flag is set, otherwise, calculate translation for that axis.
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
			rootBone->translate.x = 0.0;
		}
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
			rootBone->translate.y = 0.0;
		}
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
			rootBone->translate.z = 0.0;
		}
	}
}

/**
*	@brief	Combine 2 poses into one by performing a recursive blend starting from the given boneNode, using the given fraction as "intensity".
*	@param	fraction		When set to 1.0, it blends in the animation at 100% intensity. Take 0.5 for example, 
*							and a tpose(frac 0.5)+walk would have its arms half bend.
*	@param	addBonePose		The actual animation that you want to blend in on top of inBonePoses.
*	@param	addToBonePose	A lerped bone pose which we want to blend addBonePoses animation on to.
**/
void ES_RecursiveBlendFromBone( const model_t *model, EntitySkeletonBonePose *addBonePoses, EntitySkeletonBonePose* addToBonePoses, int32_t boneNumber, float fraction, float lerp, float backlerp ) {
	// Get 
	if (boneNumber >= 0) {
		iqm_transform_t *inBone = addBonePoses + boneNumber;
		iqm_transform_t *outBone = addToBonePoses + boneNumber;

		if (fraction == 1) {
			*inBone = *outBone;
		} else {
			//
			//	WID: Unsure if actually lerping these is favored.
			//
			// Lerp the already Lerped Translation.
			//outBone->translate[0] = outBone->translate[0] * backlerp + inBone->translate[0] * lerp;
			//outBone->translate[1] = outBone->translate[1] * backlerp + inBone->translate[1] * lerp;
			//outBone->translate[2] = outBone->translate[2] * backlerp + inBone->translate[2] * lerp;

			// Lerp the already Lerped Scale.
			//outBone->scale[0] = outBone->scale[0] * backlerp + inBone->scale[0] * lerp;
			//outBone->scale[1] = outBone->scale[1] * backlerp + inBone->scale[1] * lerp;
			//outBone->scale[2] = outBone->scale[2] * backlerp + inBone->scale[2] * lerp;
			
			// Copy Translation.
			outBone->translate = inBone->translate;

			// Copy Scale.
			outBone->scale = inBone->scale; //vec3_scale(inBone->scale, 1.175);

			// Slerp the rotation at fraction.	
			QuatSlerp(outBone->rotate, inBone->rotate, fraction, outBone->rotate);
		}
	}
	
	//
	// This badly needs a bone hierachy instead of this crap lmao.
	//
	if (model->iqmData) {
		auto *iqmData = model->iqmData;

		for ( int32_t jointIndex = 0; jointIndex < iqmData->num_joints; jointIndex++ ) {
			const int32_t parentIndex = model->iqmData->jointParents[jointIndex];

			if (parentIndex >= 0 && parentIndex == boneNumber) {
				ES_RecursiveBlendFromBone( model, addBonePoses, addToBonePoses, jointIndex, fraction, lerp, backlerp );
			}
		}
	}
}

/**
*	@brief	Compute local space matrices for the given pose transformations.
*			This is enough to work with the pose itself. For rendering it needs
*			an extra computing of its additional World Pose Transforms.
**/
void ES_ComputeLocalPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices ) {
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;
	
	// multiply by inverse of bind pose and parent 'pose mat' (bind pose transform matrix)
	const iqm_transform_t *relativeJoint = bonePoses;
	const int* jointParent = iqmModel->jointParents;
	const float* invBindMat = iqmModel->invBindJoints;
	float* poseMat = poseMatrices;
	for (uint32_t pose_idx = 0; pose_idx < iqmModel->num_poses; pose_idx++, relativeJoint++, jointParent++, invBindMat += 12, poseMat += 12) {
		float mat1[12], mat2[12];

		JointToMatrix(relativeJoint->rotate, relativeJoint->scale, relativeJoint->translate, mat1);

		if (*jointParent >= 0) {
			Matrix34Multiply(&iqmModel->bindJoints[(*jointParent) * 12], mat1, mat2);
			Matrix34Multiply(mat2, invBindMat, mat1);
			Matrix34Multiply(&poseMatrices[(*jointParent) * 12], mat1, poseMat);
		} else {
			Matrix34Multiply(mat1, invBindMat, poseMat);
		}
	}
}

/**
*	@brief	Compute world space matrices for the given pose transformations.
**/
void ES_ComputeWorldPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices ) {
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;

	ES_ComputeLocalPoseTransforms(model, bonePoses, poseMatrices);

	float *poseMat = iqmModel->bindJoints;
	float *outPose = poseMatrices;

	for (size_t i = 0; i < iqmModel->num_poses; i++, poseMat += 12, outPose += 12) {
		float inPose[12];
		memcpy(inPose, outPose, sizeof(inPose));
		Matrix34Multiply(inPose, poseMat, outPose);
	}
}