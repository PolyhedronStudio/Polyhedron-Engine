// LICENSE HERE.

//
// svgame/entities/target_goal.c
//
//
// target_goal entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_goal (1 0 1) (-8 -8 -8) (8 8 8)
Counts a goal completed.
These are single use targets.
*/
void use_target_goal(entity_t* ent, entity_t* other, entity_t* activator)
{
    gi.Sound(ent, CHAN_VOICE, ent->noiseIndex, 1, ATTN_NORM, 0);

    level.found_goals++;

    if (level.found_goals == level.total_goals)
        gi.configstring(CS_CDTRACK, "0");

    UTIL_UseTargets(ent, activator);
    G_FreeEntity(ent);
}

void SP_target_goal(entity_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(ent);
        return;
    }

    ent->Use = use_target_goal;
    if (!st.noise)
        st.noise = "misc/secret.wav";
    ent->noiseIndex = gi.SoundIndex(st.noise);
    ent->svFlags = SVF_NOCLIENT;
    level.total_goals++;
}
