// LICENSE HERE.

//
// svgame/entities/trigger_counter.c
//
//
// trigger_counter entity implementation.
//

// Include local game header.
#include "../g_local.h"
#include "../trigger.h"

//=====================================================
/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (self->count == 0)
        return;

    self->count--;

    if (self->count) {
        if (!(self->spawnflags & 1)) {
            gi.centerprintf(activator, "%i more to go...", self->count);
            gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
        }
        return;
    }

    if (!(self->spawnflags & 1)) {
        gi.centerprintf(activator, "Sequence completed!");
        gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
    }
    self->activator = activator;
    multi_trigger(self);
}

void SP_trigger_counter(edict_t* self)
{
    self->wait = -1;
    if (!self->count)
        self->count = 2;

    self->use = trigger_counter_use;
}