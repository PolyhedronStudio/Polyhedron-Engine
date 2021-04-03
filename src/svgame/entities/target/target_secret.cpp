// LICENSE HERE.

//
// svgame/entities/target_secret.c
//
//
// target_secret entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_secret (1 0 1) (-8 -8 -8) (8 8 8)
Counts a secret found.
These are single use targets.
*/
void use_target_secret(entity_t* ent, entity_t* other, entity_t* activator)
{
    gi.Sound(ent, CHAN_VOICE, ent->noiseIndex, 1, ATTN_NORM, 0);

    level.found_secrets++;

    UTIL_UseTargets(ent, activator);
    G_FreeEntity(ent);
}

void SP_target_secret(entity_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(ent);
        return;
    }

    ent->Use = use_target_secret;
    if (!st.noise)
        st.noise = "misc/secret.wav";
    ent->noiseIndex = gi.SoundIndex(st.noise);
    ent->svFlags = SVF_NOCLIENT;
    level.total_secrets++;
    // map bug hack
    if (!Q_stricmp(level.mapname, "mine3") && ent->s.origin[0] == 280 && ent->s.origin[1] == -2048 && ent->s.origin[2] == -624)
        ent->message = "You have found a secret area.";
}