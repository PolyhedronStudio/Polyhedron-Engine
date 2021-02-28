// LICENSE HERE.

//
// svgame/entities/misc_banner.c
//
//
// misc_banner entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think(edict_t* ent)
{
    ent->s.frame = (ent->s.frame + 1) % 16;
    ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_NOT;
    ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
    ent->s.frame = rand() % 16;
    gi.linkentity(ent);

    ent->think = misc_banner_think;
    ent->nextthink = level.time + FRAMETIME;
}