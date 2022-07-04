/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#pragma once


/**
*	Needed for SKM_MAX_JOINTS
**/
#include "Refresh.h"
#include "Formats/Iqm.h"

/**
*	@brief	Pre...
**/
struct SkeletalAnimation;

//! MS Frametime for animations.
//static constexpr float ANIMATION_FRAMETIME = BASE_FRAMETIME;//FRAMERATE_MS;
static constexpr int32_t SKM_MAX_JOINTS = IQM_MAX_JOINTS;


/**
*
*
*	Human Friendly Skeletal Model Data. Cached by server and client.
*
*
**/
/**
*	@brief	Game Module friendly IQM Data: Animations, BoundingBox per frame, and Joint data.
**/
struct SkeletalModelData {
	//! TODO: Indexed by name, should be a hash map instead?
	std::map<std::string, SkeletalAnimation> animationMap;
	std::vector<SkeletalAnimation*> animations;

	//! Bones, key names, index values.
	std::map<std::string, int32_t> boneNameIndices;

	//! Bounding Boxes. Stores by frame index.
	struct BoundingBox {
		vec3_t mins;
		vec3_t maxs;
	};
	std::vector<BoundingBox> boundingBoxes;

	//! Joint information. Stored by name indexing in a map, as well as numberical indexing in an array.
	struct Joint {
		//! Joint name.
		std::string name;
		//! Joint index.
		int32_t index = 0;
		//! Joint Parent Index.
		int32_t parentIndex = 0;
	};
	//! Total number of joints.
	uint32_t numberOfJoints = 0;
	//! Root Joint index.
	int32_t rootJointIndex = -1;
	//! String index joint data map.
	std::map<std::string, Joint> jointMap;
	//! Index based joint data array.
	std::array<Joint, SKM_MAX_JOINTS> jointArray;
};

/**
*
*
*	Animation Data: It says parsed from, it is not so, it comes from the IQM
*	file itself only right now.
*
*	Contains extra precalculated data regarding the total animation distance
*	traveled per animation, the exact distance and translation of the root
*	bone per frame.
*
*	On top of that, the root bone can be set render specific flags. (RootBoneAxisFlags)
*
*
**/
/**
*	@brief	Parsed from the modelname.cfg file. Stores specific data that
*			belongs to an animation. Such as: frametime, loop or no loop.
**/
struct SkeletalAnimation {
	/**
	*	Look-up Properties, (Name, index, IQM StartFrame, IQM EndFrame.)
	**/
	//! Actual animation look-up index.
	uint32_t index = 0;
	//! Actual animation loop-uk name.
	std::string name = "";


	/**
	*	Animation Properties.
	**/
	//! Animation start IQM frame number.
	uint32_t startFrame = 0;
	//! Animation end IQM frame number.
	uint32_t endFrame = 0;
	//! Animation total number of IQM frames.
	uint32_t	numFrames = 0;
	//! Number of times the animation has to loop before it ends.
	//! When set to '0', it'll play continuously until stopped by code.
	uint32_t loopingFrames = 0;
	//! When not set in the config file, defaults to whichever ANIMATION_FRAMETIME is set to.
	double frametime = BASE_FRAMETIME;
	//! When 'true', force looping the animation. (It'll never trigger a stop event after each loop.)
	bool forceLoop = false;

	/**
	*	Physical Properties.
	**/
	//! The sum of total distance travelled by the root bone per frame.
	double animationDistance = 0.0;

	//! TODO: Add a decent Bone Access API and transform code using this to using that.
	double frameStartDistance = 0.0;
	double frameEndDistance = 0.0f;
	//! Start Frame distance.
	//! The total distances travelled by the root bone per frame.
	std::vector<double> frameDistanceSum;
	//! The total distances travelled by the root bone per frame.
	std::vector<double> frameDistances;
	//! The translates of root bone per frame.
	std::vector<vec3_t> frameTranslates;
	//! Root Bone Axis Flags. 
	struct RootBoneAxisFlags {
		//! When generating the poses it'll zero out the X axis of the root bone's translation vector.
		static constexpr int32_t ZeroXTranslation = (1 << 1);
		//! When generating the poses it'll zero out the Y axis of the root bone's translation vector.
		static constexpr int32_t ZeroYTranslation = (1 << 2);
		//! When generating the poses it'll zero out the Z axis of the root bone's translation vector.
		static constexpr int32_t ZeroZTranslation = (1 << 3);

