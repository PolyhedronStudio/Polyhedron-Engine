// LICENSE HERE.

//
// svgame/entities/target_temp_entity.c
//
//
// target_temp_entity entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
Fire an origin based temp entity event to the clients.
"style"     type byte
*/
void Use_Target_Tent(Entity* ent, Entity* other, Entity* activator)
{
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(ent->style);
    gi.WritePosition(ent->state.origin);
    gi.Multicast(&ent->state.origin, MultiCast::PVS);
}

void SP_target_temp_entity(Entity* ent)
{
    ent->Use = Use_Target_Tent;
}
