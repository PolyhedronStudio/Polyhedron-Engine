/***
*
*	License here.
*
*	@file
*
*	Skeletal Animation functionality.
*
***/
#include "Shared/Shared.h"
#include "Common/Files.h"
#include "Common/SkeletalModelData.h"
#include <numeric>
#include "Shared/GeneralPurposeParser.h"

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
		const int32_t jointParentIndex = (jointIndex == 0 ? -1 : model->iqmData->jointParents[jointIndex]);

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

		//// If we are dealing with the "root" bone, store it.
		//if (jointData.name == "mixamorig8:Hips") {
		//	skm->rootJointIndex = jointIndex;
		//}
	}

	// Default to -1, as in, unset.
	//skm->rootJointIndex = -1;

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
*
*	Animation Configuration Parsing:
*	
*	Takes care of parsing animation configuration files such as this example:
*
*	----
*	rootbone "mixamo8:Spine" 
*
*	headbone "mixamo8:neckthing" 
*	torsobone "mixamo8:Spine1" 
*
*	action "idle" 0 1 -1 16.66 
*	action "run" 23 64 -1 16.66 
*	action "reload" 65 95 -1 16.66 
*
*	animation "RunAndReload" 
*		blendaction "run" 1.0 
*		blendaction "reload" 1.0 torsobone 
*	----
*
*	The following 'commands' are available:
*	'rootbone': Specifies which bone acts as a rootmotion movement origin point.
*				Can be identified by a string name, or integral number index.
*
*	'headbone':	Specifies where the head starts in case of a humanoid model.
*				Can be identified by a string name, or integral number index.
*	'torsobone':Specifies where the torso starts in case of a humanoid model.
*				Can be identified by a string name, or integral number index.
*
*	'action':	Actions define specific animation frame sets by assigning them
*				a unique distinct name, startframe, endframe, loopCount, and frameTime.
*
*	'animation':Animations are composed by packing 'blendaction' commands together. 
*				The first blend 'takes the lead', and is considered the dominating
*				animation on which we blend other animations into.
*				
*	'blendaction':	Assigns an 'action' to a bone node with the given blend fraction.
*					If no bonenode is present, it is regarded as the leading dominator
*					animation instead.
*				
*
*
**/
//! Contains a parsed token.
struct SKMParsedToken {
	//! Token Flags.
	struct Flags {
		//! This flag is set whenever a an opening double quote has matched an ending double quote.
		static constexpr int32_t QuotedString = 1 << 1;
		//! This flag is set when the token passed an integral number test.
		static constexpr int32_t IntegralNumber = 1 << 2;
		//! This flag is set when the token passed a floating point number test.
		static constexpr int32_t FloatNumber = 1 << 3;
		//! This flag is set if the token is contained within our commands list.
		static constexpr int32_t CommandIdentifier = 1 << 4;
		//! This flag is set when all of the above have failed, we'll assume that it is an identifier of sorts.
		static constexpr int32_t Identifier = 1 << 5;
	};
	//! Line number we found this token at.
	int32_t line = 0;
	//! Character offset number into our line at which we found this token at.
	int32_t offset = 0;
	//! Token character width. (To avoid having to request string size all the time.)
	int32_t width = 0;

	//! Actual token string.
	std::string value = "";
	//! Keeps track of specific token flags.
	int32_t flags = 0;	// These are set to either cmd, string(if it was encased by quotes), int, float.
						// In case of cmd, also a cmd index is set.
};

//! Stores the final parsed results of each line.
struct SKMParsedLine {
	//! Actual line number.
	int32_t number = 0;
	
	//! Total character count (width) of the line.
	int32_t width = 0;

	//! String value containing line data.
	std::string value = "";

	//! Filled with 'categorized' type tokens found in the line's string value.
	std::vector<SKMParsedToken> tokens;
};

//! Maintains state of the current configuration parsing process.
struct SKMParseState {
	//! The current animation, set by animation command.
	std::string animationName = "";

	//! Keep score of how many actions we've got so far, and use this as their index value.
	int32_t actionCount = 0;
	//! Same, but then for blend actions.
	int32_t blendActionCount = 0;
	//! Same, but then for animations.
	int32_t animationCount = 0;

	//! Stores the parsed lines and their tokens for this parse configuration state.
	std::vector<SKMParsedLine> parsedLines;
};

