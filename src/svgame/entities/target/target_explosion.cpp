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
"damage"       how much radius damage should be done, defaults to 0
*/
void target_explosion_explode(Entity* self)
{
    float       save;

    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(TempEntityEvent::Explosion1);
    gi.WriteVector3(self->state.origin);
    gi.Multicast(self->state.origin, MultiCast::PHS);

    SVG_InflictRadiusDamage(self, self->activator, self->damage, NULL, self->damage + 40, MeansOfDeath::Explosive);

    save = self->delay;
    self->delay = 0;
    UTIL_UseTargets(self, self->activator);
    self->delay = save;
}

void use_target_explosion(Entity* self, Entity* other, Entity* activator)
{
    self->activator = activator;

    if (!self->delay) {
        target_explosion_explode(self);
        return;
    }

    self->Think = target_explosion_explode;
    self->nextThinkTime = level.time + self->delay;
}

void SP_target_explosion(Entity* ent)
{
    ent->Use = use_target_explosion;
    ent->serverFlags = EntityServerFlags::NoClient;
}