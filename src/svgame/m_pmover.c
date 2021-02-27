/*
===========================================================================
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2000-2002 Mr. Hyde and Mad Dog

This file is part of Lazarus Quake 2 Mod source code.

Lazarus Quake 2 Mod source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Lazarus Quake 2 Mod source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lazarus Quake 2 Mod source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*
==============================================================================

PMover - AI using Player Movement.

==============================================================================
*/

#include "g_local.h"
#include "g_pmai.h"

// Include this one instead, for animations.
#include "m_pmover.h"

//
// NON IMPORTANT, ARE IN SHARED/SHARED.H but the GAME DLL IS STILL RIGGED WITH THAT KMQ2 ISSUE.
//
#define ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)
#define SHORT2ANGLE(x)  ((x)*(360.0/65536))

//
//=============================================================================
//
//	PMAI code for this monster entity.
//
//=============================================================================
//

//
//===============
// PMover_FillAnimations
// 
// Fill up the PMAI animations list with Player model animations.
//===============
//
void PMover_FillAnimations(edict_t* self) {
	// Ensure they are valid.
	if (!self)
		return;

	// 0 == stand01
	PMAI_FillAnimation(self, 0, FRAME_stand01,	FRAME_stand40,	true);
	// 1 == run1
	PMAI_FillAnimation(self, 1, FRAME_run1,		FRAME_run6,		true);
	// 2 == crwalk1
	PMAI_FillAnimation(self, 2, FRAME_crwalk1,	FRAME_crwalk6,	true);
	// 3 == jump1
	PMAI_FillAnimation(self, 3, FRAME_jump1,	FRAME_jump6,	true);
	// 17 == death101
	PMAI_FillAnimation(self, 17, FRAME_death101, FRAME_death106, false);
	// 18 == death201
	PMAI_FillAnimation(self, 18, FRAME_death201, FRAME_death206, false);
	// 19 == death301
	PMAI_FillAnimation(self, 19, FRAME_death301, FRAME_death308, false);
}

//
//===============
// PMover_FillAnimations
// 
// Switches active animation. In case this gets called multiple times a game
// tick, it won't reset in case the animation value is the same.
//===============
//
void PMover_SetAnimation(edict_t* self, int animationID) {
	// Ensure animationID is within bounds.
	if (animationID < 0 && animationID > 20)
		return;

	// Reset the animation at hand.
	if (self->pmai.animations.current != animationID)
		self->pmai.animations.list[animationID].currentframe = 0;

	// Set the current active animation.
	self->pmai.animations.current = animationID;
}

//
//===============
// PMover_DetectAnimation
// 
// Try and detect which animation to play based on the PMAI status.
//===============
//
void PMover_DetectAnimation(edict_t* self, usercmd_t* movecmd) {
	// State variables.
	qboolean moving_forward = false;
	qboolean moving_backward = false;
	qboolean strafing_left = false;
	qboolean strafing_right = false;
	qboolean jumping = false;
	qboolean crouching = false;

	// Ensure they are valid.
	if (!self || !movecmd)
		return;

	// Determine the boolean states based on the input.
	// Forward moving.
	if (movecmd->forwardmove > 1) {
		moving_forward = true;
	}
	// Strafe moving.
	if (movecmd->sidemove < 0) {
		strafing_left = true;
	}
	else if (movecmd->sidemove > 0) {
		strafing_right = true;
	}
	// Jumping.
	if (movecmd->upmove > 50) {
		jumping = true;
		crouching = false;
	}
	// Crouching.
	if (movecmd->upmove < 50) {
		jumping = false;
		crouching = true;
	}

	//
	// Determine the animation based on movecmd.

	// TODO:!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!! Set back the code to proper animations. This is for 
	// testing.
	// Idle by default.
	//PMover_SetAnimation(self, 0);
	//PMover_SetAnimation(self, 1);
	//if (moving_forward == false &&
	//		moving_backward == false &&
	//		//strafing_left == false &&
	//		//strafing_right == false &&
	//		jumping == false &&
	//		crouching == false) {
	//
	//	PMover_SetAnimation(self, 0);
	//} else {

	//	if (crouching == true) {
	//		// Crouch movement.
	//		PMover_SetAnimation(self, 2);
	//		return;
	//	}

	//	else if (jumping == true) {
	//		// Jump movement.
	//		PMover_SetAnimation(self, 4);
	//		return;
	//	}
	//	else {
	//		// If we aren't crouching or jumping, we are moving forward.
	//		PMover_SetAnimation(self, 1);
	//	}
	//}
}


