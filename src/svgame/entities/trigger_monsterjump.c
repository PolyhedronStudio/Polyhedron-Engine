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
/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (other->flags & (FL_FLY | FL_SWIM))
        return;
    if (other->svflags & SVF_DEADMONSTER)
        return;
    if (!(other->svflags & SVF_MONSTER))
        return;

    // set XY even if not on ground, so the jump will clear lips
    other->velocity[0] = self->movedir[0] * self->speed;
    other->velocity[1] = self->movedir[1] * self->speed;

    if (!other->groundentity)
        return;

    other->groundentity = NULL;
    other->velocity[2] = self->movedir[2];
}

void SP_trigger_monsterjump(edict_t* self)
{
    if (!self->speed)
        self->speed = 200;
    if (!st.height)
        st.height = 200;
    if (self->s.angles[YAW] == 0)
        self->s.angles[YAW] = 360;
    InitTrigger(self);
    self->touch = trigger_monsterjump_touch;
    self->movedir[2] = st.height;
}
