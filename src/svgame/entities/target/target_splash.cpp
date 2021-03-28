// LICENSE HERE.

//
// svgame/entities/target_splash.c
//
//
// target_splash entity implementation.
//

// Include local game header.
#include "../../g_local.h"

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

void use_target_splash(edict_t* self, edict_t* other, edict_t* activator)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_SPLASH);
    gi.WriteByte(self->count);
    gi.WritePosition(self->s.origin);
    gi.WriteDir(self->movedir);
    gi.WriteByte(self->sounds);
    gi.Multicast(self->s.origin, MULTICAST_PVS);

    if (self->dmg)
        T_RadiusDamage(self, activator, self->dmg, NULL, self->dmg + 40, MOD_SPLASH);
}

void SP_target_splash(edict_t* self)
{
    self->use = use_target_splash;
    G_SetMovedir(self->s.angles, self->movedir);

    if (!self->count)
        self->count = 32;

    self->svflags = SVF_NOCLIENT;
}