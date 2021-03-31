// LICENSE HERE.

//
// svgame/entities/func_train.c
//
//
// func_train entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../brushfuncs.h"   // Include Brush funcs.
#include "../../effects.h"

#include "func_door.h"          // Include func_door entity header.

//=====================================================
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
void train_next(edict_t* self);

void train_blocked(edict_t* self, edict_t* other)
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

void train_wait(edict_t* self)
{
    if (self->target_ent->pathtarget) {
        char* savetarget;
        edict_t* ent;

        ent = self->target_ent;
        savetarget = ent->target;
        ent->target = ent->pathtarget;
        UTIL_UseTargets(ent, self->activator);
        ent->target = savetarget;

        // make sure we didn't get killed by a killtarget
        if (!self->inuse)
            return;
    }

    if (self->moveinfo.wait) {
        if (self->moveinfo.wait > 0) {
            self->nextthink = level.time + self->moveinfo.wait;
            self->think = train_next;
        }
        else if (self->spawnflags & TRAIN_TOGGLE) { // && wait < 0
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
    }
    else {
        train_next(self);
    }

}

void train_next(edict_t* self)
{
    edict_t* ent;
    vec3_t      dest;
    qboolean    first;

    first = true;
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
        first = false;
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

void train_resume(edict_t* self)
{
    edict_t* ent;
    vec3_t  dest;

    ent = self->target_ent;

    VectorSubtract(ent->s.origin, self->mins, dest);
    self->moveinfo.state = STATE_TOP;
    VectorCopy(self->s.origin, self->moveinfo.start_origin);
    VectorCopy(dest, self->moveinfo.end_origin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnflags |= TRAIN_START_ON;
}

void func_train_find(edict_t* self)
{
    edict_t* ent;

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

void train_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->activator = activator;

    if (self->spawnflags & TRAIN_START_ON) {
        if (!(self->spawnflags & TRAIN_TOGGLE))
            return;
        self->spawnflags &= ~TRAIN_START_ON;
        VectorClear(self->velocity);
        self->nextthink = 0;
    }
    else {
        if (self->target_ent)
            train_resume(self);
        else
            train_next(self);
    }
}

void SP_func_train(edict_t* self)
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
    }
    else {
        gi.dprintf("func_train without a target at %s\n", vtos(self->absmin));
    }
}