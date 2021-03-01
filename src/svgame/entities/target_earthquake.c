// LICENSE HERE.

//
// svgame/entities/target_lightramp.c
//
//
// target_lightramp entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"     severity of the quake (default:200)
"count"     duration of the quake (default:5)
*/

void target_earthquake_think(edict_t* self)
{
    int     i;
    edict_t* e;

    if (self->last_move_time < level.time) {
        gi.positioned_sound(self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
        self->last_move_time = level.time + 0.5;
    }

    for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++) {
        if (!e->inuse)
            continue;
        if (!e->client)
            continue;
        if (!e->groundentity)
            continue;

        e->groundentity = NULL;
        e->velocity[0] += crandom() * 150;
        e->velocity[1] += crandom() * 150;
        e->velocity[2] = self->speed * (100.0 / e->mass);
    }

    if (level.time < self->timestamp)
        self->nextthink = level.time + FRAMETIME;
}

void target_earthquake_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->timestamp = level.time + self->count;
    self->nextthink = level.time + FRAMETIME;
    self->activator = activator;
    self->last_move_time = 0;
}

void SP_target_earthquake(edict_t* self)
{
    if (!self->targetname)
        gi.dprintf("untargeted %s at %s\n", self->classname, vtos(self->s.origin));

    if (!self->count)
        self->count = 5;

    if (!self->speed)
        self->speed = 200;

    self->svflags |= SVF_NOCLIENT;
    self->think = target_earthquake_think;
    self->use = target_earthquake_use;

    self->noise_index = gi.soundindex("world/quake.wav");
}
