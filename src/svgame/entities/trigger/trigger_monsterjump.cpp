// LICENSE HERE.

//
// svgame/entities/trigger_relay.c
//
//
// trigger_relay entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    if (other->flags & (FL_FLY | FL_SWIM))
        return;
    if (other->svFlags & SVF_DEADMONSTER)
        return;
    if (!(other->svFlags & SVF_MONSTER))
        return;

    // set XY even if not on ground, so the jump will clear lips
    other->velocity[0] = self->moveDirection[0] * self->speed;
    other->velocity[1] = self->moveDirection[1] * self->speed;

    if (!other->groundEntityPtr)
        return;

    other->groundEntityPtr = NULL;
    other->velocity[2] = self->moveDirection[2];
}

void SP_trigger_monsterjump(entity_t* self)
{
    if (!self->speed)
        self->speed = 200;
    if (!st.height)
        st.height = 200;
    if (self->s.angles[vec3_t::Yaw] == 0)
        self->s.angles[vec3_t::Yaw] = 360;
    InitTrigger(self);
    self->Touch = trigger_monsterjump_touch;
    self->moveDirection[2] = st.height;
}
