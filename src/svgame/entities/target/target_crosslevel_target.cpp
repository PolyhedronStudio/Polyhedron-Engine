// LICENSE HERE.

//
// svgame/entities/target_crosslevel_target.c
//
//
// target_crosslevel_target entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and
killTarget also work.

"delay"     delay before using targets if the trigger has been activated (default 1)
*/
void target_crosslevel_target_think(entity_t* self)
{
    if (self->spawnFlags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnFlags)) {
        UTIL_UseTargets(self, self);
        G_FreeEntity(self);
    }
}

void SP_target_crosslevel_target(entity_t* self)
{
    if (!self->delay)
        self->delay = 1;
    self->serverFlags = EntityServerFlags::NoClient;

    self->Think = target_crosslevel_target_think;
    self->nextThink = level.time + self->delay;
}