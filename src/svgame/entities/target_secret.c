// LICENSE HERE.

//
// svgame/entities/target_secret.c
//
//
// target_secret entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_secret (1 0 1) (-8 -8 -8) (8 8 8)
Counts a secret found.
These are single use targets.
*/
void use_target_secret(edict_t* ent, edict_t* other, edict_t* activator)
{
    gi.sound(ent, CHAN_VOICE, ent->noise_index, 1, ATTN_NORM, 0);

    level.found_secrets++;

    G_UseTargets(ent, activator);
    G_FreeEdict(ent);
}

void SP_target_secret(edict_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(ent);
        return;
    }

    ent->use = use_target_secret;
    if (!st.noise)
        st.noise = "misc/secret.wav";
    ent->noise_index = gi.soundindex(st.noise);
    ent->svflags = SVF_NOCLIENT;
    level.total_secrets++;
    // map bug hack
    if (!Q_stricmp(level.mapname, "mine3") && ent->s.origin[0] == 280 && ent->s.origin[1] == -2048 && ent->s.origin[2] == -624)
        ent->message = "You have found a secret area.";
}