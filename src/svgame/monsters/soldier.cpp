/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/*
==============================================================================

SOLDIER

==============================================================================
*/

#include "../g_local.h"     // SVGame.
#include "../effects.h"     // Effects.
#include "../utils.h"       // Util.
#include "soldier.h"
#include "sharedgame/sharedgame.h"

static int  sound_idle;
static int  sound_sight1;
static int  sound_sight2;
static int  sound_pain_light;
static int  sound_pain;
static int  sound_pain_ss;
static int  sound_death_light;
static int  sound_death;
static int  sound_death_ss;
static int  sound_cock;
static int  sound_step;
static int  sound_step2;
static int  sound_step3;
static int  sound_step4;

void soldier_footstep(entity_t *self)
{
	if (!cl_monsterfootsteps->integer)
		return;

	int     i;
	i = rand() % (3 + 1 - 0) + 0;

	if (i == 0)
	{
		gi.Sound(self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
	}
	else if (i == 1)
	{
		gi.Sound(self, CHAN_BODY, sound_step2, 1, ATTN_NORM, 0);
	}
	else if (i == 2)
	{
		gi.Sound(self, CHAN_BODY, sound_step3, 1, ATTN_NORM, 0);
	}
	else if (i == 3)
	{
		gi.Sound(self, CHAN_BODY, sound_step4, 1, ATTN_NORM, 0);
	}
}


void soldier_idle(entity_t *self)
{
    if (random() > 0.8)
        gi.Sound(self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void soldier_cock(entity_t *self)
{
    if (self->s.frame == FRAME_stand322)
        gi.Sound(self, CHAN_WEAPON, sound_cock, 1, ATTN_IDLE, 0);
    else
        gi.Sound(self, CHAN_WEAPON, sound_cock, 1, ATTN_NORM, 0);
}


// STAND

void soldier_stand(entity_t *self);

mframe_t soldier_frames_stand1 [] = {
    { ai_stand, 0, soldier_idle },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL }
};
mmove_t soldier_move_stand1 = {FRAME_stand101, FRAME_stand130, soldier_frames_stand1, soldier_stand};

mframe_t soldier_frames_stand3 [] = {
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, soldier_cock },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL }
};
mmove_t soldier_move_stand3 = {FRAME_stand301, FRAME_stand339, soldier_frames_stand3, soldier_stand};

#if 0
mframe_t soldier_frames_stand4 [] = {
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL },
    { ai_stand, 4, NULL },
    { ai_stand, 1, NULL },
    { ai_stand, -1, NULL },
    { ai_stand, -2, NULL },

    { ai_stand, 0, NULL },
    { ai_stand, 0, NULL }
};
mmove_t soldier_move_stand4 = {FRAME_stand401, FRAME_stand452, soldier_frames_stand4, NULL};
#endif

void soldier_stand(entity_t *self)
{
    if ((self->monsterInfo.currentmove == &soldier_move_stand3) || (random() < 0.8))
        self->monsterInfo.currentmove = &soldier_move_stand1;
    else
        self->monsterInfo.currentmove = &soldier_move_stand3;
}


//
// WALK
//

void soldier_walk1_random(entity_t *self)
{
    if (random() > 0.1)
        self->monsterInfo.nextframe = FRAME_walk101;
}

mframe_t soldier_frames_walk1 [] = {
    { ai_walk, 3,  NULL },
    { ai_walk, 6,  NULL },
    { ai_walk, 2,  NULL },
    { ai_walk, 2,  soldier_footstep },
    { ai_walk, 2,  NULL },
    { ai_walk, 1,  NULL },
    { ai_walk, 6,  NULL },
    { ai_walk, 5,  NULL },
    { ai_walk, 3,  soldier_footstep },
    { ai_walk, -1, soldier_walk1_random },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL },
    { ai_walk, 0,  NULL }
};
mmove_t soldier_move_walk1 = {FRAME_walk101, FRAME_walk133, soldier_frames_walk1, NULL};

