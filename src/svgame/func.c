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
#include "g_local.h"

// Include Brush funcs header.
#include "brushfuncs.h"

// Include func_door header.
#include "entities/func_door.h"

/*
=========================================================

  PLATS

  movement options:

  linear
  smooth start, hard stop
  smooth start, smooth stop

  start
  end
  acceleration
  speed
  deceleration
  begin sound
  end sound
  target fired when reaching end
  wait at end

  object characteristics that use move segments
  ---------------------------------------------
  movetype_push, or movetype_stop
  action when touched
  action when blocked
  action when used
    disabled?
  auto trigger spawning


=========================================================
*/


//====================================================================


/*
======================================================================

DOORS

  spawn a trigger surrounding the entire team unless it is
  already targeted by another

======================================================================
*/



/*QUAKED func_water (0 .5 .8) ? START_OPEN
func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.

START_OPEN causes the water to move to its destination when spawned and operate in reverse.

"angle"     determines the opening direction (up or down only)
"speed"     movement speed (25 default)
"wait"      wait before returning (-1 default, -1 = TOGGLE)
"lip"       lip remaining at end of move (0 default)
"sounds"    (yes, these need to be changed)
0)  no sound
1)  water
2)  lava
*/

void SP_func_water(edict_t *self)
{
    vec3_t  abs_movedir;

    G_SetMovedir(self->s.angles, self->movedir);
    self->movetype = MOVETYPE_PUSH;
    self->solid = SOLID_BSP;
    gi.setmodel(self, self->model);

    switch (self->sounds) {
    default:
        break;

    case 1: // water
        self->moveinfo.sound_start = gi.soundindex("world/mov_watr.wav");
        self->moveinfo.sound_end = gi.soundindex("world/stp_watr.wav");
        break;

    case 2: // lava
        self->moveinfo.sound_start = gi.soundindex("world/mov_watr.wav");
        self->moveinfo.sound_end = gi.soundindex("world/stp_watr.wav");
        break;
    }

    // calculate second position
    VectorCopy(self->s.origin, self->pos1);
    abs_movedir[0] = fabs(self->movedir[0]);
    abs_movedir[1] = fabs(self->movedir[1]);
    abs_movedir[2] = fabs(self->movedir[2]);
    self->moveinfo.distance = abs_movedir[0] * self->size[0] + abs_movedir[1] * self->size[1] + abs_movedir[2] * self->size[2] - st.lip;
    VectorMA(self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

    // if it starts open, switch the positions
    if (self->spawnflags & DOOR_START_OPEN) {
        VectorCopy(self->pos2, self->s.origin);
        VectorCopy(self->pos1, self->pos2);
        VectorCopy(self->s.origin, self->pos1);
    }

    VectorCopy(self->pos1, self->moveinfo.start_origin);
    VectorCopy(self->s.angles, self->moveinfo.start_angles);
    VectorCopy(self->pos2, self->moveinfo.end_origin);
    VectorCopy(self->s.angles, self->moveinfo.end_angles);

    self->moveinfo.state = STATE_BOTTOM;

    if (!self->speed)
        self->speed = 25;
    self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed = self->speed;

    if (!self->wait)
        self->wait = -1;
    self->moveinfo.wait = self->wait;

    self->use = door_use;

    if (self->wait == -1)
        self->spawnflags |= DOOR_TOGGLE;

    self->classname = "func_door";

    gi.linkentity(self);
}


#define TRAIN_START_ON      1
#define TRAIN_TOGGLE        2
#define TRAIN_BLOCK_STOPS   4

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed   default 100
dmg     default 2
noise   looping sound to play when the train is in motion

*/
void train_next(edict_t *self);

void train_blocked(edict_t *self, edict_t *other)
{
    if (!(other->svflags & SVF_MONSTER) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    if (level.time < self->touch_debounce_time)
        return;

    if (!self->dmg)
        return;
    self->touch_debounce_time = level.time + 0.5;
    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void train_wait(edict_t *self)
{
    if (self->target_ent->pathtarget) {
        char    *savetarget;
        edict_t *ent;

        ent = self->target_ent;
        savetarget = ent->target;
        ent->target = ent->pathtarget;
        G_UseTargets(ent, self->activator);
        ent->target = savetarget;

        // make sure we didn't get killed by a killtarget
        if (!self->inuse)
            return;
    }

    if (self->moveinfo.wait) {
        if (self->moveinfo.wait > 0) {
            self->nextthink = level.time + self->moveinfo.wait;
            self->think = train_next;
        } else if (self->spawnflags & TRAIN_TOGGLE) { // && wait < 0
            train_next(self);
            self->spawnflags &= ~TRAIN_START_ON;
            VectorClear(self->velocity);
            self->nextthink = 0;
        }

        if (!(self->flags & FL_TEAMSLAVE)) {
            if (self->moveinfo.sound_end)
                gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
            self->s.sound = 0;
        }
    } else {
        train_next(self);
    }

}

void train_next(edict_t *self)
{
    edict_t     *ent;
    vec3_t      dest;
    qboolean    first;

    first = qtrue;
again:
    if (!self->target) {
//      gi.dprintf ("train_next: no next target\n");
        return;
    }

    ent = G_PickTarget(self->target);
    if (!ent) {
        gi.dprintf("train_next: bad target %s\n", self->target);
        return;
    }

    self->target = ent->target;

    // check for a teleport path_corner
    if (ent->spawnflags & 1) {
        if (!first) {
            gi.dprintf("connected teleport path_corners, see %s at %s\n", ent->classname, vtos(ent->s.origin));
            return;
        }
        first = qfalse;
        VectorSubtract(ent->s.origin, self->mins, self->s.origin);
        VectorCopy(self->s.origin, self->s.old_origin);
        self->s.event = EV_OTHER_TELEPORT;
        gi.linkentity(self);
        goto again;
    }

    self->moveinfo.wait = ent->wait;
    self->target_ent = ent;

    if (!(self->flags & FL_TEAMSLAVE)) {
        if (self->moveinfo.sound_start)
            gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
        self->s.sound = self->moveinfo.sound_middle;
    }

    VectorSubtract(ent->s.origin, self->mins, dest);
    self->moveinfo.state = STATE_TOP;
    VectorCopy(self->s.origin, self->moveinfo.start_origin);
    VectorCopy(dest, self->moveinfo.end_origin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnflags |= TRAIN_START_ON;
}

void train_resume(edict_t *self)
{
    edict_t *ent;
    vec3_t  dest;

    ent = self->target_ent;

    VectorSubtract(ent->s.origin, self->mins, dest);
    self->moveinfo.state = STATE_TOP;
    VectorCopy(self->s.origin, self->moveinfo.start_origin);
    VectorCopy(dest, self->moveinfo.end_origin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnflags |= TRAIN_START_ON;
}

void func_train_find(edict_t *self)
{
    edict_t *ent;

    if (!self->target) {
        gi.dprintf("train_find: no target\n");
        return;
    }
    ent = G_PickTarget(self->target);
    if (!ent) {
        gi.dprintf("train_find: target %s not found\n", self->target);
        return;
    }
    self->target = ent->target;

    VectorSubtract(ent->s.origin, self->mins, self->s.origin);
    gi.linkentity(self);

    // if not triggered, start immediately
    if (!self->targetname)
        self->spawnflags |= TRAIN_START_ON;

    if (self->spawnflags & TRAIN_START_ON) {
        self->nextthink = level.time + FRAMETIME;
        self->think = train_next;
        self->activator = self;
    }
}

void train_use(edict_t *self, edict_t *other, edict_t *activator)
{
    self->activator = activator;

    if (self->spawnflags & TRAIN_START_ON) {
        if (!(self->spawnflags & TRAIN_TOGGLE))
            return;
        self->spawnflags &= ~TRAIN_START_ON;
        VectorClear(self->velocity);
        self->nextthink = 0;
    } else {
        if (self->target_ent)
            train_resume(self);
        else
            train_next(self);
    }
}

void SP_func_train(edict_t *self)
{
    self->movetype = MOVETYPE_PUSH;

    VectorClear(self->s.angles);
    self->blocked = train_blocked;
    if (self->spawnflags & TRAIN_BLOCK_STOPS)
        self->dmg = 0;
    else {
        if (!self->dmg)
            self->dmg = 100;
    }
    self->solid = SOLID_BSP;
    gi.setmodel(self, self->model);

    if (st.noise)
        self->moveinfo.sound_middle = gi.soundindex(st.noise);

    if (!self->speed)
        self->speed = 100;

    self->moveinfo.speed = self->speed;
    self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

    self->use = train_use;

    gi.linkentity(self);

    if (self->target) {
        // start trains on the second frame, to make sure their targets have had
        // a chance to spawn
        self->nextthink = level.time + FRAMETIME;
        self->think = func_train_find;
    } else {
        gi.dprintf("func_train without a target at %s\n", vtos(self->absmin));
    }
}


/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use(edict_t *self, edict_t *other, edict_t *activator)
{
    edict_t *target;

    if (self->movetarget->nextthink) {
//      gi.dprintf("elevator busy\n");
        return;
    }

    if (!other->pathtarget) {
        gi.dprintf("elevator used with no pathtarget\n");
        return;
    }

    target = G_PickTarget(other->pathtarget);
    if (!target) {
        gi.dprintf("elevator used with bad pathtarget: %s\n", other->pathtarget);
        return;
    }

    self->movetarget->target_ent = target;
    train_resume(self->movetarget);
}

void trigger_elevator_init(edict_t *self)
{
    if (!self->target) {
        gi.dprintf("trigger_elevator has no target\n");
        return;
    }
    self->movetarget = G_PickTarget(self->target);
    if (!self->movetarget) {
        gi.dprintf("trigger_elevator unable to find target %s\n", self->target);
        return;
    }
    if (strcmp(self->movetarget->classname, "func_train") != 0) {
        gi.dprintf("trigger_elevator target %s is not a train\n", self->target);
        return;
    }

    self->use = trigger_elevator_use;
    self->svflags = SVF_NOCLIENT;

}

void SP_trigger_elevator(edict_t *self)
{
    self->think = trigger_elevator_init;
    self->nextthink = level.time + FRAMETIME;
}


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
"wait"          base time between triggering all targets, default is 1
"random"        wait variance, default is 0

so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"delay"         delay before first firing when turned on, default is 0

"pausetime"     additional delay used only the very first time
                and only if spawned with START_ON

These can used but not touched.
*/
void func_timer_think(edict_t *self)
{
    G_UseTargets(self, self->activator);
    self->nextthink = level.time + self->wait + crandom() * self->random;
}

void func_timer_use(edict_t *self, edict_t *other, edict_t *activator)
{
    self->activator = activator;

    // if on, turn it off
    if (self->nextthink) {
        self->nextthink = 0;
        return;
    }

    // turn it on
    if (self->delay)
        self->nextthink = level.time + self->delay;
    else
        func_timer_think(self);
}

void SP_func_timer(edict_t *self)
{
    if (!self->wait)
        self->wait = 1.0;

    self->use = func_timer_use;
    self->think = func_timer_think;

    if (self->random >= self->wait) {
        self->random = self->wait - FRAMETIME;
        gi.dprintf("func_timer at %s has random >= wait\n", vtos(self->s.origin));
    }

    if (self->spawnflags & 1) {
        self->nextthink = level.time + 1.0 + st.pausetime + self->delay + self->wait + crandom() * self->random;
        self->activator = self;
    }

    self->svflags = SVF_NOCLIENT;
}


/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed   default 100
*/

void func_conveyor_use(edict_t *self, edict_t *other, edict_t *activator)
{
    if (self->spawnflags & 1) {
        self->speed = 0;
        self->spawnflags &= ~1;
    } else {
        self->speed = self->count;
        self->spawnflags |= 1;
    }

    if (!(self->spawnflags & 2))
        self->count = 0;
}

void SP_func_conveyor(edict_t *self)
{
    if (!self->speed)
        self->speed = 100;

    if (!(self->spawnflags & 1)) {
        self->count = self->speed;
        self->speed = 0;
    }

    self->use = func_conveyor_use;

    gi.setmodel(self, self->model);
    self->solid = SOLID_BSP;
    gi.linkentity(self);
}


/*QUAKED func_door_secret (0 .5 .8) ? always_shoot 1st_left 1st_down
A secret door.  Slide back and then to the side.

open_once       doors never closes
1st_left        1st move is left of arrow
1st_down        1st move is down from arrow
always_shoot    door is shootebale even if targeted

"angle"     determines the direction
"dmg"       damage to inflic when blocked (default 2)
"wait"      how long to hold in the open position (default 5, -1 means hold)
*/

#define SECRET_ALWAYS_SHOOT 1
#define SECRET_1ST_LEFT     2
#define SECRET_1ST_DOWN     4

void door_secret_move1(edict_t *self);
void door_secret_move2(edict_t *self);
void door_secret_move3(edict_t *self);
void door_secret_move4(edict_t *self);
void door_secret_move5(edict_t *self);
void door_secret_move6(edict_t *self);
void door_secret_done(edict_t *self);

void door_secret_use(edict_t *self, edict_t *other, edict_t *activator)
{
    // make sure we're not already moving
    if (!VectorCompare(self->s.origin, vec3_origin))
        return;

    Brush_Move_Calc(self, self->pos1, door_secret_move1);
    door_use_areaportals(self, qtrue);
}

void door_secret_move1(edict_t *self)
{
    self->nextthink = level.time + 1.0;
    self->think = door_secret_move2;
}

void door_secret_move2(edict_t *self)
{
    Brush_Move_Calc(self, self->pos2, door_secret_move3);
}

void door_secret_move3(edict_t *self)
{
    if (self->wait == -1)
        return;
    self->nextthink = level.time + self->wait;
    self->think = door_secret_move4;
}

void door_secret_move4(edict_t *self)
{
    Brush_Move_Calc(self, self->pos1, door_secret_move5);
}

void door_secret_move5(edict_t *self)
{
    self->nextthink = level.time + 1.0;
    self->think = door_secret_move6;
}

void door_secret_move6(edict_t *self)
{
    Brush_Move_Calc(self, vec3_origin, door_secret_done);
}

void door_secret_done(edict_t *self)
{
    if (!(self->targetname) || (self->spawnflags & SECRET_ALWAYS_SHOOT)) {
        self->health = 0;
        self->takedamage = DAMAGE_YES;
    }
    door_use_areaportals(self, qfalse);
}

void door_secret_blocked(edict_t *self, edict_t *other)
{
    if (!(other->svflags & SVF_MONSTER) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    if (level.time < self->touch_debounce_time)
        return;
    self->touch_debounce_time = level.time + 0.5;

    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void door_secret_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    self->takedamage = DAMAGE_NO;
    door_secret_use(self, attacker, attacker);
}

void SP_func_door_secret(edict_t *ent)
{
    vec3_t  forward, right, up;
    float   side;
    float   width;
    float   length;

    ent->moveinfo.sound_start = gi.soundindex("doors/dr1_strt.wav");
    ent->moveinfo.sound_middle = gi.soundindex("doors/dr1_mid.wav");
    ent->moveinfo.sound_end = gi.soundindex("doors/dr1_end.wav");

    ent->movetype = MOVETYPE_PUSH;
    ent->solid = SOLID_BSP;
    gi.setmodel(ent, ent->model);

    ent->blocked = door_secret_blocked;
    ent->use = door_secret_use;

    if (!(ent->targetname) || (ent->spawnflags & SECRET_ALWAYS_SHOOT)) {
        ent->health = 0;
        ent->takedamage = DAMAGE_YES;
        ent->die = door_secret_die;
    }

    if (!ent->dmg)
        ent->dmg = 2;

    if (!ent->wait)
        ent->wait = 5;

    ent->moveinfo.accel =
        ent->moveinfo.decel =
            ent->moveinfo.speed = 50;

    // calculate positions
    AngleVectors(ent->s.angles, forward, right, up);
    VectorClear(ent->s.angles);
    side = 1.0 - (ent->spawnflags & SECRET_1ST_LEFT);
    if (ent->spawnflags & SECRET_1ST_DOWN)
        width = fabs(DotProduct(up, ent->size));
    else
        width = fabs(DotProduct(right, ent->size));
    length = fabs(DotProduct(forward, ent->size));
    if (ent->spawnflags & SECRET_1ST_DOWN)
        VectorMA(ent->s.origin, -1 * width, up, ent->pos1);
    else
        VectorMA(ent->s.origin, side * width, right, ent->pos1);
    VectorMA(ent->pos1, length, forward, ent->pos2);

    if (ent->health) {
        ent->takedamage = DAMAGE_YES;
        ent->die = door_killed;
        ent->max_health = ent->health;
    } else if (ent->targetname && ent->message) {
        gi.soundindex("misc/talk.wav");
        ent->touch = door_touch;
    }

    ent->classname = "func_door";

    gi.linkentity(ent);
}


/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
void use_killbox(edict_t *self, edict_t *other, edict_t *activator)
{
    KillBox(self);
}

void SP_func_killbox(edict_t *ent)
{
    gi.setmodel(ent, ent->model);
    ent->use = use_killbox;
    ent->svflags = SVF_NOCLIENT;
}

