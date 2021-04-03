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
extern void SP_FixCoopSpots(edict_t* self);

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t* self)
{
    if (!coop->value) {
        G_FreeEdict(self);
        return;
    }

    if ((Q_stricmp(level.mapname, "jail2") == 0) ||
        (Q_stricmp(level.mapname, "jail4") == 0) ||
        (Q_stricmp(level.mapname, "mine1") == 0) ||
        (Q_stricmp(level.mapname, "mine2") == 0) ||
        (Q_stricmp(level.mapname, "mine3") == 0) ||
        (Q_stricmp(level.mapname, "mine4") == 0) ||
        (Q_stricmp(level.mapname, "lab") == 0) ||
        (Q_stricmp(level.mapname, "boss1") == 0) ||
        (Q_stricmp(level.mapname, "fact3") == 0) ||
        (Q_stricmp(level.mapname, "biggun") == 0) ||
        (Q_stricmp(level.mapname, "space") == 0) ||
        (Q_stricmp(level.mapname, "command") == 0) ||
        (Q_stricmp(level.mapname, "power2") == 0) ||
        (Q_stricmp(level.mapname, "strike") == 0)) {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_FixCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}