mframe_t soldier_frames_walk2 [] = {
    { ai_walk, 4,  soldier_footstep },
    { ai_walk, 4,  NULL },
    { ai_walk, 9,  NULL },
    { ai_walk, 8,  NULL },
    { ai_walk, 5,  soldier_footstep },
    { ai_walk, 1,  NULL },
    { ai_walk, 3,  NULL },
    { ai_walk, 7,  NULL },
    { ai_walk, 6,  NULL },
    { ai_walk, 7,  NULL }
};
mmove_t soldier_move_walk2 = {FRAME_walk209, FRAME_walk218, soldier_frames_walk2, NULL};

void soldier_walk(entity_t *self)
{
    if (random() < 0.5)
        self->monsterInfo.currentmove = &soldier_move_walk1;
    else
        self->monsterInfo.currentmove = &soldier_move_walk2;
}


//
// RUN
//

void soldier_run(entity_t *self);

mframe_t soldier_frames_start_run [] = {
    { ai_run, 7,  NULL },
    { ai_run, 5,  NULL }
};
mmove_t soldier_move_start_run = {FRAME_run01, FRAME_run02, soldier_frames_start_run, soldier_run};

mframe_t soldier_frames_run [] = {
    { ai_run, 10, NULL },
    { ai_run, 11, soldier_footstep },
    { ai_run, 11, NULL },
    { ai_run, 16, NULL },
    { ai_run, 10, soldier_footstep },
    { ai_run, 15, NULL }
};
mmove_t soldier_move_run = {FRAME_run03, FRAME_run08, soldier_frames_run, NULL};

void soldier_run(entity_t *self)
{
    if (self->monsterInfo.aiflags & AI_STAND_GROUND) {
        self->monsterInfo.currentmove = &soldier_move_stand1;
        return;
    }

    if (self->monsterInfo.currentmove == &soldier_move_walk1 ||
        self->monsterInfo.currentmove == &soldier_move_walk2 ||
        self->monsterInfo.currentmove == &soldier_move_start_run) {
        self->monsterInfo.currentmove = &soldier_move_run;
    } else {
        self->monsterInfo.currentmove = &soldier_move_start_run;
    }
}


//
// PAIN
//

mframe_t soldier_frames_pain1 [] = {
    { ai_move, -3, NULL },
    { ai_move, 4,  NULL },
    { ai_move, 1,  NULL },
    { ai_move, 1,  NULL },
    { ai_move, 0,  NULL }
};
mmove_t soldier_move_pain1 = {FRAME_pain101, FRAME_pain105, soldier_frames_pain1, soldier_run};

mframe_t soldier_frames_pain2 [] = {
    { ai_move, -13, NULL },
    { ai_move, -1,  NULL },
    { ai_move, 2,   NULL },
    { ai_move, 4,   NULL },
    { ai_move, 2,   NULL },
    { ai_move, 3,   NULL },
    { ai_move, 2,   NULL }
};
mmove_t soldier_move_pain2 = {FRAME_pain201, FRAME_pain207, soldier_frames_pain2, soldier_run};

mframe_t soldier_frames_pain3 [] = {
    { ai_move, -8, NULL },
    { ai_move, 10, NULL },
    { ai_move, -4, soldier_footstep },
    { ai_move, -1, NULL },
    { ai_move, -3, NULL },
    { ai_move, 0,  NULL },
    { ai_move, 3,  NULL },
    { ai_move, 0,  NULL },
    { ai_move, 0,  NULL },
    { ai_move, 0,  NULL },
    { ai_move, 0,  NULL },
    { ai_move, 1,  NULL },
    { ai_move, 0,  NULL },
    { ai_move, 1,  NULL },
    { ai_move, 2,  NULL },
    { ai_move, 4,  NULL },
    { ai_move, 3,  NULL },
    { ai_move, 2,  soldier_footstep }
};
mmove_t soldier_move_pain3 = {FRAME_pain301, FRAME_pain318, soldier_frames_pain3, soldier_run};

