/***
*
*	License here.
*
*	@file
*
*	Entity Skeleton Common Implementation: These are internal to the engine, and need to be supplied by wrapping up
*	in the Game APIs.
*
***/
#include "Shared/Shared.h"
#include "Shared/Refresh.h"

//! Shared Entity Skeleton Declarations.
#include "Shared/EntitySkeleton.h"

//! Common Internal Skeleton Declarations.
#include "Common/EntitySkeleton.h"



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
*	@brief	Expects the boneTree's head node to already be set pointing at the
*			root bone.
**/
const bool ES_GenerateBoneTreeHierachy( const SkeletalModelData *skm, EntitySkeleton *es, EntitySkeletonBoneNode &parentNode ) {
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
const bool ES_CreateFromModel( model_t *model, EntitySkeleton* es ) {
	// Ensure we got valid pointers, oh my.
	if ( !model ) {
		// TODO: Warn.
		return false;
	}
	if ( !model->skeletalModelData ) {
		// TODO: Warn
		return false;
	}
	if ( !es ) {
		// TODO: Warn.
		return false;
	}
	
	// Get pointer to skeletal model data.
	SkeletalModelData *skm = model->skeletalModelData;

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
	es->bones.resize(0);

	// Clear out Bone Map.
	es->boneMap.clear();

	/**
	*	#1:	Store model pointer, and generate a linear bone list array for fast and easy index access.
	**/
	es->modelPtr = model;
	
	// In case there is literally no joints at all in this skm data, we're done.
	if (skm->numberOfJoints <= 0) {
		es->modelPtr  = nullptr;
		es->bones.resize(0);
		return false;
	}


	// Resize the skeleton's bones array to proper size.
	es->bones.resize( (skm->numberOfJoints + 1) );

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
*	@brief	Computes all matrices for this model, assigns the {[model->num_poses] 3x4 matrices} in the (pose_matrices) array.
*
*			Treats it no different than as if it were a regular alias model going from fram A to B. And does not make use
*			of said node tree which is stored in the entity's skeleton.
**/
void ES_StandardComputeTransforms( const model_t* model, const r_entity_t* entity, float* pose_matrices ) {
	// Temporary bone Pose.
	static EntitySkeletonBonePose temporaryBonePoses[IQM_MAX_JOINTS];

	EntitySkeleton tempEs;
	tempEs.modelPtr = const_cast<model_t*>(model);

	ES_LerpSkeletonPoses( &tempEs, temporaryBonePoses, entity->frame, entity->oldframe, entity->backlerp, entity->rootBoneAxisFlags );
	//// Relative bone we're computing for, pointing to an index in our temporary bone poses.
	//EntitySkeletonBonePose* relativeBone = temporaryBonePoses;

	//// Get IQM Model Data.
	//const iqm_model_t *iqmData = model->iqmData;

	//// Get current frame.
	//const int frame = iqmData->num_frames ? entity->frame % (int)iqmData->num_frames : 0;
	//// Get old frame.
	//const int oldframe = iqmData->num_frames ? entity->oldframe % (int)iqmData->num_frames : 0;

	///**
	//*	#0: Compute relative bone transforms.
	//**/
	//// copy or lerp animation frame pose
	//if (oldframe == frame) {
	//	EntitySkeletonBonePose* pose = &iqmData->poses[frame * iqmData->num_poses];
	//	for (uint32_t pose_idx = 0; pose_idx < iqmData->num_poses; pose_idx++, pose++, relativeBone++) {
	//		relativeBone->translate = pose->translate;
	//		relativeBone->scale = pose->scale;
	//		QuatCopy(pose->rotate, relativeBone->rotate);
	//	}
	//} else {
	//	// Get back:erp.
	//	const float backLerp = entity->backlerp;
	//	// Get Lerp.
	//	const float lerp = 1.0f - backLerp;

	//	// Pose & Old Pose.
	//	EntitySkeletonBonePose* pose = &iqmData->poses[frame * iqmData->num_poses];
	//	EntitySkeletonBonePose* oldpose = &iqmData->poses[oldframe * iqmData->num_poses];

	//	// Iterate and transform bones accordingly.
	//	for ( uint32_t pose_idx = 0; pose_idx < iqmData->num_poses; pose_idx++, oldpose++, pose++, relativeBone++ ) {
	//		relativeBone->translate[0] = oldpose->translate[0] * backLerp + pose->translate[0] * lerp;
	//		relativeBone->translate[1] = oldpose->translate[1] * backLerp + pose->translate[1] * lerp;
	//		relativeBone->translate[2] = oldpose->translate[2] * backLerp + pose->translate[2] * lerp;

	//		relativeBone->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
	//		relativeBone->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
	//		relativeBone->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

	//		QuatSlerp( oldpose->rotate, pose->rotate, lerp, relativeBone->rotate );
	//	}
	//}

	// Compute world, and local bone transforms.
	ES_ComputeWorldPoseTransforms( model, temporaryBonePoses, pose_matrices );
	ES_ComputeLocalPoseTransforms( model, temporaryBonePoses, pose_matrices );
}

/**
*	@brief	Computes the LERP Pose result for in-between the old and current frame by calculating each 
*			relative transform for all bones.
*
*			
**/
void ES_LerpSkeletonPoses( EntitySkeleton *entitySkeleton, EntitySkeletonBonePose *outBonePose, int32_t currentFrame, int32_t oldFrame, float backLerp, const int32_t rootBoneAxisFlags ) {
	// Get model pointer.
	const model_t *model = entitySkeleton->modelPtr;

	if (!model) {
		// TODO: Warn.
		return;
	}

	// Get pointers to needed model data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	const SkeletalModelData *skmData = model->skeletalModelData;

	// Sanity Checks:
	if ( !iqmModel )	{ /* Todo: Warn */ return; }
	if ( !skmData )		{ /* Todo: Warn */ return; }
	if ( !outBonePose ) { /* Todo: Warn */ return; }

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

	// Relative Joints.
	EntitySkeletonBonePose *relativeBonePose = outBonePose;

	// Calculate lerp by backlerp.
	const float lerp = 1.0f - backLerp;

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

			continue;
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

/**
*	@brief	Combine 2 poses into one by performing a recursive blend starting from the given boneNode, using the given fraction as "intensity".
*	@param	fraction		When set to 1.0, it blends in the animation at 100% intensity. Take 0.5 for example, 
*							and a tpose(frac 0.5)+walk would have its arms half bend.
*	@param	addBonePose		The actual animation that you want to blend in on top of inBonePoses.
*	@param	addToBonePose	A lerped bone pose which we want to blend addBonePoses animation on to.
**/
void ES_RecursiveBlendFromBone( EntitySkeletonBonePose *addBonePoses, EntitySkeletonBonePose* addToBonePoses, EntitySkeletonBoneNode *boneNode, float backlerp, float fraction ) {
	// If the bone node is invalid, escape.
	if ( !boneNode ) {
		// TODO: Warn.
		return;
	}

	// Get the bone.
	const EntitySkeletonBone *esBone = boneNode->GetEntitySkeletonBone();
	
	// If the bone is invalid, escape.
	if ( !esBone ) {
		// TODO: Warn.
		return;
	}
	
	// Get bone number.
	const int32_t boneNumber = esBone->index;

	if (esBone->index >= 0) {
		EntitySkeletonBonePose *inBone = addBonePoses + boneNumber;
		EntitySkeletonBonePose *outBone = addToBonePoses + boneNumber;

		if (fraction == 1) {
			*outBone = *inBone;
		} else {
			//
			//	WID: Unsure if actually lerping these is favored.
			//
			const float lerp = 1.0f - backlerp;
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
			
		// Recursively blend all thise bone node's children.
		for ( auto &childBoneNode : boneNode->GetChildren() ) {
			ES_RecursiveBlendFromBone( addBonePoses, addToBonePoses, &childBoneNode, backlerp, fraction );
		}
	}

}

/**
*	@brief	Compute local space matrices for the given pose transformations.
*			This is enough to work with the pose itself. For rendering it needs
*			an extra computing of its additional World Pose Transforms.
**/
void ES_ComputeLocalPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices ) {
	if (!model || !model->iqmData) {
		// TODO: Warn.
		return;
	}

	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	
	// multiply by inverse of bind pose and parent 'pose mat' (bind pose transform matrix)
	const EntitySkeletonBonePose *relativeJoint = bonePoses;
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
	if (!model || !model->iqmData) {
		// TODO: Warn.
		return;
	}

	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;

	ES_ComputeLocalPoseTransforms(model, bonePoses, poseMatrices);

	float *poseMat = iqmModel->bindJoints;
	float *outPose = poseMatrices;

	for (size_t i = 0; i < iqmModel->num_poses; i++, poseMat += 12, outPose += 12) {
		float inPose[12];
		memcpy(inPose, outPose, sizeof(inPose));
		Matrix34Multiply(inPose, poseMat, outPose);
	}
}