// LICENSE HERE.

//
// svgame/entities/func_wall.c
//
//
// func_wall entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN   the wall will not be present until triggered
                it will then blink in to existance; it will
                kill anything that was in it's way

TOGGLE          only valid for TRIGGER_SPAWN walls
                this allows the wall to be turned on and off

START_ON        only valid for TRIGGER_SPAWN walls
                the wall will initially be present
*/

void func_wall_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (self->solid == SOLID_NOT) {
        self->solid = SOLID_BSP;
        self->svflags &= ~SVF_NOCLIENT;
        KillBox(self);
    }
    else {
        self->solid = SOLID_NOT;
        self->svflags |= SVF_NOCLIENT;
    }
    gi.linkentity(self);

    if (!(self->spawnflags & 2))
        self->use = NULL;
}

void SP_func_wall(edict_t* self)
{
    self->movetype = MOVETYPE_PUSH;
    gi.setmodel(self, self->model);

    if (self->spawnflags & 8)
        self->s.effects |= EF_ANIM_ALL;
    if (self->spawnflags & 16)
        self->s.effects |= EF_ANIM_ALLFAST;

    // just a wall
    if ((self->spawnflags & 7) == 0) {
        self->solid = SOLID_BSP;
        gi.linkentity(self);
        return;
    }

    // it must be TRIGGER_SPAWN
    if (!(self->spawnflags & 1)) {
        //      gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
        self->spawnflags |= 1;
    }

    // yell if the spawnflags are odd
    if (self->spawnflags & 4) {
        if (!(self->spawnflags & 2)) {
            gi.dprintf("func_wall START_ON without TOGGLE\n");
            self->spawnflags |= 2;
        }
    }

    self->use = func_wall_use;
    if (self->spawnflags & 4) {
        self->solid = SOLID_BSP;
    }
    else {
        self->solid = SOLID_NOT;
        self->svflags |= SVF_NOCLIENT;
    }
    gi.linkentity(self);
}