mframe_t soldier_frames_pain4 [] = {
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, -10, NULL },
    { ai_move, -6,  NULL },
    { ai_move, 8,   NULL },
    { ai_move, 4,   NULL },
    { ai_move, 1,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 2,   NULL },
    { ai_move, 5,   NULL },
    { ai_move, 2,   NULL },
    { ai_move, -1,  NULL },
    { ai_move, -1,  NULL },
    { ai_move, 3,   NULL },
    { ai_move, 2,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_pain4 = {FRAME_pain401, FRAME_pain417, soldier_frames_pain4, soldier_run};


void soldier_pain(entity_t *self, entity_t *other, float kick, int damage)
{
    float   r;
    int     n;

    if (self->health < (self->maxHealth / 2))
        self->s.skinnum |= 1;

    if (level.time < self->debouncePainTime) {
        if ((self->velocity[2] > 100) && ((self->monsterInfo.currentmove == &soldier_move_pain1) || (self->monsterInfo.currentmove == &soldier_move_pain2) || (self->monsterInfo.currentmove == &soldier_move_pain3)))
            self->monsterInfo.currentmove = &soldier_move_pain4;
        return;
    }

    self->debouncePainTime = level.time + 3;

    n = self->s.skinnum | 1;
    if (n == 1)
        gi.Sound(self, CHAN_VOICE, sound_pain_light, 1, ATTN_NORM, 0);
    else if (n == 3)
        gi.Sound(self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
    else
        gi.Sound(self, CHAN_VOICE, sound_pain_ss, 1, ATTN_NORM, 0);

    if (self->velocity[2] > 100) {
        self->monsterInfo.currentmove = &soldier_move_pain4;
        return;
    }

    if (skill->value == 3)
        return;     // no pain anims in nightmare

    r = random();

    if (r < 0.33)
        self->monsterInfo.currentmove = &soldier_move_pain1;
    else if (r < 0.66)
        self->monsterInfo.currentmove = &soldier_move_pain2;
    else
        self->monsterInfo.currentmove = &soldier_move_pain3;
}


//
// ATTACK
//

static int blaster_flash [] = {MZ2_SOLDIER_BLASTER_1, MZ2_SOLDIER_BLASTER_2, MZ2_SOLDIER_BLASTER_3, MZ2_SOLDIER_BLASTER_4, MZ2_SOLDIER_BLASTER_5, MZ2_SOLDIER_BLASTER_6, MZ2_SOLDIER_BLASTER_7, MZ2_SOLDIER_BLASTER_8};
static int shotgun_flash [] = {MZ2_SOLDIER_SHOTGUN_1, MZ2_SOLDIER_SHOTGUN_2, MZ2_SOLDIER_SHOTGUN_3, MZ2_SOLDIER_SHOTGUN_4, MZ2_SOLDIER_SHOTGUN_5, MZ2_SOLDIER_SHOTGUN_6, MZ2_SOLDIER_SHOTGUN_7, MZ2_SOLDIER_SHOTGUN_8};
static int machinegun_flash [] = {MZ2_SOLDIER_MACHINEGUN_1, MZ2_SOLDIER_MACHINEGUN_2, MZ2_SOLDIER_MACHINEGUN_3, MZ2_SOLDIER_MACHINEGUN_4, MZ2_SOLDIER_MACHINEGUN_5, MZ2_SOLDIER_MACHINEGUN_6, MZ2_SOLDIER_MACHINEGUN_7, MZ2_SOLDIER_MACHINEGUN_8};

void soldier_fire(entity_t *self, int flash_number)
{
    vec3_t  start;
    vec3_t  forward, right, up;
    vec3_t  aim;
    vec3_t  dir;
    vec3_t  end;
    float   r, u;
    int     flash_index;

    if (self->s.skinnum < 2)
        flash_index = blaster_flash[flash_number];
    else if (self->s.skinnum < 4)
        flash_index = shotgun_flash[flash_number];
    else
        flash_index = machinegun_flash[flash_number];

    AngleVectors(self->s.angles, &forward, &right, NULL);
    start = G_ProjectSource(self->s.origin, { 10.6 * 1.2, 7.7 * 1.2, 7.8 * 1.2 }, forward, right);

    if (flash_number == 5 || flash_number == 6) {
        VectorCopy(forward, aim);
    } else {
        VectorCopy(self->enemy->s.origin, end);
        end[2] += self->enemy->viewHeight;
        VectorSubtract(end, start, aim);
        vectoangles(aim, dir);
        AngleVectors(dir, &forward, &right, &up);

        r = crandom() * 1000;
        u = crandom() * 500;
        VectorMA(start, WORLD_SIZE, forward, end);
        VectorMA(end, r, right, end);
        VectorMA(end, u, up, end);

        VectorSubtract(end, start, aim);
        VectorNormalize(aim);
    }

    if (self->s.skinnum <= 1) {
        //monster_fire_blaster(self, start, aim, 5, 600, flash_index, EF_BLASTER);
    } else {
        if (!(self->monsterInfo.aiflags & AI_HOLD_FRAME))
            self->monsterInfo.pausetime = level.time + (3 + rand() % 8) * FRAMETIME;

        monster_fire_bullet(self, start, aim, 2, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_index);

        if (level.time >= self->monsterInfo.pausetime)
            self->monsterInfo.aiflags &= ~AI_HOLD_FRAME;
        else
            self->monsterInfo.aiflags |= AI_HOLD_FRAME;
    }
}

// ATTACK1 (blaster/shotgun)

void soldier_fire1(entity_t *self)
{
    soldier_fire(self, 0);
}

void soldier_attack1_refire1(entity_t *self)
{
    if (self->s.skinnum > 1)
        return;

    if (self->enemy->health <= 0)
        return;

    if (((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE))
        self->monsterInfo.nextframe = FRAME_attak102;
    else
        self->monsterInfo.nextframe = FRAME_attak110;
}

void soldier_attack1_refire2(entity_t *self)
{
    if (self->s.skinnum < 2)
        return;

    if (self->enemy->health <= 0)
        return;

    if (((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE))
        self->monsterInfo.nextframe = FRAME_attak102;
}

mframe_t soldier_frames_attack1 [] = {
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  soldier_fire1 },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  soldier_attack1_refire1 },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  soldier_cock },
    { ai_charge, 0,  soldier_attack1_refire2 },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  NULL },
    { ai_charge, 0,  NULL }
};
mmove_t soldier_move_attack1 = {FRAME_attak101, FRAME_attak112, soldier_frames_attack1, soldier_run};

