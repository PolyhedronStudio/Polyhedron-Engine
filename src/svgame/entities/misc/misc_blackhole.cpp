// LICENSE HERE.

//
// svgame/entities/misc_blackhole.c
//
//
// misc_blackhole entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use(entity_t* ent, entity_t* other, entity_t* activator)
{
    /*
    gi.WriteByte (svg_temp_entity);
    gi.WriteByte (TE_BOSSTPORT);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PVS);
    */
    G_FreeEntity(ent);
}

void misc_blackhole_think(entity_t* self)
{
    if (++self->s.frame < 19)
        self->nextThink = level.time + FRAMETIME;
    else {
        self->s.frame = 0;
        self->nextThink = level.time + FRAMETIME;
    }
}

void SP_misc_blackhole(entity_t* ent)
{
    ent->moveType = MOVETYPE_NONE;
    ent->solid = SOLID_NOT;
    VectorSet(ent->mins, -64, -64, 0);
    VectorSet(ent->maxs, 64, 64, 8);
    ent->s.modelindex = gi.ModelIndex("models/objects/black/tris.md2");
    ent->s.renderfx = RF_TRANSLUCENT;
    ent->Use = misc_blackhole_use;
    ent->Think = misc_blackhole_think;
    ent->nextThink = level.time + 2 * FRAMETIME;
    gi.LinkEntity(ent);
}