// LICENSE HERE.

//
// svgame/entities/misc_eastertank.c
//
//
// misc_eastertank entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/

void misc_eastertank_think(edict_t* self)
{
    if (++self->s.frame < 293)
        self->nextthink = level.time + FRAMETIME;
    else {
        self->s.frame = 254;
        self->nextthink = level.time + FRAMETIME;
    }
}

void SP_misc_eastertank(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    VectorSet(ent->mins, -32, -32, -16);
    VectorSet(ent->maxs, 32, 32, 32);
    ent->s.modelindex = gi.modelindex("models/monsters/tank/tris.md2");
    ent->s.frame = 254;
    ent->think = misc_eastertank_think;
    ent->nextthink = level.time + 2 * FRAMETIME;
    gi.linkentity(ent);
}