// ATTACK2 (blaster/shotgun)

void soldier_fire2(entity_t *self)
{
    soldier_fire(self, 1);
}

void soldier_attack2_refire1(entity_t *self)
{
    if (self->s.skinnum > 1)
        return;

    if (self->enemy->health <= 0)
        return;

    if (((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE))
        self->monsterInfo.nextframe = FRAME_attak204;
    else
        self->monsterInfo.nextframe = FRAME_attak216;
}

void soldier_attack2_refire2(entity_t *self)
{
    if (self->s.skinnum < 2)
        return;

    if (self->enemy->health <= 0)
        return;

    if (((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE))
        self->monsterInfo.nextframe = FRAME_attak204;
}

mframe_t soldier_frames_attack2 [] = {
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_fire2 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_attack2_refire1 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_cock },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_attack2_refire2 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL }
};
mmove_t soldier_move_attack2 = {FRAME_attak201, FRAME_attak218, soldier_frames_attack2, soldier_run};

// ATTACK3 (duck and shoot)

void soldier_duck_down(entity_t *self)
{
    if (self->monsterInfo.aiflags & AI_DUCKED)
        return;
    self->monsterInfo.aiflags |= AI_DUCKED;
    self->maxs[2] -= 32;
    self->takedamage = DAMAGE_YES;
    self->monsterInfo.pausetime = level.time + 1;
    gi.LinkEntity(self);
}

void soldier_duck_up(entity_t *self)
{
    self->monsterInfo.aiflags &= ~AI_DUCKED;
    self->maxs[2] += 32;
    self->takedamage = DAMAGE_AIM;
    gi.LinkEntity(self);
}

void soldier_fire3(entity_t *self)
{
    soldier_duck_down(self);
    soldier_fire(self, 2);
}

void soldier_attack3_refire(entity_t *self)
{
    if ((level.time + 0.4) < self->monsterInfo.pausetime)
        self->monsterInfo.nextframe = FRAME_attak303;
}

