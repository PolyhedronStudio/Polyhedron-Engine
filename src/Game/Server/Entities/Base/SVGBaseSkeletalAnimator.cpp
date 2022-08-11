/***
*
*	License here.
*
*	@file
*
*	Used for any entity that makes use of a skeletal animated model. (IQM in our case.)
*
*	Brings basic animation utilities to the entity such as playing, pausing, and switching
*	to and from animations. 
*
*	On top of that, it also introduces model events. These can be customly defined by the
*	developer and hooked to by setting a callback function. Example uses of this can be:
*	 - Play a footstep audio exactly the frame where the foot hits the floor. (Perform a trace too.)
*	 - Spawn a muzzleflash and/or bullet shell at the exact frame where required.
*	 - ... Be creative and use it :-) ...
*
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.

// Server Game Base Entity.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseSkeletalAnimator.h"


//! Constructor/Destructor.
SVGBaseSkeletalAnimator::SVGBaseSkeletalAnimator(PODEntity *svEntity) : Base(svEntity) { }

/***
*
*
*   Interface functions.
*
*
***/
/**
*   @brief 
**/
void SVGBaseSkeletalAnimator::SpawnKey(const std::string& key, const std::string& value) { Base::SpawnKey(key, value); }

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseSkeletalAnimator::Think() { 
	// Base think.
	Base::Think();

	// Get state pointer.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get animation state.
	EntityAnimationState *currentAnimationState	= &currentState->currentAnimation;
	EntityAnimationState *previousAnimationState = &currentState->previousAnimation;

	// If we got a new animation to switch to, ensure we are allowed to switch before doing so.
	if (CanSwitchAnimation(currentAnimationState, animationToSwitchTo)) {
		const std::string animName = skm->animations[animationToSwitchTo]->name;
		SwitchAnimation(animName);
		//ProcessSkeletalAnimationForTime(level.time);
		//.Otherwise keep processing the current animation frame for time.
	} else {
		ProcessSkeletalAnimationForTime(level.time);
	}
}



/***
* 
*
*   Animation functions.
*
*
***/
/**
*   @brief	Processes the server-side animation state.
**/
void SVGBaseSkeletalAnimator::ProcessSkeletalAnimationForTime(const GameTime &time) {
	// Get state references.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get Animation State references.
	EntityAnimationState *currentAnimation	= &currentState->currentAnimation;
	EntityAnimationState *previousAnimation	= &currentState->previousAnimation;

	// Backup previous animation for this entity state.
	*previousAnimation = *currentAnimation;

	// And start processing the new, current state.
    currentAnimation->backLerp = 1.0f - SG_FrameForTime(&currentAnimation->frame, // Pointer to frame storage variable.
        GameTime(time),                        // Current Time.
        GameTime(currentAnimation->startTime), // Animation Start time.
        currentAnimation->frameTime,  // Animation Frame Time.
        currentAnimation->startFrame, // Start frame.
        currentAnimation->endFrame,   // End frame.
        currentAnimation->loopCount,  // Loop count.
        currentAnimation->forceLoop   // Force loop
    );
}

/**
*	@brief	Switches the animation by blending from the current animation into the next.
*	@return	The animation index on success, -1 on failure.
**/
int32_t SVGBaseSkeletalAnimator::SwitchAnimation( const std::string& name ) {
	// Get state pointer.
	EntityState *currentState = &podEntity->currentState;
	// Get animation state.
	EntityAnimationState *currentAnimationState	= &currentState->currentAnimation;

	// Get the name matching animation. Reset and return index in case of failing to do so.
	SkeletalAnimation *animation = GetAnimation( name );//&skm->animationMap[name];
	if ( !animation ) {
		return currentAnimationState->animationIndex = 0;
	}

	// If we're already in this animation, return index but don't reset it.
	if (animation->index == currentAnimationState->animationIndex) {
		return animation->index;
	}

	// Wired Data:
	currentAnimationState->animationIndex = animation->index;
	currentAnimationState->startTime = level.time.count();

	// Get the dominating blend action of the animation. Reset and return index in case of failing to do so.
	SkeletalAnimationBlendAction *blendAction = GetBlendAction( animation, 0 );
	if ( !blendAction ) {
		return currentAnimationState->animationIndex = 0;
	}
	// Get the 'root' action, always index 0. Reset and return index in case of failing to do so.
	SkeletalAnimationAction *action = skm->actions[ animation->blendActions[0].actionIndex ];
	if ( !action ) {
		return currentAnimationState->animationIndex = 0;
	}

	// Non-Wired Data:
	//currentAnimationState->frame = action->startFrame;
	currentAnimationState->startFrame = action->startFrame;
	currentAnimationState->endFrame = action->endFrame;
	currentAnimationState->frameTime = action->frametime;
	currentAnimationState->loopCount = action->loopingFrames;
	currentAnimationState->forceLoop = action->forceLoop;

	// Engage the switch by being ahead of time and processing the animation we just set for
	// the current moment in time. The state will then be adjusted meaning that the client receives
	// the new animation data the first frame he gets from us.
	ProcessSkeletalAnimationForTime(level.time);

	// Return the newly set animation index on successfully switching.
	return animation->index;
}

