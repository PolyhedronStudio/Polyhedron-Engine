// LICENSE HERE.

//
// svgame/entities/target_lightramp.c
//
//
// target_lightramp entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"     severity of the quake (default:200)
"count"     duration of the quake (default:5)
*/

void target_earthquake_think(entity_t* self)
{
    int     i;
    entity_t* e;

    if (self->lastMoveTime < level.time) {
        gi.PositionedSound(self->state.origin, self, CHAN_AUTO, self->noiseIndex, 1.0, ATTN_NONE, 0);
        self->lastMoveTime = level.time + 0.5;
    }

    for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++) {
        if (!e->inUse)
            continue;
        if (!e->client)
            continue;
        if (!e->groundEntityPtr)
            continue;

        e->groundEntityPtr = NULL;
        e->velocity[0] += crandom() * 150;
        e->velocity[1] += crandom() * 150;
        e->velocity[2] = self->speed * (100.0 / e->mass);
    }

    if (level.time < self->timestamp)
        self->nextThink = level.time + FRAMETIME;
}

void target_earthquake_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->timestamp = level.time + self->count;
    self->nextThink = level.time + FRAMETIME;
    self->activator = activator;
    self->lastMoveTime = 0;
}

void SP_target_earthquake(entity_t* self)
{
    if (!self->targetName)
        gi.DPrintf("untargeted %s at %s\n", self->classname, Vec3ToString(self->state.origin));

    if (!self->count)
        self->count = 5;

    if (!self->speed)
        self->speed = 200;

    self->serverFlags |= EntityServerFlags::NoClient;
    self->Think = target_earthquake_think;
    self->Use = target_earthquake_use;

    self->noiseIndex = gi.SoundIndex("world/quake.wav");
}