//
//===============
// PMover_ProcessAnimation
// 
// Handles playing of the actual current animation.
//===============
//
void PMover_ProcessAnimation(edict_t* self) {
	// Ensure it is valid.
	if (!self)
		return;

	// TODO: This will be moved elsewhere.
	// Detect the animation to use.
	//PMover_DetectAnimation(self, movecmd);

	// Find the current animationID.
	int animationID = self->pmai.animations.current;

	// Calculate the frame to use, and whether to reset animation.
	int frameIndex = self->pmai.animations.list[animationID].startframe + self->pmai.animations.list[animationID].currentframe;

	// Check whether the animation needs a reset to loop.
	if (self->pmai.animations.list[animationID].loop == true) {
		if (frameIndex > self->pmai.animations.list[animationID].endframe) {
			// Reset currentframe.
			self->pmai.animations.list[animationID].currentframe = 0;

			// Recalculate frame index.
			frameIndex = self->pmai.animations.list[animationID].startframe + self->pmai.animations.list[animationID].currentframe;
		}
	} 
	// No looping, make sure it clamps.
	else {
		// In case we aren't looping, and exceeding, clamp the animation frames.
		if (frameIndex > self->pmai.animations.list[animationID].endframe) {
			// Reset.
			frameIndex = self->pmai.animations.list[animationID].currentframe = self->pmai.animations.list[animationID].endframe;
		}
	}

	// Assign the animation frame to the entity state.
	self->s.frame = frameIndex;

	// Process animation.
// Silly hack for the 20 fps, right now this model is meant for 10 fps...
	static int firstFramePass = false;

	if (firstFramePass == false) {
		self->pmai.animations.list[animationID].currentframe++;
		firstFramePass = true;
	}
	else {
		firstFramePass = false;
	}
	// End Silly hack for the 20 fps, right now this model is meant for 10 fps...
}

//
//===============
// PMover_Think
// 
// Called every game frame. 
//===============
//
void PMover_Think(edict_t* self) {
	// Clear the user input.
	usercmd_t* movecmd = &self->pmai.movement.cmd;
	memset(movecmd, 0, sizeof(movecmd));

	// TODO: Figure out why 30 is a nice speed here..?
	movecmd->msec = 30;

	//-------------------------------------------------------------------------
	// Search for enemies in sight, always a good thing to do.
	//-------------------------------------------------------------------------
	// Since we've found a target, store its pointer for less typing.
	edict_t* eTarget = self->pmai.targets.enemy.entity;
	qboolean foundEnemyTarget = PMAI_FindEnemyTarget(self);

	if (eTarget != NULL) {
		vec3_t vecDist;		// Distance vector, used for calculating angles.
		vec3_t vecAngles;	// Vector containing the actual view angles of 

		// Calculate the yaw to move to.
		VectorSubtract(eTarget->s.origin, self->s.origin, vecDist);
		vectoangles2(vecDist, vecAngles); // used for aiming at the player always.

		// Setup the move angles.
		movecmd->angles[YAW] = ANGLE2SHORT(vecAngles[YAW]);
		movecmd->angles[PITCH] = ANGLE2SHORT(vecAngles[PITCH]); // TODO: REMOVE.

		// Setup the movement speed.
		movecmd->forwardmove = 240;

		// Setup the entity's state angles.
		self->s.angles[YAW] = vecAngles[YAW];

		//// Don't allow pitch and up speeds to go haywire.
		// TODO: REMOVE.
		float pitch = vecAngles[PITCH];
		// This is for ladder climbing.
		if (pitch >= 45.f) {
			pitch = 45.f;
			movecmd->upmove = 240;
		}
		if (pitch <= -45.f) {
			pitch = -45.f;
			//movecmd->buttons->upmove = -240;
		}
	}

	//-------------------------------------------------------------------------
	// Special movement handling for jumps.
	//-------------------------------------------------------------------------
	if (!PMAI_CheckEyes(self, movecmd)) {
		// Determine what to do with the brush in front of them, if there is one.
		int brushAction = PMAI_BrushInFront(self, self->pmai.settings.view.height);
		//gi.dprintf("brushAction = %i\n", brushAction);
		if (random() > 0.1) {
			// Crouch.
			if (brushAction == 1) {
				movecmd->forwardmove = 240;
				//movecmd->upmove = -400;
			}
			// Jump
			else if (brushAction == 2) {
				movecmd->forwardmove = 240;
				movecmd->upmove = 400;
			}
			// Nothing.
			else {
				movecmd->upmove = 0;
			}
		}
	}

	//-------------------------------------------------------------------------
	// Actual processing of movement code.
	//-------------------------------------------------------------------------
	// Determine the animation to use based on input cmd.
	PMover_ProcessAnimation(self);

	// Execute the player movement using the given "AI Player Input"
	PMAI_ProcessMovement(self);

	// Setup the next think.
	self->nextthink = level.time + 0.1;
	self->think = PMover_Think;
}

