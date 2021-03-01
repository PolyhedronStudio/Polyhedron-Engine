// LICENSE HERE.

//
// svgame/entities/target_blaster.c
//
//
// target_blaster entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg     default is 15
speed   default is 1000
*/

void use_target_blaster(edict_t* self, edict_t* other, edict_t* activator)
{
#if 0
    int effect;

    if (self->spawnflags & 2)
        effect = 0;
    else if (self->spawnflags & 1)
        effect = EF_HYPERBLASTER;
    else
        effect = EF_BLASTER;
#endif

    fire_blaster(self, self->s.origin, self->movedir, self->dmg, self->speed, EF_BLASTER, MOD_TARGET_BLASTER);
    gi.sound(self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
}

void SP_target_blaster(edict_t* self)
{
    self->use = use_target_blaster;
    G_SetMovedir(self->s.angles, self->movedir);
    self->noise_index = gi.soundindex("weapons/laser2.wav");

    if (!self->dmg)
        self->dmg = 15;
    if (!self->speed)
        self->speed = 1000;

    self->svflags = SVF_NOCLIENT;
}