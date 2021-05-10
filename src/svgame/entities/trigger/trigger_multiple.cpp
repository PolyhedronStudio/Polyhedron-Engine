// LICENSE HERE.

//
// svgame/entities/trigger_multiple.c
//
//
// trigger_multiple entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

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
void trigger_enable(entity_t* self, entity_t* other, entity_t* activator)
{
    self->solid = Solid::Trigger;
    self->Use = Use_Multi;
    gi.LinkEntity(self);
}

void SP_trigger_multiple(entity_t* ent)
{
    if (ent->sounds == 1)
        ent->noiseIndex = gi.SoundIndex("misc/secret.wav");
    else if (ent->sounds == 2)
        ent->noiseIndex = gi.SoundIndex("misc/talk.wav");
    else if (ent->sounds == 3)
        ent->noiseIndex = gi.SoundIndex("misc/trigger1.wav");

    if (!ent->wait)
        ent->wait = 0.2;
    ent->Touch = Touch_Multi;
    ent->moveType = MoveType::None;
    ent->serverFlags |= EntityServerFlags::NoClient;


    if (ent->spawnFlags & 4) {
        ent->solid = Solid::Not;
        ent->Use = trigger_enable;
    }
    else {
        ent->solid = Solid::Trigger;
        ent->Use = Use_Multi;
    }

    if (!VectorCompare(ent->state.angles, vec3_origin))
        UTIL_SetMoveDir(ent->state.angles, ent->moveDirection);

    gi.SetModel(ent, ent->model);
    gi.LinkEntity(ent);
}