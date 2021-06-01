// LICENSE HERE.

//
// svgame/entities/trigger_counter.c
//
//
// trigger_counter entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(Entity* self, Entity* other, Entity* activator)
{
    if (self->count == 0)
        return;

    self->count--;

    if (self->count) {
        if (!(self->spawnFlags & 1)) {
            gi.CenterPrintf(activator, "%i more to go...", self->count);
            gi.Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
        }
        return;
    }

    if (!(self->spawnFlags & 1)) {
        gi.CenterPrintf(activator, "Sequence completed!");
        gi.Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
    }
    self->activator = activator;
    multi_trigger(self);
}

void SP_trigger_counter(Entity* self)
{
    self->wait = -1;
    if (!self->count)
        self->count = 2;

    self->Use = trigger_counter_use;
}