//! String list containing our allowed Command Identifiers.
static std::vector< std::string > skmCommandIdentifiers = {
	"rootbone",
	"headbone",
	"torsobone",
	"action",
	"animation",
	"blendaction",
};


// skeletal model config parse state.
class GPPSkeletalModelConfiguration : public GPPState {
public:
	GPPSkeletalModelConfiguration(const std::string &buffer) : GPPState(buffer) {};

	int32_t currentAnimationIndex = 0;
	int32_t currentActionIndex = 0;
	int32_t currentBlendActionIndex = 0;
};

/**
*	@brief	Returns a pointer to the token if found.
**/
static GPPSourceToken *SKC_GetToken( GPPSkeletalModelConfiguration &gppState, uint32_t index ) {
	if ( index < gppState.parsedTokens.size() ) {
		return &gppState.parsedTokens[index];
	}
	
	return nullptr;
}

/**
*	@brief	Processes the tokens for the CommandIdentifier: 'rootbone'.
**/
static const uint32_t SKC_Command_RootBone( model_t *model, GPPSkeletalModelConfiguration &gppState, uint32_t tokenIndex ) {
	//.SKM Pointer.
	SkeletalModelData *skm = model->skeletalModelData;

	// Keeps score of the next commandidentifier token position we'll be returning.
	uint32_t offsetNextToken = 2;

	// Now inspect and acquire our token values.
	// Required 'QuotedString'/'IntegralNumber' Token: rootBoneIdentifier
	const GPPSourceToken *rootBoneIdentifier = SKC_GetToken( gppState, tokenIndex + 1 );
	// See which type it is.
	if ( rootBoneIdentifier ) {
		// String Identifier.
		if ( rootBoneIdentifier->type == GPPSourceToken::Type::QuotedString ) {
			// String Identifier.
			const std::string strBoneIdentifier = rootBoneIdentifier->value.str;

			// See if the joint exists in our name mape.
			if ( skm->jointMap.contains( strBoneIdentifier ) ) {
				// Set our root joint index.
				skm->rootJointIndex = skm->jointMap[ strBoneIdentifier ].index;
			} else {
				// TODO: Error.
				skm->rootJointIndex = -1;
				return 0;
			}
		// Integral Identifier.
		} else if ( rootBoneIdentifier->type == GPPSourceToken::Type::IntegralNumber ) {
			// Integral identifier.
			const int32_t intBoneIdentifier = rootBoneIdentifier->value.integralNumber;

			// See if it's within bounds.
			if ( intBoneIdentifier >= 0 && intBoneIdentifier < skm->numberOfJoints ) {
				skm->rootJointIndex = skm->jointArray[ intBoneIdentifier ].index;
			} else {
				skm->rootJointIndex = -1;
				// TODO: Error.
				return 0;
			}
		}
	} else {
		skm->rootJointIndex = -1;
		// TODO: Error.
		return 0;
	}

	return 2;
}

