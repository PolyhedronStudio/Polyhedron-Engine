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
#include "m_pmover.h"

// AI Player Move settings are stored here.
pmoveParams_t aipmp;

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
	self->s.effects = 0;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;

	self->s.skinnum |= 1;

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
}

//
// Does the thinking for the pmover...
//

// X(PITCH) = 0
// Y(YAW) = 1
// Z(ROLL) = 2

void PMover_Think(edict_t* self) {
	edict_t* target = NULL;

	// Clear the user input.
	usercmd_t *movecmd = &self->pmai.movement.cmd;
	memset(movecmd, 0, sizeof(movecmd));

	// TODO: Figure out why 30 is a nice speed here..?
	movecmd->msec = 30;

	// If we've found an enemy target. Proceed further.
	if (PMAI_FindEnemyTarget(self)) {
		vec3_t vecyaw;
		edict_t* target = self->pmai.targets.enemy.entity;

		// Calculate the yaw to move to.
		VectorSubtract(target->s.origin, self->s.origin, vecyaw);
		float yaw = vectoyaw2(vecyaw);

		// Update the current frame pmove command.
		movecmd->forwardmove = 240;					// Speed.
		movecmd->angles[YAW] = ANGLE2SHORT(yaw);	// Yaw angle.

		// Setup the entity's state yaw.
		self->s.angles[YAW] = yaw;
	}
	else {
		movecmd->msec = 1;
	}

	//-------------------------------------------------------------------------
	// Actual movement code.
	//-------------------------------------------------------------------------
		
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
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 32);

	// Setup edict entity functions.
	self->pain = PMover_Pain;
	self->die = PMover_Die;

	// Set health and "flies".
	self->health = 100;
	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.10;

	// Setup class description.
	self->common_name = "Pmover";
	self->class_id = ENTITY_MONSTER_PMOVER;

	// Setup the think.
	self->nextthink = level.time + 0.1;
	self->think = PMover_Think;

	//-------------------------------------------------------------------------
	// Setup actual PMAI
	//-------------------------------------------------------------------------
	PMAI_Initialize(self);

	// Setup other entity data.
	self->classname = "pmover";
	self->class_id = ENTITY_MONSTER_PMOVER;
	self->deadflag = DEAD_NO;
	self->groundentity = NULL;
	self->takedamage = DAMAGE_AIM;
	self->mass = 800;
	self->flags &= ~FL_NO_KNOCKBACK;
	self->svflags &= SVF_MONSTER;
	self->svflags &= ~SVF_NOCLIENT;		// Let the server know that this is not a client either.
	self->clipmask = MASK_PLAYERSOLID;	// We want clipping to behave as if it is a player.

	// Link entity to world.
	gi.linkentity(self);
}
