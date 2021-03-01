// LICENSE HERE.

//
// svgame/entities/trigger_multiple.c
//
//
// trigger_multiple entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)  secret
2)  beep beep
3)  large switch
4)
set "message" to text string
*/
void trigger_enable(edict_t* self, edict_t* other, edict_t* activator)
{
    self->solid = SOLID_TRIGGER;
    self->use = Use_Multi;
    gi.linkentity(self);
}

void SP_trigger_multiple(edict_t* ent)
{
    if (ent->sounds == 1)
        ent->noise_index = gi.soundindex("misc/secret.wav");
    else if (ent->sounds == 2)
        ent->noise_index = gi.soundindex("misc/talk.wav");
    else if (ent->sounds == 3)
        ent->noise_index = gi.soundindex("misc/trigger1.wav");

    if (!ent->wait)
        ent->wait = 0.2;
    ent->touch = Touch_Multi;
    ent->movetype = MOVETYPE_NONE;
    ent->svflags |= SVF_NOCLIENT;


    if (ent->spawnflags & 4) {
        ent->solid = SOLID_NOT;
        ent->use = trigger_enable;
    }
    else {
        ent->solid = SOLID_TRIGGER;
        ent->use = Use_Multi;
    }

    if (!VectorCompare(ent->s.angles, vec3_origin))
        G_SetMovedir(ent->s.angles, ent->movedir);

    gi.setmodel(ent, ent->model);
    gi.linkentity(ent);
}