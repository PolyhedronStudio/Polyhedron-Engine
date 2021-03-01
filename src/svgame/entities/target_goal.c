// LICENSE HERE.

//
// svgame/entities/target_goal.c
//
//
// target_goal entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_goal (1 0 1) (-8 -8 -8) (8 8 8)
Counts a goal completed.
These are single use targets.
*/
void use_target_goal(edict_t* ent, edict_t* other, edict_t* activator)
{
    gi.sound(ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0);

    level.found_goals++;

    if (level.found_goals == level.total_goals)
        gi.configstring(CS_CDTRACK, "0");

    G_UseTargets(ent, activator);
    G_FreeEdict(ent);
}

void SP_target_goal(edict_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(ent);
        return;
    }

    ent->use = use_target_goal;
    if (!st.noise)
        st.noise = "misc/secret.wav";
    ent->noise_index = gi.soundindex(st.noise);
    ent->svflags = SVF_NOCLIENT;
    level.total_goals++;
}
