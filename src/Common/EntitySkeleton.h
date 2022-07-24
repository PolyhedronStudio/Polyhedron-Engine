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
#pragma once



/**
*	@brief	Sets up an entity skeleton using the specified skeletal model data.
**/
const bool ES_CreateFromModel( const model_t *model, EntitySkeleton *es );

/**
*	@brief	Computes the LERP Pose result for in-between the old and current frame by calculating each 
*			relative transform for all bones.
*
*			
**/
void ES_LerpSkeletonPoses( EntitySkeleton *entitySkeleton, EntitySkeletonBonePose *outBonePose, int32_t currentFrame, int32_t oldFrame, float backLerp, const int32_t rootBoneAxisFlags );

/**
*	@brief	Combine 2 poses into one by performing a recursive blend starting from the given boneNode, using the given fraction as "intensity".
*	@param	fraction		When set to 1.0, it blends in the animation at 100% intensity. Take 0.5 for example, 
*							and a tpose(frac 0.5)+walk would have its arms half bend.
*	@param	addBonePose		The actual animation that you want to blend in on top of inBonePoses.
*	@param	addToBonePose	A lerped bone pose which we want to blend addBonePoses animation on to.
**/
void ES_RecursiveBlendFromBone( EntitySkeletonBonePose *addBonePoses, EntitySkeletonBonePose* addToBonePoses, EntitySkeletonBoneNode *boneNode, float backlerp, float fraction = 1.0f );

/**
*	@brief	Calculates the bone's 
**/

/**
*	@brief	Computes all matrices for this model, assigns the {[model->num_poses] 3x4 matrices} in the (pose_matrices) array.
*
*			Treats it no different than as if it were a regular alias model going from fram A to B. And does not make use
*			of said node tree which is stored in the entity's skeleton.
**/
void ES_StandardComputeTransforms( const model_t* model, const r_entity_t* entity, float* pose_matrices );

/**
*	@brief	Compute local space matrices for the given pose transformations.
*			This is enough to work with the pose itself. For rendering it needs
*			an extra computing of its additional World Pose Transforms.
**/
void ES_ComputeLocalPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices );

/**
*	@brief	Compute world space matrices for the given pose transformations.
**/
void ES_ComputeWorldPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices );