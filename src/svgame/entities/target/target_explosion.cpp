// LICENSE HERE.

//
// svgame/entities/target_explosion.c
//
//
// target_explosion entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"     wait this long before going off
"dmg"       how much radius damage should be done, defaults to 0
*/
void target_explosion_explode(edict_t* self)
{
    float       save;

    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_EXPLOSION1);
    gi.WritePosition(self->s.origin);
    gi.Multicast(&self->s.origin, MULTICAST_PHS);

    T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);

    save = self->delay;
    self->delay = 0;
    G_UseTargets(self, self->activator);
    self->delay = save;
}

void use_target_explosion(edict_t* self, edict_t* other, edict_t* activator)
{
    self->activator = activator;

    if (!self->delay) {
        target_explosion_explode(self);
        return;
    }

    self->think = target_explosion_explode;
    self->nextthink = level.time + self->delay;
}

void SP_target_explosion(edict_t* ent)
{
    ent->use = use_target_explosion;
    ent->svflags = SVF_NOCLIENT;
}