// LICENSE HERE.

//
// svgame/entities/misc_satellite_dish.c
//
//
// misc_satellite_dish entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think(edict_t* self)
{
    self->s.frame++;
    if (self->s.frame < 38)
        self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->s.frame = 0;
    self->think = misc_satellite_dish_think;
    self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    VectorSet(ent->mins, -64, -64, 0);
    VectorSet(ent->maxs, 64, 64, 128);
    ent->s.modelindex = gi.modelindex("models/objects/satellite/tris.md2");
    ent->use = misc_satellite_dish_use;
    gi.linkentity(ent);
}