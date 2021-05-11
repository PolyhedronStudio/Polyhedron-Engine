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
extern void SP_FixCoopSpots(Entity* self);

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(Entity* self)
{
    if (!coop->value) {
        G_FreeEntity(self);
        return;
    }

    if ((Q_stricmp(level.mapName, "jail2") == 0) ||
        (Q_stricmp(level.mapName, "jail4") == 0) ||
        (Q_stricmp(level.mapName, "mine1") == 0) ||
        (Q_stricmp(level.mapName, "mine2") == 0) ||
        (Q_stricmp(level.mapName, "mine3") == 0) ||
        (Q_stricmp(level.mapName, "mine4") == 0) ||
        (Q_stricmp(level.mapName, "lab") == 0) ||
        (Q_stricmp(level.mapName, "boss1") == 0) ||
        (Q_stricmp(level.mapName, "fact3") == 0) ||
        (Q_stricmp(level.mapName, "biggun") == 0) ||
        (Q_stricmp(level.mapName, "space") == 0) ||
        (Q_stricmp(level.mapName, "command") == 0) ||
        (Q_stricmp(level.mapName, "power2") == 0) ||
        (Q_stricmp(level.mapName, "strike") == 0)) {
        // invoke one of our gross, ugly, disgusting hacks
        self->Think = SP_FixCoopSpots;
        self->nextThink = level.time + FRAMETIME;
    }
}


