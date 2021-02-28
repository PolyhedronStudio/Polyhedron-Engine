// LICENSE HERE.

//
// svgame/entities/light_mine2.c
//
//
// light_mine2 entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.modelindex = gi.modelindex("models/objects/minelite/light2/tris.md2");
    gi.linkentity(ent);
}