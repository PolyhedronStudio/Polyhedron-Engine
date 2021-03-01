// LICENSE HERE.

//
// svgame/entities/func_conveyor.c
//
//
// func_conveyor entity implementation.
//

// Include local game header.
#include "../g_local.h"

// Include Brush funcs header.
#include "../brushfuncs.h"

// Include door header.
#include "func_door.h"

//=====================================================
/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
void use_killbox(edict_t* self, edict_t* other, edict_t* activator)
{
    KillBox(self);
}

void SP_func_killbox(edict_t* ent)
{
    gi.setmodel(ent, ent->model);
    ent->use = use_killbox;
    ent->svflags = SVF_NOCLIENT;
}