mframe_t soldier_frames_attack3 [] = {
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_fire3 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_attack3_refire },
    { ai_charge, 0, soldier_duck_up },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL }
};
mmove_t soldier_move_attack3 = {FRAME_attak301, FRAME_attak309, soldier_frames_attack3, soldier_run};

// ATTACK4 (machinegun)

void soldier_fire4(entity_t *self)
{
    soldier_fire(self, 3);
//
//  if (self->enemy->health <= 0)
//      return;
//
//  if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
//      self->monsterInfo.nextframe = FRAME_attak402;
}

mframe_t soldier_frames_attack4 [] = {
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_footstep },
    { ai_charge, 0, soldier_fire4 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_footstep }
};
mmove_t soldier_move_attack4 = {FRAME_attak401, FRAME_attak406, soldier_frames_attack4, soldier_run};

#if 0
// ATTACK5 (prone)

void soldier_fire5(entity_t *self)
{
    soldier_fire(self, 4);
}

void soldier_attack5_refire(entity_t *self)
{
    if (self->enemy->health <= 0)
        return;

    if (((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE))
        self->monsterInfo.nextframe = FRAME_attak505;
}

mframe_t soldier_frames_attack5 [] = {
    { ai_charge, 8, NULL },
    { ai_charge, 8, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_fire5 },
    { ai_charge, 0, NULL },
    { ai_charge, 0, NULL },
    { ai_charge, 0, soldier_attack5_refire }
};
mmove_t soldier_move_attack5 = {FRAME_attak501, FRAME_attak508, soldier_frames_attack5, soldier_run};
#endif

// ATTACK6 (run & shoot)

void soldier_fire8(entity_t *self)
{
    soldier_fire(self, 7);
}

void soldier_attack6_refire(entity_t *self)
{
    if (self->enemy->health <= 0)
        return;

    if (range(self, self->enemy) < RANGE_MID)
        return;

    if (skill->value == 3)
        self->monsterInfo.nextframe = FRAME_runs03;
}

mframe_t soldier_frames_attack6 [] = {
    { ai_charge, 10, NULL },
    { ai_charge,  4, NULL },
    { ai_charge, 12, soldier_footstep },
    { ai_charge, 11, soldier_fire8 },
    { ai_charge, 13, NULL },
    { ai_charge, 18, NULL },
    { ai_charge, 15, soldier_footstep },
    { ai_charge, 14, NULL },
    { ai_charge, 11, NULL },
    { ai_charge,  8, soldier_footstep },
    { ai_charge, 11, NULL },
    { ai_charge, 12, NULL },
    { ai_charge, 12, soldier_footstep },
    { ai_charge, 17, soldier_attack6_refire }
};
mmove_t soldier_move_attack6 = {FRAME_runs01, FRAME_runs14, soldier_frames_attack6, soldier_run};

void soldier_attack(entity_t *self)
{
    if (self->s.skinnum < 4) {
        if (random() < 0.5)
            self->monsterInfo.currentmove = &soldier_move_attack1;
        else
            self->monsterInfo.currentmove = &soldier_move_attack2;
    } else {
        self->monsterInfo.currentmove = &soldier_move_attack4;
    }
}


//
// SIGHT
//

void soldier_sight(entity_t *self, entity_t *other)
{
    if (random() < 0.5)
        gi.Sound(self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
    else
        gi.Sound(self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);

    if ((skill->value > 0) && (range(self, self->enemy) >= RANGE_MID)) {
        if (random() > 0.5)
            self->monsterInfo.currentmove = &soldier_move_attack6;
    }
}

//
// DUCK
//

void soldier_duck_hold(entity_t *self)
{
    if (level.time >= self->monsterInfo.pausetime)
        self->monsterInfo.aiflags &= ~AI_HOLD_FRAME;
    else
        self->monsterInfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t soldier_frames_duck [] = {
    { ai_move, 5, soldier_duck_down },
    { ai_move, -1, soldier_duck_hold },
    { ai_move, 1,  NULL },
    { ai_move, 0,  soldier_duck_up },
    { ai_move, 5,  NULL }
};
mmove_t soldier_move_duck = {FRAME_duck01, FRAME_duck05, soldier_frames_duck, soldier_run};

void soldier_dodge(entity_t *self, entity_t *attacker, float eta)
{
    float   r;

    r = random();
    if (r > 0.25)
        return;

    if (!self->enemy)
        self->enemy = attacker;

    if (skill->value == 0) {
        self->monsterInfo.currentmove = &soldier_move_duck;
        return;
    }

    self->monsterInfo.pausetime = level.time + eta + 0.3;
    r = random();

    if (skill->value == 1) {
        if (r > 0.33)
            self->monsterInfo.currentmove = &soldier_move_duck;
        else
            self->monsterInfo.currentmove = &soldier_move_attack3;
        return;
    }

    if (skill->value >= 2) {
        if (r > 0.66)
            self->monsterInfo.currentmove = &soldier_move_duck;
        else
            self->monsterInfo.currentmove = &soldier_move_attack3;
        return;
    }

    self->monsterInfo.currentmove = &soldier_move_attack3;
}


//
// DEATH
//

void soldier_fire6(entity_t *self)
{
    soldier_fire(self, 5);
}

void soldier_fire7(entity_t *self)
{
    soldier_fire(self, 6);
}

void soldier_dead(entity_t *self)
{
    VectorSet(self->mins, -16, -16, -24);
    VectorSet(self->maxs, 16, 16, -8);
    self->moveType = MOVETYPE_TOSS;
    self->svFlags |= SVF_DEADMONSTER;
    self->nextThink = 0;
    gi.LinkEntity(self);
}

mframe_t soldier_frames_death1 [] = {
    { ai_move, 0,   NULL },
    { ai_move, -10, NULL },
    { ai_move, -10, NULL },
    { ai_move, -10, NULL },
    { ai_move, -5,  NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   soldier_fire6 },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   soldier_fire7 },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_death1 = {FRAME_death101, FRAME_death136, soldier_frames_death1, soldier_dead};

mframe_t soldier_frames_death2 [] = {
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_death2 = {FRAME_death201, FRAME_death235, soldier_frames_death2, soldier_dead};

mframe_t soldier_frames_death3 [] = {
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
};
mmove_t soldier_move_death3 = {FRAME_death301, FRAME_death345, soldier_frames_death3, soldier_dead};

mframe_t soldier_frames_death4 [] = {
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_death4 = {FRAME_death401, FRAME_death453, soldier_frames_death4, soldier_dead};

mframe_t soldier_frames_death5 [] = {
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, -5,  NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },

    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_death5 = {FRAME_death501, FRAME_death524, soldier_frames_death5, soldier_dead};

mframe_t soldier_frames_death6 [] = {
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL },
    { ai_move, 0,   NULL }
};
mmove_t soldier_move_death6 = {FRAME_death601, FRAME_death610, soldier_frames_death6, soldier_dead};

void soldier_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point)
{
    int     n;

// check for gib
    if (self->health <= self->gibHealth) {
        gi.Sound(self, CHAN_VOICE, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 3; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib(self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
        ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
        self->deadFlag = DEAD_DEAD;
        return;
    }

    if (self->deadFlag == DEAD_DEAD)
        return;

// regular death
    self->deadFlag = DEAD_DEAD;
    self->takedamage = DAMAGE_YES;
    self->s.skinnum |= 1;

    if (self->s.skinnum == 1)
        gi.Sound(self, CHAN_VOICE, sound_death_light, 1, ATTN_NORM, 0);
    else if (self->s.skinnum == 3)
        gi.Sound(self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
    else // (self->s.skinnum == 5)
        gi.Sound(self, CHAN_VOICE, sound_death_ss, 1, ATTN_NORM, 0);

    if (fabs((self->s.origin[2] + self->viewHeight) - point[2]) <= 4) {
        // head shot
        self->monsterInfo.currentmove = &soldier_move_death3;
        return;
    }

    n = rand() % 5;
    if (n == 0)
        self->monsterInfo.currentmove = &soldier_move_death1;
    else if (n == 1)
        self->monsterInfo.currentmove = &soldier_move_death2;
    else if (n == 2)
        self->monsterInfo.currentmove = &soldier_move_death4;
    else if (n == 3)
        self->monsterInfo.currentmove = &soldier_move_death5;
    else
        self->monsterInfo.currentmove = &soldier_move_death6;
}


//
// SPAWN
//

void SP_monster_soldier_x(entity_t *self)
{

    self->s.modelindex = gi.ModelIndex("models/monsters/soldier/tris.md2");
    self->monsterInfo.scale = MODEL_SCALE;
    VectorSet(self->mins, -16, -16, -24);
    VectorSet(self->maxs, 16, 16, 32);
    self->moveType = MOVETYPE_STEP;
    self->solid = Solid::BoundingBox;

    sound_idle =    gi.SoundIndex("soldier/solidle1.wav");
    sound_sight1 =  gi.SoundIndex("soldier/solsght1.wav");
    sound_sight2 =  gi.SoundIndex("soldier/solsrch1.wav");
    sound_cock =    gi.SoundIndex("infantry/infatck3.wav");
	sound_step = gi.SoundIndex("player/step1.wav");
	sound_step2 = gi.SoundIndex("player/step2.wav");
	sound_step3 = gi.SoundIndex("player/step3.wav");
	sound_step4 = gi.SoundIndex("player/step4.wav");

    self->mass = 100;

    self->Pain = soldier_pain;
    self->Die = soldier_die;

    self->monsterInfo.stand = soldier_stand;
    self->monsterInfo.walk = soldier_walk;
    self->monsterInfo.run = soldier_run;
    self->monsterInfo.dodge = soldier_dodge;
    self->monsterInfo.attack = soldier_attack;
    self->monsterInfo.melee = NULL;
    self->monsterInfo.sight = soldier_sight;

    gi.LinkEntity(self);

    self->monsterInfo.stand(self);

    walkmonster_start(self);
}


/*QUAKED monster_soldier_light (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_light(entity_t *self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    SP_monster_soldier_x(self);

    sound_pain_light = gi.SoundIndex("soldier/solpain2.wav");
    sound_death_light = gi.SoundIndex("soldier/soldeth2.wav");
	sound_step = gi.SoundIndex("player/step1.wav");
	sound_step2 = gi.SoundIndex("player/step2.wav");
	sound_step3 = gi.SoundIndex("player/step3.wav");
	sound_step4 = gi.SoundIndex("player/step4.wav");
    gi.ModelIndex("models/objects/laser/tris.md2");
    gi.SoundIndex("misc/lasfly.wav");
    gi.SoundIndex("soldier/solatck2.wav");

    self->s.skinnum = 0;
    self->health = 20;
    self->gibHealth = -30;
}

/*QUAKED monster_soldier (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier(entity_t *self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    SP_monster_soldier_x(self);

    sound_pain = gi.SoundIndex("soldier/solpain1.wav");
    sound_death = gi.SoundIndex("soldier/soldeth1.wav");
	sound_step = gi.SoundIndex("player/step1.wav");
	sound_step2 = gi.SoundIndex("player/step2.wav");
	sound_step3 = gi.SoundIndex("player/step3.wav");
	sound_step4 = gi.SoundIndex("player/step4.wav");
    gi.SoundIndex("soldier/solatck1.wav");

    self->s.skinnum = 2;
    self->health = 30;
    self->gibHealth = -30;
}

/*QUAKED monster_soldier_ss (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_soldier_ss(entity_t *self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    SP_monster_soldier_x(self);

    sound_pain_ss = gi.SoundIndex("soldier/solpain3.wav");
    sound_death_ss = gi.SoundIndex("soldier/soldeth3.wav");
	sound_step = gi.SoundIndex("player/step1.wav");
	sound_step2 = gi.SoundIndex("player/step2.wav");
	sound_step3 = gi.SoundIndex("player/step3.wav");
	sound_step4 = gi.SoundIndex("player/step4.wav");
    gi.SoundIndex("soldier/solatck3.wav");

    self->s.skinnum = 4;
    self->health = 40;
    self->gibHealth = -30;
}
