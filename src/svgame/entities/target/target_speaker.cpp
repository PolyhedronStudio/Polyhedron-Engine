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
void Use_Target_Speaker(entity_t* ent, entity_t* other, entity_t* activator)
{
    int     chan;

    if (ent->spawnFlags & 3) {
        // looping sound toggles
        if (ent->state.sound)
            ent->state.sound = 0;   // turn it off
        else
            ent->state.sound = ent->noiseIndex;    // start it
    }
    else {
        // normal sound
        if (ent->spawnFlags & 4)
            chan = CHAN_VOICE | CHAN_RELIABLE;
        else
            chan = CHAN_VOICE;
        // use a positioned_sound, because this entity won't normally be
        // sent to any clients because it is invisible
        gi.PositionedSound(ent->state.origin, ent, chan, ent->noiseIndex, ent->volume, ent->attenuation, 0);
    }
}

void SP_target_speaker(entity_t* ent)
{
    char    buffer[MAX_QPATH];

    if (!st.noise) {
        gi.DPrintf("target_speaker with no noise set at %s\n", Vec3ToString(ent->state.origin));
        return;
    }
    if (!strstr(st.noise, ".wav"))
        Q_snprintf(buffer, sizeof(buffer), "%s.wav", st.noise);
    else
        strncpy(buffer, st.noise, sizeof(buffer));
    ent->noiseIndex = gi.SoundIndex(buffer);

    if (!ent->volume)
        ent->volume = 1.0;

    if (!ent->attenuation)
        ent->attenuation = 1.0;
    else if (ent->attenuation == -1)    // use -1 so 0 defaults to 1
        ent->attenuation = 0;

    // check for prestarted looping sound
    if (ent->spawnFlags & 1)
        ent->state.sound = ent->noiseIndex;

    ent->Use = Use_Target_Speaker;

    // must link the entity so we get areas and clusters so
    // the server can determine who to send updates to
    gi.LinkEntity(ent);
}