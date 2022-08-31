/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.


// Base Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseSkeletalAnimator.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

#include "Game/Shared/Physics/Physics.h"
#include "Game/Shared/Physics/RootMotionMove.h"

// Base Slide Monster.
#include "SVGBaseRootMotionAnimator.h"

// Game World.
#include "../../World/ServerGameWorld.h"



//! Constructor/Destructor.
SVGBaseRootMotionAnimator::SVGBaseRootMotionAnimator(PODEntity *svEntity) : Base(svEntity) { 
}



/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseRootMotionAnimator::Spawn() { 
	// Use an Octagon Shaped Hull by default. Allows for more realistic character sliding.
    SetSolid( Solid::OctagonBox );
	// Set move type.
    SetMoveType( MoveType::RootMotionMove );
}



/***
*
*
*	Animation Functionality:
*
*
***/
/**
*	@return	A pointer to the current animation state.
**/
const EntityAnimationState* SVGBaseRootMotionAnimator::GetCurrentAnimationState() {
	return &podEntity->currentState.currentAnimation;
}

/**
*	@return	Calculates a 0 starting index based current frame for the given
*			animation state.
**/
const int32_t SVGBaseRootMotionAnimator::GetAnimationStateRelativeFrame( const EntityAnimationState* animationState ) {
	// Return 0 in case there is no valid state data.
	if ( !animationState ) {
		return 0;
	}

	// The actual relative frame we're in.
	int32_t actionRelativeFrame = animationState->frame;

	// The animation runs by the 'dominant' blend action, so we calculate the actual 'relative' end frame,
	// since IQM stores all animations in 1 continuous array of frames.
	const int32_t actionRelativeEndFrame = animationState->endFrame - animationState->startFrame;

	// Ensure if it is -1, it means the action's animation has ended so we want to remain in the end frame for this moment in time.
	if (actionRelativeFrame == -1) {
		actionRelativeFrame = actionRelativeEndFrame;
	}

	// Subtract from current frame to get and return the 0 start index based frame number.
	return Clampf( actionRelativeFrame - animationState->startFrame, 0, animationState->endFrame );
}

/**
*	@brief	Sets the 'translate' vector to the value of the 'root bone's' requested frame number 
*			translation
*	@return	True if the 'translate' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionAnimator::GetActionFrameTranslate( const int32_t actionIndex, const int32_t actionFrame, vec3_t& rootBoneTranslation ) {
	// Get animation, if failing to do so reset animation switching to none.
	SkeletalAnimationAction *action = GetAction( actionIndex );
	if ( !action ) {
		return false;
	}

	// We assume the pointer is not tempered with.
	auto &frameTranslates = action ->frameTranslates;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( actionFrame < 0 || actionFrame >= frameTranslates.size() ) {
		// Can't find Translation data.
		rootBoneTranslation = vec3_zero();
		// Failure.
		return false;
	} else {
		rootBoneTranslation = frameTranslates[ actionFrame ];

		// See if there are any specific rootBoneAxisFlags set.
		const int32_t rootBoneAxisFlags = action->rootBoneAxisFlags;

		// Zero out X Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation) ) {
			rootBoneTranslation.x = 0.0;
		}
		// Zero out Y Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation) ) {
			rootBoneTranslation.y = 0.0;
		}
		// Zero out Z Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation) ) {
			rootBoneTranslation.z = 0.0;
		}

		// Success.
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionAnimator::GetActionFrameTranslate( const std::string &actionName, const int32_t actionFrame, vec3_t& rootBoneTranslation ) {
	// Need a valid skm.
	if ( !skm ) {
		return false;
	}

	// See if the action data exists.
	if ( !skm->actionMap.contains( actionName ) ) {
		return false;
	}

	return GetActionFrameTranslate( skm->actionMap[ actionName ].index, actionFrame, rootBoneTranslation);
}
/**
*	@brief	Sets the 'distance' double to the value of the 'root bones' requested frame number 
*			translation distance. (vec3_dlength)
*	@return	True if the 'distance' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionAnimator::GetActionFrameDistance( const int32_t actionIndex, const int32_t actionFrame, double &rootBoneDistance ) {
	// Get animation, if failing to do so reset animation switching to none.
	SkeletalAnimationAction *action = GetAction( actionIndex );
	if ( !action ) {
		return false;
	}

	// We assume the pointer is not tempered with.
	auto &frameDistances = action->frameDistances;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( actionFrame < 0 || actionFrame >= frameDistances.size() ) {
		// TODO: gi.DPrintf(..)
		rootBoneDistance = 0.0;
		return false;
	} else {
		rootBoneDistance = frameDistances[actionFrame];
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionAnimator::GetActionFrameDistance( const std::string &actionName, const int32_t actionFrame, double &rootBoneDistance ) {
	// Need a valid skm.
	if ( !skm ) {
		return false;
	}

	// See if the action data exists.
	if ( !skm->actionMap.contains( actionName ) ) {
		return false;
	}

	return GetActionFrameDistance( skm->actionMap[ actionName ].index, actionFrame, rootBoneDistance );
}

/**
*	@brief	Calculated the move speed of the root bone for the given 'totalMoveDistance' and moveTranslate.
*	@return	Value of the calculated move speed.
**/
const double SVGBaseRootMotionAnimator::GetMoveSpeedForTraversedFrameDistance(const double &totalMoveDistance, const float &frameMoveDistance, const double &unitScale) {
	// Calculate move distance scaled to Quake units.
	const double scaledMoveDistance = unitScale * frameMoveDistance; //moveDistance * unitScale;

	// Length of move translate.
	const double moveFrameMoveDistance = frameMoveDistance; //vec3_dlength(moveTranslate);

	// Calculate move speed.
	if (scaledMoveDistance == 0) {
		//return 0;
	} else if (moveFrameMoveDistance == 0) {
		return 0;
	}
	return scaledMoveDistance / moveFrameMoveDistance;
}

/**
*	@brief	For future filling.
**/
const bool SVGBaseRootMotionAnimator::AnimationFinished( const EntityAnimationState *animationState ) {
	return Base::AnimationFinished( animationState );
}

/**
*	@brief	For future filling.
**/
const bool SVGBaseRootMotionAnimator::CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex ) {
	return Base::CanSwitchAnimation( animationState, wishedAnimationIndex );
}

/**
*	@brief	Can be overridden to add custom Monster animation change inspection.
*			When doing so, call Base::RefreshAnimation in case you want to resort
*			to so called 'basic behavior', explained blow.
*
*			The animation's are determined by te moveResultMasks. We inspect them for
*			steps, falling, and if there are steps ahead in order to keep on playing
*			a step up or down animation.
*
*			When none of the above is happening, we check to see if an animation has
*			finished playing so we can resort to our standard walk/run animation instead.
*			
**/
void SVGBaseRootMotionAnimator::RefreshAnimationState() {

}