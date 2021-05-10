// LICENSE HERE.

//
// svgame/entities/target_blaster.c
//
//
// target_blaster entity implementation.
//
#include "../../g_local.h"         // Include SVGame funcs.
#include "../../utils.h"           // Include Utilities funcs.

//=====================================================
/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg     default is 15
speed   default is 1000
*/

void use_target_blaster(entity_t* self, entity_t* other, entity_t* activator)
{
#if 0
    int effect;

    if (self->spawnFlags & 2)
        effect = 0;
    else if (self->spawnFlags & 1)
        effect = EF_HYPERBLASTER;
    else
        effect = EF_BLASTER;
#endif

    fire_blaster(self, self->state.origin, self->moveDirection, self->dmg, self->speed, EntityEffectType::Blaster, MOD_TARGET_BLASTER);
    gi.Sound(self, CHAN_VOICE, self->noiseIndex, 1, ATTN_NORM, 0);
}

void SP_target_blaster(entity_t* self)
{
    self->Use = use_target_blaster;
    UTIL_SetMoveDir(self->state.angles, self->moveDirection);
    self->noiseIndex = gi.SoundIndex("weapons/laser2.wav");

    if (!self->dmg)
        self->dmg = 15;
    if (!self->speed)
        self->speed = 1000;

    self->serverFlags = EntityServerFlags::NoClient;
}