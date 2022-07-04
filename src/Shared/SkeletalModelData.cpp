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
*	Entity Skeleton
*
*
**/
/**
*	@brief	Generates a Linear Bone List, as well as a BoneNode Tree Hierachy, using the current
*			actual SkeletalModelData.
*
*			@param boneNode	Pointer to the node which we check and see if it has any kids, of so, we assign them.
*
*			If boneIndex == -1, we take the root bone index instead.
**/
//static void CreateBoneTreeNode(SkeletalModelData* skm, EntitySkeleton* es, int32_t parentIndex, EntitySkeletonBoneNode *parentNode = nullptr ) {
//	// If bonenumber == -1, we take the skm root bone number and the es boneTree node.
//	if ( parentIndex == -1 ) {
//		CreateBoneTreeNode( skm, es, skm->rootJointIndex, &es->boneTree );
//		// Done.
//		return;
//	}
//	
//	// Ensure the bone number index is valid.
//	if ( parentIndex < 0 || parentIndex > es->bones.size() ) {
//		// TODO: Warn
//		// SG_DPrint("Bone number out of range: %i %i %i %I ... details you know");
//		return;
//	}
//
//	// No boneNode? Wut?
//	if ( !parentNode ) {
//		// TODO: Warn that there is no parentNode given however there is a parentIndex.
//		return;
//	}
//
//
//	// Start creating our tree, if parentNode == nullptr, we take the tree head.
////	parentNode = ( parentNode == nullptr ? &es->boneTree : parentNode );
//
//	// Go through all bones, if their parentIndex matches our boneIndex, we add a
//	// pointer to the hierachy.
//	// (We do not use iterators here.)
//	for (int32_t i = 0; i < es->bones.size(); i++) {
//		// Get a reference to our bone.
//		const EntitySkeletonBone &bone = es->bones[i];
//
//		// It matches, time to add it as our child.
//		if ( bone.parentIndex == parentIndex ) {
//			// Push back the child data.
//			parentNode->childBoneNodes.push_back(EntitySkeletonBoneNode{
//				.index = bone.index,
//				.parentBoneNode = parentNode
//			});
//
//			// Assign the last pushed child node pointer to the linear bone list.
//			EntitySkeletonBoneNode *boneNode = es->bones[i].node = &parentNode->childBoneNodes.back();
//
//			// Recurse on.
//			CreateBoneTreeNode( skm, es, bone.index, boneNode );
//		}
//	}
//}
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
	//// Scan our linear bone list and check each bone's boneNode for children, and clear those out.
	//for ( int32_t i = 0; i < es->bones.size(); i++ ) {
	//	// Get boneNode pointer.
	//	EntitySkeletonBoneNode *boneNode = es->bones[i].boneNode;

	//	// Make sure it is valid.
	//	if ( boneNode ) {
	//		// Clear children.
	//		boneNode->boneChildren.clear();
	//		// Reset bone number.
	//		boneNode->boneIndex = -1;
	//	}
	//}

	// Clear out Linear Bone List.
	es->bones.clear();

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


























































