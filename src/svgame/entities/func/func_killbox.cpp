// LICENSE HERE.

//
// svgame/entities/func_conveyor.c
//
//
// func_conveyor entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../brushfuncs.h"   // Include Brush funcs.

#include "func_door.h"          // Include func_door entity header.

//=====================================================
/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/
void use_killbox(Entity* self, Entity* other, Entity* activator)
{
    SVG_KillBox(self);
}

void SP_func_killbox(Entity* ent)
{
    gi.SetModel(ent, ent->model);
    ent->Use = use_killbox;
    ent->serverFlags = EntityServerFlags::NoClient;
}
