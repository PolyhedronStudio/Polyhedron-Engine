// LICENSE HERE.

//
// svgame/entities/func_conveyor.c
//
//
// func_conveyor entity implementation.
//

// Include local game header.
#include "../../g_local.h"

// Include Brush funcs header.
#include "../../brushfuncs.h"

// Include door header.
#include "func_door.h"

//=====================================================
/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed   default 100
*/

void func_conveyor_use(entity_t* self, entity_t* other, entity_t* activator)
{
    if (self->spawnFlags & 1) {
        self->speed = 0;
        self->spawnFlags &= ~1;
    }
    else {
        self->speed = self->count;
        self->spawnFlags |= 1;
    }

    if (!(self->spawnFlags & 2))
        self->count = 0;
}

void SP_func_conveyor(entity_t* self)
{
    if (!self->speed)
        self->speed = 100;

    if (!(self->spawnFlags & 1)) {
        self->count = self->speed;
        self->speed = 0;
    }

    self->Use = func_conveyor_use;

    gi.SetModel(self, self->model);
    self->solid = SOLID_BSP;
    gi.LinkEntity(self);
}