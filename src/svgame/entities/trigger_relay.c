// LICENSE HERE.

//
// svgame/entities/trigger_relay.c
//
//
// trigger_relay entity implementation.
//

// Include local game header.
#include "../g_local.h"
#include "../trigger.h"

//=====================================================
/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use(edict_t* self, edict_t* other, edict_t* activator)
{
    G_UseTargets(self, activator);
}

void SP_trigger_relay(edict_t* self)
{
    self->use = trigger_relay_use;
}