/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
//#pragma once
//
//
///**
//*	Needed for SKM_MAX_JOINTS
//**/
//#include "Refresh.h"
//#include "Formats/Iqm.h"
//
///**
//*	@brief	Pre...
//**/
//struct SkeletalAnimation;
//
////! MS Frametime for animations.
////static constexpr float ANIMATION_FRAMETIME = BASE_FRAMETIME;//FRAMERATE_MS;
//static constexpr int32_t SKM_MAX_JOINTS = IQM_MAX_JOINTS;
//
//
///**
//*
//*
//*	Human Friendly Skeletal Model Data. Cached by server and client.
//*
//*
//**/
///**
//*	@brief	Game Module friendly IQM Data: Animations, BoundingBox per frame, and Joint data.
//**/
//struct SkeletalModelData {
//	//! TODO: Indexed by name, should be a hash map instead?
//	std::map<std::string, SkeletalAnimation> animationMap;
//	std::vector<SkeletalAnimation*> animations;
//
//	//! Bones, key names, index values.
//	std::map<std::string, int32_t> boneNameIndices;
//
//	//! Bounding Boxes. Stores by frame index.
//	struct BoundingBox {
//		vec3_t mins;
//		vec3_t maxs;
//	};
//	std::vector<BoundingBox> boundingBoxes;
//
//	//! Joint information. Stored by name indexing in a map, as well as numberical indexing in an array.
//	struct Joint {
//		//! Joint name.
//		std::string name;
//		//! Joint index.
//		int32_t index = 0;
//		//! Joint Parent Index.
//		int32_t parentIndex = 0;
//	};
//	//! Total number of joints.
//	uint32_t numberOfJoints = 0;
//	//! Root Joint index.
//	int32_t rootJointIndex = -1;
//	//! String index joint data map.
//	std::map<std::string, Joint> jointMap;
//	//! Index based joint data array.
//	std::array<Joint, SKM_MAX_JOINTS> jointArray;
//};
//
///**
//*	@return	Game compatible skeletal model data including: animation name, 
//*			and frame data, bounding box data(per frame), and joints(name, index, parentindex)
//**/
//void SKM_GenerateModelData(model_t* model);
//
//
//
///**
//*
//*
//*	Entity Skeleton: Each entity using a skeletal model can generate an Entity Skeleton
//*	for use of performing animation pose blending magic with.
//*
//*
//**/
///**
//*	@brief	Bone Node storing its number and pointers to its child bone
//*			nodes. Used for maintaining an actual bone tree hierachy.			
//**/
//struct EntitySkeletonBoneNode {
//	//! The bone its own index.
//	int32_t index = 0;
//
//	//! Actual bone number of this node.
//	//int32_t boneNumber = 0;
//	EntitySkeletonBoneNode *parentBoneNode = nullptr;
//
//	//! Pointers to children bone nodes. (size == total num of bones)
//	std::vector<EntitySkeletonBoneNode> childBoneNodes;
//};
//
///**
//*	@brief	Stores partial generated copy of non spatial bone data that 
//*			was contained in the Skeletal Model Data at the actual time 
//*			of creating this skeleton.			
//*
//*			To allow for fast access in the node tree by linear array indexing
//*			we also store a pointer to its belonging node.
//**/
//struct EntitySkeletonBone {
//	//! Actual string bone name.
//	std::string name;
//
//	//! The bone its own number.
//	int32_t index = 0;
//
//	//! Parent bone number.
//	int32_t parentIndex = 0;
//
//	//! A pointer 
//	EntitySkeletonBoneNode *node = nullptr;
//};
//
///**
//*	@brief	Skeleton consisting of the current frame's blend combined bone poses
//*			to use for rendering and/or in-game bone position work.
//*
//*			When changing an entity's model, the skeleton needs to be regenerated.
//**/
//struct EntitySkeleton {
//	//! Pointer to the internal model data. (Also used to retreive the SKM data from there.)
//	model_t *model;
//
//	//! Stores a list of all bones we got in our entity's skeleton.
//	std::vector<EntitySkeletonBone> bones;
//
//	//! Stores the bones as an actual tree hierachy.
//	EntitySkeletonBoneNode boneTree;
//};
//
///**
//*	@brief	Sets up an entity skeleton using the specified skeletal model data.
//**/
//const bool SKM_CreateEntitySkeletonFrom(SkeletalModelData *skm, EntitySkeleton *es);
//
//
///**
//*
//*
//*	Bone Poses
//*
//*
//**/
///**
//*	@brief Currently is just iqm_transform_t, there's nothing to it.
//**/
//using EntitySkeletonBonePose = iqm_transform_t;
////struct EntitySkeletonBonePose {
////	iqm_transform_t transform;
////};
//
//
//
///**
//*
//*
//*	Animation Data: It says parsed from, it is not so, it comes from the IQM
//*	file itself only right now.
//*
//*	Contains extra precalculated data regarding the total animation distance
//*	traveled per animation, the exact distance and translation of the root
//*	bone per frame.
//*
//*	On top of that, the root bone can be set render specific flags. (RootBoneAxisFlags)
//*
//*
//**/
///**
//*	@brief	Parsed from the modelname.cfg file. Stores specific data that
//*			belongs to an animation. Such as: frametime, loop or no loop.
//**/
//struct SkeletalAnimation {
//	/**
//	*	Look-up Properties, (Name, index, IQM StartFrame, IQM EndFrame.)
//	**/
//	//! Actual animation look-up index.
//	uint32_t index = 0;
//	//! Actual animation loop-uk name.
//	std::string name = "";
//
//
//	/**
//	*	Animation Properties.
//	**/
//	//! Animation start IQM frame number.
//	uint32_t startFrame = 0;
//	//! Animation end IQM frame number.
//	uint32_t endFrame = 0;
//	//! Animation total number of IQM frames.
//	uint32_t	numFrames = 0;
//	//! Number of times the animation has to loop before it ends.
//	//! When set to '0', it'll play continuously until stopped by code.
//	uint32_t loopingFrames = 0;
//	//! When not set in the config file, defaults to whichever ANIMATION_FRAMETIME is set to.
//	double frametime = BASE_FRAMETIME;
//	//! When 'true', force looping the animation. (It'll never trigger a stop event after each loop.)
//	bool forceLoop = false;
//
//	/**
//	*	Physical Properties.
//	**/
//	//! The sum of total distance travelled by the root bone per frame.
//	double animationDistance = 0.0;
//
//	//! TODO: Add a decent Bone Access API and transform code using this to using that.
//	double frameStartDistance = 0.0;
//	double frameEndDistance = 0.0f;
//	//! Start Frame distance.
//	//! The total distances travelled by the root bone per frame.
//	std::vector<double> frameDistanceSum;
//	//! The total distances travelled by the root bone per frame.
//	std::vector<double> frameDistances;
//	//! The translates of root bone per frame.
//	std::vector<vec3_t> frameTranslates;
//	//! Root Bone Axis Flags. 
//	struct RootBoneAxisFlags {
//		//! When generating the poses it'll zero out the X axis of the root bone's translation vector.
//		static constexpr int32_t ZeroXTranslation = (1 << 1);
//		//! When generating the poses it'll zero out the Y axis of the root bone's translation vector.
//		static constexpr int32_t ZeroYTranslation = (1 << 2);
//		//! When generating the poses it'll zero out the Z axis of the root bone's translation vector.
//		static constexpr int32_t ZeroZTranslation = (1 << 3);
//
//		//! The default is to only zero out X and Y for the root motion system.
//		static constexpr int32_t DefaultTranslationMask = (ZeroXTranslation | ZeroYTranslation);
//	};
//
//	//! Tells the skeletal animation system how to treat our root bone for this animation.
//	int32_t rootBoneAxisFlags = RootBoneAxisFlags::DefaultTranslationMask;
//};
//
//
//
///**
//*	@brief	Simple TreeNode Hierachy Template.
//*
//*			Each node keeps track of its parent while having unlimited amount of children.
//*
//**/
//template < class T > 
//class lstdTreeNodeParentPointer {
//public:
//	/**
//	*
//	*	Constructors/Destructor(s).
//	*
//	**/
//	/**
//	*	@brief	Default Constructor.
//	**/
//	lstdTreeNodeParentPointer()
//	{
//		this->parent = nullptr;
//	}
//	/**
//	*	@brief	Default constructor accepting a const reference to the data to hold.
//	**/
//	lstdTreeNodeParentPointer( const T& data , lstdTreeNodeParentPointer* parent = nullptr )
//	{
//		this->data = data;
//		this->parent = parent;
//	}
//	/**
//	*	@brief	Copy Constructor.
//	**/
//	lstdTreeNodeParentPointer( const lstdTreeNodeParentPointer& node ) 
//	{
//		this->data = node.data;
//		this->parent = node.parent;
//		this->children = node.children;
//		// fix the parent pointers
//		for ( size_t i = 0 ; i < this->children.size() ; ++i )
//		{
//			this->children.at(i).parent = this;
//		}
//	}
//	/**
//	*	@brief	Copy assignment operator
//	**/
//    lstdTreeNodeParentPointer& operator=( const lstdTreeNodeParentPointer& node )
//    {
//		if ( this != &node ) 
//		{
//			this->data = node.data;
//			this->parent = node.parent;
//			this->children = node.children;
//
//			// Fix the parent pointers.
//			for ( size_t i = 0 ; i < this->children.size() ; ++i )
//			{
//				this->children.at(i).parent = this;
//			}
//		}
//		return *this;
//    }
//
//	/**
//	*	@brief	Usual virtual destructor.
//	**/
//	virtual ~lstdTreeNodeParentPointer(){
//	}
//
//
//	/**
//	*
//	*	Add/Remove Child Node Functions.
//	*
//	**/
//	/**
//	*	@brief	Adds a child node storing 'data' to this node's children.
//	**/
//	void AddChild( const T& data ) {
//		this->children.push_back( lstdTreeNodeParentPointer( data , this ) );
//	}
//
//	/**
//	*	@brief	Remove a child node by value, note: if the node has multiple children with the same value, this will only delete the first child
//	**/
//	void RemoveChildByValue( const T& value ) {
//		for ( size_t i = 0 ; i < this->children.size() ; ++i )
//		{
//			if ( this->children.at(i).data == value )
//			{
//				this->children.erase( this->children.begin()+i );
//				return;
//			}
//		}
//	}
//
//	/**
//	*	@brief	Removes a child node by index.
//	**/
//	void RemoveChildByIndex( const int index ) {
//		this->children.erase( this->children.begin()+index );
//	}
//
//
//	/**
//	*
//	*	Get/Set Functions.
//	*
//	**/
//	/**
//	*	@brief	Sets a new data value for this node.
//	**/
//	void SetData( const T& data ) {
//		this->data = data;
//	}
//	/**
//	*	@return	Reference to this node's data.
//	**/
//	T& GetData() {
//		return this->data;
//	}
//	/**
//	*	@return	Const Reference to this node's data.
//	**/
//	const T& GetValue() const {
//		return this->data;
//	}
//
//	/**
//	*	@return	Reference to the parent node.
//	**/
//	lstdTreeNodeParentPointer& GetParent() {
//		return *this->parent;
//	}
//	/**
//	*	@return	Const Reference to the parent node.
//	**/
//	const lstdTreeNodeParentPointer& GetParent() const {
//		return *this->parent;
//	}
//	
//	/**
//	*	@return	Reference to the child node vector.
//	**/
//	std::vector< lstdTreeNodeParentPointer >& GetChildren() {
//		return this->children;
//	}
//	/**
//	*	@return	Const Reference to the child node vector.
//	**/
//	const std::vector< lstdTreeNodeParentPointer >& GetChildren() const	{
//		return this->children;
//	}
//
//
//private:
//	//! Pointer to the parent node, if null, then this IS the parent node.
//	lstdTreeNodeParentPointer* parent;
//
//	//! Vector containing all possible children of this node.
//	std::vector< lstdTreeNodeParentPointer > children;
//
//	//! Actual data stored in this node.
//	T data;
//
//
//	// the type has to have an overloaded std::ostream << operator for print to work
//	//void print( const int depth = 0 ) const
//	//{
//	//	std::string printStr = "";
//
//	//	for ( int i = 0 ; i < depth ; ++i )
//	//	{
//	//		if ( i != depth-1 ) printStr << "    ";
//	//		else printStr << "|-- ";
//	//	}
//	//	printStr << this->t << "\n";
//	//	// SG_DPrint...
//	//	for ( size_t i = 0 ; i < this->children.size() ; ++i )
//	//	{
//	//		this->children.at(i).print( depth+1 );
//	//	}
//	//}
//};