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
#include "m_player.h"

//
// NON IMPORTANT, ARE IN SHARED/SHARED.H but the GAME DLL IS STILL RIGGED WITH THAT KMQ2 ISSUE.
//
#define ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)
#define SHORT2ANGLE(x)  ((x)*(360.0/65536))

//
// NON IMPORTANT FUNCTION.
//
void PMover_Pain(edict_t* self, edict_t* other, float kick, int damage)
{

}

//
// NON IMPORTANT FUNCTION.
//
void PMover_Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
	//self->nextthink = 0;

	//VectorSet(self->mins, -16, -16, -24);
	//VectorSet(self->maxs, 16, 16, -8);
	//self->movetype = MOVETYPE_TOSS;
	//self->svflags |= SVF_DEADMONSTER;
	//gi.linkentity(self);
	//M_FlyCheck(self);
	if (self->health > 0)
		return;

	// Lazarus
	int n = 0;

	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;

	gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n = 0; n < 4; n++)
		ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}






// ------------------------------------------------------------------------------
// CODE STARTS HERE, AT LEAST THE CODE THAT MATTERS LOL HAHA LOLOLOL. 1337 H4X0R

//
// Detects the current animation frame to use for the AI.
//
void PMover_FillAnimations(edict_t* self) {
	// Ensure they are valid.
	if (!self)
		return;

	// 0 == stand
	self->pmai.animations.list[0].startframe	= FRAME_stand01;
	self->pmai.animations.list[0].endframe		= FRAME_stand40;
	self->pmai.animations.list[0].currentframe	= 0;

	// 1 == run
	self->pmai.animations.list[1].startframe = FRAME_run1;
	self->pmai.animations.list[1].endframe = FRAME_run6;
	self->pmai.animations.list[1].currentframe = 0;

	// 2 == crouchwalk
	self->pmai.animations.list[2].startframe = FRAME_crwalk1;
	self->pmai.animations.list[2].endframe = FRAME_crwalk6;
	self->pmai.animations.list[2].currentframe = 0;

	// 4 == jump
	self->pmai.animations.list[4].startframe = FRAME_jump1;
	self->pmai.animations.list[4].endframe = FRAME_jump6;
	self->pmai.animations.list[4].currentframe = 0;
}

//
// Handles the animations for the pmover...
//
void PMover_SetAnimation(edict_t* self, int animationID) {
	// Ensure animationID is within bounds.
	if (animationID < 0 && animationID > 20)
		return;

	// Reset the animation at hand.
	//self->pmai.animations.list[animationID].currentframe = 0;

	// Set the current active animation.
	self->pmai.animations.current = animationID;
}

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
	if (movecmd->forwardmove >= 1) {
		moving_forward = true;
	}
	// Strafe moving.
	if (movecmd->sidemove < 0) {
		strafing_left = true;
	} else if (movecmd->sidemove > 0) {
		strafing_right = true;
	}
	// Jumping.
	if (movecmd->upmove > 0) {
		jumping = true;
		crouching = false;
	}
	// Crouching.
	if (movecmd->upmove < 0) {
		jumping = false;
		crouching = true;
	}

	//
	// Determine the animation based on movecmd.

	// Idle by default.
	PMover_SetAnimation(self, 0);

	// Moving forward, so.
	if (moving_forward == true) {
		// Just regular movement.
		PMover_SetAnimation(self, 1);

	}

	if (crouching == true) {
		// Crouch movement.
		PMover_SetAnimation(self, 2);
		//return;
	}

	if (jumping == true) {
		// Jump movement.
		PMover_SetAnimation(self, 4);
		//return;
	}
}


//
// Processes the actual animation.
//
void PMover_ProcessAnimation(edict_t* self, usercmd_t* movecmd) {
	// Ensure it is valid.
	if (!self || !movecmd)
		return;

	// Detect the animation to use.
	PMover_DetectAnimation(self, movecmd);

	// Find the current animationID.
	int animationid = self->pmai.animations.current;

	// Process animation.
	// Silly hack for the 20 fps, right now this model is meant for 10 fps...
	static int firstFramePass = true;

	if (firstFramePass == false) {
		self->pmai.animations.list[animationid].currentframe++;
		firstFramePass = true;
	} else {
		firstFramePass = false;
	}
	// End Silly hack for the 20 fps, right now this model is meant for 10 fps...

	if (self->pmai.animations.list[animationid].currentframe > self->pmai.animations.list[animationid].endframe) {
		self->pmai.animations.list[animationid].currentframe = self->pmai.animations.list[animationid].startframe;
	}

	// Set the animation to entity state.
	int startframe	= self->pmai.animations.list[animationid].startframe;			// T
	int actualframe	= self->pmai.animations.list[animationid].currentframe;			// T 
	self->s.frame = actualframe;
}

