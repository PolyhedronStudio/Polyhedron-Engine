// LICENSE HERE.

//
// svgame/entities/trigger_hurt.c
//
//
// trigger_hurt entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does damage points of damage each server frame

SILENT          supresses playing the sound
SLOW            changes the damage rate to once per second
NO_PROTECTION   *nothing* stops the damage

"damage"           default 5 (whole numbers only)

*/
void hurt_use(Entity* self, Entity* other, Entity* activator)
{
    if (self->solid == Solid::Not)
        self->solid = Solid::Trigger;
    else
        self->solid = Solid::Not;
    gi.LinkEntity(self);

    if (!(self->spawnFlags & 2))
        self->Use = NULL;
}


void hurt_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf)
{
    int     dflags;

    if (!other->takeDamage)
        return;

    if (self->timeStamp > level.time)
        return;

    if (self->spawnFlags & 16)
        self->timeStamp = level.time + 1;
    else
        self->timeStamp = level.time + FRAMETIME;

    if (!(self->spawnFlags & 4)) {
        if ((level.frameNumber % 10) == 0)
            gi.Sound(other, CHAN_AUTO, self->noiseIndex, 1, ATTN_NORM, 0);
    }

    if (self->spawnFlags & 8)
        dflags = DamageFlags::IgnoreProtection;
    else
        dflags = 0;
    T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->damage, self->damage, dflags, MeansOfDeath::TriggerHurt);
}

void SP_trigger_hurt(Entity* self)
{
    InitTrigger(self);

    self->noiseIndex = gi.SoundIndex("world/electro.wav");
    self->Touch = hurt_touch;

    if (!self->damage)
        self->damage = 5;

    if (self->spawnFlags & 1)
        self->solid = Solid::Not;
    else
        self->solid = Solid::Trigger;

    if (self->spawnFlags & 2)
        self->Use = hurt_use;

    gi.LinkEntity(self);
}