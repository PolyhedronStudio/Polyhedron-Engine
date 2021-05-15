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

void trigger_push_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf)
{
    if (strcmp(other->className, "grenade") == 0) {
        VectorScale(self->moveDirection, self->speed * 10, other->velocity);
    }
    else if (other->health > 0) {
        VectorScale(self->moveDirection, self->speed * 10, other->velocity);

        if (other->client) {
            other->client->playerState.pmove.flags |= PMF_TIME_PUSHED;
            other->client->playerState.pmove.time = 240;

            // don't take falling damage immediately from this
            VectorCopy(other->velocity, other->client->oldVelocity);
            if (other->debounceSoundTime < level.time) {
                other->debounceSoundTime = level.time + 1.5;
                gi.Sound(other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
            }
        }
    }
    if (self->spawnFlags & PUSH_ONCE)
        SVG_FreeEntity(self);
}


/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"     defaults to 1000
*/
void SP_trigger_push(Entity* self)
{
    InitTrigger(self);
    windsound = gi.SoundIndex("misc/windfly.wav");
    self->Touch = trigger_push_touch;
    if (!self->speed)
        self->speed = 1000;
    gi.LinkEntity(self);
}