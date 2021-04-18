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
void TH_viewthing(entity_t* ent)
{
    ent->s.frame = (ent->s.frame + 1) % 7;
    ent->nextThink = level.time + FRAMETIME;
}

void SP_viewthing(entity_t* ent)
{
    gi.DPrintf("viewthing spawned\n");

    ent->moveType = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.renderfx = RenderEffects::FrameLerp;
    VectorSet(ent->mins, -16, -16, -24);
    VectorSet(ent->maxs, 16, 16, 32);
    ent->s.modelindex = gi.ModelIndex("models/objects/banner/tris.md2");
    gi.LinkEntity(ent);
    ent->nextThink = level.time + 0.5;
    ent->Think = TH_viewthing;
    return;
}