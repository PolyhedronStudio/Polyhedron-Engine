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
#include "../../ServerGameLocals.h"   // SVGame.
#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.
#include "../../Physics/StepMove.h"  // Stepmove funcs.

// Server Game Base Entity.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseSkeletalAnimator.h"


//! Constructor/Destructor.
SVGBaseSkeletalAnimator::SVGBaseSkeletalAnimator(PODEntity *svEntity) : Base(svEntity) { }
SVGBaseSkeletalAnimator::~SVGBaseSkeletalAnimator() { }


/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseSkeletalAnimator::Precache() { 
	Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseSkeletalAnimator::Spawn() { Base::Spawn(); }
/**
*   @brief 
**/
void SVGBaseSkeletalAnimator::PostSpawn() { Base::PostSpawn(); }
/**
*   @brief 
**/
void SVGBaseSkeletalAnimator::Respawn() { Base::Respawn(); }
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
}


/***
* 
*   Entity functions.
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
int32_t SVGBaseSkeletalAnimator::SwitchAnimation(const std::string& name) {
	// Get state pointer.
	EntityState *currentState	= &podEntity->currentState;
	// Get animation state.
	EntityAnimationState *currentAnimationState	= &currentState->currentAnimation;

	// Can't switch animations if we got no skm.
	if (!skm) {
		currentAnimationState->animationIndex = 0;
		return 0;
	}

	// Can't switch without containing information info matching the name.
	if (!skm->animationMap.contains(name)) {
		currentAnimationState->animationIndex = 0;
		return 0;
	}

	// Get AnimationMap.
	auto animationMap = &skm->animationMap;

	// Fail if this animation is non existent.
	if ( !animationMap->contains( name ) ) {
		currentAnimationState->animationIndex = 0;
		return 0;
	}

	// Get the name matching animation specific info.
	SkeletalAnimation *animation = &skm->animationMap[name];

	// If we're already in this animation, return index but don't reset it.
	if (animation->index == currentAnimationState->animationIndex) {
		return animation->index;
	}

	// Wired Data:
	currentAnimationState->animationIndex = animation->index;
	currentAnimationState->startTime = level.time.count();

	// Non-Wired Data:
	currentAnimationState->frame = animation->startFrame;
	currentAnimationState->startFrame = animation->startFrame;
	currentAnimationState->endFrame = animation->endFrame;
	currentAnimationState->frameTime = animation->frametime;
	currentAnimationState->loopCount = animation->loopingFrames;
	currentAnimationState->forceLoop = animation->forceLoop;

	// Engage the switch by being ahead of time and processing the animation we just set for
	// the current moment in time. The state will then be adjusted meaning that the client receives
	// the new animation data the first frame he gets from us.
	ProcessSkeletalAnimationForTime(level.time);

	return animation->index;
}