// LICENSE HERE.

//
// svgame/entities/trigger_elevator.c
//
//
// trigger_elevator entity implementation.
//

// Include local game header.
#include "../g_local.h"

// extern, is in func_train.c
extern void train_resume(edict_t* self);

//=====================================================
/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use(edict_t* self, edict_t* other, edict_t* activator)
{
    edict_t* target;

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

void trigger_elevator_init(edict_t* self)
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

void SP_trigger_elevator(edict_t* self)
{
    self->think = trigger_elevator_init;
    self->nextthink = level.time + FRAMETIME;
}