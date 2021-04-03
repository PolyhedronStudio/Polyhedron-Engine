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
/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t* self)
{
    if (!coop->value)
        return;
    if (Q_stricmp(level.mapname, "security") == 0) {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_CreateCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}

}