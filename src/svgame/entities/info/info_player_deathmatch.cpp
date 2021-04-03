// LICENSE HERE.

//
// svgame/entities/info_player_start.cpp
//
//
// info_player_start entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t* self)
{
    if (!deathmatch->value) {
        G_FreeEdict(self);
        return;
    }
    SP_misc_teleporter_dest(self);
}

