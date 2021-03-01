// LICENSE HERE.

//
// svgame/entities/target_help.c
//
//
// target_help entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================

void Use_Target_Help(edict_t* ent, edict_t* other, edict_t* activator)
{
    if (ent->spawnflags & 1)
        strncpy(game.helpmessage1, ent->message, sizeof(game.helpmessage2) - 1);
    else
        strncpy(game.helpmessage2, ent->message, sizeof(game.helpmessage1) - 1);

    game.helpchanged++;
}

/*QUAKED target_help (1 0 1) (-16 -16 -24) (16 16 24) help1
When fired, the "message" key becomes the current personal computer string, and the message light will be set on all clients status bars.
*/
void SP_target_help(edict_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(ent);
        return;
    }

    if (!ent->message) {
        gi.dprintf("%s with no message at %s\n", ent->classname, vtos(ent->s.origin));
        G_FreeEdict(ent);
        return;
    }
    ent->use = Use_Target_Help;
}