/***
*
*	License here.
*
*	@file
*
*	Contains data structures to make the magic of skeletal model animations tick.
* 
***/
#pragma once

//// Probably should move to shared/math...
//struct Orientation {
//	mat3_t axis;
//	vec3_t origin;
//};
//
//// Probably should move to shared/math...
//struct BonePose {
//	dualquat_t dualQuat;
//};
//
//
///**
//*	Bone node which keeps a list of child nodes.
//**/
//struct IQMBoneNode {
//	//! Bone number this node belongs to.
//	int32_t boneNumber = 0;
//
//	//! Contains  the child bone nodes.
//	//std::vector<IQMBoneNode> children;
//
//	//! Number of childnodes.
//	int32 numberOfChildNodes = 0;
//	//! Actual childnode pointers.
//	IQMBoneNode **childNodes = nullptr;
//};
//
///**
//*	Tags store their offset and rotation and are linked to a bone number.
//**/
//struct IQMTagMask {
//	//! Textual name identifier of this tag.
//	std::string tagName		= "";
//	//! Textual bonename identifier matching this tag.
//	std::string boneName	= "";
//
//	//! Bone index number matching this tag.
//	int32_t boneNumber = 0;
//
//	//! Next tag in list.
//	IQMTaskMask *next = nullptr;
//
//	//! Offset and Rotation.
//	vec3_t offset	= vec3_zero();
//	vec3_t rotation	= vec3_zero();
//};
//
///**
//*	Stores actual Bone data, name, parent index, flags and a pointer to a bonetree node.
//**/
//struct IQMBone {
//	//! Textual bone name identifier.
//	std::string name = "";
//		// Bone flags.
//	int32_t flags = 0;
//
//	//! Parent bone ID.
//	int32_t parent = 0;
//	//! Bone treenode pointer.
//	IQMBone *node = nullptr;
//};
//
//
///**
//*	Actual mesh data structure storing a pointer to the iqm model it belongs to. Also stores a list
//*	of actual bone nodes, bone poses, tag masks and a bonetree.
//**/
//struct Skeleton {
//	struct model_s *model;
//
//	//int32 numberOfBones = 0;
//	std::vector<IQMBone> bones;
//
//	//int32_t numberOfFrames;
//	std::vector<BonePose> bonePoses;
//	//bonepose_t **bonePoses;
//
//	Skeleton *next;
//
//	// store the tagmasks as part of the skeleton (they are only used by player models, tho)
//	std::vector<IQMTagMask> tagMasks;
//
//	std::vector<IQMBoneNode> *boneTree;
//};



//void CG_AddEntityToScene( entity_t *ent );
//cgs_skeleton_t *CG_SkeletonForModel( struct model_s *model );
//
//bonepose_t *CG_RegisterTemporaryExternalBoneposes( cgs_skeleton_t *skel );
//cgs_skeleton_t *CG_SetBoneposesForTemporaryEntity( entity_t *ent );
//void CG_InitTemporaryBoneposesCache( void );
//void CG_ResetTemporaryBoneposesCache( void );
//void CG_FreeTemporaryBoneposesCache( void );
//bonenode_t *CG_BoneNodeFromNum( cgs_skeleton_t *skel, int bonenum );
//void CG_RecurseBlendSkeletalBone( bonepose_t *inboneposes, bonepose_t *outboneposes,
//								  bonenode_t *bonenode, float frac );
//bool CG_LerpBoneposes( cgs_skeleton_t *skel, bonepose_t *curboneposes, bonepose_t *oldboneposes,
//					   bonepose_t *outboneposes, float frontlerp );
//bool CG_LerpSkeletonPoses( cgs_skeleton_t *skel, int curframe, int oldframe,
//						   bonepose_t *outboneposes, float frontlerp );
//void CG_TransformBoneposes( cgs_skeleton_t *skel, bonepose_t *boneposes,
//							bonepose_t *sourceboneposes );
//void CG_RotateBonePose( vec3_t angles, bonepose_t *bonepose );
//bool CG_SkeletalPoseGetAttachment( orientation_t *orient, cgs_skeleton_t *skel,
//								   bonepose_t *boneposes, const char *bonename );