//
// Does the thinking for the pmover...
//

// X(PITCH) = 0
// Y(YAW) = 1
// Z(ROLL) = 2
void PMover_Think(edict_t* self) {

	// Clear the user input.
	usercmd_t *movecmd = &self->pmai.movement.cmd;
	memset(movecmd, 0, sizeof(movecmd));

	// TODO: Figure out why 30 is a nice speed here..?
	movecmd->msec = 30;

	//-------------------------------------------------------------------------
	// Search for enemies in sight, always a good thing to do.
	//-------------------------------------------------------------------------
	qboolean foundEnemyTarget = PMAI_FindEnemyTarget(self);

	if (foundEnemyTarget == true) {
		vec3_t vecDist;		// Distance vector, used for calculating angles.
		vec3_t vecAngles;	// Vector containing the actual view angles of 
		
		// Since we've found a target, store its pointer for less typing.
		edict_t *eTarget = self->pmai.targets.enemy.entity;

		// Calculate the yaw to move to.
		VectorSubtract(eTarget->s.origin, self->s.origin, vecDist);
		vectoangles2(vecDist, vecAngles); // used for aiming at the player always.

		// Setup the move angles.
		movecmd->angles[YAW]	= ANGLE2SHORT(vecAngles[YAW]);
//		movecmd->angles[PITCH]	= ANGLE2SHORT(vecAngles[PITCH]);

		// Setup the movement speed.
		movecmd->forwardmove = 240;

		// Setup the entity's state angles.
		self->s.angles[YAW] = vecAngles[YAW];

		//// Don't allow pitch and up speeds to go haywire.
		//float pitch = vecangles[PITCH];
		//// This is for ladder climbing.
		//if (pitch >= 45.f) {
		//	pitch = 45.f;
		//	movecmd->upmove = 240;
		//}
		//if (pitch <= -45.f) {
		//	pitch = -45.f;
		//	//movecmd->buttons->upmove = -240;
		//}
	}

	//-------------------------------------------------------------------------
	// Brush detection, for jumping and crouching.
	//-------------------------------------------------------------------------
	// Determine what to do with the brush in front of them, if there is one.
	int brushAction = PMAI_BrushInFront(self, self->pmai.settings.view.height);
	gi.dprintf("brushAction = %i\n", brushAction);

	// Jump.
	//if (brushAction == 1) {
	//	movecmd->forwardmove = 240;
		movecmd->upmove = 400;
	//}
	// Crouch.
	if (brushAction == 2) {
		movecmd->forwardmove = 240;
		movecmd->upmove = -400;
	}

	//-------------------------------------------------------------------------
	// Actual processing of movement code.
	//-------------------------------------------------------------------------
	// Determine the animation to use based on input cmd.
	PMover_ProcessAnimation(self, movecmd);
	
	// Execute the player movement using the given "AI Player Input"
	PMAI_ProcessMovement(self);

	// Setup the next think.
	self->nextthink = level.time + 0.1;
	self->think = PMover_Think;
}

/*QUAKED monster_pmover (1 .5 0) (-16 -16 -24) (16 16 32) 
*/
void SP_monster_pmover(edict_t* self)
{
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

	// Set health and "flies".
	self->health = 100;
	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.10;

	// Setup the think.
	self->nextthink = level.time + 0.1;
	self->think = PMover_Think;

	//-------------------------------------------------------------------------
	// Setup actual PMAI
	//-------------------------------------------------------------------------
	// Initialize the AI.
	PMAI_Initialize(self);
	self->pmai.pmove.s.pm_type = PM_NORMAL;
	//self->pmai.pmove.s.pm_time = 0;

	// Initialize the animations list.
	PMover_FillAnimations(self);

	// Setup other entity data.
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

	// Mapper entity data.
	if (!self->health)
		self->health = 100;
	if (!self->gib_health)
		self->gib_health = 0;
	if (!self->mass)
		self->mass = 200;

	// Link entity to world.
	gi.linkentity(self);
}