/**
*	@brief	Processes the tokens for the CommandIdentifier: 'action'.
**/
static const uint32_t SKC_Command_Action( model_t *model, GPPSkeletalModelConfiguration &gppState, uint32_t tokenIndex ) {
	//.SKM Pointer.
	SkeletalModelData *skm = model->skeletalModelData;

	// Keeps score of the next commandidentifier token position we'll be returning.
	uint32_t offsetNextToken = 5;

	// Now inspect and acquire our token values.
	// Required 'QuotedString' Token: actionName
	const GPPSourceToken *actionNameToken		= SKC_GetToken( gppState, tokenIndex + 1 );
	if ( !actionNameToken || actionNameToken->type != GPPSourceToken::Type::QuotedString || actionNameToken->value.str.empty() ) {
		// TODO: Error, it is out of bounds, bad type, or empty string value.
		return 0;
	}
	// Required 'IntegralNumber' Token: actionStart
	const GPPSourceToken *actionStartToken		= SKC_GetToken( gppState, tokenIndex + 2 );
	if ( !actionStartToken || actionStartToken->type != GPPSourceToken::Type::IntegralNumber ) {
		// TODO: Error, it is out of bounds, bad type.
		return 0;
	}
	// Required 'IntegralNumber' Token: actionEnd
	const GPPSourceToken *actionEndToken		= SKC_GetToken( gppState, tokenIndex + 3 );
	if ( !actionEndToken || actionEndToken->type != GPPSourceToken::Type::IntegralNumber ) {
		// TODO: Error, it is out of bounds, bad type.
		return 0;
	}
	// Required 'IntegralNumber' Token: actionLoopCount
	const GPPSourceToken *actionLoopCountToken	= SKC_GetToken( gppState, tokenIndex + 4 );
	if ( !actionLoopCountToken || actionLoopCountToken->type != GPPSourceToken::Type::IntegralNumber ) {
		// TODO: Error, it is out of bounds, bad type.
		return 0;
	}
	// Optional 'FloatNumber' Token: actionLoopCount
	const GPPSourceToken *actionFrametimeToken	= SKC_GetToken( gppState, tokenIndex + 5 );
	if ( !actionFrametimeToken || actionFrametimeToken->type != GPPSourceToken::Type::FloatNumber ) {
		// Be sure to nullptr it for its later condition check.
		actionFrametimeToken = nullptr;			
	// If it exists and is valid, decrement 1 token from our expected next offset.
	} else {
				offsetNextToken += 1;
	}

	// Now we can get our values to work with.
	const std::string actionName = actionNameToken->value.str;
	const uint32_t actionStart = actionStartToken->value.integralNumber;
	const uint32_t actionEnd = actionEndToken->value.integralNumber;
	const int32_t actionLoopCount = actionLoopCountToken->value.integralNumber;
	// Default to frametime if nullptr.
	const float actionFrametime = (actionFrametimeToken != nullptr ? actionFrametimeToken->value.floatNumber : BASE_FRAMETIME );

	// Do NOT re-add action if our map already contains one sharing the same name.
	if ( skm->actionMap.contains( actionName ) ) {
		// TODO: Error text.
	//	return false;
	}

	// Acquire our action index, we base it on the actions size for now.
	size_t actionIndex = gppState.currentActionIndex = skm->actions.size();

	// Emplace the action data.
	SkeletalAnimationAction *action = &( skm->actionMap[ actionName ] = (SkeletalAnimationAction{
		.index = static_cast< uint32_t >( actionIndex ),
		.name = actionName,
		.startFrame = actionStart,
		.endFrame = actionEnd,
		.numFrames = actionEnd - actionStart,
		.frametime = actionFrametime,
		.loopingFrames = static_cast< int32_t >( actionLoopCount < 0 ? 0 : actionLoopCount ),
		.forceLoop = (actionLoopCount < 0 ? true : false)
	}) );

	// Add a pointer to it into our linear access actions list.
	if ( skm->actions.size() <= actionIndex ) {
		skm->actions.resize( actionIndex + 1 );
	}
	skm->actions[ actionIndex ] = action;
	//skm->actions[ actionIndex ] = (*(skm->actions.end() - 1));

	// Calculate distances.
	if (skm->rootJointIndex != -1 && model->iqmData && model->iqmData->poses) {
		// Start and end pose pointers.
		const iqm_transform_t *startPose = &model->iqmData->poses[ skm->rootJointIndex + ( action->startFrame * model->iqmData->num_poses ) ];
		const iqm_transform_t *endPose = (action->startFrame == 0 ? startPose : &model->iqmData->poses[skm->rootJointIndex + ( (action->endFrame - 1) * model->iqmData->num_poses)] );

		// Get the start and end pose translations.
		const vec3_t startFrameTranslate	= startPose->translate;
		const vec3_t endFrameTranslate		= endPose->translate;

		// Used to store the total translation distance from startFrame to end Frame,
		// We use this in order to calculate the appropriate distance between start and end frame.
		// (Ie, assuming an animation loops, we need that.)
		vec3_t totalTranslateDistance = vec3_zero();

		// The offset between the previous processed and the current processing frame.
		vec3_t offsetFrom = vec3_zero();

		for (int32_t i = action->startFrame; i < action->endFrame; i++) {
			// Get the Frame Pose.
			const iqm_transform_t *framePose = &model->iqmData->poses[skm->rootJointIndex + (i * model->iqmData->num_poses)];

			// Special Case: First frame has no offset really.
			if (i == action->startFrame) {
				//const vec3_t totalStartTranslation = endFrameTranslate - startFrameTranslate;
				// Push the total traversed frame distance.
				const vec3_t frameTranslate = offsetFrom - framePose->translate;

				action->frameDistances.push_back( vec3_length( frameTranslate ) );

				// Push the total translation between each frame.					
				action->frameTranslates.push_back( frameTranslate );

				// Prepare offsetFrom with the current pose's translate for calculating next frame.
				offsetFrom = framePose->translate;
			// General Case: Set the offset we'll be coming from next frame.
			} else {
						
				// Calculate translation between the two frames.
				const vec3_t translate = offsetFrom - framePose->translate;
				//const vec3_t translate = offsetFrom - framePose->translate;

				// Calculate the distance between the two frame translations.
				const double frameDistance = vec3_distance_squared( offsetFrom, framePose->translate ); //vec3_dlength( translate );

				// Push the total traversed frame distance.
				action->frameDistances.push_back( frameDistance );

				// Push the total translation between each frame.					
				action->frameTranslates.push_back( translate );

				// Prepare offsetFrom with the current pose's translate for calculating next frame.
				offsetFrom = framePose->translate;
			}
		}

		// Sum up all frame distances into one single value.
		//vec3_dlength(totalTranslationSum); //0.0;
		action->animationDistance = 0.0;
		for (auto& distance : action->frameDistances) {
			action->animationDistance += distance;
		}
	}

	return offsetNextToken;
}

