// LICENSE HERE.

//
// svgame/entities/misc_easterchick2.c
//
//
// misc_easterchick2 entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick2_think(edict_t* self)
{
    if (++self->s.frame < 287)
        self->nextthink = level.time + FRAMETIME;
    else {
        self->s.frame = 248;
        self->nextthink = level.time + FRAMETIME;
    }
}

void SP_misc_easterchick2(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    VectorSet(ent->mins, -32, -32, 0);
    VectorSet(ent->maxs, 32, 32, 32);
    ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
    ent->s.frame = 248;
    ent->think = misc_easterchick2_think;
    ent->nextthink = level.time + 2 * FRAMETIME;
    gi.linkentity(ent);
}