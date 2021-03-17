// LICENSE HERE.

//
// svgame/entities/misc_easterchick.c
//
//
// misc_easterchick entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick_think(edict_t* self)
{
    if (++self->s.frame < 247)
        self->nextthink = level.time + FRAMETIME;
    else {
        self->s.frame = 208;
        self->nextthink = level.time + FRAMETIME;
    }
}

void SP_misc_easterchick(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    Vec3_Set(ent->mins, -32, -32, 0);
    Vec3_Set(ent->maxs, 32, 32, 32);
    ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
    ent->s.frame = 208;
    ent->think = misc_easterchick_think;
    ent->nextthink = level.time + 2 * FRAMETIME;
    gi.linkentity(ent);
}