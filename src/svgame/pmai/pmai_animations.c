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
	self->pmai.animations.list[animationID].startframe	= startFrame;
	self->pmai.animations.list[animationID].endframe = endFrame;
	self->pmai.animations.list[animationID].currentframe = 0;
	self->pmai.animations.list[animationID].loop = loop;
}