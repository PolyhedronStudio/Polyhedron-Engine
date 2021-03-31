// LICENSE HERE.

//
// svgame/entities/trigger_always.c
//
//
// trigger_always entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always(edict_t* ent)
{
    // we must have some delay to make sure our use targets are present
    if (ent->delay < 0.2)
        ent->delay = 0.2;
    UTIL_UseTargets(ent, ent);
}