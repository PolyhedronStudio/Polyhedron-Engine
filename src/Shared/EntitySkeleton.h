/***
*
*	License here.
*
*	@file
*
*	This header contains the declarations for the 'EntitySkeleton'. An EntitySkeleton can be used to
*	work with, and manipulate skeletal model animation pose data.
*
*	The engine provides a simple API to work with these, the implementation resides in the 'Common'
*	source group, and is wrapped up nicely in two small easy to use Game APIs. (Client, and Server)
*
***/
#pragma once


/**
*	Needed for ES_MAX_JOINTS
**/
#include "Formats/Iqm.h"

/**
*	@brief Currently is just iqm_transform_t, there's nothing to it.
**/
using EntitySkeletonBonePose = iqm_transform_t;

/**
*	
**/
#include "Refresh.h"




/**
*	@brief	Predeclare.
**/
struct SkeletalAnimation;



/**
*	@brief	The actual Bone Data, stores the name, index, parentIndex, a pointer to the
*			Bone Node in our Bone Tree Hierachy as well as the bone's (IQM-)flags.
**/
class EntitySkeletonBoneNode;
struct EntitySkeletonBone {
	//! This bone's name.
	std::string name = "";
	//! This bone's parent index.
	int32_t parentIndex = -1;	//! Defaults to -1, meaning, None.
	//! This bone's index.
	int32_t index = 0;
	//! This bone's flags. (If any.)
	int32_t flags = 0;

	//! A pointer to the node matching this bone in our linear bone list.
	EntitySkeletonBoneNode *boneTreeNode = nullptr;
};



/**
*	@brief	Simple TreeNode Hierachy.
*
*			Each node keeps track of its parent, other than the factual limit of 256 bones per (IQM-)mesh,
*			it allows for 'unlimited' child nodes.
**/
class EntitySkeletonBoneNode {
public:
	/**
	*
	*	Constructors/Destructor(s).
	*
	**/
	/**
	*	@brief	Default Constructor.
	**/
	EntitySkeletonBoneNode()
	{
		this->parent = nullptr;
	}
	/**
	*	@brief	Default constructor accepting a const reference to the data to hold.
	**/
	EntitySkeletonBoneNode( EntitySkeletonBone *esBone, EntitySkeletonBoneNode* parentBoneNode = nullptr ) {
		this->esBone = esBone;
		this->parent = parentBoneNode;
	}
	/**
	*	@brief	Copy Constructor.
	**/
	EntitySkeletonBoneNode( const EntitySkeletonBoneNode& node ) 
	{
		this->esBone = node.esBone;
		this->parent = node.parent;
		this->children = node.children;
		// fix the parent pointers
		for ( size_t i = 0 ; i < this->children.size() ; ++i )
		{
			this->children.at(i).parent = this;
		}
	}
	/**
	*	@brief	Copy assignment operator
	**/
    EntitySkeletonBoneNode& operator=( const EntitySkeletonBoneNode& node )
    {
		if ( this != &node ) 
		{
			this->esBone = node.esBone;
			this->parent = node.parent;
			this->children = node.children;

			// Fix the parent pointers.
			for ( size_t i = 0 ; i < this->children.size() ; ++i )
			{
				this->children.at(i).parent = this;
			}
		}
		return *this;
    }

	/**
	*	@brief	Usual virtual destructor.
	**/
	virtual ~EntitySkeletonBoneNode(){
	}


	/**
	*
	*	Add/Remove Child Node Functions.
	*
	**/
	/**
	*	@brief	Adds a child node storing 'data' to this node's children.
	**/
	inline EntitySkeletonBoneNode &AddChild( EntitySkeletonBone *esBone ) {
		// Add the bone to our children vector.
		this->children.emplace_back( EntitySkeletonBoneNode( esBone , this ) );

		// Return a reference to it.
		return children.back();
	}

	/**
	*	@brief	Removes a child bone node if its boneIndex matches.
	**/
	inline void RemoveChildByBoneIndex( const int32_t boneIndex ) {
		if ( boneIndex < children.size() ) {
			children.erase( children.begin() + boneIndex );
		}
	}

	/**
	*	@brief	Removes a child bone node if its name matches.
	**/
	inline void RemoveChildByName( const std::string &boneName ) {
		// Test function
		auto removeIterator = std::remove_if(children.begin(), children.end(),
			[&boneName]( const EntitySkeletonBoneNode &boneNode ) -> auto {
				const EntitySkeletonBone *bone = boneNode.GetEntitySkeletonBone();

				return (bone && bone->name == boneName);
			}
		);
	}


