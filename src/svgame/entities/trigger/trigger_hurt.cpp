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

It does dmg points of damage each server frame

SILENT          supresses playing the sound
SLOW            changes the damage rate to once per second
NO_PROTECTION   *nothing* stops the damage

"dmg"           default 5 (whole numbers only)

*/
void hurt_use(entity_t* self, entity_t* other, entity_t* activator)
{
    if (self->solid == Solid::Not)
        self->solid = Solid::Trigger;
    else
        self->solid = Solid::Not;
    gi.LinkEntity(self);

    if (!(self->spawnFlags & 2))
        self->Use = NULL;
}


void hurt_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    int     dflags;

    if (!other->takeDamage)
        return;

    if (self->timestamp > level.time)
        return;

    if (self->spawnFlags & 16)
        self->timestamp = level.time + 1;
    else
        self->timestamp = level.time + FRAMETIME;

    if (!(self->spawnFlags & 4)) {
        if ((level.frameNumber % 10) == 0)
            gi.Sound(other, CHAN_AUTO, self->noiseIndex, 1, ATTN_NORM, 0);
    }

    if (self->spawnFlags & 8)
        dflags = DAMAGE_NO_PROTECTION;
    else
        dflags = 0;
    T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt(entity_t* self)
{
    InitTrigger(self);

    self->noiseIndex = gi.SoundIndex("world/electro.wav");
    self->Touch = hurt_touch;

    if (!self->dmg)
        self->dmg = 5;

    if (self->spawnFlags & 1)
        self->solid = Solid::Not;
    else
        self->solid = Solid::Trigger;

    if (self->spawnFlags & 2)
        self->Use = hurt_use;

    gi.LinkEntity(self);
}