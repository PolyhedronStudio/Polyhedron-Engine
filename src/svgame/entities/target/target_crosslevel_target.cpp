// LICENSE HERE.

//
// svgame/entities/target_crosslevel_target.c
//
//
// target_crosslevel_target entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and
killtarget also work.

"delay"     delay before using targets if the trigger has been activated (default 1)
*/
void target_crosslevel_target_think(edict_t* self)
{
    if (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags)) {
        G_UseTargets(self, self);
        G_FreeEdict(self);
    }
}

void SP_target_crosslevel_target(edict_t* self)
{
    if (!self->delay)
        self->delay = 1;
    self->svflags = SVF_NOCLIENT;

    self->think = target_crosslevel_target_think;
    self->nextthink = level.time + self->delay;
}