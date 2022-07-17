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
#include "EntitySkeleton.h"
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
	uint32_t startFrame		= 0;
	//! Animation end IQM frame number.
	uint32_t endFrame		= 0;
	//! Animation total number of IQM frames.
	uint32_t numFrames		= 0;
	//! When not set in the config file, defaults to whichever ANIMATION_FRAMETIME is set to.
	double frametime		= BASE_FRAMETIME;
	//! Number of times the animation has to loop before it ends.
	//! When set to '0', it'll play continuously until stopped by code.
	uint32_t loopingFrames	= 0;
	//! When 'true', force looping the animation. (It'll never trigger a stop event after each loop.)
	bool forceLoop			= false;

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
*	@brief	Loads up a skeletal model configuration file and passes its buffer over
*			to the parsing process. The process tokenizes the data and generates game
*			code friendly POD to work with.
**/
bool SKM_LoadAndParseConfiguration( const std::string &filePath );

/**
*	@brief	Sets up an entity skeleton using the specified skeletal model data.
**/
const bool ES_CreateFromModel( const model_t *model, EntitySkeleton *es );
