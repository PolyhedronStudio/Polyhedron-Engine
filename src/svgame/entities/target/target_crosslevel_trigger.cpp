// LICENSE HERE.

//
// svgame/entities/target_crosslevel_trigger.c
//
//
// target_crosslevel_trigger entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killTarget also work.
*/
void trigger_crosslevel_trigger_use(entity_t* self, entity_t* other, entity_t* activator)
{
    game.serverflags |= self->spawnFlags;
    G_FreeEntity(self);
}

void SP_target_crosslevel_trigger(entity_t* self)
{
    self->serverFlags = EntityServerFlags::NoClient;
    self->Use = trigger_crosslevel_trigger_use;
}