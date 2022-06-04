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
*	@brief	Game Module friendly IQM Data: Animations, BoundingBox per frame, and Joint data.
**/
struct SkeletalModelData {
	//! TODO: Indexed by name, should be a hash map instead?
	std::map<std::string, SkeletalAnimation> animations;

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
	//! The total distances travelled by the root bone per frame.
	std::vector<float> frameDistances;
};

/**
*	@return	Game compatible skeletal model data including: animation name, 
*			and frame data, bounding box data(per frame), and joints(name, index, parentindex)
**/
void SKM_GenerateModelData(model_t* model);