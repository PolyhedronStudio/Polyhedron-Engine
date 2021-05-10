// LICENSE HERE.

//
// svgame/entities/misc_teleporter_dest.c
//
//
// misc_teleporter_dest entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest(entity_t* ent)
{
    gi.SetModel(ent, "models/objects/dmspot/tris.md2");
    ent->state.skinNumber = 0;
    ent->solid = Solid::BoundingBox;
    //  ent->state.effects |= EF_FLIES;
    VectorSet(ent->mins, -32, -32, -24);
    VectorSet(ent->maxs, 32, 32, -16);
    gi.LinkEntity(ent);
}
