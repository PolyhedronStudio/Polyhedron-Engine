// LICENSE HERE.

//
// svgame/entities/target_splash.c
//
//
// target_splash entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_splash (1 0 0) (-8 -8 -8) (8 8 8)
Creates a particle splash effect when used.

Set "sounds" to one of the following:
  1) sparks
  2) blue water
  3) brown water
  4) slime
  5) lava
  6) blood

"count" how many pixels in the splash
"dmg"   if set, does a radius damage at this location when it splashes
        useful for lava/sparks
*/

void use_target_splash(entity_t* self, entity_t* other, entity_t* activator)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TempEntityEvent::Splash);
    gi.WriteByte(self->count);
    gi.WritePosition(self->state.origin);
    gi.WriteDirection(self->moveDirection);
    gi.WriteByte(self->sounds);
    gi.Multicast(&self->state.origin, MultiCast::PVS);

    if (self->dmg)
        T_RadiusDamage(self, activator, self->dmg, NULL, self->dmg + 40, MOD_SPLASH);
}

void SP_target_splash(entity_t* self)
{
    self->Use = use_target_splash;
    UTIL_SetMoveDir(self->state.angles, self->moveDirection);

    if (!self->count)
        self->count = 32;

    self->serverFlags = EntityServerFlags::NoClient;
}