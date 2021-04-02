// LICENSE HERE.

//
// svgame/entities/target_speaker.c
//
//
// target_speaker entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"     wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"    0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/
void Use_Target_Speaker(edict_t* ent, edict_t* other, edict_t* activator)
{
    int     chan;

    if (ent->spawnflags & 3) {
        // looping sound toggles
        if (ent->s.sound)
            ent->s.sound = 0;   // turn it off
        else
            ent->s.sound = ent->noise_index;    // start it
    }
    else {
        // normal sound
        if (ent->spawnflags & 4)
            chan = CHAN_VOICE | CHAN_RELIABLE;
        else
            chan = CHAN_VOICE;
        // use a positioned_sound, because this entity won't normally be
        // sent to any clients because it is invisible
        gi.positioned_sound(ent->s.origin, ent, chan, ent->noise_index, ent->volume, ent->attenuation, 0);
    }
}

void SP_target_speaker(edict_t* ent)
{
    char    buffer[MAX_QPATH];

    if (!st.noise) {
        gi.dprintf("target_speaker with no noise set at %s\n", Vec3ToString(ent->s.origin));
        return;
    }
    if (!strstr(st.noise, ".wav"))
        Q_snprintf(buffer, sizeof(buffer), "%s.wav", st.noise);
    else
        strncpy(buffer, st.noise, sizeof(buffer));
    ent->noise_index = gi.soundindex(buffer);

    if (!ent->volume)
        ent->volume = 1.0;

    if (!ent->attenuation)
        ent->attenuation = 1.0;
    else if (ent->attenuation == -1)    // use -1 so 0 defaults to 1
        ent->attenuation = 0;

    // check for prestarted looping sound
    if (ent->spawnflags & 1)
        ent->s.sound = ent->noise_index;

    ent->use = Use_Target_Speaker;

    // must link the entity so we get areas and clusters so
    // the server can determine who to send updates to
    gi.linkentity(ent);
}