/**
*	@brief	Prepares an animation to switch to after the current active animation has
*			finished its current cycle from start to end -frame.
**/
int32_t SVGBaseSkeletalAnimator::PrepareAnimation( const std::string &name, const bool force ) {
	// Get animation, if failing to do so reset animation switching to none.
	SkeletalAnimation *animation = GetAnimation( name );
	if ( !animation ) {
		forcedAnimationSwitch = false;
		return animationToSwitchTo = -1;
	}

	// Store force switch or not.
	forcedAnimationSwitch = force;

	// Set and return switch to index.
	return animationToSwitchTo = animation->index;
}

/**
*	@brief
**/
const bool SVGBaseSkeletalAnimator::AnimationFinished( const EntityAnimationState *animationState )  {
		// Get the frame and end frame.
	const int32_t animationStartFrame	= animationState->startFrame;
	const int32_t animationEndFrame	= animationState->endFrame;
	const int32_t animationFrame	= animationState->frame;
	const float animationBacklerp = animationState->backLerp;

	// We purposely do not do: animationFrame <= startFrame, because even
	// though -1 complies to this rule, if we are coming from an animation
	// that has a lesser frame index it'd still be a true condition and make
	// things messy.
	if (animationBacklerp == 0 && (animationFrame == -1 || animationFrame == animationEndFrame) ) {
		return true;
	}

	// Did not end.
	return false;
}

/**
*	@brief
**/
const bool SVGBaseSkeletalAnimator::CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex )  {
		if (wishedAnimationIndex < 0) {
			return false;
		}
		if ( forcedAnimationSwitch ) {
			forcedAnimationSwitch = false;
			return true;
		}
		if ( animationState->animationIndex != wishedAnimationIndex ) {

			if ( AnimationFinished( animationState ) ) {
				return true;
			}
		}

		return false;
}



/***
*
*
*	Utility Functions, for easy bounds checking and sorts of tasks alike.
*
*
***/
/**
*	@brief	Utility function to test whether an animation is existent and within range.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified action.
**/
SkeletalAnimation *SVGBaseSkeletalAnimator::GetAnimation( const std::string &name ) {
	// Return (nullptr) since we had no Skeletal Model Data to check on.
	if ( !skm ) {
		return nullptr;
	}

	// Return (nullptr) in case the name is nonexistent in our Animation map.
	if ( !skm->animationMap.contains(name) ) {
		return nullptr;
	}

	// We're good, return a pointer to the SkeletalAnimation.
	return &skm->animationMap[ name ];
}
SkeletalAnimation *SVGBaseSkeletalAnimator::GetAnimation( const int32_t index ) {
	// Return (nullptr) since we had no Skeletal Model Data to check on.
	if ( !skm ) {
		return nullptr;
	}

	// Return (nullptr) in case the index is out of bounds.
	if ( index < 0 || index >= skm->animations.size() ) {
		return nullptr;
	}

	// Return the pointer stored by index within the Animations vector.
	return skm->animations[ index ];
}

/**
*	@brief	Utility function to easily get a pointer to an Action by name or index.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified Action.
**/
SkeletalAnimationAction *SVGBaseSkeletalAnimator::GetAction( const std::string &name ) {
	// Return (nullptr) since we had no Skeletal Model Data to check on.
	if ( !skm ) {
		return nullptr;
	}

	// Return (nullptr) in case the name is nonexistent in our Action map.
	if ( !skm->actionMap.contains(name) ) {
		return nullptr;
	}

	// We're good, return a pointer to the SkeletalAnimationAction.
	return &skm->actionMap[ name ];
}
SkeletalAnimationAction *SVGBaseSkeletalAnimator::GetAction( const int32_t index ) {
	// Return (nullptr) since we had no Skeletal Model Data to check on.
	if ( !skm ) {
		return nullptr;
	}

	// Return (nullptr) in case the index is out of bounds.
	if ( index < 0 || index >= skm->actions.size() ) {
		return nullptr;
	}

	// Return the pointer stored by index within the Actions vector.
	return skm->actions[ index ];
}

/**
*	@brief	Utility function to test whether a BlendAction is existent and within range.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified BlendAction action.
**/
SkeletalAnimationBlendAction *SVGBaseSkeletalAnimator::GetBlendAction( SkeletalAnimation *animation, const int32_t index ) {
	// Return (nullptr) since we had no Skeletal Model Data to check on.
	if ( !skm ) {
		return nullptr;
	}

	// Return (nullptr) since we had no SkeletalAnimation to check on.
	if ( !animation ) {
		return nullptr;
	}

	// Return (nullptr) in case the index is out of bounds.
	if ( index < 0 || index >= animation->blendActions.size() ) {
		return nullptr;
	}

	// We're good, return a pointer to the SkeletalAnimationAction.
	return &animation->blendActions[ index ];
}
