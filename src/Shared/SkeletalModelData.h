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
//struct SkeletalAnimation;
struct SkeletalAnimationBlendAction;
struct SkeletalAnimationAction;
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
	/**
	*	Action and BlendAction Data.
	**/
	//! Action map for named indexing.
	std::map<std::string, SkeletalAnimationAction> actionMap;
	//! Action vector, for numeric indexing, pointing to our map.
	std::vector<SkeletalAnimationAction*> actions;

	//! Blend Action map for named indexing.
	//std::map<std::string, SkeletalAnimationBlendAction> blendActionMap;
	//! Blend Action vector, for numeric indexing, pointing to our map.
	//std::vector<SkeletalAnimationBlendAction*> blendActions;

	//! Animation map for named indexing.
	std::map<std::string, SkeletalAnimation> animationMap;
	//! Animation vector, for numeric indexing, pointer to our map.
	std::vector<SkeletalAnimation*> animations;

	/**
	*	Bounding Box Data.
	**/
	//! Bounding Boxes. Stores by frame index.
	struct BoundingBox {
		vec3_t mins;
		vec3_t maxs;
	};
	std::vector<BoundingBox> boundingBoxes;


	/**
	*	Bone/Joint Data.
	**/
	//! Bones, key names, index values.
	std::map<std::string, int32_t> boneNameIndices;

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
*	Animation Actions:
*
*	An animation action consists out of a start and end frame which displays
*	at frametime(ms) per frame. They all got a unique distinctive name for
*	look-up operations.
*
*	An 'Action' can be the frames of a full walk forward animation, but also
*	an idle reload animation. Together they can be blend together starting from
*	a bone node, let's say the spine. Where the walk forward serves as the
*	dominating animation, the reload animation blends in starting from the spine
*	to bring in the final result: walk forward + reload animation.
*
**/
/**
*	@brief	Parsed from modelname.sck animation action data structure.
**/
struct SkeletalAnimationAction {
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
*	@brief	Stores the parsed data of each blend action.
**/
struct SkeletalAnimationBlendAction {
	//! Index into our model's actions.
	uint16_t actionIndex = 0;

	//! The fraction of which to blend at.
	float fraction = 0;

	//! The bone index number.
	int32_t boneNumber = 0;
};

/**
*	@brief	Parsed from modelname.sck, contains the animation blend action data structure.
**/
struct SkeletalAnimation {
	//! Stores the actual index.
	int32_t index = 0;
	//! Stores the name.
	std::string name = "";
	//! Stores the root bone axis flags for this animation.
	// TODO: Implement.
	//int32_t rootBoneAxisFlags = 0;
	//! Stores the root bone number for this animation.
	// TODO: Let each animation assign their own root bone instead of to the ES itself.
	//int32_t rootBoneNumber = 0;

	//! Stores the [indexes, fraction, bone index] of each blend action
	//! in the same order as that they were placed in the configuration file.
	std::deque< SkeletalAnimationBlendAction > blendActions;
};