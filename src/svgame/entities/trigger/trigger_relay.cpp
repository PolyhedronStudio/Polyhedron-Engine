// LICENSE HERE.

//
// svgame/entities/trigger_relay.c
//
//
// trigger_relay entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.
#include "../../brushfuncs.h"   // Brush funcs.
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use(entity_t* self, entity_t* other, entity_t* activator)
{
    UTIL_UseTargets(self, activator);
}

void SP_trigger_relay(entity_t* self)
{
    self->Use = trigger_relay_use;
}
