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
void hurt_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (self->solid == SOLID_NOT)
        self->solid = SOLID_TRIGGER;
    else
        self->solid = SOLID_NOT;
    gi.linkentity(self);

    if (!(self->spawnflags & 2))
        self->use = NULL;
}


void hurt_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    int     dflags;

    if (!other->takedamage)
        return;

    if (self->timestamp > level.time)
        return;

    if (self->spawnflags & 16)
        self->timestamp = level.time + 1;
    else
        self->timestamp = level.time + FRAMETIME;

    if (!(self->spawnflags & 4)) {
        if ((level.framenum % 10) == 0)
            gi.sound(other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
    }

    if (self->spawnflags & 8)
        dflags = DAMAGE_NO_PROTECTION;
    else
        dflags = 0;
    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt(edict_t* self)
{
    InitTrigger(self);

    self->noise_index = gi.soundindex("world/electro.wav");
    self->touch = hurt_touch;

    if (!self->dmg)
        self->dmg = 5;

    if (self->spawnflags & 1)
        self->solid = SOLID_NOT;
    else
        self->solid = SOLID_TRIGGER;

    if (self->spawnflags & 2)
        self->use = hurt_use;

    gi.linkentity(self);
}