/**
*	@brief	Processes the tokens for the CommandIdentifier: 'animation'.
**/
static const uint32_t SKC_Command_Animation( model_t *model, GPPSkeletalModelConfiguration &gppState, uint32_t tokenIndex ) {
	//.SKM Pointer.
	SkeletalModelData *skm = model->skeletalModelData;

	// Keeps score of the next commandidentifier token position we'll be returning.
	uint32_t offsetNextToken = 2;

	// Now inspect and acquire our token values.
	// Required 'QuotedString' Token: actionName
	const GPPSourceToken *animationNameToken		= SKC_GetToken( gppState, tokenIndex + 1 );
	if ( !animationNameToken || animationNameToken->type != GPPSourceToken::Type::QuotedString || animationNameToken->value.str.empty() ) {
		// TODO: Error, it is out of bounds, bad type, or empty string value.
		return 0;
	}

	// Now we can get our values to work with.
	const std::string animationName = animationNameToken->value.str;

	// Make sure it is not already existent.
	if ( skm->animationMap.contains( animationName ) ) {
		// TODO: Error.
		//return 0;
	}

	gppState.currentAnimationIndex = skm->animations.size();

	// Actually emplace the animation if nonexistent.
	skm->animationMap[ animationName ] = SkeletalAnimation{ 
		.index = gppState.currentAnimationIndex,
		.name = animationName
	};
		
	// Add a pointer to it into our linear access actions list.
	if ( skm->animations.size() <= gppState.currentAnimationIndex ) {
		skm->animations.resize( gppState.currentAnimationIndex + 1 );
	}
	skm->animations[ gppState.currentAnimationIndex ] = &skm->animationMap[ animationName ];

	// Done.
	return offsetNextToken;
}