	/**
	*
	*	Get/Set Functions.
	*
	**/
	/**
	*	@return	On success, a pointer to the first node that has a matching bone name.
	**/
	inline EntitySkeletonBoneNode *GetBoneNodeByName( const std::string &boneName ) {
		// Test function
		auto removeIterator = std::find_if(children.begin(), children.end(),
			[&boneName]( const EntitySkeletonBoneNode &boneNode ) -> auto {
				const EntitySkeletonBone *bone = boneNode.GetEntitySkeletonBone();

				return (bone && bone->name == boneName ? &boneNode : nullptr);
			}
		);
	}
	/**
	*	@return	Const Pointer to this node's matching Entity Skeleton Bone.
	**/
	inline EntitySkeletonBone *GetEntitySkeletonBone() {
		return esBone;
	}
	/**
	*	@return	Const Pointer to this node's matching Entity Skeleton Bone.
	**/
	inline const EntitySkeletonBone *GetEntitySkeletonBone() const {
		return esBone;
	}
	/**
	*	@return	Reference to the parent node.
	**/
	inline EntitySkeletonBoneNode &GetParent() {
		return *this->parent;
	}
	/**
	*	@return	Const Reference to the parent node.
	**/
	inline const EntitySkeletonBoneNode &GetParent() const {
		return *this->parent;
	}
	
	/**
	*	@return	Reference to the child node vector.
	**/
	inline std::vector< EntitySkeletonBoneNode > &GetChildren() {
		return this->children;
	}
	/**
	*	@return	Const Reference to the child node vector.
	**/
	inline const std::vector< EntitySkeletonBoneNode > &GetChildren() const	{
		return this->children;
	}


private:
	//! Actual data stored in this node.
	//T data;
	EntitySkeletonBone *esBone = nullptr;

	//! Pointer to the parent node, if null, then this IS the parent node.
	EntitySkeletonBoneNode* parent = nullptr;
	//! Vector containing all possible children of this node.
	std::vector< EntitySkeletonBoneNode > children;
};



/**
*	@brief	An Entity Skeleton consists of all the data needed in order to properly work with Skeletal Animation Bone Poses.
*
*			
**/
struct EntitySkeleton {
	//! Pointer to the internal model data. When not set, the skeleton is unusable.
	model_t *modelPtr = nullptr;

	//! A simple tree hierachy for working with this skeleton's bone data.
	EntitySkeletonBoneNode boneTree;
	//! A std::map making bone nodes easily accessable by their name. Generated after creating the bone Tree, and allows only distinct bone names.
	std::map<std::string, EntitySkeletonBoneNode*> boneMap;
	//! The actual skeleton bone data, stored linearly for fast access.
	std::vector<EntitySkeletonBone> bones;
};

/**
*	@brief	Sets up an entity skeleton using the specified skeletal model data.
**/
const bool ES_CreateFromModel( model_t *model, EntitySkeleton* es );



///**
//*	@brief	Computes all matrices for this model, assigns the {[model->num_poses] 3x4 matrices} in the (pose_matrices) array.
//*
//*			Treats it no different than as if it were a regular alias model going from fram A to B. And does not make use
//*			of said node tree which is stored in the entity's skeleton.
//**/
//void ES_StandardComputeTransforms( const model_t* model, const r_entity_t* entity, float* pose_matrices );
///**
//*	@brief	Computes the LERP Pose result for in-between the old and current frame by calculating each 
//*			relative transform for all bones.
//*
//*			
//**/
//void ES_LerpSkeletonPoses( const model_t *model, const int32_t rootBoneAxisFlags, int32_t currentFrame, int32_t oldFrame, float lerp, float backLerp, EntitySkeletonBonePose *outBonePose );
//
///**
//*	@brief	Combine 2 poses into one by performing a recursive blend starting from the given boneNode, using the given fraction as "intensity".
//*	@param	fraction		When set to 1.0, it blends in the animation at 100% intensity. Take 0.5 for example, 
//*							and a tpose(frac 0.5)+walk would have its arms half bend.
//*	@param	addBonePose		The actual animation that you want to blend in on top of inBonePoses.
//*	@param	addToBonePose	A lerped bone pose which we want to blend addBonePoses animation on to.
//**/
//void ES_RecursiveBlendFromBone( const model_t *model, EntitySkeletonBonePose *addBonePoses, EntitySkeletonBonePose* addToBonePoses, EntitySkeletonBoneNode *boneNode, float fraction, float lerp, float backlerp );
//
///**
//*	@brief	Compute local space matrices for the given pose transformations.
//*			This is enough to work with the pose itself. For rendering it needs
//*			an extra computing of its additional World Pose Transforms.
//**/
//void ES_ComputeLocalPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices );
//
///**
//*	@brief	Compute world space matrices for the given pose transformations.
//**/
//void ES_ComputeWorldPoseTransforms( const model_t *model, const EntitySkeletonBonePose *bonePoses, float *poseMatrices );