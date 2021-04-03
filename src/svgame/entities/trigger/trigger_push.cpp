// LICENSE HERE.

//
// svgame/entities/trigger_push.c
//
//
// trigger_push entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================

#define PUSH_ONCE       1

static int windsound;

void trigger_push_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (strcmp(other->classname, "grenade") == 0) {
        VectorScale(self->movedir, self->speed * 10, other->velocity);
    }
    else if (other->health > 0) {
        VectorScale(self->movedir, self->speed * 10, other->velocity);

        if (other->client) {
            // don't take falling damage immediately from this
            VectorCopy(other->velocity, other->client->oldVelocity);
            if (other->fly_sound_debounce_time < level.time) {
                other->fly_sound_debounce_time = level.time + 1.5;
                gi.sound(other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
            }
        }
    }
    if (self->spawnflags & PUSH_ONCE)
        G_FreeEdict(self);
}


/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"     defaults to 1000
*/
void SP_trigger_push(edict_t* self)
{
    InitTrigger(self);
    windsound = gi.soundindex("misc/windfly.wav");
    self->touch = trigger_push_touch;
    if (!self->speed)
        self->speed = 1000;
    gi.linkentity(self);
}