/**
*	@brief	Processes the tokens for the CommandIdentifier: 'rootbone'.
**/
static const uint32_t SKC_Command_BlendAction( model_t *model, GPPSkeletalModelConfiguration &gppState, uint32_t tokenIndex ) {
	//.SKM Pointer.
	SkeletalModelData *skm = model->skeletalModelData;

	// Keeps score of the next commandidentifier token position we'll be returning.
	uint32_t offsetNextToken = 4;

	// Now inspect and acquire our token values.
	// Required 'QuotedString' Token: actionName
	const GPPSourceToken *actionNameToken		= SKC_GetToken( gppState, tokenIndex + 1 );
	if ( !actionNameToken || actionNameToken->type != GPPSourceToken::Type::QuotedString || actionNameToken->value.str.empty() ) {
		// TODO: Error, it is out of bounds, bad type, or empty string value.
		return 0;
	}
	// Required 'FloatNumber' Token: blendaction fraction.
	const GPPSourceToken *blendActionFractionToken = SKC_GetToken( gppState, tokenIndex + 2 );
	if ( !blendActionFractionToken || blendActionFractionToken->type != GPPSourceToken::Type::FloatNumber ) {
		// TODO: Error, it is out of bounds, bad type.
		return 0;
	}
	// Optional 'QuotedString' Token: actionLoopCount
	const GPPSourceToken *blendActionBonenameToken = SKC_GetToken( gppState, tokenIndex + 3 );
	if ( !blendActionBonenameToken || blendActionBonenameToken->type != GPPSourceToken::Type::QuotedString || blendActionBonenameToken->value.str.empty()) {
		// Do nothing, optional token.
	// If it exists and is valid, decrement 1 token from our expected next offset.
		offsetNextToken -= 1;

		// Be sure to nullptr it for its later condition check.
		blendActionBonenameToken = nullptr;
	} else {

	}

	// Now we can get our values to work with.
	const std::string actionName = actionNameToken->value.str;
	const float blendActionFraction = blendActionFractionToken->value.floatNumber;
	// Default to nothing if nullptr.
	const std::string blendActionBonename = (blendActionBonenameToken != nullptr ? blendActionBonenameToken->value.str : "" );

	// Animation is existent, now find the action.
	if ( skm->actionMap.contains( actionName ) ) {
		// Get action index.
		const uint16_t actionIndex = skm->actionMap[ actionName ].index;
		
		// Get the animation name to index with, it's the last in the vector list.
		const std::string animationName = skm->animations[ gppState.currentAnimationIndex ]->name;

		// If the bone name is empty, we assume it is a dominating animator.
		if ( blendActionBonename.empty() ) { 
				// We're ready, time to push back our blend action.
				skm->animationMap[ animationName ].blendActions.push_front({
					.actionIndex = actionIndex,
					.fraction = blendActionFraction,
					.boneNumber = 0
				});
		} else {
			// Ensure the bone is existent.
			if ( skm->jointMap.contains( blendActionBonename ) ) {
				// Bone index.
				const uint16_t boneIndex = skm->jointMap[ blendActionBonename ].index;

				// We're ready, time to push back our blend action.
				skm->animationMap[ animationName ].blendActions.push_back({
					.actionIndex = actionIndex,
					.fraction = blendActionFraction,
					.boneNumber = boneIndex
				});
			} else {
				//errorString = "Couldn't find bone: '" + blendActionBonename + "'";
				return false;
			}
		}


	} else {
		//errorString = "Error finding '" + blendActionBonename + "' for blendaction.";
		return false;
	}


	return offsetNextToken;
}

/**
*	@brief	Processes all parsed GPPSourceTokens in-order, executing each command identifier
*			resulting in our final skeletal model data.
**/
static const bool SKC_ProcessTokens( model_t *model, GPPSkeletalModelConfiguration &gppState ) {
	// Ensure we got tokens to process.
	if ( !gppState.parsedTokens.size() ) {
		//Com_Error( ErrorType::Drop, fmt::format("{}: No tokens to process, &tokens has size 0.\n", __func__ ).c_str() );
		return false;
	}

	uint32_t offsetNextToken = 0;
	uint32_t tokenIndex = 0;

	GPPSourceToken *token = SKC_GetToken( gppState, 0 );

	// We go over all tokens, however, additioning the token count that was processed for each action.
	while ( tokenIndex < gppState.parsedTokens.size() && token != nullptr ) {

		// It MUST be type CommandIdentifier.
		if ( !(token->type == GPPSourceToken::Type::CommandIdentifier ) ) {
		//	Com_Error( ErrorType::Drop, fmt::format("{}: Bad token data, expected CommandIdentifier, didn't get it....\n", __func__ ).c_str() );
			return false;
		}

		/*
		*	CommandIdentifier: 'rootbone'
		*/
		if ( token->value.str == "rootbone" ) { 
			// Process command tokens and break out in case of errors.
			offsetNextToken = SKC_Command_RootBone( model, gppState, tokenIndex );
		}
		/*
		*	CommandIdentifier: 'action'
		*/
		else if ( token->value.str == "action" ) { 
			// Process command tokens and break out in case of errors.
			offsetNextToken = SKC_Command_Action( model, gppState, tokenIndex );
		}
		/*
		*	CommandIdentifier: 'animation'
		*/
		else if ( token->value.str == "animation" ) { 
			// Process command tokens and break out in case of errors.
			offsetNextToken = SKC_Command_Animation( model, gppState, tokenIndex );
		}
		/*
		*	CommandIdentifier: 'blendaction'
		*/
		else if ( token->value.str == "blendaction" ) { 
			// Process command tokens and break out in case of errors.
			offsetNextToken = SKC_Command_BlendAction( model, gppState, tokenIndex );
	
			if (offsetNextToken) {
				gppState.currentBlendActionIndex++;
			}
			//continue;
		} else {
			offsetNextToken = 1;
		}


		if (!offsetNextToken) {
			// Some error output.
			return false;
		}

		// Otherwise, add our next token offset to our current index.
		tokenIndex += offsetNextToken;

		// Get proper next token.
		token = SKC_GetToken( gppState, tokenIndex );
	}

	return true;
}