		//! The default is to only zero out X and Y for the root motion system.
		static constexpr int32_t DefaultTranslationMask = (ZeroXTranslation | ZeroYTranslation);
	};

	//! Tells the skeletal animation system how to treat our root bone for this animation.
	int32_t rootBoneAxisFlags = RootBoneAxisFlags::DefaultTranslationMask;
};

/**
*	@return	Game compatible skeletal model data including: animation name, 
*			and frame data, bounding box data(per frame), and joints(name, index, parentindex)
**/
void SKM_GenerateModelData(model_t* model);



/**
*
*
*	Entity Skeleton: Each entity using a skeletal model can generate an Entity Skeleton
*	for use of performing animation pose blending magic with.
*
*
**/
/**
*	@brief	Stores the actual data of each bone making up this Entity Skeleton.
*			Including a pointer to a matching node in our Bone Tree Hierachy.
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
*	@brief	Simple TreeNode Hierachy Template.
*
*			Each node keeps track of its parent while having unlimited amount of children.
*
**/
//template < class T > 
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
	EntitySkeletonBoneNode &AddChild( EntitySkeletonBone *esBone ) {
		// Add the bone to our children vector.
		this->children.emplace_back( EntitySkeletonBoneNode( esBone , this ) );

		// Return a reference to it.
		return children.back();
	}

	/**
	*	@brief	Removes a child bone node if its boneIndex matches.
	**/
	void RemoveChildByBoneIndex( const int32_t boneIndex ) {
		if ( boneIndex < children.size() ) {
			children.erase( children.begin() + boneIndex );
		}
	}

	/**
	*	@brief	Removes a child bone node if its name matches.
	**/
	void RemoveChildByName( const std::string &boneName ) {
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
	*	@return	Const Pointer to this node's matching Entity Skeleton Bone.
	**/
	EntitySkeletonBone *GetEntitySkeletonBone() {
		return esBone;
	}
	/**
	*	@return	Const Pointer to this node's matching Entity Skeleton Bone.
	**/
	const EntitySkeletonBone *GetEntitySkeletonBone() const {
		return esBone;
	}
	/**
	*	@return	Reference to the parent node.
	**/
	EntitySkeletonBoneNode &GetParent() {
		return *this->parent;
	}
	/**
	*	@return	Const Reference to the parent node.
	**/
	const EntitySkeletonBoneNode &GetParent() const {
		return *this->parent;
	}
	
	/**
	*	@return	Reference to the child node vector.
	**/
	std::vector< EntitySkeletonBoneNode > &GetChildren() {
		return this->children;
	}
	/**
	*	@return	Const Reference to the child node vector.
	**/
	const std::vector< EntitySkeletonBoneNode > &GetChildren() const	{
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


	// the type has to have an overloaded std::ostream << operator for print to work
	//void print( const int depth = 0 ) const
	//{
	//	std::string printStr = "";

	//	for ( int i = 0 ; i < depth ; ++i )
	//	{
	//		if ( i != depth-1 ) printStr << "    ";
	//		else printStr << "|-- ";
	//	}
	//	printStr << this->t << "\n";
	//	// SG_DPrint...
	//	for ( size_t i = 0 ; i < this->children.size() ; ++i )
	//	{
	//		this->children.at(i).print( depth+1 );
	//	}
	//}
};

/**
*	@brief	Skeleton consisting of the current frame's blend combined bone poses
*			to use for rendering and/or in-game bone position work.
*
*			When changing an entity's model, the skeleton needs to be regenerated.
**/
struct EntitySkeleton {
	//! Pointer to the internal model data. (Also used to retreive the SKM data from there.)
	model_t *model = nullptr;


	//! A simple tree hierachy for working with this skeleton's bone data.
	EntitySkeletonBoneNode boneTree;
	//! The actual skeleton bone data, stored linearly for fast access.
	std::vector<EntitySkeletonBone> bones;
};

/**
*	@brief	Sets up an entity skeleton using the specified skeletal model data.
**/
const bool SKM_CreateEntitySkeletonFrom(SkeletalModelData *skm, EntitySkeleton *es);


/**
*
*
*	Bone Poses
*
*
**/
/**
*	@brief Currently is just iqm_transform_t, there's nothing to it.
**/
using EntitySkeletonBonePose = iqm_transform_t;
//struct EntitySkeletonBonePose {
//	iqm_transform_t transform;
//};
