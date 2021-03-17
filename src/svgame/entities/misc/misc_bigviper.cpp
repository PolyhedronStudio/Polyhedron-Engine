// LICENSE HERE.

//
// svgame/entities/misc_bigviper.c
//
//
// misc_bigviper entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72)
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    Vec3_Set(ent->mins, -176, -120, -24);
    Vec3_Set(ent->maxs, 176, 120, 72);
    ent->s.modelindex = gi.modelindex("models/ships/bigviper/tris.md2");
    gi.linkentity(ent);
}