/**
*	@brief	Parsed a Skeletal Model Configuration file for animation actions, tag and blend data.
**/
static const bool SKM_ParseConfiguration( model_t *model, const std::string &cfgBuffer ) {
	// Ensure model is valid.
	if ( !model ) {
		//Com_Error( ErrorType::Drop, fmt::format( "{}: Can't parse configuration for a model_t(nullptr).\n", __func__ ).c_str() );
		return false;
	}
	/**
	*	Our SKC GPP State.
	**/
	GPPSkeletalModelConfiguration gppSKC( cfgBuffer );

	/**
	*	First parse using our General Purpose Parser(GPP).
	**/
	const bool tokenized = GPP_ParseTokens( gppSKC, skmCommandIdentifiers, false );

	/**
	*	Every time we reach around, 
	**/	
	return (tokenized ? SKC_ProcessTokens( model, gppSKC ) : false);
}

/**
*	@brief	Loads up a skeletal model configuration file and passes its buffer over
*			to the parsing process. The process tokenizes the data and generates game
*			code friendly POD to work with.
**/
bool SKM_LoadAndParseConfiguration(model_t *model, const std::string &filePath) {
	// Stores the actual buffer content.
	char *fileBuffer; //, * data, * p;// , * cmd;
	qerror_t ret = 0;

	// Load file into buffer.
	ret = FS_LoadFile(filePath.c_str(), (void **)&fileBuffer);

	if (!fileBuffer) {
		if (ret != Q_ERR_NOENT) {
			// Generate our development output string using fmt.
			const std::string devPrintStr = fmt::format(
				"Couldn't load '{}': {}",
				filePath,
				Q_ErrorString(ret)
			);
			// Output.
			Com_DPrintf(devPrintStr.c_str());
		}
		// Return failure.
		return false;
	}

	// Start parsing our buffer.
	SKM_ParseConfiguration(model, fileBuffer);

	////////////////////////////
	//// Debug Output.
	//SkeletalModelData *skm = model->skeletalModelData;

	//Com_DPrintf("------------------------------------------------------\n");
	//Com_DPrintf("Configuration resulted in the following action data:\n");
	//Com_DPrintf("Actions:\n");
	//int32_t index = 0;
	//for (auto &iterator : skm->actionMap) {
	//	const std::string name = iterator.first;
	//	auto *action = &iterator.second;

	//	Com_DPrintf("Action(#%i, %s): (startFrame=%i, endFrame=%i, numFrames=%i), (loop=%s, loopFrames=%i), (animationDistance=%f):\n",
	//		action->index,
	//		name.c_str(), //animation.name, Since, temp var and .c_str()
	//		action->startFrame,
	//		action->endFrame,
	//		action->numFrames,
	//		action->forceLoop == true ? "true" : "false",
	//		action->loopingFrames,
	//		action->animationDistance);
	//}
	//Com_DPrintf("---:\n");
	//Com_DPrintf("Animations:\n");

	//int32_t i = 0;
	//for (auto &iterator : skm->animationMap) {
	//	const std::string name = iterator.first;
	//	auto *blendAction = &iterator.second;

	//	Com_DPrintf("    Animation(#%i, %s):\n", i, name.c_str() );
	//	int32_t j = 0;
	//	for (auto &blendAction : blendAction->blendActions) {
	//		const uint16_t actionIndex = blendAction.actionIndex;
	//		const float fraction = blendAction.fraction;

	//		SkeletalAnimationAction *action = skm->actions[ actionIndex ];
	//		if (j == 0) {
	//			// Get the actual action belonging to this
	//			Com_DPrintf("        blendAction(#%i, %s, %f, [Animation Dominator]):\n", j, action->name.c_str(), fraction );
	//		} else {
	//			// Get bone index.
	//			const int32_t boneIndex = blendAction.boneNumber;

	//			// Get bone name.
	//			const std::string boneName = skm->jointArray[boneIndex].name;

	//			Com_DPrintf("        blendAction(#%i, %s, %f, From Bone: %s):\n", j, action->name.c_str(), fraction, boneName.c_str() );

	//		}
	//		j++;
	//	}
	//	i++;
	//}
	//Com_DPrintf("------------------------------------------------------\n");
	////////////////////

	// We're done working with this file, free it from memory.
	FS_FreeFile( fileBuffer );

	// Return success.
	return true;
}