//
//=============================================================================
//
//	General entity code for this monster.
//
//=============================================================================
//

//
//===============
// PMover_Pain
// 
// Engine callback in case this entity has been inflicted pain.
//===============
//
void PMover_Pain(edict_t* self, edict_t* other, float kick, int damage)
{

}

//
//===============
// PMover_DeadThink
// 
// Called for processing the death animation in case the AI is dead.
//===============
//
void PMover_DeadThink(edict_t* self) {
	// Process animation.
	PMover_ProcessAnimation(self);

	// Next frame.
	self->nextthink = 0.05;
	self->think = PMover_DeadThink;
}

//
//===============
// PMover_Die
// 
// Engine callback in case this entity is about to die.
//===============
//
void PMover_Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
	// For whichever reason, we might want to ensure it has health...
	if (self->health > 0)
		return;

	// Check whether to gib to death. This could happen when the monster gets
	// assaulted too hard, or when its dead body is assaulted.
	if (self->health < self->gib_health) {
		// Set flies effect.
		self->s.effects |= EF_FLIES;

		// Disable any active entity sound ID that might be there.
		self->s.sound = gi.soundindex("infantry/inflies1.wav");

		// Play the death sound.
		gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);

		// Throw some meat gibs around!
		int gibCount = 0;
		for (gibCount = 0; gibCount < 4; gibCount++)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

		// Throw the head gib around!
		ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
	} else {
		// Set the bounding box for our dead body.
		VectorSet(self->mins, -16, -16, -24);
		VectorSet(self->maxs, 16, 16, -8);

		// Change the movetype and add SVF_DEADMONSTER to sv flags.
		self->movetype = MOVETYPE_TOSS;
		self->svflags |= SVF_DEADMONSTER;

		// Remove the weapon model index, it doesn't have death animations.
		self->s.modelindex2 = 0;

		// Last but not least, relink the entity into the world.
		gi.linkentity(self);

		// Setup death animation.
		PMover_SetAnimation(self, 17);

		// Setup the dead think.
		self->nextthink = 0.05;
		self->think = PMover_DeadThink;
	}
}

/*QUAKED monster_pmover (1 .5 0) (-16 -16 -24) (16 16 32) 
*/
void SP_monster_pmover(edict_t* self)
{
	//-------------------------------------------------------------------------
	// Setup general entity settings.
	//-------------------------------------------------------------------------
	// Setup general entity data.
	self->classname = "pmover";
	self->class_id = ENTITY_MONSTER_PMOVER;
	self->deadflag = DEAD_NO;
	self->takedamage = DAMAGE_AIM;
	self->groundentity = NULL;
	self->flags &= ~FL_NO_KNOCKBACK;
	//self->svflags |= SVF_MONSTER;
	self->svflags &= ~SVF_NOCLIENT;		// Let the server know that this is not a client either.
	self->clipmask = MASK_MONSTERSOLID;	// We want clipping to behave as if it is a monster.
	self->waterlevel = 0;
	self->watertype = 0;
	self->is_jumping = false;

	// Set movetype, solid, model, and bounds.
	self->movetype = MOVETYPE_WALK;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("players/marine/tris.md2");
	self->s.modelindex2 = gi.modelindex("players/marine/w_blaster.md2");
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 32);

	// Setup edict entity functions.
	self->pain = PMover_Pain;
	self->die = PMover_Die;

	// Setup the think.
	self->nextthink = level.time + 0.1;
	self->think = PMover_Think;

	// Mapper entity data.
	if (!self->health)
		self->health = 100;
	if (!self->gib_health)
		self->gib_health = -20;
	if (!self->mass)
		self->mass = 200;

	//-------------------------------------------------------------------------
	// Setup actual PMAI
	//-------------------------------------------------------------------------
	// Initialize the AI.
	PMAI_Initialize(self);
	self->pmai.pmove.s.pm_type = PM_NORMAL;
	//self->pmai.pmove.s.pm_time = 0;

	// Initialize the animations list.
	PMover_FillAnimations(self);

	// Set Idle animation.
	PMover_SetAnimation(self, 0);

	// Link entity to world.
	gi.linkentity(self);
}
