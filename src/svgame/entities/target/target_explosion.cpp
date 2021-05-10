// LICENSE HERE.

//
// svgame/entities/target_explosion.c
//
//
// target_explosion entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"     wait this long before going off
"dmg"       how much radius damage should be done, defaults to 0
*/
void target_explosion_explode(entity_t* self)
{
    float       save;

    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TempEntityEvent::Explosion1);
    gi.WritePosition(self->state.origin);
    gi.Multicast(&self->state.origin, MultiCast::PHS);

    T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);

    save = self->delay;
    self->delay = 0;
    UTIL_UseTargets(self, self->activator);
    self->delay = save;
}

void use_target_explosion(entity_t* self, entity_t* other, entity_t* activator)
{
    self->activator = activator;

    if (!self->delay) {
        target_explosion_explode(self);
        return;
    }

    self->Think = target_explosion_explode;
    self->nextThink = level.time + self->delay;
}

void SP_target_explosion(entity_t* ent)
{
    ent->Use = use_target_explosion;
    ent->serverFlags = EntityServerFlags::NoClient;
}