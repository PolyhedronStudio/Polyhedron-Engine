// LICENSE HERE.

//
// svgame/entities/viewthing.c
//
//
// viewthing entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================

/*QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
void TH_viewthing(edict_t* ent)
{
    ent->s.frame = (ent->s.frame + 1) % 7;
    ent->nextthink = level.time + FRAMETIME;
}

void SP_viewthing(edict_t* ent)
{
    gi.dprintf("viewthing spawned\n");

    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.renderfx = RF_FRAMELERP;
    VectorSet(ent->mins, -16, -16, -24);
    VectorSet(ent->maxs, 16, 16, 32);
    ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
    gi.linkentity(ent);
    ent->nextthink = level.time + 0.5;
    ent->think = TH_viewthing;
    return;
}