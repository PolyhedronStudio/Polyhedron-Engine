// LICENSE HERE.

//
// pmai_animations.c
//
//
// Animation functions for the PMAI system.
//
#include "../g_local.h"
#include "../g_pmai.h"

//
//==========================================================================
//
// ANIMATIONS
//
//==========================================================================
//

//
//===============
// PMAI_FillAnimation
// 
// Utility functions for filling up animations with.
//===============
//
void PMAI_FillAnimation(edict_t* self, int animationID, int startFrame, int endFrame, qboolean loop) {
	// Ensure animationID is within bounds.
	if (animationID < 0 && animationID > 20)
		return;

	// Fill animation.
	self->pmai.animations.list[animationID].startFrame		= startFrame;
	self->pmai.animations.list[animationID].endFrame		= endFrame;
	self->pmai.animations.list[animationID].currentFrame	= 0;
	self->pmai.animations.list[animationID].loop			= loop;
	self->pmai.animations.list[animationID].frameCallback	= NULL;
}

//
//===============
// PMAI_SetAnimation
// 
// Switches active animation. In case this gets called multiple times a game
// tick, it won't reset in case the animation value is the same.
//===============
//
void PMAI_SetAnimation(edict_t* self, int animationID) {
	// Ensure animationID is within bounds.
	if (animationID < 0 && animationID > 20)
		return;

	// Reset the animation at hand.
	if (self->pmai.animations.current != animationID)
		self->pmai.animations.list[animationID].currentFrame = 0;

	// Set the current active animation.
	self->pmai.animations.current = animationID;
}

//
//===============
// PMAI_SetAnimationFrameCallback
// 
// Sets a function pointer callback which will be called upon each frame of the
// animation so it can execute specific code belonging to the frame.
//===============
//
void PMAI_SetAnimationFrameCallback(edict_t* self, int animationID, PMAI_AnimationFrameCallback callback) {
	// Ensure it is valid.
	if (animationID < 0 && animationID > 20)
		return;

	// Setup the callback.
	self->pmai.animations.list[animationID].frameCallback = callback;
}

//
//===============
// PMAI_ProcessAnimation
// 
// Handles playing of the actual current animation.
//===============
//
void PMAI_ProcessAnimation(edict_t* self) {
	// Ensure it is valid.
	if (!self)
		return;

	// Find the current animationID.
	int animationID = self->pmai.animations.current;

	// Calculate the frame to use, and whether to reset animation.
	int frameIndex = self->pmai.animations.list[animationID].startFrame + self->pmai.animations.list[animationID].currentFrame;

	// Check whether the animation needs a reset to loop.
	if (self->pmai.animations.list[animationID].loop == true) {
		if (frameIndex > self->pmai.animations.list[animationID].endFrame) {
			// Reset currentframe.
			self->pmai.animations.list[animationID].currentFrame = 0;

			// Recalculate frame index.
			frameIndex = self->pmai.animations.list[animationID].startFrame + self->pmai.animations.list[animationID].currentFrame;
		}
	}
	// No looping, make sure it clamps.
	else {
		// In case we aren't looping, and exceeding, clamp the animation frames.
		if (frameIndex > self->pmai.animations.list[animationID].endFrame) {
			// Cap animation index to the end frame.
			frameIndex = self->pmai.animations.list[animationID].currentFrame = self->pmai.animations.list[animationID].endFrame;
		}
	}

	// In case the animation has a callback, call it.
	if (self->pmai.animations.list[animationID].frameCallback) {
		self->pmai.animations.list[animationID].frameCallback(self, animationID, frameIndex);
	}

	// Assign the animation frame to the entity state.
	self->s.frame = frameIndex;

	// Process animation.
// Silly hack for the 20 fps, right now this model is meant for 10 fps...
	static int firstFramePass = false;

	if (firstFramePass == false) {
		self->pmai.animations.list[animationID].currentFrame++;
		firstFramePass = true;
	}
	else {
		firstFramePass = false;
	}
// End Silly hack for the 20 fps, right now this model is meant for 10 fps...
}
