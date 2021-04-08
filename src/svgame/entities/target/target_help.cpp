// LICENSE HERE.

//
// svgame/entities/target_help.c
//
//
// target_help entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================

void Use_Target_Help(entity_t* ent, entity_t* other, entity_t* activator)
{
    if (ent->spawnFlags & 1)
        strncpy(game.helpmessage1, ent->message, sizeof(game.helpmessage2) - 1);
    else
        strncpy(game.helpmessage2, ent->message, sizeof(game.helpmessage1) - 1);

    game.helpchanged++;
}

/*QUAKED target_help (1 0 1) (-16 -16 -24) (16 16 24) help1
When fired, the "message" key becomes the current personal computer string, and the message light will be set on all clients status bars.
*/
void SP_target_help(entity_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(ent);
        return;
    }

    if (!ent->message) {
        gi.DPrintf("%s with no message at %s\n", ent->classname, Vec3ToString(ent->s.origin));
        G_FreeEntity(ent);
        return;
    }
    ent->Use = Use_Target_Help;
}