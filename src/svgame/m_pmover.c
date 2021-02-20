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

brain

==============================================================================
*/

#include "g_local.h"
#include "m_brain.h"


static int	sound_chest_open;
static int	sound_tentacles_extend;
static int	sound_tentacles_retract;
static int	sound_death;
static int	sound_idle1;
static int	sound_idle2;
static int	sound_idle3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_sight;
static int	sound_search;
static int	sound_melee1;
static int	sound_melee2;
static int	sound_melee3;


void pmover_sight(edict_t* self, edict_t* other)
{
	gi.sound(self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void pmover_search(edict_t* self)
{
	gi.sound(self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


void pmover_run(edict_t* self);
void pmover_dead(edict_t* self);


//
// STAND
//

mframe_t pmover_frames_stand[] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,

	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,

	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t pmover_move_stand = { FRAME_stand01, FRAME_stand30, pmover_frames_stand, NULL };

void pmover_stand(edict_t* self)
{
	self->monsterinfo.currentmove = &pmover_move_stand;
}


//
// IDLE
//

mframe_t pmover_frames_idle[] =
{
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,

	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,

	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL,
	ai_stand,	0,	NULL
};
mmove_t pmover_move_idle = { FRAME_stand31, FRAME_stand60, pmover_frames_idle, pmover_stand };

void pmover_idle(edict_t* self)
{
	if (!(self->spawnflags & SF_MONSTER_AMBUSH)) //Knightmare- play all 3 idle sounds
	{
		float r = random();

		if (r < 0.33)
			gi.sound(self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);
		else if (r < 0.67)
			gi.sound(self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
		else
			gi.sound(self, CHAN_VOICE, sound_idle3, 1, ATTN_IDLE, 0); //Knightmare- changed channel
	}
	self->monsterinfo.currentmove = &pmover_move_idle;
}


//
// WALK
//
mframe_t pmover_frames_walk1[] =
{
	ai_walk,	7,	NULL,
	ai_walk,	2,	NULL,
	ai_walk,	3,	NULL,
	ai_walk,	3,	NULL,
	ai_walk,	1,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	9,	NULL,
	ai_walk,	-4,	NULL,
	ai_walk,	-1,	NULL,
	ai_walk,	2,	NULL
};
mmove_t pmover_move_walk1 = { FRAME_walk101, FRAME_walk111, pmover_frames_walk1, NULL };

// walk2 is FUBAR, do not use
#if 0
void pmover_walk2_cycle(edict_t* self)
{
	if (random() > 0.1)
		self->monsterinfo.nextframe = FRAME_walk220;
}

mframe_t pmover_frames_walk2[] =
{
	ai_walk,	3,	NULL,
	ai_walk,	-2,	NULL,
	ai_walk,	-4,	NULL,
	ai_walk,	-3,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	1,	NULL,
	ai_walk,	12,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	-3,	NULL,
	ai_walk,	0,	NULL,

	ai_walk,	-2,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	1,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	10,	NULL,		// Cycle Start

	ai_walk,	-1,	NULL,
	ai_walk,	7,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	3,	NULL,
	ai_walk,	-3,	NULL,
	ai_walk,	2,	NULL,
	ai_walk,	4,	NULL,
	ai_walk,	-3,	NULL,
	ai_walk,	2,	NULL,
	ai_walk,	0,	NULL,

	ai_walk,	4,	pmover_walk2_cycle,
	ai_walk,	-1,	NULL,
	ai_walk,	-1,	NULL,
	ai_walk,	-8,	NULL,
	ai_walk,	0,	NULL,
	ai_walk,	1,	NULL,
	ai_walk,	5,	NULL,
	ai_walk,	2,	NULL,
	ai_walk,	-1,	NULL,
	ai_walk,	-5,	NULL
};
//mmove_t pmover_move_walk2 = {FRAME_walk201, FRAME_walk240, pmover_frames_walk2, NULL};
#endif

void pmover_walk(edict_t* self)
{
	//	if (random() <= 0.5)
	self->monsterinfo.currentmove = &pmover_move_walk1;
	//	else
	//		self->monsterinfo.currentmove = &pmover_move_walk2;
}



mframe_t pmover_frames_defense[] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t pmover_move_defense = { FRAME_defens01, FRAME_defens08, pmover_frames_defense, NULL };

mframe_t pmover_frames_pain3[] =
{
	ai_move,	-2,	NULL,
	ai_move,	2,	NULL,
	ai_move,	1,	NULL,
	ai_move,	3,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-4,	NULL
};
mmove_t pmover_move_pain3 = { FRAME_pain301, FRAME_pain306, pmover_frames_pain3, pmover_run };

mframe_t pmover_frames_pain2[] =
{
	ai_move,	-2,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	3,	NULL,
	ai_move,	1,	NULL,
	ai_move,	-2,	NULL
};
mmove_t pmover_move_pain2 = { FRAME_pain201, FRAME_pain208, pmover_frames_pain2, pmover_run };

mframe_t pmover_frames_pain1[] =
{
	ai_move,	-6,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	-6,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	2,	NULL,
	ai_move,	0,	NULL,
	ai_move,	2,	NULL,
	ai_move,	1,	NULL,
	ai_move,	7,	NULL,
	ai_move,	0,	NULL,
	ai_move,	3,	NULL,
	ai_move,	-1,	NULL
};
mmove_t pmover_move_pain1 = { FRAME_pain101, FRAME_pain121, pmover_frames_pain1, pmover_run };


//
// DUCK
//

void pmover_duck_down(edict_t* self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	gi.linkentity(self);
}

void pmover_duck_hold(edict_t* self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void pmover_duck_up(edict_t* self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity(self);
}

mframe_t pmover_frames_duck[] =
{
	ai_move,	0,	NULL,
	ai_move,	-2,	pmover_duck_down,
	ai_move,	17,	pmover_duck_hold,
	ai_move,	-3,	NULL,
	ai_move,	-1,	pmover_duck_up,
	ai_move,	-5,	NULL,
	ai_move,	-6,	NULL,
	ai_move,	-6,	NULL
};
mmove_t pmover_move_duck = { FRAME_duck01, FRAME_duck08, pmover_frames_duck, pmover_run };

void pmover_dodge(edict_t* self, edict_t* attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.pausetime = level.time + eta + 0.5;
	self->monsterinfo.currentmove = &pmover_move_duck;
}

mframe_t pmover_frames_death2[] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	9,	NULL,
	ai_move,	0,	NULL
};
mmove_t pmover_move_death2 = { FRAME_death201, FRAME_death205, pmover_frames_death2, pmover_dead };

mframe_t pmover_frames_death1[] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	-2,	NULL,
	ai_move,	9,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t pmover_move_death1 = { FRAME_death101, FRAME_death118, pmover_frames_death1, pmover_dead };


//
// MELEE
//

void pmover_swing_right(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sound_melee1, 1, ATTN_NORM, 0);
}

void pmover_hit_right(edict_t* self)
{
	vec3_t	aim;

	VectorSet(aim, MELEE_DISTANCE, self->maxs[0], 8);
	if (fire_hit(self, aim, (15 + (rand() % 5)), 40))
		gi.sound(self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

void pmover_swing_left(edict_t* self)
{
	gi.sound(self, CHAN_BODY, sound_melee2, 1, ATTN_NORM, 0);
}

void pmover_hit_left(edict_t* self)
{
	vec3_t	aim;

	VectorSet(aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit(self, aim, (15 + (rand() % 5)), 40))
		gi.sound(self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

mframe_t pmover_frames_attack1[] =
{
	ai_charge,	8,	NULL,
	ai_charge,	3,	NULL,
	ai_charge,	5,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	-3,	pmover_swing_right,
	ai_charge,	0,	NULL,
	ai_charge,	-5,	NULL,
	ai_charge,	-7,	pmover_hit_right,
	ai_charge,	0,	NULL,
	ai_charge,	6,	pmover_swing_left,
	ai_charge,	1,	NULL,
	ai_charge,	2,	pmover_hit_left,
	ai_charge,	-3,	NULL,
	ai_charge,	6,	NULL,
	ai_charge,	-1,	NULL,
	ai_charge,	-3,	NULL,
	ai_charge,	2,	NULL,
	ai_charge,	-11,NULL
};
mmove_t pmover_move_attack1 = { FRAME_attak101, FRAME_attak118, pmover_frames_attack1, pmover_run };

void pmover_chest_open(edict_t* self)
{
	self->spawnflags &= ~65536;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	gi.sound(self, CHAN_BODY, sound_chest_open, 1, ATTN_NORM, 0);
}

void pmover_tentacle_attack(edict_t* self)
{
	vec3_t	aim;

	VectorSet(aim, MELEE_DISTANCE, 0, 8);
	if (fire_hit(self, aim, (10 + (rand() % 5)), -600) && skill->value > 0)
		self->spawnflags |= 65536;
	gi.sound(self, CHAN_WEAPON, sound_tentacles_retract, 1, ATTN_NORM, 0);
}

void pmover_chest_closed(edict_t* self)
{
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->spawnflags & 65536)
	{
		self->spawnflags &= ~65536;
		self->monsterinfo.currentmove = &pmover_move_attack1;
	}
}

mframe_t pmover_frames_attack2[] =
{
	ai_charge,	5,	NULL,
	ai_charge,	-4,	NULL,
	ai_charge,	-4,	NULL,
	ai_charge,	-3,	NULL,
	ai_charge,	0,	pmover_chest_open,
	ai_charge,	0,	NULL,
	ai_charge,	13,	pmover_tentacle_attack,
	ai_charge,	0,	NULL,
	ai_charge,	2,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	-9,	pmover_chest_closed,
	ai_charge,	0,	NULL,
	ai_charge,	4,	NULL,
	ai_charge,	3,	NULL,
	ai_charge,	2,	NULL,
	ai_charge,	-3,	NULL,
	ai_charge,	-6,	NULL
};
mmove_t pmover_move_attack2 = { FRAME_attak201, FRAME_attak217, pmover_frames_attack2, pmover_run };

void pmover_melee(edict_t* self)
{
	if (random() <= 0.5)
		self->monsterinfo.currentmove = &pmover_move_attack1;
	else
		self->monsterinfo.currentmove = &pmover_move_attack2;
}


//
// RUN
//

mframe_t pmover_frames_run[] =
{
	ai_run,	9,	NULL,
	ai_run,	2,	NULL,
	ai_run,	3,	NULL,
	ai_run,	3,	NULL,
	ai_run,	1,	NULL,
	ai_run,	0,	NULL,
	ai_run,	0,	NULL,
	ai_run,	10,	NULL,
	ai_run,	-4,	NULL,
	ai_run,	-1,	NULL,
	ai_run,	2,	NULL
};
mmove_t pmover_move_run = { FRAME_walk101, FRAME_walk111, pmover_frames_run, NULL };

void pmover_run(edict_t* self)
{
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &pmover_move_stand;
	else
		self->monsterinfo.currentmove = &pmover_move_run;
}


void pmover_pain(edict_t* self, edict_t* other, float kick, int damage)
{
	float	r;

	if (self->health < (self->max_health / 2))
		self->s.skinnum |= 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	r = random();
	if (r < 0.33)
	{
		gi.sound(self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &pmover_move_pain1;
	}
	else if (r < 0.66)
	{
		gi.sound(self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &pmover_move_pain2;
	}
	else
	{
		gi.sound(self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = &pmover_move_pain3;
	}
}

void pmover_dead(edict_t* self)
{
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity(self);
	M_FlyCheck(self);

	// Lazarus monster fade
	if (world->effects & FX_WORLDSPAWN_CORPSEFADE)
	{
		self->think = FadeDieSink;
		self->nextthink = level.time + corpse_fadetime->value;
	}
}



void pmover_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
	int		n;

	self->s.effects = 0;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;

	self->s.skinnum |= 1;

	// check for gib
	if (self->health <= self->gib_health && !(self->spawnflags & SF_MONSTER_NOGIB))
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound(self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	if (random() <= 0.5)
		self->monsterinfo.currentmove = &pmover_move_death1;
	else
		self->monsterinfo.currentmove = &pmover_move_death2;
}

/*QUAKED monster_pmover (1 .5 0) (-16 -16 -24) (16 16 32) 
*/
void SP_monster_pmover(edict_t* self)
{
	// It is free to stick around in Multiplayer.
	//if (deathmatch->value)
	//{
	//	G_FreeEdict(self);
	//	return;
	//}

	// Set movetype, solid, model, and bounds.
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 32);

	// Setup edict entity functions.
	self->pain = pmover_pain;
	self->die = pmover_die;

	// Setup edict->MonsterInfo functions.
	self->monsterinfo.stand = pmover_stand;
	self->monsterinfo.walk = pmover_walk;
	self->monsterinfo.run = pmover_run;
	self->monsterinfo.dodge = pmover_dodge;
	//	self->monsterinfo.attack = pmover_attack;
	self->monsterinfo.melee = pmover_melee;
	self->monsterinfo.sight = pmover_sight;
	self->monsterinfo.search = pmover_search;
	self->monsterinfo.idle = pmover_idle;

	// Set health and "flies".
	self->health = 100;
	if (!self->monsterinfo.flies)
		self->monsterinfo.flies = 0.10;

	// PMover.
	self->common_name = "Pmover";
	self->class_id = ENTITY_MONSTER_PMOVER;

	// Link entity to world.
	gi.linkentity(self);

	// Setup its current move function, scale, and start walking.
	self->monsterinfo.currentmove = &pmover_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	walkmonster_start(self);
}
